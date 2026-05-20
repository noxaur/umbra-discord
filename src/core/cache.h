#ifndef CACHE_H
#define CACHE_H

#include "core/types/discord_types.h"
#include <QObject>
#include <QHash>
#include <QList>
#include <optional>

class Cache : public QObject
{
    Q_OBJECT

public:
    explicit Cache(QObject *parent = nullptr);

    // Read methods
    User self() const;
    std::optional<Guild> guild(Snowflake id) const;
    QList<Guild> allGuilds() const;
    std::optional<Channel> channel(Snowflake id) const;
    QList<Channel> guildChannels(Snowflake guildId) const;
    QList<Message> channelMessages(Snowflake channelId, int limit = 50) const;
    std::optional<Message> message(Snowflake channelId, Snowflake messageId) const;

    // Write methods
    void setSelf(const User &user);
    void upsertGuild(const Guild &guild);
    void upsertChannel(const Channel &channel);
    void insertMessage(const Message &message);
    void updateMessage(Snowflake channelId, Snowflake messageId, const QString &newContent);
    void removeMessage(Snowflake channelId, Snowflake messageId);

signals:
    void selfUpdated(const User &user);
    void guildAdded(Snowflake id);
    void guildUpdated(Snowflake id);
    void channelAdded(Snowflake id);
    void channelUpdated(Snowflake id);
    void messageAdded(Snowflake channelId, Snowflake messageId);
    void messageUpdated(Snowflake channelId, Snowflake messageId);
    void messageDeleted(Snowflake channelId, Snowflake messageId);

private:
    User m_self;
    QHash<Snowflake, Guild> m_guilds;
    QHash<Snowflake, Channel> m_channels;
    QHash<Snowflake, QHash<Snowflake, Message>> m_messagesByChannel;
};

#endif // CACHE_H
