#ifndef GATEWAY_CLIENT_H
#define GATEWAY_CLIENT_H

#include "opcodes.h"
#include "session.h"
#include "../types/discord_types.h"
#include "../types/snowflake.h"
#include "../types/json_parser.h"
#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QSslError>
#include <nlohmann/json.hpp>

class GatewayClient : public QObject
{
    Q_OBJECT
public:
    enum class State
    {
        Disconnected,
        Connecting,
        Identifying,
        Ready,
        Resuming,
        Reconnecting
    };
    Q_ENUM(State)

    explicit GatewayClient(QObject *parent = nullptr);

    void connectToGateway(const QString &url);
    void disconnectFromGateway();
    void sendOpcode(GatewayOpcode op, const nlohmann::json &payload);
    void startHeartbeat(qint64 intervalMs);

    void setToken(const QString &token) { m_token = token; }
    void setIntents(Intents intents) { m_intents = intents; }
    State state() const { return m_state; }

signals:
    void ready(const User &self, const QList<Guild> &guilds, const QString &sessionId);
    void guildAvailable(const Guild &guild);
    void channelAvailable(const Channel &channel);
    void messageCreate(const Message &msg);
    void messageUpdate(Snowflake id, Snowflake channelId, const QString &newContent);
    void messageDelete(Snowflake id, Snowflake channelId);
    void typingStart(Snowflake channelId, Snowflake userId, const QDateTime &when);
    void disconnected(int closeCode, const QString &reason);
    void gatewayError(const QString &message);
    void stateChanged(State newState);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);

private:
    int reconnectDelayMs() const;
    void sendHeartbeat();
    void handleDispatch(const nlohmann::json &payload);
    void handleHello(const nlohmann::json &payload);
    void handleInvalidSession(const nlohmann::json &payload);
    void handleReconnect();
    void attemptIdentify();
    void attemptResume();
    void processEvent(const QString &eventType, const nlohmann::json &data);

    void setState(State newState);

    QWebSocket *m_websocket;
    QString m_token;
    Intents m_intents;
    Session m_session;

    qint64 m_heartbeatInterval = 0;
    QTimer *m_heartbeatTimer;
    QTimer *m_zombieTimer;
    bool m_ackReceived = true;
    int m_lastSeq = 0;

    State m_state = State::Disconnected;
    QString m_gatewayUrl;

    int m_reconnectAttempts = 0;
    bool m_intentionalDisconnect = false;
    static constexpr int kMaxReconnectDelayMs = 60000;
    static constexpr int kBaseReconnectDelayMs = 1000;
};

#endif // GATEWAY_CLIENT_H
