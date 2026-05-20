#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "core/types/discord_types.h"
#include <nlohmann/json.hpp>

class JsonParser
{
public:
    static User parseUser(const nlohmann::json &json);
    static Guild parseGuild(const nlohmann::json &json);
    static Channel parseChannel(const nlohmann::json &json);
    static Message parseMessage(const nlohmann::json &json);

private:
    static Embed parseEmbed(const nlohmann::json &json);
    static Attachment parseAttachment(const nlohmann::json &json);
};

#endif // JSON_PARSER_H
