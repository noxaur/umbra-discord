#include "core/formatting/discord_markdown.h"
#include <QRegularExpression>
#include <QDateTime>
#include <tuple>

QString DiscordMarkdown::escapeHtml(const QString &text)
{
    QString result = text;
    result.replace(QLatin1String("&"), QLatin1String("&amp;"));
    result.replace(QLatin1String("<"), QLatin1String("&lt;"));
    result.replace(QLatin1String(">"), QLatin1String("&gt;"));
    result.replace(QLatin1String("\""), QLatin1String("&quot;"));
    return result;
}

QString DiscordMarkdown::toHtml(const QString &markdown)
{
    if (markdown.isEmpty()) return QString();

    QString result = markdown;

    // Process code blocks first (to protect their content)
    result = processCodeBlocks(result);

    // Process blockquotes
    result = processBlockquotes(result);

    // Process lists
    result = processLists(result);

    // Process inline elements
    result = processInline(result);

    return result;
}

QString DiscordMarkdown::processCodeBlocks(const QString &text)
{
    // Fenced code blocks: ```lang\ncode\n```
    QRegularExpression fencedRe(
        QStringLiteral("```(\\w*)\\n([\\s\\S]*?)```"),
        QRegularExpression::MultilineOption
    );

    QString result = text;
    QRegularExpressionMatchIterator it = fencedRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> replacements;

    int offset = 0;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString lang = match.captured(1);
        QString code = escapeHtml(match.captured(2));

        QString replacement = QStringLiteral("<pre><code>%1</code></pre>").arg(code);
        if (!lang.isEmpty()) {
            replacement = QStringLiteral("<pre class=\"lang-%1\"><code>%2</code></pre>")
                .arg(lang, code);
        }

        replacements.append({match.capturedStart(), match.capturedLength(), replacement});
    }

    // Apply replacements in reverse order to preserve positions
    for (int i = replacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(replacements[i]), std::get<1>(replacements[i]), std::get<2>(replacements[i]));
    }

    return result;
}

QString DiscordMarkdown::processBlockquotes(const QString &text)
{
    QRegularExpression quoteRe(
        QStringLiteral("^&gt;\\s+(.+)$"),
        QRegularExpression::MultilineOption
    );

    QString result = text;
    result.replace(quoteRe, QStringLiteral("<blockquote>\\1</blockquote>"));

    // Merge consecutive blockquotes
    result.replace(
        QStringLiteral("</blockquote>\n<blockquote>"),
        QStringLiteral("\n")
    );

    return result;
}

QString DiscordMarkdown::processLists(const QString &text)
{
    // Unordered lists: - item or * item
    QRegularExpression ulRe(
        QStringLiteral("^[\\*\\-]\\s+(.+)$"),
        QRegularExpression::MultilineOption
    );

    QString result = text;
    result.replace(ulRe, QStringLiteral("<li>\\1</li>"));

    // Wrap consecutive <li> in <ul>
    QRegularExpression ulWrapRe(
        QStringLiteral("((?:<li>.*</li>\\n?)+)"),
        QRegularExpression::MultilineOption
    );
    result.replace(ulWrapRe, QStringLiteral("<ul>\\1</ul>"));

    // Ordered lists: 1. item
    QRegularExpression olRe(
        QStringLiteral("^\\d+\\.\\s+(.+)$"),
        QRegularExpression::MultilineOption
    );
    result.replace(olRe, QStringLiteral("<li>\\1</li>"));

    return result;
}

QString DiscordMarkdown::processInline(const QString &text)
{
    QString result = text;

    // Order matters: process code first, then bold/underline/italic/etc.
    result = processInlineCode(result);
    result = processUnderline(result);  // __underline__ before **bold**
    result = processBold(result);
    result = processItalic(result);
    result = processStrikethrough(result);
    result = processSpoilers(result);
    result = processLinks(result);
    result = processMentions(result);
    result = processTimestamps(result);
    result = processAutoLinks(result);

    return result;
}

QString DiscordMarkdown::processInlineCode(const QString &text)
{
    // Double backticks first (to handle single backticks inside)
    QRegularExpression doubleBacktickRe(QStringLiteral("``([^`]+)``"));
    QString result = text;
    result.replace(doubleBacktickRe, QStringLiteral("<code>\\1</code>"));

    // Single backticks
    QRegularExpression singleBacktickRe(QStringLiteral("`([^`]+)`"));
    result.replace(singleBacktickRe, QStringLiteral("<code>\\1</code>"));

    return result;
}

QString DiscordMarkdown::processBold(const QString &text)
{
    // **bold**
    QRegularExpression boldRe(QStringLiteral("\\*\\*(.+?)\\*\\*"));
    QString result = text;
    result.replace(boldRe, QStringLiteral("<strong>\\1</strong>"));

    return result;
}

