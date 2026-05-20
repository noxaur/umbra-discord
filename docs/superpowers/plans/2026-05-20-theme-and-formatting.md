# Theme System + Message Formatting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a Raycast-themed dark UI with theme switching, full Discord markdown rendering, embed/attachment display, and polished message formatting.

**Architecture:** Theme tokens drive dynamic QSS generation. Discord markdown is converted to HTML via regex-based parser. Embeds and attachments render as styled HTML blocks. All styling is theme-aware through CSS classes.

**Tech Stack:** Qt6 (Core, Network, WebSockets, Widgets), nlohmann/json, C++17

---

## File Structure

```
src/
├── main.cpp                              # Modify: replace hardcoded QSS, integrate theme manager, wire formatting
├── core/
│   ├── ui/
│   │   ├── theme.h                       # Create: Theme struct, token definitions
│   │   └── theme.cpp                     # Create: Theme presets, QSS generator, message CSS generator
│   └── formatting/
│       ├── discord_markdown.h            # Create: Markdown → HTML converter header
│       ├── discord_markdown.cpp          # Create: Markdown → HTML converter implementation
│       ├── embed_renderer.h              # Create: Embed → HTML block header
│       ├── embed_renderer.cpp            # Create: Embed → HTML block implementation
│       ├── attachment_renderer.h         # Create: Attachment → HTML block header
│       └── attachment_renderer.cpp       # Create: Attachment → HTML block implementation
CMakeLists.txt                            # Modify: add new source files
```

---

### Task 1: Theme System Core

**Files:**
- Create: `src/core/ui/theme.h`
- Create: `src/core/ui/theme.cpp`

- [ ] **Step 1: Create theme.h**

```cpp
#ifndef THEME_H
#define THEME_H

#include <QString>
#include <QHash>

struct Theme {
    QString name;

    // Surface ladder
    QString canvas;
    QString surface;
    QString surfaceElevated;
    QString surfaceCard;
    QString buttonFg;

    // Text colors
    QString ink;
    QString body;
    QString charcoal;
    QString mute;
    QString ash;
    QString stone;
    QString onDark;
    QString onDarkMute;

    // Borders
    QString hairline;
    QString hairlineSoft;
    QString hairlineStrong;

    // Accents
    QString accentBlue;
    QString accentBlueSoft;
    QString accentRed;
    QString accentRedSoft;
    QString accentGreen;
    QString accentGreenSoft;
    QString accentYellow;
    QString accentYellowSoft;

    // Interactive
    QString primary;
    QString primaryPressed;
    QString onPrimary;

    // Rounded scale (pixels)
    int roundedXS = 4;
    int roundedSM = 6;
    int roundedMD = 8;
    int roundedLG = 10;
    int roundedXL = 16;

    // Font
    QString fontFamily = "Inter, \"SF Pro Text\", \"Segoe UI\", \"Helvetica Neue\", Arial, sans-serif";

    // Factory methods
    static Theme raycast();
    static Theme discord();

    // QSS generation
    QString generateQSS() const;
    QString generateMessageCSS() const;
};

// Theme manager
class ThemeManager {
public:
    static ThemeManager &instance();
    const Theme &currentTheme() const;
    void setTheme(const QString &name);
    void setTheme(const Theme &theme);
    QString regenerateQSS() const;
    QString regenerateMessageCSS() const;

private:
    ThemeManager();
    Theme m_current;
    QHash<QString, Theme> m_themes;
};

#endif // THEME_H
```

- [ ] **Step 2: Create theme.cpp**

