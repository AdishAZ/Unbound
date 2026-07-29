cmake_minimum_required(VERSION 3.20)
project(unboundmp_protocol CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Protobuf CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
set(libpqxx_USE_STATIC_LIBS ON)
find_package(libpqxx CONFIG REQUIRED)
find_package(unofficial-argon2 CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../proto/packets.proto)

protobuf_generate(
    LANGUAGE cpp
    PROTOS ${PROTO_FILE}
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../proto
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
    OUT_VAR PROTO_SRCS
)
set(PROTO_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/packets.pb.h
)

# ---------------------------------------------------------------------------
# protocol - wire schema, framing, dispatch. No emulator/network deps.
# ---------------------------------------------------------------------------


add_library(unboundmp_protocol STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    src/protocol/serialization.cpp
    src/protocol/packet_dispatcher.cpp
)

target_include_directories(unboundmp_protocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(unboundmp_protocol
    PUBLIC
        protobuf::libprotobuf
)

target_compile_features(unboundmp_protocol PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# emulator - IEmulatorCore interface + backend-agnostic helpers
# (rom_loader, save_loader). The real libmgba-backed core is opt-in via
# UNBOUNDMP_WITH_MGBA so the rest of the client builds/tests on machines
# without libmgba installed.
# ---------------------------------------------------------------------------
add_library(unboundmp_emulator STATIC
    src/emulator/rom_loader.cpp
    src/emulator/save_loader.cpp
)
target_include_directories(unboundmp_emulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(unboundmp_emulator PUBLIC cxx_std_20)

option(UNBOUNDMP_WITH_MGBA "Build the real libmgba-backed emulator core (requires libmgba-dev)" OFF)
if(UNBOUNDMP_WITH_MGBA)

    target_sources(unboundmp_emulator PRIVATE
        src/emulator/mgba_emulator_core.cpp
        src/emulator/game_bootstrap.cpp
    )

    target_include_directories(unboundmp_emulator PRIVATE
        ${CMAKE_SOURCE_DIR}/../third_party/mgba/include
        ${CMAKE_SOURCE_DIR}/../third_party/mgba/build/include
    )

    target_link_libraries(unboundmp_emulator PRIVATE
        ${CMAKE_SOURCE_DIR}/../third_party/mgba/build/Release/mgba.lib
        SDL2::SDL2main SDL2::SDL2
    )
    if(WIN32)
        target_link_libraries(unboundmp_emulator PRIVATE Shlwapi)
    endif()

endif()

# ---------------------------------------------------------------------------
# memory - AddressTable + game-state readers, on top of IEmulatorCore.
# ---------------------------------------------------------------------------
add_library(unboundmp_memory STATIC
    src/memory/address_table.cpp
)
target_include_directories(unboundmp_memory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_memory PUBLIC unboundmp_emulator)
target_compile_features(unboundmp_memory PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# game_state - Parsers and core GameState
# ---------------------------------------------------------------------------
add_library(unboundmp_game_state STATIC
    src/parser/domain_types.cpp
    src/parser/position_parser.cpp
    src/parser/map_parser.cpp
    src/parser/direction_parser.cpp
    src/parser/movement_parser.cpp
    src/parser/local_player_parser.cpp
    src/parser/party_parser.cpp
    src/parser/follower_parser.cpp
    src/parser/inventory_parser.cpp
    src/parser/event_object_parser.cpp
    src/parser/parser_registry.cpp
    src/game_state/game_state.cpp
)
target_include_directories(unboundmp_game_state PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_game_state PUBLIC unboundmp_memory)
target_compile_features(unboundmp_game_state PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# network - Standalone Asio-based async networking, packets, connections.
# ---------------------------------------------------------------------------
add_library(unboundmp_network STATIC
    src/network/serializer.cpp
    src/network/packet.cpp
    src/network/packet_dispatcher.cpp
    src/network/client_packet_dispatcher.cpp
    src/network/connection.cpp
    src/network/multiplayer_client.cpp
    src/network/multiplayer_server.cpp
    src/network/network_manager.cpp
    src/network/client_session_manager.cpp
    src/network/network_clock.cpp
)
target_include_directories(unboundmp_network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_network
    PUBLIC
        unboundmp_protocol
        Threads::Threads
        asio::asio
)
target_compile_definitions(unboundmp_network PUBLIC ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(unboundmp_network PUBLIC _WIN32_WINNT=0x0A00)
    target_link_libraries(unboundmp_network PUBLIC ws2_32)
endif()
target_compile_features(unboundmp_network PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# gameplay - Milestone 9: follower Pokemon (species sync, movement trail /
# offset, direction, walk animation). Depends only on protocol (packet
# types) and memory (FollowerState/FacingDirection types) - no networking
# or emulator access of its own, and no rendering.
# ---------------------------------------------------------------------------
add_library(unboundmp_gameplay STATIC
    src/gameplay/direction.cpp
    src/gameplay/follower_manager.cpp
    src/gameplay/entity_manager.cpp
    src/gameplay/visibility_manager.cpp
    src/gameplay/world_sync.cpp
    src/gameplay/movement_sync.cpp
    src/gameplay/position_interpolator.cpp
    src/gameplay/remote_player_manager.cpp
    src/gameplay/sync_scheduler.cpp
    src/gameplay/gameplay_sync_manager.cpp
    src/gameplay/world_manager.cpp
    src/gameplay/inventory_manager.cpp
)
target_include_directories(unboundmp_gameplay PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/server)
target_link_libraries(unboundmp_gameplay
    PUBLIC
        unboundmp_protocol
        unboundmp_network
        unboundmp_core
        unboundmp_game_state
)

# Temporary tool to find sprites in ROM
add_executable(find_sprites "C:/Users/sj782/.gemini/antigravity/brain/d406f0d7-7e3a-4c22-a5b0-057fbac3ff83/scratch/find_sprites.cpp")
target_compile_features(unboundmp_gameplay PUBLIC cxx_std_20)



# ---------------------------------------------------------------------------
# render - Milestone 10: 16:9 widescreen foundation (dynamic viewport,
# camera expansion, overlay-position math, aspect ratio classification,
# resolution scaling). Pure geometry - no SDL2/OpenGL, no window, no draw
# calls. Depends on nothing but the standard library, so it builds/tests on
# any machine exactly like the memory/gameplay layers do without libmgba.
# ---------------------------------------------------------------------------
add_library(unboundmp_render STATIC
    src/render/aspect_ratio.cpp
    src/render/resolution_scaling.cpp
    src/render/viewport.cpp
    src/render/camera.cpp
    src/render/camera_controller.cpp
    src/render/overlay_layout.cpp
    src/render/render_manager.cpp
    src/render/world_renderer.cpp
    src/render/map_renderer.cpp
    src/render/sdl_renderer.cpp
    src/render/remote_player_renderer.cpp
    src/render/render_queue.cpp
    src/render/render_layer.cpp
    src/render/particle_system.cpp
    src/render/weather_renderer.cpp
    src/render/lighting_manager.cpp
    src/render/animation_manager.cpp
)
target_include_directories(unboundmp_render
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(unboundmp_render
    PUBLIC
        SDL2::SDL2
)

target_compile_features(unboundmp_render PUBLIC cxx_std_20)


# ---------------------------------------------------------------------------
# interaction - Milestone 11: interaction detection (facing, distance,
# candidate tracking, target selection). Depends on protocol
# (PlayerStateUpdate) and gameplay (StepFor/direction helpers, reused
# rather than duplicated) - no networking or emulator access of its own,
# and no rendering.
# ---------------------------------------------------------------------------
add_library(unboundmp_interaction STATIC
    src/interaction/distance.cpp
    src/interaction/facing.cpp
    src/interaction/interaction_manager.cpp
    src/interaction/player_selector.cpp
)
target_include_directories(unboundmp_interaction PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_interaction
    PUBLIC
        unboundmp_protocol
        unboundmp_gameplay
)
target_compile_features(unboundmp_interaction PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# save - Milestone 12: save synchronization (save manager, conflict
# detection, trade/battle persistence checkpoints, automatic rotating
# backups). Depends only on emulator (reuses SaveLoader's path/validation
# conventions) - no networking or memory access of its own. Like save_loader
# before it, this never reads ROM data and treats save file bytes as an
# opaque blob; unlike the pure-math gameplay/render/interaction layers, it
# does real file I/O (backups are real files on disk), so its example is a
# real-filesystem integration smoke test rather than a pure-logic one.
# ---------------------------------------------------------------------------
add_library(unboundmp_save STATIC
    src/save/hashing.cpp
    src/save/conflict_detector.cpp
    src/save/backup_manager.cpp
    src/save/save_manager.cpp
)
target_include_directories(unboundmp_save PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_save PUBLIC unboundmp_emulator)
target_compile_features(unboundmp_save PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# input - Android-support milestone: normalized-coordinate touch-to-GBA-
# button mapping (multi-finger tracking, drag-between-regions). Depends
# only on emulator (reuses GbaButton/InputState) - platform-agnostic on
# purpose so it's testable on desktop; the Android JNI bridge
# (android/app/src/main/cpp) is what feeds it real touch events on-device.
# ---------------------------------------------------------------------------
add_library(unboundmp_input STATIC
    src/input/touch_input_mapper.cpp
)
target_include_directories(unboundmp_input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_input PUBLIC unboundmp_emulator)
target_compile_features(unboundmp_input PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# platform - Android-support milestone: asset path resolution (where ROM/
# save/backup/config files live on a given platform). Pure string logic,
# no filesystem calls, no dependency on anything else in this project -
# same "compute the paths, let another layer touch the disk" split as
# save/backup_manager.h and render/viewport.h.
# ---------------------------------------------------------------------------
add_library(unboundmp_platform STATIC
    src/platform/asset_path_resolver.cpp
)
target_include_directories(unboundmp_platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(unboundmp_platform PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# core - Stage 9: infrastructure utilities (JSON parser, logging, config,
# task scheduling, crash reporting). No SDL2, no networking, pure C++20.
# ---------------------------------------------------------------------------
add_library(unboundmp_core STATIC
    src/core/json.cpp
    src/core/log_manager.cpp
    src/core/client_config.cpp
    src/core/task_scheduler.cpp
    src/core/crash_reporter.cpp
    src/core/event_system.cpp
    src/core/game_context.cpp
)
target_include_directories(unboundmp_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_core PUBLIC Threads::Threads SDL2::SDL2 unboundmp_game_state)
if(WIN32)
    target_link_libraries(unboundmp_core PUBLIC dbghelp Psapi)
endif()
target_compile_features(unboundmp_core PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# ui - Stage 9: UI engine, widget system, screen management, HUD, profiler,
# notifications, dev overlay, theme, localization, animation, asset
# management, and Stage 10 preparation components. Depends on SDL2 for
# rendering and core for config/logging.
# ---------------------------------------------------------------------------
add_library(unboundmp_ui STATIC
    src/ui/ui_types.cpp
    src/ui/widget.cpp
    src/ui/widgets.cpp
    src/ui/item_widget.cpp
    src/ui/screen.cpp
    src/ui/screen_manager.cpp
    src/ui/theme.cpp
    src/ui/localization.cpp
    src/ui/animation.cpp
    src/ui/ui_engine.cpp
    src/ui/window_manager.cpp
      src/ui/font_manager.cpp
      src/ui/text_renderer.cpp
    src/ui/asset_manager.cpp
    src/ui/profiler.cpp
    src/ui/hud_manager.cpp
    src/ui/hud_widgets.cpp
    src/ui/notification_center.cpp
    src/ui/dev_overlay.cpp
    src/ui/screens/login_screen.cpp
    src/ui/screens/character_select_screen.cpp
    src/ui/screens/character_creation_screen.cpp
    src/ui/screens/create_account_screen.cpp
    src/ui/screens/loading_screen.cpp
    src/ui/screens/settings_screen.cpp
    src/ui/screens/game_screen.cpp
    src/ui/components/avatar_renderer.cpp
    src/ui/components/player_card.cpp
    src/ui/components/context_menu.cpp
    src/ui/inventory_screen.cpp
    src/ui/components/popup_menu.cpp
)
target_include_directories(unboundmp_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_ui
    PUBLIC
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
        unboundmp_core
)
if(WIN32)
    target_link_libraries(unboundmp_ui PUBLIC Psapi)
endif()
target_compile_features(unboundmp_ui PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# audio_mgr - Stage 9: audio volume scaling, procedural notification sounds.
# Depends on SDL2 for audio device access.
# ---------------------------------------------------------------------------
add_library(unboundmp_audio_mgr STATIC
    src/audio/audio_manager.cpp
)
target_include_directories(unboundmp_audio_mgr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unboundmp_audio_mgr PUBLIC SDL2::SDL2)
target_compile_features(unboundmp_audio_mgr PUBLIC cxx_std_20)

# Expand input with the input manager (rebinding, gamepad, hotkeys)
target_sources(unboundmp_input PRIVATE
    src/input/input_manager.cpp
    src/input/menu_input_interceptor.cpp
)
target_link_libraries(unboundmp_input PUBLIC SDL2::SDL2)

# ---------------------------------------------------------------------------
# Examples - standalone smoke tests, not a real client. Each proves one
# layer builds/links/behaves correctly in isolation.
# ---------------------------------------------------------------------------
if(UNBOUNDMP_WITH_MGBA)
    add_executable(unboundmp_client
        client/main.cpp
        src/persistence/client_save_manager.cpp
    )

    target_link_libraries(unboundmp_client
        PRIVATE
            SDL2::SDL2
            SDL2_ttf::SDL2_ttf
            unboundmp_platform
            unboundmp_emulator
            unboundmp_memory
            unboundmp_protocol
            unboundmp_network
            unboundmp_gameplay
            unboundmp_render
            unboundmp_interaction
            unboundmp_save
            unboundmp_input
            unboundmp_ui
            unboundmp_core
            unboundmp_audio_mgr
    )

    target_include_directories(unboundmp_client PRIVATE
        ${CMAKE_SOURCE_DIR}/../third_party/mgba/include
        ${CMAKE_SOURCE_DIR}/../third_party/mgba/build/include
    )
    target_link_libraries(unboundmp_client PRIVATE
        ${CMAKE_SOURCE_DIR}/../third_party/mgba/build/Release/mgba.lib
    )
endif()
option(UNBOUNDMP_BUILD_EXAMPLES "Build example/smoke-test binaries" ON)
if(UNBOUNDMP_BUILD_EXAMPLES)
    add_executable(protocol_roundtrip_example examples/protocol_roundtrip_example.cpp)
    target_link_libraries(protocol_roundtrip_example PRIVATE unboundmp_protocol)

    add_executable(memory_probe_example examples/memory_probe_example.cpp)
    target_link_libraries(memory_probe_example PRIVATE unboundmp_memory)


    add_executable(asio_network_example examples/asio_network_example.cpp)
    target_link_libraries(asio_network_example PRIVATE unboundmp_network)

    add_library(unboundmp_persistence STATIC
        src/persistence/character_service.cpp
        src/persistence/inventory_service.cpp
        src/persistence/pokemon_blob_service.cpp
        src/persistence/story_service.cpp
        src/persistence/player_persistence.cpp
        src/persistence/autosave_manager.cpp
    )
    target_include_directories(unboundmp_persistence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/server)
    target_link_libraries(unboundmp_persistence PUBLIC unboundmp_protocol unboundmp_network unboundmp_memory libpqxx::pqxx)
    target_compile_features(unboundmp_persistence PUBLIC cxx_std_20)

    add_executable(unboundmp_server
        server/main.cpp
        server/server_context.cpp
        server/config/server_config.cpp
        server/database/database.cpp
        server/authentication/authentication.cpp
        server/accounts/account_manager.cpp
        server/database/character_repository.cpp
        server/sessions/session_manager.cpp
        server/network/player_registry.cpp
        server/world/world_entity.cpp
        server/world/world_map.cpp
        server/world/map_manager.cpp
        server/world/player_manager.cpp
        server/world/server_visibility_manager.cpp
        server/world/world_server.cpp
        server/database/inventory_repository.cpp
        server/gameplay/inventory_manager.cpp
        server/gameplay/item_manager.cpp
    )
    target_include_directories(unboundmp_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/server ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(unboundmp_server
        PRIVATE
            libpqxx::pqxx
            unboundmp_network
            unboundmp_protocol
            unboundmp_persistence
            unofficial::argon2::libargon2
            Threads::Threads
    )
    if(MSVC)
        set_target_properties(unboundmp_server PROPERTIES LINK_FLAGS "/FORCE:MULTIPLE")
    endif()
    if(MSVC)
        
    endif()





    add_executable(follower_manager_example examples/follower_manager_example.cpp)
    target_link_libraries(follower_manager_example PRIVATE unboundmp_gameplay)

    add_executable(widescreen_example examples/widescreen_example.cpp)
    target_link_libraries(widescreen_example PRIVATE unboundmp_render)

    add_executable(interaction_example examples/interaction_example.cpp)
    target_link_libraries(interaction_example PRIVATE unboundmp_interaction)

    add_executable(save_manager_example examples/save_manager_example.cpp)
    target_link_libraries(save_manager_example PRIVATE unboundmp_save)

    add_executable(touch_input_example examples/touch_input_example.cpp)
    target_link_libraries(touch_input_example PRIVATE unboundmp_input)

    add_executable(asset_path_example examples/asset_path_example.cpp)
    target_link_libraries(asset_path_example PRIVATE unboundmp_platform)


endif()

add_executable(character_system_test examples/character_system_test.cpp)
target_link_libraries(character_system_test PRIVATE
    unboundmp_core
    unboundmp_network
    )

add_executable(pqxx_test examples/pqxx_test.cpp)
target_link_libraries(pqxx_test PRIVATE libpqxx::pqxx unboundmp_core)
add_executable(libpq_test examples/libpq_test.cpp)
target_include_directories(libpq_test PRIVATE /x64-windows/include)
find_package(PostgreSQL REQUIRED)
target_link_libraries(libpq_test PRIVATE PostgreSQL::PostgreSQL unboundmp_core)
