#include "core/api/rest_client.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(discordRest, "discord.rest")

RestClient::RestClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_rateLimiter(new RateLimiter(this))
{
}

void RestClient::setToken(const QString &token)
{
    m_token = token;
}

QNetworkReply *RestClient::get(const QString &endpoint)
{
    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);
    qCDebug(discordRest) << "GET" << request.url().toString();
    return m_nam->get(request);
}

QNetworkReply *RestClient::post(const QString &endpoint, const nlohmann::json &body)
{
    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);
    qCDebug(discordRest) << "POST" << request.url().toString();
    return m_nam->post(request, QByteArray::fromStdString(body.dump()));
}

bool RestClient::checkRateLimit(const QString &endpoint, const QString &errorSignal)
{
    QString key = m_rateLimiter->bucketKey(endpoint);
    if (!m_rateLimiter->canProceed(key))
    {
        qint64 retry = m_rateLimiter->retryAfterMs(key);
        QString msg = retry > 0
            ? QStringLiteral("Rate limited, retry after %1 ms").arg(retry)
            : QStringLiteral("Rate limited");
        qCWarning(discordRest) << "Rate limited:" << msg;
        if (errorSignal == QStringLiteral("userError"))
            emit userError(msg);
        else if (errorSignal == QStringLiteral("gatewayUrlError"))
            emit gatewayUrlError(msg);
        else if (errorSignal == QStringLiteral("guildsError"))
            emit guildsError(msg);
        else if (errorSignal == QStringLiteral("guildChannelsError"))
            emit guildChannelsError(msg);
        else if (errorSignal == QStringLiteral("messagesError"))
            emit messagesError(msg);
        else if (errorSignal == QStringLiteral("messageSendError"))
            emit messageSendError(msg);
        return false;
    }
    return true;
}

void RestClient::updateRateLimits(QNetworkReply *reply)
{
    auto header = reply->rawHeader("X-RateLimit-Bucket");
    QString bucket = QString::fromUtf8(header);

    bool ok = false;
    int limit = reply->rawHeader("X-RateLimit-Limit").toInt(&ok);
    ok = false;
    int remaining = reply->rawHeader("X-RateLimit-Remaining").toInt(&ok);
    ok = false;
    qint64 resetAfterMs = static_cast<qint64>(reply->rawHeader("X-RateLimit-Reset-After").toDouble(&ok) * 1000);

    bool isGlobal = !reply->rawHeader("X-RateLimit-Global").isEmpty();

    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 429)
    {
        auto data = parseJson(reply);
        qint64 retryAfter = 0;
        if (data.contains("retry_after"))
            retryAfter = data["retry_after"].get<double>() * 1000;
        m_rateLimiter->handle429(retryAfter, isGlobal);
    }
    else
    {
        m_rateLimiter->recordResponse(bucket, limit, remaining, resetAfterMs, isGlobal);
    }
}

nlohmann::json RestClient::parseJson(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    auto json = nlohmann::json::parse(data.constData(), nullptr, false);
    if (json.is_discarded()) {
        throw std::runtime_error("Failed to parse JSON response");
    }
    return json;
}

void RestClient::fetchCurrentUser()
{
    QString endpoint = Routes::currentUser();
    if (!checkRateLimit(endpoint, QStringLiteral("userError")))
        return;

    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);

    qCDebug(discordRest) << "GET" << request.url().toString();

    QNetworkReply *reply = m_nam->get(request);
    QPointer<QNetworkReply> ptr(reply);
    connect(reply, &QNetworkReply::finished, this, [this, ptr]() {
        if (!ptr)
            return;
        updateRateLimits(ptr);
        int httpCode = ptr->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qCDebug(discordRest) << "Response HTTP:" << httpCode;
        if (ptr->error() != QNetworkReply::NoError || httpCode >= 400)
        {
            QString body = QString::fromUtf8(ptr->readAll());
            QString msg = QString("HTTP %1: %2").arg(httpCode).arg(ptr->errorString());
            if (!body.isEmpty()) msg += QString(" — %1").arg(body);
            qCCritical(discordRest) << "Error:" << msg;
            emit userError(msg);
            ptr->deleteLater();
            return;
        }
        try
        {
            auto json = parseJson(ptr);
            qCDebug(discordRest) << "User:" << QString::fromStdString(json.value("username", std::string("unknown")));
            emit userReady(JsonParser::parseUser(json));
        }
        catch (const std::exception &e)
        {
            qCCritical(discordRest) << "Parse error:" << e.what();
            emit userError(QString::fromUtf8(e.what()));
        }
        ptr->deleteLater();
    });
}

