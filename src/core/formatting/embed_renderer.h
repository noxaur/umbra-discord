#ifndef EMBED_RENDERER_H
#define EMBED_RENDERER_H

#include "core/types/discord_types.h"
#include <QString>

class EmbedRenderer {
public:
    static QString toHtml(const Embed &embed);

private:
    static QString renderAuthor(const Embed &embed);
    static QString renderTitle(const Embed &embed);
    static QString renderDescription(const Embed &embed);
    static QString renderThumbnail(const Embed &embed);
    static QString renderFields(const Embed &embed);
    static QString renderFooter(const Embed &embed);
    static QString embedColorHex(const Embed &embed);
};

#endif // EMBED_RENDERER_H
