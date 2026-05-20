#include "gateway_client.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(discordGateway, "discord.gateway")

int GatewayClient::reconnectDelayMs() const
{
    int delay = std::min(kBaseReconnectDelayMs * (1 << m_reconnectAttempts), kMaxReconnectDelayMs);
    double jitter = QRandomGenerator::global()->generateDouble();
    return static_cast<int>(delay * (0.5 + 0.5 * jitter));
}

GatewayClient::GatewayClient(QObject *parent)
    : QObject(parent)
    , m_websocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_intents(defaultIntents())
    , m_heartbeatTimer(new QTimer(this))
    , m_zombieTimer(new QTimer(this))
{
    // Check SSL support at startup
    if (!QSslSocket::supportsSsl()) {
        qCritical() << "[Gateway] SSL not supported — Qt was built without SSL or OpenSSL not found";
        qCritical() << "[Gateway] SSL library build version:" << QSslSocket::sslLibraryBuildVersionString();
    } else {
        qDebug() << "[Gateway] SSL supported — backend:" << QSslSocket::sslLibraryBuildVersionString()
                 << "runtime:" << QSslSocket::sslLibraryVersionString();
    }

    m_heartbeatTimer->setSingleShot(true);
    m_zombieTimer->setSingleShot(true);

    connect(m_websocket, &QWebSocket::connected, this, &GatewayClient::onConnected);
    connect(m_websocket, &QWebSocket::disconnected, this, &GatewayClient::onDisconnected);
    connect(m_websocket, &QWebSocket::textMessageReceived, this, &GatewayClient::onTextMessageReceived);
    connect(m_websocket, &QWebSocket::errorOccurred, this, &GatewayClient::onErrorOccurred);
    connect(m_websocket, &QWebSocket::sslErrors, this, &GatewayClient::onSslErrors);

    connect(m_heartbeatTimer, &QTimer::timeout, this, [this]() {
        sendHeartbeat();
    });

    connect(m_zombieTimer, &QTimer::timeout, this, [this]() {
        if (!m_ackReceived) {
            qWarning() << "[Gateway] Zombie connection detected, reconnecting";
            m_websocket->close(QWebSocketProtocol::CloseCode::CloseCodeGoingAway, "Zombie connection");
            setState(State::Reconnecting);
        }
    });
}

void GatewayClient::connectToGateway(const QString &url)
{
    m_gatewayUrl = url;
    m_session.resumeGatewayUrl = url;
    setState(State::Connecting);
    qCDebug(discordGateway) << "Connecting to:" << url;

    // Create a fresh WebSocket for each connection attempt
    delete m_websocket;
    m_websocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    // Reconnect signals to the new socket
    connect(m_websocket, &QWebSocket::connected, this, &GatewayClient::onConnected);
    connect(m_websocket, &QWebSocket::disconnected, this, &GatewayClient::onDisconnected);
    connect(m_websocket, &QWebSocket::textMessageReceived, this, &GatewayClient::onTextMessageReceived);
    connect(m_websocket, &QWebSocket::errorOccurred, this, &GatewayClient::onErrorOccurred);
    connect(m_websocket, &QWebSocket::sslErrors, this, &GatewayClient::onSslErrors);

    qCDebug(discordGateway) << "Opening WebSocket...";
    m_websocket->open(QUrl(url));
}

void GatewayClient::disconnectFromGateway()
{
    m_intentionalDisconnect = true;
    m_heartbeatTimer->stop();
    m_zombieTimer->stop();
    m_session.canResume = false;
    m_websocket->close();
    setState(State::Disconnected);
}

void GatewayClient::sendOpcode(GatewayOpcode op, const nlohmann::json &payload)
{
    if (m_websocket->state() != QAbstractSocket::ConnectedState) return;
    QString json = QString::fromStdString(payload.dump());
    m_websocket->sendTextMessage(json);
}

void GatewayClient::startHeartbeat(qint64 intervalMs)
{
    m_heartbeatInterval = intervalMs;
    m_ackReceived = true;
    qint64 jitter = static_cast<qint64>(QRandomGenerator::global()->generateDouble() * intervalMs);
    m_heartbeatTimer->start(jitter);
}

void GatewayClient::sendHeartbeat()
{
    auto payload = makeHeartbeat(m_lastSeq);
    sendOpcode(GatewayOpcode::Heartbeat, payload);
    m_ackReceived = false;
    m_zombieTimer->start(m_heartbeatInterval * 2);
    m_heartbeatTimer->start(m_heartbeatInterval);
}