```cpp
#include "core/ui/theme.h"

Theme Theme::raycast()
{
    Theme t;
    t.name = "Raycast";

    // Surface ladder
    t.canvas = "#07080a";
    t.surface = "#0d0d0d";
    t.surfaceElevated = "#101111";
    t.surfaceCard = "#121212";
    t.buttonFg = "#18191a";

    // Text
    t.ink = "#f4f4f6";
    t.body = "#cdcdcd";
    t.charcoal = "#d3d3d4";
    t.mute = "#9c9c9d";
    t.ash = "#6a6b6c";
    t.stone = "#434345";
    t.onDark = "#ffffff";
    t.onDarkMute = "rgba(255,255,255,0.72)";

    // Borders
    t.hairline = "#242728";
    t.hairlineSoft = "rgba(255,255,255,0.08)";
    t.hairlineStrong = "rgba(255,255,255,0.16)";

    // Accents
    t.accentBlue = "#57c1ff";
    t.accentBlueSoft = "rgba(87,193,255,0.15)";
    t.accentRed = "#ff6161";
    t.accentRedSoft = "rgba(255,97,97,0.15)";
    t.accentGreen = "#59d499";
    t.accentGreenSoft = "rgba(89,212,153,0.15)";
    t.accentYellow = "#ffc533";
    t.accentYellowSoft = "rgba(255,197,51,0.15)";

    // Interactive
    t.primary = "#ffffff";
    t.primaryPressed = "#e8e8e8";
    t.onPrimary = "#000000";

    return t;
}

Theme Theme::discord()
{
    Theme t;
    t.name = "Discord";

    // Surface ladder (Discord dark)
    t.canvas = "#1e1f22";
    t.surface = "#2b2d31";
    t.surfaceElevated = "#313338";
    t.surfaceCard = "#383a40";
    t.buttonFg = "#313338";

    // Text
    t.ink = "#f2f3f5";
    t.body = "#dbdee1";
    t.charcoal = "#dbdee1";
    t.mute = "#949ba4";
    t.ash = "#4e5058";
    t.stone = "#404249";
    t.onDark = "#ffffff";
    t.onDarkMute = "rgba(255,255,255,0.72)";

    // Borders
    t.hairline = "#1e1f22";
    t.hairlineSoft = "rgba(255,255,255,0.06)";
    t.hairlineStrong = "rgba(255,255,255,0.12)";

    // Accents
    t.accentBlue = "#5865f2";
    t.accentBlueSoft = "rgba(88,101,242,0.15)";
    t.accentRed = "#f23f43";
    t.accentRedSoft = "rgba(242,63,67,0.15)";
    t.accentGreen = "#23a559";
    t.accentGreenSoft = "rgba(35,165,89,0.15)";
    t.accentYellow = "#f0b232";
    t.accentYellowSoft = "rgba(240,178,50,0.15)";

    // Interactive
    t.primary = "#5865f2";
    t.primaryPressed = "#4752c4";
    t.onPrimary = "#ffffff";

    return t;
}

QString Theme::generateQSS() const
{
    return QString(R"(
        QMainWindow, QWidget {
            background-color: %1;
            color: %2;
            font-family: %3;
            font-size: 14px;
        }

        /* Server list */
        QListWidget#serverList {
            background-color: %4;
            border: none;
            outline: none;
            padding: 8px 0;
        }
        QListWidget#serverList::item {
            padding: 6px 12px;
            margin: 1px 8px;
            border-radius: %5px;
            color: %6;
            font-weight: 500;
        }
        QListWidget#serverList::item:selected {
            background-color: %7;
            color: %8;
        }
        QListWidget#serverList::item:hover {
            background-color: %9;
            color: %8;
            border-radius: %5px;
        }

        /* Channel panel */
        QFrame#channelFrame {
            background-color: %10;
            border: none;
        }
        QLabel#channelHeader {
            background-color: %1;
            color: %8;
            font-weight: 600;
            font-size: 15px;
            padding: 12px;
            border-bottom: 1px solid %4;
        }

        /* Channel tree */
        QTreeWidget#channelTree {
            background-color: %10;
            border: none;
            outline: none;
            font-size: 14px;
            padding: 4px 0;
        }
        QTreeWidget#channelTree::item {
            padding: 4px 8px;
            margin: 1px 4px;
            border-radius: %11px;
            color: %6;
        }
        QTreeWidget#channelTree::item:hover {
            background-color: %9;
            color: %8;
        }
        QTreeWidget#channelTree::item:selected {
            background-color: %7;
            color: %8;
        }
        QTreeWidget#channelTree::branch {
            background: transparent;
        }
        QTreeWidget#channelTree::branch:selected {
            background: transparent;
        }
        QTreeWidget#channelTree::item:has-children {
            color: %6;
            font-weight: 600;
            font-size: 12px;
            padding: 12px 8px 4px 8px;
        }

        /* Message area */
        QTextEdit#messageView {
            background-color: %1;
            color: %2;
            border: none;
            padding: 12px;
            font-size: 14px;
            line-height: 1.4;
        }
        QTextEdit#messageView:focus {
            outline: none;
        }

        /* Input area */
        QLineEdit#messageInput {
            background-color: %12;
            color: %2;
            border: none;
            border-radius: %5px;
            padding: 10px 14px;
            font-size: 14px;
        }
        QLineEdit#messageInput:focus {
            outline: none;
            background-color: %7;
        }
        QLineEdit#messageInput:disabled {
            background-color: %10;
            color: %13;
        }

        /* Channel name label */
        QLabel#channelNameLabel {
            color: %8;
            font-weight: 600;
            font-size: 14px;
            padding: 0 8px;
        }

        /* Buttons */
        QPushButton {
            background-color: %14;
            color: %15;
            border: none;
            border-radius: %11px;
            padding: 8px 16px;
            font-weight: 500;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: %16;
        }
        QPushButton:pressed {
            background-color: %17;
        }
        QPushButton:disabled {
            background-color: %7;
            color: %6;
        }

        /* Login input */
        QLineEdit#tokenInput {
            background-color: %12;
            color: %2;
            border: 1px solid %4;
            border-radius: %11px;
            padding: 8px 12px;
            font-size: 14px;
        }
        QLineEdit#tokenInput:focus {
            outline: none;
            border-color: %14;
        }

        /* Status bar */
        QStatusBar {
            background-color: %10;
            color: %6;
            border-top: 1px solid %4;
            font-size: 12px;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            background: %10;
            width: 8px;
            border-radius: %11px;
        }
        QScrollBar::handle:vertical {
            background: %4;
            border-radius: %11px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: %9;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar:horizontal {
            background: %10;
            height: 8px;
            border-radius: %11px;
        }
        QScrollBar::handle:horizontal {
            background: %4;
            border-radius: %11px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background: %9;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }

        /* Splitter handle */
        QSplitter::handle {
            background: %4;
            width: 1px;
        }

        /* Settings button */
        QToolButton#settingsBtn {
            background: transparent;
            color: %6;
            border: none;
            border-radius: %11px;
            font-size: 18px;
        }
        QToolButton#settingsBtn:hover {
            background-color: %9;
            color: %8;
        }
    )").arg(canvas, body, fontFamily,
            canvas,              // server list bg (same as canvas)
            QString::number(roundedSM),  // 6px
            mute,                // item color
            surfaceCard,         // selected bg
            ink,                 // selected text / header text
            surfaceElevated,     // hover bg
            surface,             // channel frame bg
            QString::number(roundedXS),  // 4px
            surfaceCard,         // input bg
            ash,                 // disabled text
            primary,             // button bg
            onPrimary,           // button text
            primaryPressed,      // button hover
            primary              // button pressed (same for now)
    );
}

QString Theme::generateMessageCSS() const
{
    return QString(R"(
        /* Message content */
        .message-content { color: %1; font-size: 14px; line-height: 1.4; }

        /* Markdown */
        strong { font-weight: 600; }
        em { font-style: italic; }
        u { text-decoration: underline; }
        s { text-decoration: line-through; }

        /* Inline code */
        code {
            background-color: %2;
            color: %3;
            padding: 2px 4px;
            border-radius: %4px;
            font-family: "JetBrains Mono", "Fira Code", "SF Mono", "Consolas", monospace;
            font-size: 13px;
        }

        /* Code blocks */
        pre {
            background-color: %2;
            color: %3;
            padding: 12px;
            border-radius: %5px;
            font-family: "JetBrains Mono", "Fira Code", "SF Mono", "Consolas", monospace;
            font-size: 13px;
            line-height: 1.5;
            overflow-x: auto;
            margin: 4px 0;
        }
        pre code {
            background: none;
            padding: 0;
            color: inherit;
        }

        /* Links */
        a {
            color: %6;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }

        /* Blockquotes */
        blockquote {
            border-left: 2px solid %7;
            margin: 4px 0;
            padding: 4px 12px;
            color: %8;
        }

        /* Lists */
        ul, ol {
            margin: 4px 0;
            padding-left: 24px;
        }
        li {
            margin: 2px 0;
        }

        /* Spoilers */
        .spoiler {
            background-color: %9;
            color: transparent;
            cursor: pointer;
            border-radius: %4px;
            padding: 0 2px;
        }
        .spoiler.revealed {
            background-color: transparent;
            color: %1;
        }

        /* Mentions */
        .mention {
            background-color: %10;
            color: %6;
            padding: 1px 4px;
            border-radius: %4px;
            cursor: pointer;
        }
        .mention:hover {
            background-color: %6;
            color: %11;
        }

        /* Channel mentions */
        .channel {
            background-color: %10;
            color: %6;
            padding: 1px 4px;
            border-radius: %4px;
            cursor: pointer;
        }

        /* Embeds */
        .embed {
            background-color: %12;
            border-left: 4px solid %13;
            border-radius: %5px;
            padding: 12px;
            margin: 8px 0;
            max-width: 520px;
        }
        .embed-author {
            display: flex;
            align-items: center;
            gap: 8px;
            margin-bottom: 6px;
            font-size: 13px;
            font-weight: 500;
            color: %1;
        }
        .embed-author-icon {
            width: 24px;
            height: 24px;
            border-radius: 50%%;
        }
        .embed-title {
            color: %6;
            font-weight: 600;
            font-size: 14px;
            display: block;
            margin-bottom: 6px;
        }
        .embed-description {
            color: %1;
            font-size: 14px;
            line-height: 1.4;
        }
        .embed-thumbnail {
            max-width: 80px;
            max-height: 80px;
            border-radius: %5px;
            float: right;
            margin-left: 12px;
        }
        .embed-fields {
            display: flex;
            flex-wrap: wrap;
            gap: 8px;
            margin-top: 8px;
        }
        .embed-field {
            min-width: 100px;
            flex: 1;
        }
        .embed-field.inline {
            flex: 0 0 calc(33%% - 8px);
        }
        .embed-field-name {
            font-weight: 600;
            font-size: 13px;
            color: %1;
            margin-bottom: 2px;
        }
        .embed-field-value {
            font-size: 13px;
            color: %8;
        }
        .embed-footer {
            display: flex;
            align-items: center;
            gap: 6px;
            margin-top: 8px;
            font-size: 12px;
            color: %8;
        }
        .embed-footer-icon {
            width: 20px;
            height: 20px;
            border-radius: 50%%;
        }

        /* Attachments */
        .attachment-image {
            max-width: 400px;
            max-height: 300px;
            border-radius: %5px;
            margin: 4px 0;
            cursor: pointer;
        }
        .attachment-file {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            background-color: %12;
            padding: 8px 12px;
            border-radius: %5px;
            color: %6;
            text-decoration: none;
            margin: 4px 0;
        }
        .attachment-file:hover {
            background-color: %7;
        }
        .attachment-filename {
            font-weight: 500;
            font-size: 13px;
        }
        .attachment-size {
            font-size: 12px;
            color: %8;
        }
    )").arg(body,         // 1: text color
            canvas,        // 2: code bg
            ink,           // 3: code text
            QString::number(roundedXS),  // 4: xs radius
            QString::number(roundedMD),  // 5: md radius
            accentBlue,    // 6: link/accent color
            hairlineStrong,// 7: border color
            mute,          // 8: muted text
            ash,           // 9: spoiler bg
            accentBlueSoft,// 10: mention bg
            onPrimary,     // 11: mention hover text
            surfaceCard,   // 12: embed/file bg
            accentBlue     // 13: embed border color (default)
    );
}

ThemeManager::ThemeManager()
{
    m_themes["Raycast"] = Theme::raycast();
    m_themes["Discord"] = Theme::discord();
    m_current = Theme::raycast(); // Default theme
}

ThemeManager &ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

const Theme &ThemeManager::currentTheme() const
{
    return m_current;
}

void ThemeManager::setTheme(const QString &name)
{
    if (m_themes.contains(name)) {
        m_current = m_themes[name];
    }
}

void ThemeManager::setTheme(const Theme &theme)
{
    m_current = theme;
}

QString ThemeManager::regenerateQSS() const
{
    return m_current.generateQSS();
}

QString ThemeManager::regenerateMessageCSS() const
{
    return m_current.generateMessageCSS();
}
```

