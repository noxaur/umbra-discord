#include "core/cache.h"
#include <algorithm>

Cache::Cache(QObject *parent)
    : QObject(parent)
{
}

User Cache::self() const
{
    return m_self;
}

std::optional<Guild> Cache::guild(Snowflake id) const
{
    auto it = m_guilds.find(id);
    if (it != m_guilds.end())
        return *it;
    return std::nullopt;
}

QList<Guild> Cache::allGuilds() const
{
    return m_guilds.values();
}

std::optional<Channel> Cache::channel(Snowflake id) const
{
    auto it = m_channels.find(id);
    if (it != m_channels.end())
        return *it;
    return std::nullopt;
}

QList<Channel> Cache::guildChannels(Snowflake guildId) const
{
    QList<Channel> result;
    for (auto it = m_channels.begin(); it != m_channels.end(); ++it)
    {
        if (it->guildId.has_value() && it->guildId.value() == guildId)
            result.append(*it);
    }
    return result;
}

QList<Message> Cache::channelMessages(Snowflake channelId, int limit) const
{
    auto channelIt = m_messagesByChannel.find(channelId);
    if (channelIt == m_messagesByChannel.end())
        return {};

    QList<Message> messages = channelIt->values();
    std::sort(messages.begin(), messages.end(),
              [](const Message &a, const Message &b)
              {
                  return a.timestamp > b.timestamp;
              });

    if (messages.size() > limit)
        messages = messages.mid(0, limit);

    return messages;
}

std::optional<Message> Cache::message(Snowflake channelId, Snowflake messageId) const
{
    auto channelIt = m_messagesByChannel.find(channelId);
    if (channelIt == m_messagesByChannel.end())
        return std::nullopt;

    auto msgIt = channelIt->find(messageId);
    if (msgIt != channelIt->end())
        return *msgIt;

    return std::nullopt;
}

void Cache::setSelf(const User &user)
{
    m_self = user;
    emit selfUpdated(user);
}

void Cache::upsertGuild(const Guild &guild)
{
    bool isNew = !m_guilds.contains(guild.id);
    m_guilds.insert(guild.id, guild);
    if (isNew)
        emit guildAdded(guild.id);
    else
        emit guildUpdated(guild.id);
}

void Cache::upsertChannel(const Channel &channel)
{
    bool isNew = !m_channels.contains(channel.id);
    m_channels.insert(channel.id, channel);
    if (isNew)
        emit channelAdded(channel.id);
    else
        emit channelUpdated(channel.id);
}

void Cache::insertMessage(const Message &message)
{
    auto &channelMap = m_messagesByChannel[message.channelId];
    bool isNew = !channelMap.contains(message.id);
    channelMap.insert(message.id, message);
    if (isNew)
        emit messageAdded(message.channelId, message.id);
    else
        emit messageUpdated(message.channelId, message.id);
}

void Cache::updateMessage(Snowflake channelId, Snowflake messageId, const QString &newContent)
{
    auto channelIt = m_messagesByChannel.find(channelId);
    if (channelIt == m_messagesByChannel.end())
        return;

    auto msgIt = channelIt->find(messageId);
    if (msgIt == channelIt->end())
        return;

    msgIt->content = newContent;
    emit messageUpdated(channelId, messageId);
}

void Cache::removeMessage(Snowflake channelId, Snowflake messageId)
{
    auto channelIt = m_messagesByChannel.find(channelId);
    if (channelIt == m_messagesByChannel.end())
        return;

    channelIt->remove(messageId);
    emit messageDeleted(channelId, messageId);
}
