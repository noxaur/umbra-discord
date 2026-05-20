#include "core/formatting/embed_renderer.h"
#include "core/formatting/discord_markdown.h"
#include <QString>

QString EmbedRenderer::toHtml(const Embed &embed)
{
    QString color = embedColorHex(embed);

    QString html = QStringLiteral("<div class=\"embed\" style=\"border-left-color: %1;\">").arg(color);

    html += renderAuthor(embed);
    html += renderTitle(embed);
    html += renderDescription(embed);
    html += renderThumbnail(embed);
    html += renderFields(embed);
    html += renderFooter(embed);

    html += QStringLiteral("</div>");

    return html;
}

QString EmbedRenderer::renderAuthor(const Embed &embed)
{
    if (!embed.title.has_value() && !embed.description.has_value()) {
        return QString();
    }
    return QString();
}

QString EmbedRenderer::renderTitle(const Embed &embed)
{
    if (!embed.title.has_value()) return QString();

    QString title = DiscordMarkdown::escapeHtml(embed.title.value());
    if (embed.url.has_value()) {
        return QStringLiteral("<a href=\"%1\" class=\"embed-title\">%2</a>")
            .arg(embed.url.value(), title);
    }
    return QStringLiteral("<div class=\"embed-title\">%1</div>").arg(title);
}

QString EmbedRenderer::renderDescription(const Embed &embed)
{
    if (!embed.description.has_value()) return QString();

    QString desc = DiscordMarkdown::toHtml(embed.description.value());
    return QStringLiteral("<div class=\"embed-description\">%1</div>").arg(desc);
}

QString EmbedRenderer::renderThumbnail(const Embed &embed)
{
    if (!embed.thumbnailUrl.has_value()) return QString();

    return QStringLiteral("<img src=\"%1\" class=\"embed-thumbnail\"/>")
        .arg(embed.thumbnailUrl.value());
}

QString EmbedRenderer::renderFields(const Embed &embed)
{
    if (embed.fields.isEmpty()) return QString();

    QString html = QStringLiteral("<div class=\"embed-fields\">");

    for (const auto &field : embed.fields) {
        QString inlineClass = field.inline_ ? QStringLiteral(" inline") : QString();
        QString name = DiscordMarkdown::toHtml(DiscordMarkdown::escapeHtml(field.name));
        QString value = DiscordMarkdown::toHtml(field.value);

        html += QStringLiteral("<div class=\"embed-field%1\">"
                              "<div class=\"embed-field-name\">%2</div>"
                              "<div class=\"embed-field-value\">%3</div>"
                              "</div>")
            .arg(inlineClass, name, value);
    }

    html += QStringLiteral("</div>");
    return html;
}

QString EmbedRenderer::renderFooter(const Embed &embed)
{
    if (!embed.footerText.has_value()) return QString();

    QString footer = DiscordMarkdown::escapeHtml(embed.footerText.value());
    return QStringLiteral("<div class=\"embed-footer\"><span>%1</span></div>").arg(footer);
}

QString EmbedRenderer::embedColorHex(const Embed &embed)
{
    if (!embed.color.has_value()) return "#5865f2";

    int color = embed.color.value();
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    return QStringLiteral("#%1%2%3")
        .arg(r, 2, 16, QChar('0'))
        .arg(g, 2, 16, QChar('0'))
        .arg(b, 2, 16, QChar('0'));
}