- [ ] **Step 3: Build to verify compilation**

Run: `cd build && cmake --build .`
Expected: PASS (no new files in CMakeLists yet, but theme.h/cpp should compile if added)

---

### Task 2: CMakeLists + Theme Integration in main.cpp

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/main.cpp`

- [ ] **Step 1: Update CMakeLists.txt**

Add new source files to the `add_executable` call:

```cmake
add_executable(discord-qt MACOSX_BUNDLE
    src/main.cpp
    src/core/client.h
    src/core/client.cpp
    src/core/types/snowflake.h
    src/core/types/discord_types.h
    src/core/types/json_parser.h
    src/core/types/json_parser.cpp
    src/core/cache.h
    src/core/cache.cpp
    src/core/api/api_routes.h
    src/core/api/rate_limiter.h
    src/core/api/rate_limiter.cpp
    src/core/api/rest_client.h
    src/core/api/rest_client.cpp
    src/core/gateway/intents.h
    src/core/gateway/opcodes.h
    src/core/gateway/session.h
    src/core/gateway/gateway_client.h
    src/core/gateway/gateway_client.cpp
    src/core/ui/theme.h
    src/core/ui/theme.cpp
)
```

- [ ] **Step 2: Update main.cpp imports and replace hardcoded QSS**

At the top of main.cpp, replace the hardcoded `kQSS` string and add theme import:

```cpp
#include <QDebug>
#include <QSettings>
#include <QToolButton>
#include "core/client.h"
#include "core/ui/theme.h"
```

Remove the entire `static const char *kQSS = R"( ... )";` block (lines 25-244).

- [ ] **Step 3: Update main() to use theme system**

Replace the `main()` function:

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Umbra");
    app.setApplicationVersion("0.1.0");
    app.setStyle("Fusion");

    // Load saved theme
    QSettings settings("Umbra", "discord-qt");
    QString savedTheme = settings.value("theme_name", "Raycast").toString();
    ThemeManager::instance().setTheme(savedTheme);

    app.setStyleSheet(ThemeManager::instance().regenerateQSS());

    MainWindow window;
    window.show();

    return app.exec();
}
```

