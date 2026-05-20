#ifndef DISCORD_MARKDOWN_H
#define DISCORD_MARKDOWN_H

#include <QString>

class DiscordMarkdown {
public:
    static QString toHtml(const QString &markdown);
    static QString escapeHtml(const QString &text);

private:
    static QString processInline(const QString &text);
    static QString processCodeBlocks(const QString &text);
    static QString processBlockquotes(const QString &text);
    static QString processLists(const QString &text);
    static QString processInlineCode(const QString &text);
    static QString processBold(const QString &text);
    static QString processItalic(const QString &text);
    static QString processUnderline(const QString &text);
    static QString processStrikethrough(const QString &text);
    static QString processSpoilers(const QString &text);
    static QString processLinks(const QString &text);
    static QString processMentions(const QString &text);
    static QString processTimestamps(const QString &text);
    static QString formatDuration(qint64 seconds);
    static QString processAutoLinks(const QString &text);
};

#endif // DISCORD_MARKDOWN_H
