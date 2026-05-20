#ifndef DISCORD_TYPES_H
#define DISCORD_TYPES_H

#include "snowflake.h"
#include <QString>
#include <QDateTime>
#include <QList>
#include <optional>
#include <QMetaType>

enum class ChannelType
{
    Text = 0,
    DM = 1,
    Voice = 2,
    GroupDM = 3,
    GuildCategory = 4,
    GuildAnnouncement = 5,
    AnnouncementThread = 10,
    PublicThread = 11,
    PrivateThread = 12,
    GuildStageVoice = 13,
    GuildForum = 15,
    GuildMedia = 16
};
Q_DECLARE_METATYPE(ChannelType)

struct User
{
    Snowflake id;
    QString username;
    std::optional<QString> avatar;
    std::optional<QString> discriminator;
    bool bot = false;
    std::optional<QString> globalName;
    std::optional<quint32> publicFlags;
    std::optional<QString> banner;
    std::optional<int> accentColor;
};

struct Guild
{
    Snowflake id;
    QString name;
    std::optional<QString> icon;
    Snowflake ownerId;
    bool unavailable = false;
    std::optional<Snowflake> systemChannelId;
};

struct Channel
{
    Snowflake id;
    ChannelType type;
    QString name;
    std::optional<Snowflake> guildId;
    std::optional<Snowflake> parentId;
    int position = 0;
    std::optional<QString> topic;
    std::optional<bool> nsfw;
    std::optional<Snowflake> lastMessageId;
};

struct Attachment
{
    Snowflake id;
    QString filename;
    QString url;
    int size = 0;
    std::optional<int> height;
    std::optional<int> width;
};

struct Embed
{
    struct Field
    {
        QString name;
        QString value;
        bool inline_ = false;
    };

    std::optional<QString> title;
    std::optional<QString> description;
    std::optional<QString> url;
    std::optional<QString> thumbnailUrl;
    std::optional<int> color;
    std::optional<QString> footerText;
    std::optional<QString> authorName;
    std::optional<QString> authorUrl;
    std::optional<QString> authorIconUrl;
    QList<Field> fields;
};

struct Message
{
    Snowflake id;
    Snowflake channelId;
    User author;
    QString content;
    QDateTime timestamp;
    std::optional<QDateTime> editedTimestamp;
    QList<Embed> embeds;
    QList<Attachment> attachments;
    QList<Snowflake> mentions;
    bool pinned = false;
    bool tts = false;
    std::optional<int> type;
};

Q_DECLARE_METATYPE(User)
Q_DECLARE_METATYPE(Guild)
Q_DECLARE_METATYPE(Channel)
Q_DECLARE_METATYPE(Message)
Q_DECLARE_METATYPE(Embed)
Q_DECLARE_METATYPE(Attachment)
Q_DECLARE_METATYPE(Snowflake)

#endif // DISCORD_TYPES_H
