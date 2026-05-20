cmake_minimum_required(VERSION 3.16)
project(discord-qt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Widgets)

# Fetch nlohmann/json
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

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
    src/core/formatting/discord_markdown.h
    src/core/formatting/discord_markdown.cpp
    src/core/formatting/embed_renderer.h
    src/core/formatting/embed_renderer.cpp
)

set_target_properties(discord-qt PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.discordqt.app"
    MACOSX_BUNDLE_BUNDLE_NAME "DiscordQt"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)

target_include_directories(discord-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(discord-qt PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Widgets
    nlohmann_json::nlohmann_json
)