- [ ] **Step 4: Build and run**

Run: `cd build && cmake .. && cmake --build .`
Expected: PASS

Run: `./discord-qt.app/Contents/MacOS/discord-qt`
Expected: App launches with Raycast theme (near-black background)

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt src/main.cpp src/core/ui/theme.h src/core/ui/theme.cpp
git commit -m "feat: add theme system with Raycast and Discord presets"
```

---

### Task 3: Discord Markdown Converter

**Files:**
- Create: `src/core/formatting/discord_markdown.h`
- Create: `src/core/formatting/discord_markdown.cpp`

- [ ] **Step 1: Create discord_markdown.h**

```cpp
#ifndef DISCORD_MARKDOWN_H
#define DISCORD_MARKDOWN_H

#include <QString>

class DiscordMarkdown {
public:
    static QString toHtml(const QString &markdown);

private:
    static QString escapeHtml(const QString &text);
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
```

- [ ] **Step 2: Create discord_markdown.cpp**

```cpp
#include "core/formatting/discord_markdown.h"
#include <QRegularExpression>
#include <QDateTime>

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
    QVector<QPair<int, int>> replacements;

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
        result.replace(replacements[i].first, replacements[i].second, replacements[i].third);
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
    QVector<QPair<int, int>> replacements;

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
        result.replace(replacements[i].first, replacements[i].second, replacements[i].third);
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
```

- [ ] **Step 3: Add to CMakeLists.txt**

Add to `add_executable`:
```cmake
    src/core/formatting/discord_markdown.h
    src/core/formatting/discord_markdown.cpp
```

- [ ] **Step 4: Build**

Run: `cd build && cmake .. && cmake --build .`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt src/core/formatting/discord_markdown.h src/core/formatting/discord_markdown.cpp
git commit -m "feat: add Discord markdown to HTML converter"
```

---

### Task 4: Embed Renderer

**Files:**
- Create: `src/core/formatting/embed_renderer.h`
- Create: `src/core/formatting/embed_renderer.cpp`

- [ ] **Step 1: Create embed_renderer.h**

```cpp
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
```

- [ ] **Step 2: Create embed_renderer.cpp**

```cpp
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
        // Check if there's author info (not in current struct, skip)
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
    if (!embed.color.has_value()) return "#5865f2"; // Default accent

    int color = embed.color.value();
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    return QStringLiteral("#%1%2%3")
        .arg(r, 2, 16, QChar('0'))
        .arg(g, 2, 16, QChar('0'))
        .arg(b, 2, 16, QChar('0'));
}
```

- [ ] **Step 3: Add to CMakeLists.txt**

```cmake
    src/core/formatting/embed_renderer.h
    src/core/formatting/embed_renderer.cpp
```

- [ ] **Step 4: Build**

Run: `cd build && cmake .. && cmake --build .`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt src/core/formatting/embed_renderer.h src/core/formatting/embed_renderer.cpp
git commit -m "feat: add embed HTML renderer"
```

---

### Task 5: Attachment Renderer

**Files:**
- Create: `src/core/formatting/attachment_renderer.h`
- Create: `src/core/formatting/attachment_renderer.cpp`

- [ ] **Step 1: Create attachment_renderer.h**

```cpp
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
```

- [ ] **Step 2: Create attachment_renderer.cpp**

```cpp
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

