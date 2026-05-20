#ifndef OPCODES_H
#define OPCODES_H

#include "intents.h"
#include <QString>
#include <optional>
#include <nlohmann/json.hpp>

enum class GatewayOpcode
{
    Dispatch = 0,
    Heartbeat = 1,
    Identify = 2,
    PresenceUpdate = 3,
    VoiceStateUpdate = 4,
    Resume = 6,
    Reconnect = 7,
    RequestGuildMembers = 8,
    InvalidSession = 9,
    Hello = 10,
    HeartbeatAck = 11
};

inline nlohmann::json makeIdentify(const QString &token, Intents intents, const QString &os = "linux", const QString &browser = "discord-qt", const QString &device = "discord-qt")
{
    nlohmann::json payload;
    payload["op"] = static_cast<int>(GatewayOpcode::Identify);
    payload["d"]["token"] = token.toStdString();
    payload["d"]["properties"]["os"] = os.toStdString();
    payload["d"]["properties"]["browser"] = browser.toStdString();
    payload["d"]["properties"]["device"] = device.toStdString();
    payload["d"]["intents"] = static_cast<quint64>(intents);
    return payload;
}

inline nlohmann::json makeResume(const QString &token, const QString &sessionId, int seq)
{
    nlohmann::json payload;
    payload["op"] = static_cast<int>(GatewayOpcode::Resume);
    payload["d"]["token"] = token.toStdString();
    payload["d"]["session_id"] = sessionId.toStdString();
    payload["d"]["seq"] = seq;
    return payload;
}

inline nlohmann::json makeHeartbeat(std::optional<int> seq)
{
    nlohmann::json payload;
    payload["op"] = static_cast<int>(GatewayOpcode::Heartbeat);
    if (seq.has_value()) {
        payload["d"] = seq.value();
    } else {
        payload["d"] = nullptr;
    }
    return payload;
}

#endif // OPCODES_H
