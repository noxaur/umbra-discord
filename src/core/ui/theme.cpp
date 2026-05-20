#include "core/ui/theme.h"

#include <QDebug>

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
        QListWidget#serverList::item:focus {
            outline: 2px solid %14;
            outline-offset: -2px;
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
        QTreeWidget#channelTree::item:focus {
            outline: 2px solid %14;
            outline-offset: -2px;
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
        QPushButton:focus {
            outline: 2px solid %14;
            outline-offset: 2px;
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
        QToolButton#settingsBtn:focus {
            outline: 2px solid %14;
            outline-offset: 2px;
        }

        /* Menu bar */
        QMenuBar {
            background-color: %10;
            color: %2;
            border: none;
        }
        QMenuBar::item {
            padding: 4px 8px;
            background: transparent;
            border-radius: %11px;
        }
        QMenuBar::item:selected {
            background-color: %9;
        }

        /* Separator line */
        QFrame#separatorLine {
            background-color: %18;
            max-height: 1px;
        }

        /* Group box */
        QGroupBox {
            border: 1px solid %18;
            border-radius: %5px;
            margin-top: 12px;
            padding: 12px 12px 12px 12px;
            font-weight: 600;
            color: %8;
            font-size: 13px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
        }

        /* General input styling (applies to all QLineEdit unless overridden) */
        QLineEdit {
            background-color: %12;
            color: %2;
            border: 1px solid %18;
            border-radius: %11px;
            padding: 8px 12px;
            font-size: 14px;
        }
        QLineEdit:focus {
            outline: 2px solid %14;
            outline-offset: -2px;
            border-color: %14;
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
            primary,             // button hover
            primaryPressed,      // button pressed
            hairline             // %18
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
            background-color: %2;
            border: 1px solid %7;
            border-radius: %4px;
            margin: 4px 0;
            padding: 8px 12px;
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
            border: 1px solid %7;
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
            border-radius: 50%;
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
            flex: 0 0 calc(33% - 8px);
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
            border-radius: 50%;
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

        /* Loading skeleton */
        .loading-skeleton {
            padding: 16px 0;
        }
        .skeleton-row {
            height: 14px;
            background-color: %2;
            border-radius: %4px;
            margin: 8px 0;
            animation: pulse 1.5s ease-in-out infinite;
        }
        .skeleton-row.short {
            width: 60%;
        }
        @keyframes pulse {
            0%, 100% { opacity: 0.4; }
            50% { opacity: 0.8; }
        }

        /* Empty state */
        .empty-state {
            color: %8;
            text-align: center;
            padding: 48px 24px;
        }
        .empty-state-title {
            color: %1;
            font-weight: 500;
            font-size: 16px;
            margin-bottom: 8px;
        }
        .empty-state-body {
            font-size: 14px;
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
    auto it = m_themes.find(name);
    if (it != m_themes.end()) {
        m_current = it.value();
        emit themeChanged(m_current);
    } else {
        qWarning() << "ThemeManager: unknown theme" << name;
    }
}

void ThemeManager::setTheme(const Theme &theme)
{
    m_current = theme;
    emit themeChanged(m_current);
}

QString ThemeManager::regenerateQSS() const
{
    return m_current.generateQSS();
}

QString ThemeManager::regenerateMessageCSS() const
{
    return m_current.generateMessageCSS();
}
