cmake_minimum_required(VERSION 3.22)
project(unboundmp_android_native CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Pull in the SAME cpp/ tree the desktop build uses - not a fork or copy of
# it. This is the whole point of the Android milestone: the game-logic
# layers (input, save, and eventually gameplay/render/interaction) are
# written once and reused, only the platform glue below is Android-
# specific.
# ---------------------------------------------------------------------------
set(UNBOUNDMP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../cpp)

# The protocol/network libraries pull in Protobuf via find_package(), which
# is not guaranteed to resolve for an NDK cross-compile out of the box (it
# needs an Android-targeted protobuf build, e.g. from vcpkg's
# arm64-android/armeabi-v7a-android triplets, made visible via
# CMAKE_FIND_ROOT_PATH or Protobuf_DIR). Rather than have the whole
# configure step fail on a stock NDK toolchain with no such build present,
# networking is opt-in here exactly like UNBOUNDMP_WITH_MGBA is opt-in on
# desktop - flip this on once an Android protobuf build is wired into the
# project's toolchain file. See android/README.md.
option(UNBOUNDMP_ANDROID_WITH_NETWORK "Build unboundmp_protocol/unboundmp_network for Android (requires an Android-targeted Protobuf build)" OFF)

set(UNBOUNDMP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(UNBOUNDMP_WITH_MGBA OFF CACHE BOOL "" FORCE)

if(NOT UNBOUNDMP_ANDROID_WITH_NETWORK)
    # Trim the top-level CMakeLists.txt's protocol/network targets out of
    # this build by building only the specific subset of libraries the
    # bridge below actually needs, rather than add_subdirectory()'ing the
    # whole tree (which would hard-fail at find_package(Protobuf REQUIRED)
    # before this option is even read). Each add_library here mirrors its
    # desktop counterpart in cpp/CMakeLists.txt exactly - same sources, same
    # flags - it is not a reimplementation.
    add_library(unboundmp_emulator STATIC
        ${UNBOUNDMP_ROOT}/src/emulator/rom_loader.cpp
        ${UNBOUNDMP_ROOT}/src/emulator/save_loader.cpp
    )
    target_include_directories(unboundmp_emulator PUBLIC ${UNBOUNDMP_ROOT}/include)
    target_compile_features(unboundmp_emulator PUBLIC cxx_std_20)

    add_library(unboundmp_save STATIC
        ${UNBOUNDMP_ROOT}/src/save/hashing.cpp
        ${UNBOUNDMP_ROOT}/src/save/conflict_detector.cpp
        ${UNBOUNDMP_ROOT}/src/save/backup_manager.cpp
        ${UNBOUNDMP_ROOT}/src/save/save_manager.cpp
    )
    target_include_directories(unboundmp_save PUBLIC ${UNBOUNDMP_ROOT}/include)
    target_link_libraries(unboundmp_save PUBLIC unboundmp_emulator)
    target_compile_features(unboundmp_save PUBLIC cxx_std_20)

    add_library(unboundmp_input STATIC
        ${UNBOUNDMP_ROOT}/src/input/touch_input_mapper.cpp
    )
    target_include_directories(unboundmp_input PUBLIC ${UNBOUNDMP_ROOT}/include)
    target_link_libraries(unboundmp_input PUBLIC unboundmp_emulator)
    target_compile_features(unboundmp_input PUBLIC cxx_std_20)

    add_library(unboundmp_platform STATIC
        ${UNBOUNDMP_ROOT}/src/platform/asset_path_resolver.cpp
    )
    target_include_directories(unboundmp_platform PUBLIC ${UNBOUNDMP_ROOT}/include)
    target_compile_features(unboundmp_platform PUBLIC cxx_std_20)
else()
    # Opt-in path: build the real top-level tree, protocol/network included.
    add_subdirectory(${UNBOUNDMP_ROOT} unboundmp_desktop_build)
endif()

# ---------------------------------------------------------------------------
# jni_bridge - the only Android-specific C++ in this project. Thin on
# purpose: it translates JNI calls into the same C++ APIs the desktop
# client and examples already exercise, and holds no game logic itself.
# ---------------------------------------------------------------------------
add_library(unboundmp_jni SHARED
    jni_bridge.cpp
)
target_include_directories(unboundmp_jni PRIVATE ${UNBOUNDMP_ROOT}/include)
target_link_libraries(unboundmp_jni PRIVATE
    unboundmp_save
    unboundmp_input
    unboundmp_platform
    log
)
if(UNBOUNDMP_ANDROID_WITH_NETWORK)
    target_link_libraries(unboundmp_jni PRIVATE unboundmp_network)
    target_compile_definitions(unboundmp_jni PRIVATE UNBOUNDMP_ANDROID_WITH_NETWORK=1)
endif()
target_compile_features(unboundmp_jni PRIVATE cxx_std_20)