    // Generic file
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
```

- [ ] **Step 3: Add to CMakeLists.txt**

```cmake
    src/core/formatting/attachment_renderer.h
    src/core/formatting/attachment_renderer.cpp
```

- [ ] **Step 4: Build**

Run: `cd build && cmake .. && cmake --build .`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt src/core/formatting/attachment_renderer.h src/core/formatting/attachment_renderer.cpp
git commit -m "feat: add attachment HTML renderer"
```

---

### Task 6: Wire Formatting into Message Display

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Add imports to main.cpp**

Add after existing imports:

```cpp
#include "core/formatting/discord_markdown.h"
#include "core/formatting/embed_renderer.h"
#include "core/formatting/attachment_renderer.h"
```

- [ ] **Step 2: Replace appendMessage with full formatting**

Replace the `appendMessage` method in MainWindow:

```cpp
private:
    void appendMessage(const Message &msg)
    {
        QString tag = msg.author.globalName.value_or(msg.author.username);
        QString timeStr = msg.timestamp.isValid() ? msg.timestamp.toString("HH:mm") : "";

        // Build message header
        QString header = QString(
            "<div class=\"message-content\" style=\"margin: 4px 0;\">"
            "<span style=\"color: %1; font-weight: 600;\">%2</span>"
            "<span style=\"color: %3; font-size: 12px; margin-left: 6px;\">%4</span>"
            "</div>"
        ).arg(ThemeManager::instance().currentTheme().accentBlue,
              tag,
              ThemeManager::instance().currentTheme().mute,
              timeStr);

        m_messageView->append(header);

        // Render content with markdown
        if (!msg.content.isEmpty()) {
            QString contentHtml = DiscordMarkdown::toHtml(msg.content);
            m_messageView->append(QString("<div class=\"message-content\">%1</div>").arg(contentHtml));
        }

        // Render embeds
        for (const auto &embed : msg.embeds) {
            m_messageView->append(EmbedRenderer::toHtml(embed));
        }

        // Render attachments
        for (const auto &attachment : msg.attachments) {
            m_messageView->append(AttachmentRenderer::toHtml(attachment));
        }
    }
```

- [ ] **Step 3: Add message CSS on startup**

In the MainWindow constructor, after client setup, inject the message CSS:

```cpp
// Inject message CSS into QTextEdit document
m_messageView->document()->setDefaultStyleSheet(ThemeManager::instance().regenerateMessageCSS());
```

- [ ] **Step 4: Build and run**

Run: `cd build && cmake --build .`
Run: `./discord-qt.app/Contents/MacOS/discord-qt`
Expected: Messages render with markdown formatting, embeds, and attachments

- [ ] **Step 5: Commit**

```bash
git add src/main.cpp
git commit -m "feat: wire markdown, embed, and attachment rendering into message display"
```

---

### Task 7: Theme Switcher in Settings Dialog

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Update settings dialog to include theme selector**

In `onSettingsClicked()`, add a theme dropdown before the token edit:

```cpp
auto *themeLabel = new QLabel("Theme:", dialog);
layout->addWidget(themeLabel);

auto *themeCombo = new QComboBox(dialog);
themeCombo->addItems({"Raycast", "Discord"});
themeCombo->setCurrentText(ThemeManager::instance().currentTheme().name);
themeCombo->setStyleSheet(
    "QComboBox { background-color: #1e1f22; color: #dbdee1; border: 1px solid #404249; border-radius: 4px; padding: 8px; }"
    "QComboBox::drop-down { border: none; }"
);
layout->addWidget(themeCombo);
```

Add `#include <QComboBox>` to imports.

- [ ] **Step 2: Apply theme on save**

In the save button's lambda, before the token logic:

```cpp
QString newTheme = themeCombo->currentText();
if (newTheme != ThemeManager::instance().currentTheme().name) {
    ThemeManager::instance().setTheme(newTheme);
    QSettings settings("Umbra", "discord-qt");
    settings.setValue("theme_name", newTheme);
    qApp->setStyleSheet(ThemeManager::instance().regenerateQSS());
}
```

- [ ] **Step 3: Build and test theme switching**

Run: `cd build && cmake --build .`
Run: `./discord-qt.app/Contents/MacOS/discord-qt`
Expected: Settings dialog shows theme dropdown, switching applies new theme immediately

- [ ] **Step 4: Commit**

```bash
git add src/main.cpp
git commit -m "feat: add theme switcher to settings dialog"
```

---

### Task 8: Polish & Edge Cases

**Files:**
- Modify: `src/main.cpp`
- Modify: `src/core/formatting/discord_markdown.cpp`

- [ ] **Step 1: Handle empty messages with embeds/attachments only**

Update `appendMessage` to not show "[embed/attachment]" placeholder when embeds or attachments exist:

```cpp
// Only show placeholder if no content AND no embeds AND no attachments
if (msg.content.isEmpty() && msg.embeds.isEmpty() && msg.attachments.isEmpty()) {
    m_messageView->append(
        QString("<div class=\"message-content\" style=\"color: %1;\">[empty message]</div>")
            .arg(ThemeManager::instance().currentTheme().mute)
    );
}
```

- [ ] **Step 2: Handle spoiler click-to-reveal**

Add click handler for spoilers in QTextEdit. In MainWindow constructor:

```cpp
// Note: QTextEdit doesn't natively support JS click handlers for spoilers.
// For now, spoilers render as dark backgrounds. Full click-to-reveal requires
// a custom widget or JavaScript injection via QTextBrowser.
```

- [ ] **Step 3: Handle edited timestamp display**

In `appendMessage`, after the header:

```cpp
if (msg.editedTimestamp.isValid()) {
    m_messageView->append(
        QString("<span style=\"color: %1; font-size: 11px;\">(edited)</span>")
            .arg(ThemeManager::instance().currentTheme().mute)
    );
}
```

- [ ] **Step 4: Build final version**

Run: `cd build && cmake --build .`
Expected: PASS

- [ ] **Step 5: Final commit**

```bash
git add src/main.cpp src/core/formatting/discord_markdown.cpp
git commit -m "polish: handle edge cases, edited timestamps, empty messages"
```