void RestClient::fetchGatewayUrl()
{
    QString endpoint = Routes::gatewayBot();
    if (!checkRateLimit(endpoint, QStringLiteral("gatewayUrlError")))
        return;

    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);

    qCDebug(discordRest) << "GET" << request.url().toString();

    QNetworkReply *reply = m_nam->get(request);
    QPointer<QNetworkReply> ptr(reply);
    connect(reply, &QNetworkReply::finished, this, [this, ptr]() {
        if (!ptr)
            return;
        updateRateLimits(ptr);
        int httpCode = ptr->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qCDebug(discordRest) << "Gateway URL response HTTP:" << httpCode;
        if (ptr->error() != QNetworkReply::NoError)
        {
            QString body = QString::fromUtf8(ptr->readAll());
            QString msg = QString("HTTP %1: %2").arg(httpCode).arg(ptr->errorString());
            if (!body.isEmpty()) msg += QString(" — %1").arg(body);
            qCCritical(discordRest) << "Gateway URL error:" << msg;
            emit gatewayUrlError(msg);
            ptr->deleteLater();
            return;
        }
        try
        {
            auto json = parseJson(ptr);
            QString url = QString::fromStdString(json.value("url", ""));
            if (!url.isEmpty()) {
                url += QStringLiteral("?v=10&encoding=json");
            }
            qCDebug(discordRest) << "Gateway URL:" << url;
            emit gatewayUrlReady(url);
        }
        catch (const std::exception &e)
        {
            qCCritical(discordRest) << "Gateway URL parse error:" << e.what();
            emit gatewayUrlError(QString::fromUtf8(e.what()));
        }
        ptr->deleteLater();
    });
}

void RestClient::fetchGuilds()
{
    QString endpoint = Routes::guilds();
    if (!checkRateLimit(endpoint, QStringLiteral("guildsError")))
        return;

    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);

    QNetworkReply *reply = m_nam->get(request);
    QPointer<QNetworkReply> ptr(reply);
    connect(reply, &QNetworkReply::finished, this, [this, ptr]() {
        if (!ptr)
            return;
        updateRateLimits(ptr);
        if (ptr->error() != QNetworkReply::NoError)
        {
            emit guildsError(ptr->errorString());
            ptr->deleteLater();
            return;
        }
        try
        {
            auto json = parseJson(ptr);
            QList<Guild> guilds;
            for (const auto &item : json)
                guilds.append(JsonParser::parseGuild(item));
            emit guildsReady(guilds);
        }
        catch (const std::exception &e)
        {
            emit guildsError(QString::fromUtf8(e.what()));
        }
        ptr->deleteLater();
    });
}

void RestClient::fetchGuildChannels(Snowflake guildId)
{
    QString endpoint = Routes::guildChannels(guildId);
    if (!checkRateLimit(endpoint, QStringLiteral("guildChannelsError")))
        return;

    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);

    QNetworkReply *reply = m_nam->get(request);
    QPointer<QNetworkReply> ptr(reply);
    connect(reply, &QNetworkReply::finished, this, [this, ptr, guildId]() {
        if (!ptr)
            return;
        updateRateLimits(ptr);
        if (ptr->error() != QNetworkReply::NoError)
        {
            emit guildChannelsError(ptr->errorString());
            ptr->deleteLater();
            return;
        }
        try
        {
            auto json = parseJson(ptr);
            QList<Channel> channels;
            for (const auto &item : json)
                channels.append(JsonParser::parseChannel(item));
            emit guildChannelsReady(guildId, channels);
        }
        catch (const std::exception &e)
        {
            emit guildChannelsError(QString::fromUtf8(e.what()));
        }
        ptr->deleteLater();
    });
}

void RestClient::fetchChannelMessages(Snowflake channelId, int limit, std::optional<Snowflake> before, std::optional<Snowflake> after)
{
    QString endpoint = Routes::channelMessages(channelId, limit, before, after);
    if (!checkRateLimit(endpoint, QStringLiteral("messagesError")))
        return;

    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);

    QNetworkReply *reply = m_nam->get(request);
    QPointer<QNetworkReply> ptr(reply);
    connect(reply, &QNetworkReply::finished, this, [this, ptr]() {
        if (!ptr)
            return;
        updateRateLimits(ptr);
        if (ptr->error() != QNetworkReply::NoError)
        {
            emit messagesError(ptr->errorString());
            ptr->deleteLater();
            return;
        }
        try
        {
            auto json = parseJson(ptr);
            QList<Message> messages;
            for (const auto &item : json)
                messages.append(JsonParser::parseMessage(item));
            emit messagesReady(messages);
        }
        catch (const std::exception &e)
        {
            emit messagesError(QString::fromUtf8(e.what()));
        }
        ptr->deleteLater();
    });
}

void RestClient::sendMessage(Snowflake channelId, const QString &content)
{
    QString endpoint = Routes::sendMessage(channelId);
    if (!checkRateLimit(endpoint, QStringLiteral("messageSendError")))
        return;

    nlohmann::json body;
    body["content"] = content.toStdString();

    QNetworkRequest request(QUrl(Routes::baseUrl() + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bot %1").arg(m_token).toUtf8());
    request.setRawHeader("User-Agent", "DiscordBot (https://github.com/example/discord-qt, 0.1.0)");
    request.setTransferTimeout(30000);

    QNetworkReply *reply = m_nam->post(request, QByteArray::fromStdString(body.dump()));
    QPointer<QNetworkReply> ptr(reply);
    connect(reply, &QNetworkReply::finished, this, [this, ptr]() {
        if (!ptr)
            return;
        updateRateLimits(ptr);
        if (ptr->error() != QNetworkReply::NoError)
        {
            emit messageSendError(ptr->errorString());
            ptr->deleteLater();
            return;
        }
        try
        {
            auto json = parseJson(ptr);
            emit messageSent(JsonParser::parseMessage(json));
        }
        catch (const std::exception &e)
        {
            emit messageSendError(QString::fromUtf8(e.what()));
        }
        ptr->deleteLater();
    });
}
