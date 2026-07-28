# Android client (Milestone 13 - foundation)

Ports the multiplayer *wrapper* to Android by reusing the same `cpp/`
libraries the desktop client uses, plus Android-specific glue. Like every
earlier milestone in this project, this is a foundation layer, not a
finished app - see "What works today" / "What's deliberately deferred"
below for the exact boundary.

This was written and its C++ logic tested (via the desktop examples it
shares code with) without access to a real Android SDK/NDK/device in this
environment - the Gradle/CMake/JNI files are believed correct against
current Android tooling conventions, but **have not been run through an
actual Gradle build**. Treat the first real build as the point where any
NDK/AGP-version mismatches get discovered and fixed, not as a formality.

## What works today

- **Android build** (`settings.gradle.kts`, `build.gradle.kts`,
  `app/build.gradle.kts`) - a standard Gradle/AGP/CMake project. `minSdk 24`,
  targets `arm64-v8a` and `armeabi-v7a`. `externalNativeBuild` points at
  `app/src/main/cpp/CMakeLists.txt`.
- **Touch controls** (`cpp/include/input/touch_input_mapper.h`,
  `NativeBridge.kt`, `MainActivity.kt`) - `TouchInputMapper` is
  platform-agnostic (normalized coordinates, no Android types in it at all -
  see `touch_input_example.cpp`, which exercises the exact same class on
  desktop). `MainActivity` converts raw `MotionEvent` pixel coordinates to
  normalized ones and feeds them through the JNI bridge; the resulting
  held-button mask is logged. The default d-pad/face-button/shoulder-button
  layout (`DefaultLayout()`) is defined but **not yet drawn on screen** -
  see deferred list.
- **Asset loading** (`cpp/include/platform/asset_path_resolver.h`) - resolves
  where the ROM cache, save file, backups, and config should live in the
  app's private storage, namespaced per-ROM so multiple ROMs' saves never
  collide. `MainActivity` uses this after a ROM is picked via the system
  file picker (Storage Access Framework) to copy the ROM's bytes,
  unmodified, into a real filesystem path (SAF `content://` URIs generally
  aren't one, and the future emulator core will need one).
- **Save synchronization on Android** - `SaveManager` (Milestone 12) is
  reused as-is via JNI (`nativeInitSaveManager`, `nativeBeginLinkSession`,
  `nativeEndLinkSession`); no Android-specific save logic exists because
  none was needed once `AssetPathResolver` gives it a real path.
- **Network layer (partially)** - `cpp/src/network/tcp_socket.cpp` is plain
  POSIX BSD sockets already, which the Android NDK supports unchanged; the
  manifest declares `INTERNET`/`ACCESS_NETWORK_STATE`. What's *not* done is
  building `unboundmp_network` for Android - see below.

## What's deliberately deferred

- **`unboundmp_network` / `unboundmp_protocol` on Android.** These need
  Protobuf, and `find_package(Protobuf)` isn't guaranteed to resolve
  cross-compiling under the NDK toolchain without an Android-targeted
  Protobuf build (e.g. vcpkg's `arm64-android` triplet) wired into the
  project's CMake toolchain setup. Rather than have the whole native build
  fail on a stock NDK checkout, `app/src/main/cpp/CMakeLists.txt` gates
  this behind `UNBOUNDMP_ANDROID_WITH_NETWORK` (default `OFF`), the same
  opt-in pattern the desktop build already uses for
  `UNBOUNDMP_WITH_MGBA`. Flipping it on once an Android Protobuf build
  exists should be the only change needed - the C++ source is shared
  unmodified.
- **Emulator integration.** No `MgbaEmulatorCore`-for-Android exists yet
  (needs libmgba built for `arm64-v8a`/`armeabi-v7a`, likely via its own
  CMake/NDK build, then linked in the same `UNBOUNDMP_WITH_MGBA`-style
  opt-in). Without it there is no running game and nothing to render, so:
- **Rendering / a game surface.** `MainActivity` currently shows a plain
  status `TextView`, not a `SurfaceView`/`GLSurfaceView`. This mirrors
  where the desktop client also stands (see the root `README.md`: "the
  actual SDL2/OpenGL rendering milestone" is not yet implemented there
  either) - Android doesn't have a game screen to draw for the same reason
  desktop doesn't yet.
- **Drawing the touch control layout.** `DefaultLayout()`'s regions are
  live (touches inside them map to the correct button) but nothing paints
  the d-pad/buttons on screen yet - that's part of the same future
  rendering milestone above, not separate work.

## Layout

```
android/
  settings.gradle.kts / build.gradle.kts     project-level Gradle files
  app/
    build.gradle.kts                         app module: NDK/CMake wiring, deps
    src/main/
      AndroidManifest.xml                    INTERNET permission, MainActivity
      java/com/unboundmp/client/
        MainActivity.kt                      ROM picker, save init, touch harness
        NativeBridge.kt                      JNI external fun declarations
      cpp/
        CMakeLists.txt                       builds cpp/ libraries for Android + jni_bridge
        jni_bridge.cpp                       thin JNI <-> C++ translation, no logic of its own
      res/values/
        strings.xml, themes.xml
```

## Building (once a real Android SDK/NDK environment is available)

```sh
cd android
./gradlew assembleDebug
```

Requires Android Studio / the Android SDK with NDK side-by-side (the CMake
build is invoked automatically by AGP) and a Gradle wrapper
(`gradlew`/`gradle-wrapper.properties`, not yet checked in - generate one
with `gradle wrapper` once Gradle is available, or open the project in
Android Studio, which will offer to do so).
