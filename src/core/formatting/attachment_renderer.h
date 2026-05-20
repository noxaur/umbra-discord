#ifndef ATTACHMENT_RENDERER_H
#define ATTACHMENT_RENDERER_H

#include "core/types/discord_types.h"
#include <QString>

class AttachmentRenderer {
public:
    static QString toHtml(const Attachment &attachment);

private:
    static bool isImage(const QString &filename);
    static bool isVideo(const QString &filename);
    static QString formatFileSize(int bytes);
};

#endif // ATTACHMENT_RENDERER_H
