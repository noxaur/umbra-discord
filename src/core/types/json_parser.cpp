#include "core/types/json_parser.h"

User JsonParser::parseUser(const nlohmann::json &json)
{
    User user;
    user.id = Snowflake(QString::fromStdString(json.value("id", "0")));
    user.username = QString::fromStdString(json.value("username", ""));
    if (json.contains("avatar") && !json["avatar"].is_null())
        user.avatar = QString::fromStdString(json["avatar"].get<std::string>());
    if (json.contains("discriminator") && !json["discriminator"].is_null())
        user.discriminator = QString::fromStdString(json["discriminator"].get<std::string>());
    user.bot = json.value("bot", false);
    if (json.contains("global_name") && !json["global_name"].is_null())
        user.globalName = QString::fromStdString(json["global_name"].get<std::string>());
    if (json.contains("public_flags"))
        user.publicFlags = json["public_flags"].get<quint32>();
    if (json.contains("banner") && !json["banner"].is_null())
        user.banner = QString::fromStdString(json["banner"].get<std::string>());
    if (json.contains("accent_color") && !json["accent_color"].is_null())
        user.accentColor = json["accent_color"].get<int>();
    return user;
}

Guild JsonParser::parseGuild(const nlohmann::json &json)
{
    Guild guild;
    guild.id = Snowflake(QString::fromStdString(json.value("id", "0")));
    guild.name = QString::fromStdString(json.value("name", ""));
    if (json.contains("icon") && !json["icon"].is_null())
        guild.icon = QString::fromStdString(json["icon"].get<std::string>());
    if (json.contains("owner_id"))
        guild.ownerId = Snowflake(QString::fromStdString(json["owner_id"].get<std::string>()));
    guild.unavailable = json.value("unavailable", false);
    if (json.contains("system_channel_id") && !json["system_channel_id"].is_null())
        guild.systemChannelId = Snowflake(QString::fromStdString(json["system_channel_id"].get<std::string>()));
    return guild;
}

Channel JsonParser::parseChannel(const nlohmann::json &json)
{
    Channel channel;
    channel.id = Snowflake(QString::fromStdString(json.value("id", "0")));
    channel.type = static_cast<ChannelType>(json.value("type", 0));
    if (json.contains("name") && !json["name"].is_null())
        channel.name = QString::fromStdString(json["name"].get<std::string>());
    if (json.contains("guild_id") && !json["guild_id"].is_null())
        channel.guildId = Snowflake(QString::fromStdString(json["guild_id"].get<std::string>()));
    if (json.contains("parent_id") && !json["parent_id"].is_null())
        channel.parentId = Snowflake(QString::fromStdString(json["parent_id"].get<std::string>()));
    channel.position = json.value("position", 0);
    if (json.contains("topic") && !json["topic"].is_null())
        channel.topic = QString::fromStdString(json["topic"].get<std::string>());
    if (json.contains("nsfw"))
        channel.nsfw = json["nsfw"].get<bool>();
    if (json.contains("last_message_id") && !json["last_message_id"].is_null())
        channel.lastMessageId = Snowflake(QString::fromStdString(json["last_message_id"].get<std::string>()));
    return channel;
}

Embed JsonParser::parseEmbed(const nlohmann::json &json)
{
    Embed embed;
    if (json.contains("title") && !json["title"].is_null())
        embed.title = QString::fromStdString(json["title"].get<std::string>());
    if (json.contains("description") && !json["description"].is_null())
        embed.description = QString::fromStdString(json["description"].get<std::string>());
    if (json.contains("url") && !json["url"].is_null())
        embed.url = QString::fromStdString(json["url"].get<std::string>());
    if (json.contains("color"))
        embed.color = json["color"].get<int>();
    if (json.contains("thumbnail") && !json["thumbnail"].is_null())
    {
        if (json["thumbnail"].contains("url"))
            embed.thumbnailUrl = QString::fromStdString(json["thumbnail"]["url"].get<std::string>());
    }
    if (json.contains("footer") && !json["footer"].is_null() && json["footer"].contains("text"))
        embed.footerText = QString::fromStdString(json["footer"]["text"].get<std::string>());
    if (json.contains("author") && !json["author"].is_null())
    {
        const auto &author = json["author"];
        if (author.contains("name") && !author["name"].is_null())
            embed.authorName = QString::fromStdString(author["name"].get<std::string>());
        if (author.contains("url") && !author["url"].is_null())
            embed.authorUrl = QString::fromStdString(author["url"].get<std::string>());
        if (author.contains("icon_url") && !author["icon_url"].is_null())
            embed.authorIconUrl = QString::fromStdString(author["icon_url"].get<std::string>());
    }
    if (json.contains("fields") && json["fields"].is_array())
    {
        for (const auto &f : json["fields"])
        {
            Embed::Field field;
            if (f.contains("name"))
                field.name = QString::fromStdString(f["name"].get<std::string>());
            if (f.contains("value"))
                field.value = QString::fromStdString(f["value"].get<std::string>());
            if (f.contains("inline"))
                field.inline_ = f["inline"].get<bool>();
            embed.fields.append(field);
        }
    }
    return embed;
}

Attachment JsonParser::parseAttachment(const nlohmann::json &json)
{
    Attachment attachment;
    attachment.id = Snowflake(QString::fromStdString(json.value("id", "0")));
    attachment.filename = QString::fromStdString(json.value("filename", ""));
    attachment.url = QString::fromStdString(json.value("url", ""));
    attachment.size = json.value("size", 0);
    if (json.contains("height") && !json["height"].is_null())
        attachment.height = json["height"].get<int>();
    if (json.contains("width") && !json["width"].is_null())
        attachment.width = json["width"].get<int>();
    return attachment;
}

Message JsonParser::parseMessage(const nlohmann::json &json)
{
    Message message;
    message.id = Snowflake(QString::fromStdString(json.value("id", "0")));
    message.channelId = Snowflake(QString::fromStdString(json.value("channel_id", "0")));
    if (json.contains("author"))
        message.author = parseUser(json["author"]);
    message.content = QString::fromStdString(json.value("content", ""));
    if (json.contains("timestamp") && !json["timestamp"].is_null())
        message.timestamp = QDateTime::fromString(QString::fromStdString(json["timestamp"].get<std::string>()), Qt::ISODate);
    if (json.contains("edited_timestamp") && !json["edited_timestamp"].is_null())
        message.editedTimestamp = QDateTime::fromString(QString::fromStdString(json["edited_timestamp"].get<std::string>()), Qt::ISODate);
    message.pinned = json.value("pinned", false);
    message.tts = json.value("tts", false);
    if (json.contains("type"))
        message.type = json["type"].get<int>();

    if (json.contains("embeds") && json["embeds"].is_array())
    {
        for (const auto &e : json["embeds"])
            message.embeds.append(parseEmbed(e));
    }

    if (json.contains("attachments") && json["attachments"].is_array())
    {
        for (const auto &a : json["attachments"])
            message.attachments.append(parseAttachment(a));
    }

    if (json.contains("mentions") && json["mentions"].is_array())
    {
        for (const auto &m : json["mentions"])
        {
            if (m.contains("id"))
                message.mentions.append(Snowflake(QString::fromStdString(m["id"].get<std::string>())));
        }
    }

    return message;
}
