# Umbra — Discord Client

A lightweight Discord client built with **Qt6** and **C++**, featuring a complete REST API client, Gateway WebSocket connection, in-memory cache, Raycast-inspired theming system, and full Discord message formatting.

## Features

### Core
- **REST API** — Full v10 endpoint coverage: user, guilds, channels, messages
- **Gateway WebSocket** — Connect, identify, heartbeat, resume, event dispatch
- **Rate Limiter** — Bucket-based rate limiting with global limit support
- **In-Memory Cache** — Signal-based store for guilds, channels, and messages
- **Exponential Backoff** — Automatic reconnection with jitter (1s → 60s)
- **SSL/TLS** — Secure WebSocket connections with OpenSSL backend

### UI & Theming
- **Theme System** — Token-based theming with dynamic QSS generation
- **Raycast Theme** — Near-black canvas, hairline borders, surface ladder elevation (default)
- **Discord Theme** — Classic Discord dark palette
- **Theme Switcher** — Change themes via settings dialog, persisted across sessions
- **Token Persistence** — Bot token saved securely, auto-connects on launch

### Message Formatting
- **Discord Markdown** — Full markdown support: `**bold**`, `*italic*`, `__underline__`, `~~strikethrough~~`, `` `code` ``, ```code blocks```, `[links](url)`, `> blockquotes`, lists, `||spoilers||`
- **Embeds** — Colored border, title, description, thumbnail, fields (inline/non-inline), footer
- **Attachments** — Images, videos, and file downloads with size formatting
- **Mentions** — User (`<@id>`) and channel (`<#id>`) mentions with styled badges
- **Timestamps** — All Discord timestamp styles (`<t:timestamp:style>`) including relative time
- **Auto-links** — Bare URLs automatically linked
- **XSS Protection** — HTML escaping, URL scheme validation, username sanitization

## Project Structure

```
├── CMakeLists.txt
├── DESIGN.md                           # Raycast design system tokens
├── README.md
├── src/
│   ├── main.cpp                        # MainWindow, theme integration, message display
│   └── core/
│       ├── types/
│       │   ├── snowflake.h             # Discord snowflake ID wrapper
│       │   ├── discord_types.h         # User, Guild, Channel, Message, Embed, Attachment
│       │   └── json_parser.h/cpp       # Shared JSON parsing
│       ├── api/
│       │   ├── api_routes.h            # REST endpoint URL builders
│       │   ├── rate_limiter.h/cpp      # Bucket-based rate limiter
│       │   └── rest_client.h/cpp       # QNetworkAccessManager wrapper
│       ├── gateway/
│       │   ├── opcodes.h               # Gateway opcodes + payload builders
│       │   ├── intents.h               # Gateway intent flags
│       │   ├── session.h               # Session state (id, seq, resume URL)
│       │   └── gateway_client.h/cpp    # QWebSocket client with state machine
│       ├── cache.h/cpp                 # In-memory QHash store with signals
│       ├── client.h/cpp                # Orchestrator: wires REST + Gateway + Cache
│       ├── ui/
│       │   └── theme.h/cpp             # Theme struct, presets, QSS generator, ThemeManager
│       └── formatting/
│           ├── discord_markdown.h/cpp  # Discord markdown → HTML converter
│           ├── embed_renderer.h/cpp    # Embed → HTML block renderer
│           └── attachment_renderer.h/cpp # Attachment → HTML block renderer
└── build/
```

## Dependencies

| Dependency | Minimum Version | Purpose |
|---|---|---|
| Qt6 | 6.5+ | Core, Network, WebSockets, Widgets |
| CMake | 3.19+ | Build system |
| nlohmann/json | 3.11+ | JSON parsing (auto-fetched via FetchContent) |
| OpenSSL | 1.1+ | SSL/TLS (required for Gateway WebSocket) |

### macOS

```bash
brew install qt cmake
```

### Linux (Ubuntu/Debian)

```bash
sudo apt install qt6-base-dev qt6-websockets-dev libssl-dev cmake
```

### Windows

Install Qt6 via the [official installer](https://www.qt.io/download-qt-installer) with **Qt WebSockets** and **OpenSSL** components.

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### macOS App Bundle

```bash
macdeployqt build/discord-qt.app
codesign --force --deep --sign - build/discord-qt.app
```

## Running

```bash
./build/discord-qt.app/Contents/MacOS/discord-qt
```

### Debug Logging

```bash
QT_LOGGING_RULES="discord.*=true" ./build/discord-qt.app/Contents/MacOS/discord-qt
```

## Usage

1. **Launch** — App opens with Raycast theme, auto-connects if token was saved
2. **Login** — Enter bot token on first launch, or change via settings (⚙ button)
3. **View Servers** — Guilds appear in the left panel
4. **View Channels** — Click a server to load its channels (grouped by category)
5. **View Messages** — Click a text channel to load recent messages with full formatting
6. **Send Messages** — Type in the input field and press Enter
7. **Change Theme** — Click ⚙ → select Raycast or Discord → Save & Reconnect

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Client                                │
│         (orchestrates REST + Gateway + Cache)                │
├──────────────┬──────────────────┬────────────────────────────┤
│  RestClient  │  GatewayClient   │         Cache              │
│              │                  │                            │
│ • GET/POST   │ • WebSocket      │ • QHash store              │
│ • Rate limit │ • Heartbeat      │ • Signals                  │
│ • Parse JSON │ • Identify/Resume│ • Guilds, Channels, Messages│
│              │ • Event dispatch │                            │
└──────────────┴──────────────────┴────────────────────────────┘
         │               │               │
         ▼               ▼               ▼
   QNetworkAccess   QWebSocket      QHash<Snowflake, T>
     Manager

┌─────────────────────────────────────────────────────────────┐
│                    Message Pipeline                          │
│                                                              │
│  Message.content ──→ DiscordMarkdown::toHtml() ──→ HTML     │
│  Message.embeds  ──→ EmbedRenderer::toHtml()     ──→ HTML   │
│  Message.attachments → AttachmentRenderer::toHtml() → HTML  │
│                                                              │
│  All HTML ──→ QTextEdit (styled by Theme::generateMessageCSS)│
└─────────────────────────────────────────────────────────────┘
```

### Key Design Decisions

- **Token-based theming** — All colors flow through `Theme` struct, QSS generated dynamically
- **Raycast design system** — Default theme uses DESIGN.md tokens (near-black canvas, hairline borders)
- **Markdown processing order** — Code blocks first, then block-level, then inline (prevents formatting inside code)
- **XSS defense** — HTML escape before markdown processing, URL scheme validation, username sanitization
- **Fresh QWebSocket per connection** — Prevents stale state
- **Shared JSON parser** — Single `JsonParser` class for REST and Gateway

## Getting a Bot Token

1. Go to [Discord Developer Portal](https://discord.com/developers/applications)
2. Create a new application
3. Go to **Bot** → **Reset Token** (or **Copy Token**)
4. Enable **Message Content Intent** in Developer Portal
5. Invite the bot to a server using the OAuth2 URL generator

## License

MIT
