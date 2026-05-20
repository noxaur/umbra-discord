#include "core/formatting/attachment_renderer.h"
#include <QString>

QString AttachmentRenderer::toHtml(const Attachment &attachment)
{
    if (isImage(attachment.filename)) {
        return QStringLiteral("<img src=\"%1\" class=\"attachment-image\" alt=\"%2\"/>")
            .arg(attachment.url, attachment.filename);
    }

    if (isVideo(attachment.filename)) {
        return QStringLiteral("<video controls class=\"attachment-image\">"
                             "<source src=\"%1\"/>"
                             "</video>")
            .arg(attachment.url);
    }

    QString sizeStr = formatFileSize(attachment.size);
    return QStringLiteral("<a href=\"%1\" class=\"attachment-file\">"
                         "<span class=\"attachment-filename\">%2</span>"
                         "<span class=\"attachment-size\">%3</span>"
                         "</a>")
        .arg(attachment.url, attachment.filename, sizeStr);
}

bool AttachmentRenderer::isImage(const QString &filename)
{
    QString lower = filename.toLower();
    return lower.endsWith(".png") || lower.endsWith(".jpg") || lower.endsWith(".jpeg") ||
           lower.endsWith(".gif") || lower.endsWith(".webp") || lower.endsWith(".bmp") ||
           lower.endsWith(".svg");
}

bool AttachmentRenderer::isVideo(const QString &filename)
{
    QString lower = filename.toLower();
    return lower.endsWith(".mp4") || lower.endsWith(".webm") || lower.endsWith(".mov") ||
           lower.endsWith(".avi");
}

QString AttachmentRenderer::formatFileSize(int bytes)
{
    if (bytes < 1024) return QStringLiteral("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
}
