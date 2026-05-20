#ifndef CLIENT_H
#define CLIENT_H

#include "core/api/rest_client.h"
#include "core/gateway/gateway_client.h"
#include "core/gateway/intents.h"
#include "core/cache.h"
#include <QObject>

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);
    ~Client() override;

    void setIntents(Intents intents);
    Cache *cache() const;
    RestClient *rest() const { return m_rest; }

public slots:
    void login(const QString &token);
    void logout();
    void fetchGuilds();
    void fetchChannelMessages(Snowflake channelId, int limit = 50, std::optional<Snowflake> before = {}, std::optional<Snowflake> after = {});
    void sendMessage(Snowflake channelId, const QString &content);

signals:
    void ready();
    void guildAdded(const Guild &guild);
    void channelAdded(const Channel &channel);
    void messagesLoaded(const QList<Message> &messages);
    void messageReceived(const Message &msg);
    void messageUpdated(Snowflake channelId, Snowflake messageId);
    void messageDeleted(Snowflake channelId, Snowflake messageId);
    void typingStarted(Snowflake channelId, Snowflake userId, const QDateTime &when);
    void connectionStateChanged(bool connected);
    void error(const QString &message);

private:
    void wireSignals();
    void onReady(const User &self, const QList<Guild> &guilds, const QString &sessionId);
    void onGuildAvailable(const Guild &guild);
    void onChannelAvailable(const Channel &channel);
    void onGuildsReady(const QList<Guild> &guilds);
    void onGuildChannelsReady(Snowflake guildId, const QList<Channel> &channels);
    void onChannelMessagesReady(const QList<Message> &messages);
    void onMessageSent(const Message &msg);
    void onMessageCreate(const Message &msg);
    void onMessageUpdate(Snowflake id, Snowflake channelId, const QString &newContent);
    void onMessageDelete(Snowflake id, Snowflake channelId);
    void onTypingStart(Snowflake channelId, Snowflake userId, const QDateTime &when);
    void onGatewayDisconnected(int closeCode, const QString &reason);
    void onGatewayError(const QString &message);
    void onRestError(const QString &message);

    RestClient *m_rest;
    GatewayClient *m_gateway;
    Cache *m_cache;
    QString m_token;
    Intents m_intents;
};

#endif // CLIENT_H