QString DiscordMarkdown::processItalic(const QString &text)
{
    // *italic* or _italic_
    QRegularExpression italicRe(QStringLiteral("\\*(.+?)\\*"));
    QString result = text;
    result.replace(italicRe, QStringLiteral("<em>\\1</em>"));

    // Single underscore italic (not double)
    QRegularExpression underscoreItalicRe(QStringLiteral("(?<!_)_(?!_)(.+?)(?<!_)_(?!_)"));
    result.replace(underscoreItalicRe, QStringLiteral("<em>\\1</em>"));

    return result;
}

QString DiscordMarkdown::processUnderline(const QString &text)
{
    // __underline__ (must run before bold to avoid conflict)
    QRegularExpression underlineRe(QStringLiteral("__(.+?)__"));
    QString result = text;
    result.replace(underlineRe, QStringLiteral("<u>\\1</u>"));

    return result;
}

QString DiscordMarkdown::processStrikethrough(const QString &text)
{
    // ~~strikethrough~~
    QRegularExpression strikeRe(QStringLiteral("~~(.+?)~~"));
    QString result = text;
    result.replace(strikeRe, QStringLiteral("<s>\\1</s>"));

    return result;
}

QString DiscordMarkdown::processSpoilers(const QString &text)
{
    // ||spoiler||
    QRegularExpression spoilerRe(QStringLiteral("\\|\\|(.+?)\\|\\|"));
    QString result = text;
    result.replace(spoilerRe, QStringLiteral("<span class=\"spoiler\">\\1</span>"));

    return result;
}

QString DiscordMarkdown::processLinks(const QString &text)
{
    // [text](url)
    QRegularExpression linkRe(QStringLiteral("\\[([^\\]]+)\\]\\(([^)]+)\\)"));
    QString result = text;
    result.replace(linkRe, QStringLiteral("<a href=\"\\2\">\\1</a>"));

    return result;
}

QString DiscordMarkdown::processMentions(const QString &text)
{
    // <@userid>
    QRegularExpression userMentionRe(QStringLiteral("&lt;@(\\d+)&gt;"));
    QString result = text;
    result.replace(userMentionRe, QStringLiteral("<span class=\"mention\">@%1</span>"));

    // <#channelid>
    QRegularExpression channelMentionRe(QStringLiteral("&lt;#(\\d+)&gt;"));
    result.replace(channelMentionRe, QStringLiteral("<span class=\"channel\">#%1</span>"));

    return result;
}

QString DiscordMarkdown::processTimestamps(const QString &text)
{
    // <t:timestamp> or <t:timestamp:style>
    QRegularExpression timestampRe(
        QStringLiteral("&lt;t:(\\d+)(?::([tTdDfFR]))?&gt;")
    );
    QString result = text;

    QRegularExpressionMatchIterator it = timestampRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> replacements;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        qint64 ts = match.captured(1).toLongLong();
        QString style = match.captured(2);
        if (style.isEmpty()) style = QStringLiteral("f");

        QDateTime dt = QDateTime::fromSecsSinceEpoch(ts);
        QString formatted;

        switch (style.at(0).toLatin1()) {
        case 't': formatted = dt.toString("HH:mm"); break;
        case 'T': formatted = dt.toString("HH:mm:ss"); break;
        case 'd': formatted = dt.toString("dd/MM/yyyy"); break;
        case 'D': formatted = dt.toString("dd MMMM yyyy"); break;
        case 'f': formatted = dt.toString("dd MMMM yyyy HH:mm"); break;
        case 'F': formatted = dt.toString("dd MMMM yyyy HH:mm:ss"); break;
        case 'R': {
            qint64 diff = dt.secsTo(QDateTime::currentDateTime());
            if (diff < 0) {
                formatted = QStringLiteral("in %1").arg(formatDuration(-diff));
            } else {
                formatted = QStringLiteral("%1 ago").arg(formatDuration(diff));
            }
            break;
        }
        default: formatted = dt.toString("dd MMMM yyyy HH:mm"); break;
        }

        replacements.append({match.capturedStart(), match.capturedLength(), formatted});
    }

    for (int i = replacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(replacements[i]), std::get<1>(replacements[i]), std::get<2>(replacements[i]));
    }

    return result;
}

QString DiscordMarkdown::formatDuration(qint64 seconds)
{
    if (seconds < 60) return QStringLiteral("%1 seconds").arg(seconds);
    if (seconds < 3600) return QStringLiteral("%1 minutes").arg(seconds / 60);
    if (seconds < 86400) return QStringLiteral("%1 hours").arg(seconds / 3600);
    return QStringLiteral("%1 days").arg(seconds / 86400);
}

QString DiscordMarkdown::processAutoLinks(const QString &text)
{
    // Auto-link bare URLs (not already inside <a> tags)
    QRegularExpression urlRe(
        QStringLiteral("(?<!href=\")\\b(https?://[^\\s<]+)"),
        QRegularExpression::CaseInsensitiveOption
    );
    QString result = text;
    result.replace(urlRe, QStringLiteral("<a href=\"\\1\">\\1</a>"));

    return result;
}