void GatewayClient::onConnected()
{
    qCDebug(discordGateway) << "WebSocket connected";
    m_reconnectAttempts = 0;
    if (m_session.canResume) {
        qCDebug(discordGateway) << "Attempting resume";
        attemptResume();
    } else {
        qCDebug(discordGateway) << "Attempting identify";
        attemptIdentify();
    }
}

void GatewayClient::onDisconnected()
{
    int code = m_websocket->closeCode();
    QString reason = m_websocket->closeReason();
    qCDebug(discordGateway) << "WebSocket disconnected, code=" << code << "reason=" << reason;
    m_heartbeatTimer->stop();
    m_zombieTimer->stop();

    if (m_intentionalDisconnect) {
        m_intentionalDisconnect = false;
        setState(State::Disconnected);
        emit disconnected(code, reason);
        return;
    }

    if (m_state == State::Ready || m_state == State::Reconnecting) {
        setState(State::Reconnecting);
        if (m_session.canResume && !m_gatewayUrl.isEmpty()) {
            m_reconnectAttempts++;
            int delay = reconnectDelayMs();
            qCWarning(discordGateway) << "Attempting reconnect with resume in" << delay << "ms...";
            QString reconnectUrl = m_session.resumeGatewayUrl.isEmpty() ? m_gatewayUrl : m_session.resumeGatewayUrl;

            delete m_websocket;
            m_websocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

            connect(m_websocket, &QWebSocket::connected, this, &GatewayClient::onConnected);
            connect(m_websocket, &QWebSocket::disconnected, this, &GatewayClient::onDisconnected);
            connect(m_websocket, &QWebSocket::textMessageReceived, this, &GatewayClient::onTextMessageReceived);
            connect(m_websocket, &QWebSocket::errorOccurred, this, &GatewayClient::onErrorOccurred);
            connect(m_websocket, &QWebSocket::sslErrors, this, &GatewayClient::onSslErrors);

            QTimer::singleShot(delay, this, [this, reconnectUrl]() {
                qCDebug(discordGateway) << "Reconnecting to:" << reconnectUrl;
                m_websocket->open(QUrl(reconnectUrl));
            });
        }
    } else {
        setState(State::Disconnected);
        emit disconnected(code, reason);
    }
}

void GatewayClient::onTextMessageReceived(const QString &message)
{
    try {
        auto json = nlohmann::json::parse(message.toStdString());

        int op = json.value("op", -1);
        qCDebug(discordGateway) << "Received op=" << op;

        if (json.contains("s") && !json["s"].is_null()) {
            m_lastSeq = json["s"].get<int>();
            m_session.lastSeq = m_lastSeq;
        }

        if (json.contains("op")) {
            auto opcode = static_cast<GatewayOpcode>(json["op"].get<int>());
            switch (opcode) {
            case GatewayOpcode::Dispatch:
                handleDispatch(json["d"]);
                break;
            case GatewayOpcode::Hello:
                handleHello(json["d"]);
                break;
            case GatewayOpcode::InvalidSession:
                handleInvalidSession(json["d"]);
                break;
            case GatewayOpcode::Reconnect:
                handleReconnect();
                break;
            case GatewayOpcode::HeartbeatAck:
                m_ackReceived = true;
                m_zombieTimer->stop();
                break;
            default:
                qCWarning(discordGateway) << "Unknown opcode:" << op;
                break;
            }
        }
    } catch (const std::exception &e) {
        qCCritical(discordGateway) << "Parse error:" << e.what();
        qCCritical(discordGateway) << "Raw message:" << message.left(200);
        emit gatewayError(QString("Failed to parse message: %1").arg(e.what()));
    }
}

void GatewayClient::onErrorOccurred(QAbstractSocket::SocketError error)
{
    qCCritical(discordGateway) << "WebSocket error: code=" << (int)error << "msg=" << m_websocket->errorString();
    emit gatewayError(m_websocket->errorString());
}

void GatewayClient::onSslErrors(const QList<QSslError> &errors)
{
    for (const auto &err : errors) {
        qCCritical(discordGateway) << "SSL error:" << err.errorString();
    }
    m_websocket->ignoreSslErrors();
}

