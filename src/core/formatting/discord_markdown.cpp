#include "core/formatting/discord_markdown.h"
#include "core/utils.h"
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

    // Extract mentions and timestamps BEFORE escaping (they use <...> syntax)
    // Use placeholder tokens so escapeHtml doesn't destroy the generated HTML
    QMap<QString, QString> placeholders;
    result = extractMentions(result, placeholders);
    result = extractTimestamps(result, placeholders);

    // Escape ALL HTML to prevent XSS
    result = escapeHtml(result);

    // Process code blocks
    result = processCodeBlocks(result);

    // Process inline code BEFORE other inline formatters
    // so that backtick-wrapped content is protected from bold/italic/etc
    result = processInlineCode(result);

    // Process blockquotes
    result = processBlockquotes(result);

    // Process lists
    result = processLists(result);

    // Process remaining inline elements
    result = processUnderline(result);
    result = processBold(result);
    result = processItalic(result);
    result = processStrikethrough(result);
    result = processSpoilers(result);
    result = processLinks(result);
    result = processAutoLinks(result);

    // Convert remaining newlines to <br> outside block elements
    result.replace(QRegularExpression("(?<!>)\\n(?!<)"), QStringLiteral("<br/>"));

    // Restore mention and timestamp placeholders
    for (auto it = placeholders.constBegin(); it != placeholders.constEnd(); ++it) {
        result.replace(it.key(), it.value());
    }

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
        QStringLiteral("^>\\s+(.+)$"),
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

    // Escape HTML in list item content using tuple replacements
    QRegularExpressionMatchIterator ulIt = ulRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> ulReplacements;
    while (ulIt.hasNext()) {
        auto match = ulIt.next();
        ulReplacements.append({match.capturedStart(), match.capturedLength(),
            QStringLiteral("<li>%1</li>").arg(escapeHtml(match.captured(1)))});
    }
    for (int i = ulReplacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(ulReplacements[i]), std::get<1>(ulReplacements[i]), std::get<2>(ulReplacements[i]));
    }

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

    // Escape HTML in ordered list item content
    QRegularExpressionMatchIterator olIt = olRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> olReplacements;
    while (olIt.hasNext()) {
        auto match = olIt.next();
        olReplacements.append({match.capturedStart(), match.capturedLength(),
            QStringLiteral("<li>%1</li>").arg(escapeHtml(match.captured(1)))});
    }
    for (int i = olReplacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(olReplacements[i]), std::get<1>(olReplacements[i]), std::get<2>(olReplacements[i]));
    }

    // Wrap consecutive <li> in <ol>
    QRegularExpression olWrapRe(
        QStringLiteral("((?:<li>.*</li>\\n?)+)"),
        QRegularExpression::MultilineOption
    );
    result.replace(olWrapRe, QStringLiteral("<ol>\\1</ol>"));

    return result;
}

QString DiscordMarkdown::processInlineCode(const QString &text)
{
    // Double backticks first (to handle single backticks inside)
    QRegularExpression doubleBacktickRe(QStringLiteral("``([^`]+)``"));
    QString result = text;

    QRegularExpressionMatchIterator it2 = doubleBacktickRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> replacements2;
    while (it2.hasNext()) {
        auto match = it2.next();
        replacements2.append({match.capturedStart(), match.capturedLength(),
            QStringLiteral("<code>%1</code>").arg(escapeHtml(match.captured(1)))});
    }
    for (int i = replacements2.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(replacements2[i]), std::get<1>(replacements2[i]), std::get<2>(replacements2[i]));
    }

    // Single backticks
    QRegularExpression singleBacktickRe(QStringLiteral("`([^`]+)`"));
    QRegularExpressionMatchIterator it1 = singleBacktickRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> replacements1;
    while (it1.hasNext()) {
        auto match = it1.next();
        replacements1.append({match.capturedStart(), match.capturedLength(),
            QStringLiteral("<code>%1</code>").arg(escapeHtml(match.captured(1)))});
    }
    for (int i = replacements1.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(replacements1[i]), std::get<1>(replacements1[i]), std::get<2>(replacements1[i]));
    }

    return result;
}

QString DiscordMarkdown::processBold(const QString &text)
{
    // **bold** (don't match across newlines)
    QRegularExpression boldRe(QStringLiteral("\\*\\*([^\\n]+?)\\*\\*"));
    QString result = text;
    result.replace(boldRe, QStringLiteral("<strong>\\1</strong>"));

    return result;
}

QString DiscordMarkdown::processItalic(const QString &text)
{
    // *italic* (don't match nested asterisks or across newlines)
    QRegularExpression italicRe(QStringLiteral("\\*([^\\n*]+?)\\*"));
    QString result = text;
    result.replace(italicRe, QStringLiteral("<em>\\1</em>"));

    // Single underscore italic (not double)
    QRegularExpression underscoreItalicRe(QStringLiteral("(?<!_)_(?!_)([^\\n_]+?)(?<!_)_(?!_)"));
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
    QRegularExpression linkRe(QStringLiteral("\\[([^\\]]+)\\]\\(([^)]+)\\)"));
    QString result = text;

    QRegularExpressionMatchIterator it = linkRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> replacements;

    while (it.hasNext()) {
        auto match = it.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        if (!isSafeUrl(url)) {
            replacements.append({match.capturedStart(), match.capturedLength(), linkText});
        } else {
            replacements.append({match.capturedStart(), match.capturedLength(),
                QStringLiteral("<a href=\"%1\">%2</a>").arg(url, linkText)});
        }
    }

    for (int i = replacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(replacements[i]), std::get<1>(replacements[i]), std::get<2>(replacements[i]));
    }

    return result;
}

