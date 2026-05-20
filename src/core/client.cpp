#include "core/client.h"
#include <QDebug>

Client::Client(QObject *parent)
    : QObject(parent)
    , m_rest(new RestClient(this))
    , m_gateway(new GatewayClient(this))
    , m_cache(new Cache(this))
    , m_intents{}
{
    wireSignals();
}

Client::~Client() = default;

void Client::setIntents(Intents intents)
{
    m_intents = intents;
}

Cache *Client::cache() const
{
    return m_cache;
}

void Client::login(const QString &token)
{
    // Strip "Bot " prefix if user included it — we add it ourselves in REST calls
    QString cleanToken = token;
    if (cleanToken.startsWith(QLatin1String("Bot "), Qt::CaseInsensitive)) {
        cleanToken = cleanToken.mid(4);
    }
    cleanToken = cleanToken.trimmed();

    m_token = cleanToken;
    m_rest->setToken(cleanToken);
    m_gateway->setToken(cleanToken);
    m_gateway->setIntents(m_intents);
    m_rest->fetchCurrentUser();
}

void Client::logout()
{
    m_gateway->disconnectFromGateway();
}

void Client::fetchGuilds()
{
    m_rest->fetchGuilds();
}

void Client::fetchChannelMessages(Snowflake channelId, int limit, std::optional<Snowflake> before, std::optional<Snowflake> after)
{
    m_rest->fetchChannelMessages(channelId, limit, before, after);
}

void Client::sendMessage(Snowflake channelId, const QString &content)
{
    m_rest->sendMessage(channelId, content);
}

void Client::wireSignals()
{
    connect(m_rest, &RestClient::userReady, this, [this](const User &user) {
        m_cache->setSelf(user);
        m_rest->fetchGatewayUrl();
    });

    connect(m_rest, &RestClient::gatewayUrlReady, this, [this](const QString &url) {
        m_gateway->connectToGateway(url);
    });

    connect(m_rest, &RestClient::guildsReady, this, &Client::onGuildsReady);
    connect(m_rest, &RestClient::guildChannelsReady, this, &Client::onGuildChannelsReady);
    connect(m_rest, &RestClient::messagesReady, this, &Client::onChannelMessagesReady);
    connect(m_rest, &RestClient::messageSent, this, &Client::onMessageSent);
    connect(m_rest, &RestClient::messageSendError, this, &Client::onRestError);

    connect(m_gateway, &GatewayClient::ready, this, &Client::onReady);
    connect(m_gateway, &GatewayClient::guildAvailable, this, &Client::onGuildAvailable);
    connect(m_gateway, &GatewayClient::channelAvailable, this, &Client::onChannelAvailable);
    connect(m_gateway, &GatewayClient::messageCreate, this, &Client::onMessageCreate);
    connect(m_gateway, &GatewayClient::messageUpdate, this, &Client::onMessageUpdate);
    connect(m_gateway, &GatewayClient::messageDelete, this, &Client::onMessageDelete);
    connect(m_gateway, &GatewayClient::typingStart, this, &Client::onTypingStart);
    connect(m_gateway, &GatewayClient::disconnected, this, &Client::onGatewayDisconnected);
    connect(m_gateway, &GatewayClient::gatewayError, this, &Client::onGatewayError);

    connect(m_rest, &RestClient::userError, this, &Client::onRestError);
    connect(m_rest, &RestClient::gatewayUrlError, this, &Client::onRestError);
    connect(m_rest, &RestClient::guildsError, this, &Client::onRestError);
    connect(m_rest, &RestClient::messagesError, this, &Client::onRestError);
}

void Client::onReady(const User &self, const QList<Guild> &guilds, const QString &sessionId)
{
    Q_UNUSED(sessionId);
    qDebug() << "[Client] onReady: user=" << self.username << "guilds=" << guilds.size();
    m_cache->setSelf(self);
    for (const auto &guild : guilds) {
        if (!guild.unavailable) {
            m_cache->upsertGuild(guild);
        }
    }
    qDebug() << "[Client] Emitting ready() signal";
    emit ready();
}

void Client::onGuildAvailable(const Guild &guild)
{
    qDebug() << "[Client] Guild available:" << guild.name << guild.id.toString();
    m_cache->upsertGuild(guild);
    emit guildAdded(guild);

    auto channels = m_cache->guildChannels(guild.id);
    if (channels.isEmpty()) {
        m_rest->fetchGuildChannels(guild.id);
    }
}

void Client::onChannelAvailable(const Channel &channel)
{
    qDebug() << "[Client] Channel available:" << channel.name << channel.id.toString();
    m_cache->upsertChannel(channel);
    emit channelAdded(channel);
}

void Client::onGuildsReady(const QList<Guild> &guilds)
{
    for (const auto &guild : guilds) {
        m_cache->upsertGuild(guild);
    }
}

void Client::onGuildChannelsReady(Snowflake guildId, const QList<Channel> &channels)
{
    for (const auto &channel : channels) {
        m_cache->upsertChannel(channel);
    }
}

void Client::onChannelMessagesReady(const QList<Message> &messages)
{
    for (const auto &msg : messages) {
        m_cache->insertMessage(msg);
    }
    emit messagesLoaded(messages);
}

void Client::onMessageSent(const Message &msg)
{
    m_cache->insertMessage(msg);
}

void Client::onMessageCreate(const Message &msg)
{
    auto existing = m_cache->message(msg.channelId, msg.id);
    if (!existing.has_value()) {
        m_cache->insertMessage(msg);
    }
    emit messageReceived(msg);
}

void Client::onMessageUpdate(Snowflake id, Snowflake channelId, const QString &newContent)
{
    m_cache->updateMessage(channelId, id, newContent);
    emit messageUpdated(channelId, id);
}

void Client::onMessageDelete(Snowflake id, Snowflake channelId)
{
    m_cache->removeMessage(channelId, id);
    emit messageDeleted(channelId, id);
}

void Client::onTypingStart(Snowflake channelId, Snowflake userId, const QDateTime &when)
{
    emit typingStarted(channelId, userId, when);
}

void Client::onGatewayDisconnected(int closeCode, const QString &reason)
{
    Q_UNUSED(closeCode);
    Q_UNUSED(reason);
    emit connectionStateChanged(false);
}

void Client::onGatewayError(const QString &message)
{
    emit error(message);
}

void Client::onRestError(const QString &message)
{
    qWarning() << "REST error:" << message;
    emit error(message);
}
