#ifndef INTENTS_H
#define INTENTS_H

#include <QFlags>

enum class Intent : quint64
{
    Guilds = (1ULL << 0),
    GuildMembers = (1ULL << 1),
    GuildModeration = (1ULL << 2),
    GuildExpressions = (1ULL << 3),
    GuildIntegrations = (1ULL << 4),
    GuildWebhooks = (1ULL << 5),
    GuildInvites = (1ULL << 6),
    GuildVoiceStates = (1ULL << 7),
    GuildPresences = (1ULL << 8),
    GuildMessages = (1ULL << 9),
    GuildMessageReactions = (1ULL << 10),
    GuildMessageTyping = (1ULL << 11),
    DirectMessages = (1ULL << 12),
    DirectMessageReactions = (1ULL << 13),
    DirectMessageTyping = (1ULL << 14),
    MessageContent = (1ULL << 15),
    GuildScheduledEvents = (1ULL << 16),
    AutoModerationConfiguration = (1ULL << 20),
    AutoModerationExecution = (1ULL << 21),
    GuildMessagePolls = (1ULL << 24),
    DirectMessagePolls = (1ULL << 25)
};

Q_DECLARE_FLAGS(Intents, Intent)
Q_DECLARE_OPERATORS_FOR_FLAGS(Intents)

inline Intents defaultIntents()
{
    return Intent::Guilds | Intent::GuildMessages | Intent::MessageContent | Intent::DirectMessages;
}

#endif // INTENTS_H