void GatewayClient::handleDispatch(const nlohmann::json &payload)
{
    if (payload.contains("t") && payload.contains("d")) {
        QString eventType = QString::fromStdString(payload["t"].get<std::string>());
        processEvent(eventType, payload["d"]);
    }
}

void GatewayClient::handleHello(const nlohmann::json &payload)
{
    if (payload.contains("heartbeat_interval")) {
        qint64 interval = static_cast<qint64>(payload["heartbeat_interval"].get<double>());
        qCDebug(discordGateway) << "Hello received, heartbeat interval:" << interval << "ms";
        startHeartbeat(interval);
    }
}

void GatewayClient::attemptIdentify()
{
    setState(State::Identifying);
    auto payload = makeIdentify(m_token, m_intents);
    qCDebug(discordGateway) << "Sending Identify payload (intents=" << m_intents << ")";
    sendOpcode(GatewayOpcode::Identify, payload);
}

void GatewayClient::handleInvalidSession(const nlohmann::json &payload)
{
    bool resumable = payload.get<bool>();

    if (resumable) {
        qDebug() << "[Gateway] Invalid session, attempting resume";
        setState(State::Resuming);
        QMetaObject::invokeMethod(this, [this]() {
            attemptResume();
        }, Qt::QueuedConnection);
    } else {
        qDebug() << "[Gateway] Invalid session, re-identifying";
        m_session.canResume = false;
        m_session.sessionId.clear();
        setState(State::Identifying);
        QMetaObject::invokeMethod(this, [this]() {
            attemptIdentify();
        }, Qt::QueuedConnection);
    }
}

void GatewayClient::handleReconnect()
{
    qDebug() << "[Gateway] Reconnect requested";
    m_websocket->close(QWebSocketProtocol::CloseCode::CloseCodeGoingAway, "Server requested reconnect");
    setState(State::Reconnecting);
}

void GatewayClient::attemptResume()
{
    setState(State::Resuming);
    auto payload = makeResume(m_token, m_session.sessionId, m_session.lastSeq);
    sendOpcode(GatewayOpcode::Resume, payload);
    qDebug() << "[Gateway] Sent Resume";
}

void GatewayClient::processEvent(const QString &eventType, const nlohmann::json &data)
{
    if (eventType == "READY") {
        User self = JsonParser::parseUser(data["user"]);
        QList<Guild> guilds;
        if (data.contains("guilds")) {
            for (const auto &g : data["guilds"]) {
                guilds.append(JsonParser::parseGuild(g));
            }
        }
        if (data.contains("session_id")) {
            m_session.sessionId = QString::fromStdString(data["session_id"].get<std::string>());
            m_session.canResume = true;
        }
        if (data.contains("resume_gateway_url")) {
            m_session.resumeGatewayUrl = QString::fromStdString(data["resume_gateway_url"].get<std::string>());
        }
        setState(State::Ready);
        emit ready(self, guilds, m_session.sessionId);
    } else if (eventType == "MESSAGE_CREATE") {
        Message msg = JsonParser::parseMessage(data);
        emit messageCreate(msg);
    } else if (eventType == "MESSAGE_UPDATE") {
        Snowflake id(QString::fromStdString(data.value("id", "0")));
        Snowflake channelId(QString::fromStdString(data.value("channel_id", "0")));
        QString newContent = data.contains("content") && !data["content"].is_null()
            ? QString::fromStdString(data["content"].get<std::string>())
            : QString();
        emit messageUpdate(id, channelId, newContent);
    } else if (eventType == "MESSAGE_DELETE") {
        Snowflake id(QString::fromStdString(data.value("id", "0")));
        Snowflake channelId(QString::fromStdString(data.value("channel_id", "0")));
        emit messageDelete(id, channelId);
    } else if (eventType == "TYPING_START") {
        Snowflake channelId(QString::fromStdString(data.value("channel_id", "0")));
        Snowflake userId(QString::fromStdString(data.value("user_id", "0")));
        if (data.contains("timestamp")) {
            qint64 timestamp = data["timestamp"].get<qint64>();
            QDateTime when = QDateTime::fromMSecsSinceEpoch(timestamp * 1000);
            emit typingStart(channelId, userId, when);
        }
    } else if (eventType == "RESUMED") {
        setState(State::Ready);
        qDebug() << "[Gateway] Session resumed";
    }
}

void GatewayClient::setState(State newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