QString DiscordMarkdown::processMentions(const QString &text)
{
    // <@userid>
    QRegularExpression userMentionRe(QStringLiteral("<@(\\d+)>"));
    QString result = text;
    result.replace(userMentionRe, QStringLiteral("<span class=\"mention\">@%1</span>"));

    // <#channelid>
    QRegularExpression channelMentionRe(QStringLiteral("<#(\\d+)>"));
    result.replace(channelMentionRe, QStringLiteral("<span class=\"channel\">#%1</span>"));

    return result;
}

QString DiscordMarkdown::extractMentions(const QString &text, QMap<QString, QString> &placeholders)
{
    QString result = text;
    int counter = 0;

    // <@userid>
    QRegularExpression userMentionRe(QStringLiteral("<@(\\d+)>"));
    QRegularExpressionMatchIterator userIt = userMentionRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> userReplacements;
    while (userIt.hasNext()) {
        auto match = userIt.next();
        QString placeholder = QString("\x00M%1\x00").arg(counter++);
        placeholders.insert(placeholder, QStringLiteral("<span class=\"mention\">@%1</span>").arg(match.captured(1)));
        userReplacements.append({match.capturedStart(), match.capturedLength(), placeholder});
    }
    for (int i = userReplacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(userReplacements[i]), std::get<1>(userReplacements[i]), std::get<2>(userReplacements[i]));
    }

    // <#channelid>
    QRegularExpression channelMentionRe(QStringLiteral("<#(\\d+)>"));
    QRegularExpressionMatchIterator channelIt = channelMentionRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> channelReplacements;
    while (channelIt.hasNext()) {
        auto match = channelIt.next();
        QString placeholder = QString("\x00M%1\x00").arg(counter++);
        placeholders.insert(placeholder, QStringLiteral("<span class=\"channel\">#%1</span>").arg(match.captured(1)));
        channelReplacements.append({match.capturedStart(), match.capturedLength(), placeholder});
    }
    for (int i = channelReplacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(channelReplacements[i]), std::get<1>(channelReplacements[i]), std::get<2>(channelReplacements[i]));
    }

    return result;
}

QString DiscordMarkdown::processTimestamps(const QString &text)
{
    // <t:timestamp> or <t:timestamp:style>
    QRegularExpression timestampRe(
        QStringLiteral("<t:(\\d+)(?::([tTdDfFR]))?>")
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

QString DiscordMarkdown::extractTimestamps(const QString &text, QMap<QString, QString> &placeholders)
{
    // <t:timestamp> or <t:timestamp:style>
    QRegularExpression timestampRe(
        QStringLiteral("<t:(\\d+)(?::([tTdDfFR]))?>")
    );
    QString result = text;

    QRegularExpressionMatchIterator it = timestampRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> replacements;
    int counter = placeholders.size();

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

        QString placeholder = QString("\x00T%1\x00").arg(counter++);
        placeholders.insert(placeholder, formatted);
        replacements.append({match.capturedStart(), match.capturedLength(), placeholder});
    }

    for (int i = replacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(replacements[i]), std::get<1>(replacements[i]), std::get<2>(replacements[i]));
    }

    return result;
}

QString DiscordMarkdown::formatDuration(qint64 seconds)
{
    if (seconds < 60) return seconds == 1 ? QStringLiteral("1 second") : QStringLiteral("%1 seconds").arg(seconds);
    if (seconds < 3600) { qint64 m = seconds / 60; return m == 1 ? QStringLiteral("1 minute") : QStringLiteral("%1 minutes").arg(m); }
    if (seconds < 86400) { qint64 h = seconds / 3600; return h == 1 ? QStringLiteral("1 hour") : QStringLiteral("%1 hours").arg(h); }
    { qint64 d = seconds / 86400; return d == 1 ? QStringLiteral("1 day") : QStringLiteral("%1 days").arg(d); }
}

QString DiscordMarkdown::processAutoLinks(const QString &text)
{
    QRegularExpression urlRe(
        QStringLiteral("(?<!href=\")\\b(https?://[^\\s<]+)"),
        QRegularExpression::CaseInsensitiveOption
    );
    QString result = text;

    QRegularExpressionMatchIterator it = urlRe.globalMatch(result);
    QVector<std::tuple<int, int, QString>> replacements;

    while (it.hasNext()) {
        auto match = it.next();
        QString url = match.captured(1);
        if (isSafeUrl(url)) {
            replacements.append({match.capturedStart(), match.capturedLength(),
                QStringLiteral("<a href=\"%1\">%1</a>").arg(url)});
        }
    }

    for (int i = replacements.size() - 1; i >= 0; --i) {
        result.replace(std::get<0>(replacements[i]), std::get<1>(replacements[i]), std::get<2>(replacements[i]));
    }

    return result;
}
