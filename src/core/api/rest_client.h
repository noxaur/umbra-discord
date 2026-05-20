#ifndef REST_CLIENT_H
#define REST_CLIENT_H

#include "core/types/discord_types.h"
#include "core/types/json_parser.h"
#include "core/api/api_routes.h"
#include "core/api/rate_limiter.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <nlohmann/json.hpp>
#include <QObject>

class RestClient : public QObject
{
    Q_OBJECT

public:
    explicit RestClient(QObject *parent = nullptr);

    void setToken(const QString &token);
    QNetworkReply *get(const QString &endpoint);
    QNetworkReply *post(const QString &endpoint, const nlohmann::json &body);

    void fetchCurrentUser();
    void fetchGatewayUrl();
    void fetchGuilds();
    void fetchGuildChannels(Snowflake guildId);
    void fetchChannelMessages(Snowflake channelId, int limit = 50, std::optional<Snowflake> before = {}, std::optional<Snowflake> after = {});
    void sendMessage(Snowflake channelId, const QString &content);

signals:
    void userReady(User user);
    void userError(QString error);
    void gatewayUrlReady(QString url);
    void gatewayUrlError(QString error);
    void guildsReady(QList<Guild> guilds);
    void guildsError(QString error);
    void guildChannelsReady(Snowflake guildId, QList<Channel> channels);
    void guildChannelsError(QString error);
    void messagesReady(QList<Message> messages);
    void messagesError(QString error);
    void messageSent(Message message);
    void messageSendError(QString error);

private:
    void applyAuthHeaders(QNetworkRequest &request) const;
    void updateRateLimits(QNetworkReply *reply);
    nlohmann::json parseJson(QNetworkReply *reply);

    QNetworkAccessManager *m_nam;
    QString m_token;
    RateLimiter *m_rateLimiter;
};

#endif // REST_CLIENT_H
