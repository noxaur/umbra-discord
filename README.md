# Umbra — Discord Bot Client

A lightweight Discord bot client built with **Qt6** and **C++**, featuring a complete REST API client, Gateway WebSocket connection, in-memory cache, and a minimal test UI.

## Features

- **REST API** — Full v10 endpoint coverage: user, guilds, channels, messages, send message
- **Gateway WebSocket** — Connect, identify, heartbeat, resume, event dispatch (MESSAGE_CREATE, MESSAGE_UPDATE, MESSAGE_DELETE, TYPING_START)
- **Rate Limiter** — Bucket-based rate limiting with global limit support and pre-request checks
- **In-Memory Cache** — Signal-based store for guilds, channels, and messages
- **Exponential Backoff** — Automatic reconnection with jitter (1s → 60s)
- **SSL/TLS** — Secure WebSocket connections with OpenSSL backend
- **Minimal UI** — Server list, channel list, message view, and send input for testing

## Project Structure

```
├── CMakeLists.txt
├── Info.plist.in
├── src/
│   ├── main.cpp                          # Minimal QMainWindow test UI
│   └── core/
│       ├── types/
│       │   ├── snowflake.h               # Discord snowflake ID wrapper
│       │   ├── discord_types.h           # User, Guild, Channel, Message, Embed, Attachment
│       │   └── json_parser.h/cpp         # Shared JSON parsing (no duplication)
│       ├── api/
│       │   ├── api_routes.h              # REST endpoint URL builders
│       │   ├── rate_limiter.h/cpp        # Bucket-based rate limiter
│       │   └── rest_client.h/cpp         # QNetworkAccessManager wrapper
│       ├── gateway/
│       │   ├── opcodes.h                 # Gateway opcodes + payload builders
│       │   ├── intents.h                 # Gateway intent flags
│       │   ├── session.h                 # Session state (id, seq, resume URL)
│       │   └── gateway_client.h/cpp      # QWebSocket client with state machine
│       ├── cache.h/cpp                   # In-memory QHash store with signals
│       └── client.h/cpp                  # Orchestrator: wires REST + Gateway + Cache
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

Qt6 includes its own OpenSSL backend on macOS. Verify with:

```bash
otool -L /opt/homebrew/opt/qt/lib/QtNetwork.framework/Versions/A/QtNetwork | grep ssl
```

### Linux (Ubuntu/Debian)

```bash
sudo apt install qt6-base-dev qt6-websockets-dev libssl-dev cmake
```

### Windows

Install Qt6 via the [official installer](https://www.qt.io/download-qt-installer) with the **Qt WebSockets** and **OpenSSL** components selected.

## Building

```bash
# Clone
git clone https://github.com/YOUR_USERNAME/umbra-discord.git
cd umbra-discord

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build
```

### macOS App Bundle

The build produces a `.app` bundle. To deploy on another Mac:

```bash
# Bundle Qt frameworks into the .app
macdeployqt build/discord-qt.app

# Codesign (required for macOS to run)
codesign --force --deep --sign - build/discord-qt.app
```

## Running

### With a Bot Token

```bash
./build/discord-qt.app/Contents/MacOS/discord-qt
```

The UI will prompt for your bot token. Enter it and click **Login**.

### Debug Logging

Enable verbose logging by setting the `QT_LOGGING_RULES` environment variable:

```bash
# Show all Discord-related logs
QT_LOGGING_RULES="discord.*=true" ./build/discord-qt.app/Contents/MacOS/discord-qt

# Show only REST logs
QT_LOGGING_RULES="discord.rest=true" ./build/discord-qt.app/Contents/MacOS/discord-qt

# Show only Gateway logs
QT_LOGGING_RULES="discord.gateway=true" ./build/discord-qt.app/Contents/MacOS/discord-qt

# Show everything with debug level
QT_LOGGING_RULES="*.debug=true" ./build/discord-qt.app/Contents/MacOS/discord-qt
```

## Usage

1. **Login** — Enter your bot token (from [Discord Developer Portal](https://discord.com/developers/applications)) and click Login
2. **View Servers** — After connecting, your guilds appear in the left panel
3. **View Channels** — Click a server to load its channels
4. **View Messages** — Click a text channel to load recent messages
5. **Send Messages** — Type in the input field at the bottom and press Enter

## Architecture

```
┌─────────────────────────────────────────────────┐
│                   Client                         │
│  (orchestrates REST + Gateway + Cache)           │
├──────────────┬──────────────────┬────────────────┤
│  RestClient  │  GatewayClient   │    Cache       │
│              │                  │                │
│ • GET/POST   │ • WebSocket      │ • QHash store  │
│ • Rate limit │ • Heartbeat      │ • Signals      │
│ • Parse JSON │ • Identify/Resume│ • Guilds       │
│              │ • Event dispatch │ • Channels     │
│              │                  │ • Messages     │
└──────────────┴──────────────────┴────────────────┘
         │               │               │
         ▼               ▼               ▼
   QNetworkAccess   QWebSocket      QHash<Snowflake, T>
     Manager
```

### Key Design Decisions

- **Snowflakes as QString** — Avoids 64-bit integer overflow on all platforms
- **Fresh QWebSocket per connection** — Prevents stale state from previous connections
- **User-Agent header required** — Discord's Cloudflare proxy rejects requests without it
- **Default intents exclude privileged** — `Guilds | GuildMessages | DirectMessages` (4609); enable `MessageContent` in Developer Portal if needed
- **Shared JSON parser** — Single `JsonParser` class used by both REST and Gateway to avoid duplication

## Getting a Bot Token

1. Go to [Discord Developer Portal](https://discord.com/developers/applications)
2. Create a new application
3. Go to **Bot** → **Reset Token** (or **Copy Token**)
4. Enable **Presence Intent** and **Server Members Intent** if needed
5. Invite the bot to a server using the OAuth2 URL generator (Bot scope + required permissions)

## License

MIT
