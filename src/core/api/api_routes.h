#ifndef API_ROUTES_H
#define API_ROUTES_H

#include "core/types/snowflake.h"
#include <QString>
#include <QUrlQuery>
#include <optional>

namespace Routes
{
inline QString baseUrl()
{
    return QStringLiteral("https://discord.com/api/v10");
}

inline QString currentUser()
{
    return QStringLiteral("/users/@me");
}

inline QString gatewayBot()
{
    return QStringLiteral("/gateway/bot");
}

inline QString guilds()
{
    return QStringLiteral("/guilds");
}

inline QString guildChannels(Snowflake guildId)
{
    return QStringLiteral("/guilds/%1/channels").arg(guildId.toString());
}

inline QString channelMessages(Snowflake channelId, int limit = 50, std::optional<Snowflake> before = {}, std::optional<Snowflake> after = {})
{
    QString path = QStringLiteral("/channels/%1/messages").arg(channelId.toString());
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    if (before.has_value())
        query.addQueryItem(QStringLiteral("before"), before.value().toString());
    if (after.has_value())
        query.addQueryItem(QStringLiteral("after"), after.value().toString());
    if (!query.isEmpty())
        path += QStringLiteral("?") + query.toString();
    return path;
}

inline QString sendMessage(Snowflake channelId)
{
    return QStringLiteral("/channels/%1/messages").arg(channelId.toString());
}
}

#endif // API_ROUTES_H
