#pragma once

// Draws TouchInputMapper's on-screen control layout (Milestone 14 follow-
// up to items 6/7 - "Overlay Rendering" / drawing the touch layout that
// android/README.md previously listed as still-missing: "DefaultLayout()'s
// regions are live... but nothing paints the d-pad/buttons on screen
// yet"). This closes that specific gap - a flat-colored, semi-transparent
// rectangle per TouchRegion, brightened while its button is held - without
// reaching into actual GBA video output, which still doesn't exist on
// desktop or Android (see the root README.md).
//
// GL-only: must be constructed/used/destroyed entirely from the render
// thread while a context is current (RenderLoop's frame callback is the
// only caller). No dependency on JNI, Android SDK, or anything beyond
// GLES2 + this project's already-platform-agnostic input::TouchRegion.

#include <GLES2/gl2.h>

#include <cstdint>
#include <vector>

#include "input/touch_input_mapper.h"

namespace unboundmp::android_engine {

class OverlayRenderer {
 public:
  OverlayRenderer() = default;
  ~OverlayRenderer();

  OverlayRenderer(const OverlayRenderer&) = delete;
  OverlayRenderer& operator=(const OverlayRenderer&) = delete;

  // Compiles the (tiny) shader program. Must be called once with a GL
  // context current on the calling thread before the first Draw(); safe
  // to call repeatedly (a no-op after the first success). Returns false
  // (and logs why) on shader compile/link failure - callers should treat
  // that as "skip drawing the overlay this run", not fatal.
  bool EnsureInitialized();

  // Draws every region in `layout` as a flat rectangle - brighter/more
  // opaque if its button is currently held per `held_mask`
  // (emulator::InputState::held_mask bit layout). `width`/`height` are the
  // current surface size in pixels (only used to know the viewport is
  // already set - regions are normalized so no pixel math is needed here
  // beyond that).
  void Draw(const std::vector<input::TouchRegion>& layout, uint32_t held_mask, int32_t width,
            int32_t height);

  // Releases the GL program. Must be called from the render thread with
  // the context still current (mirrors GlContext::Shutdown()'s ordering
  // requirement).
  void Shutdown();

 private:
  GLuint program_ = 0;
  GLint position_attrib_ = -1;
  GLint color_uniform_ = -1;
  bool initialized_ = false;
};

}  // namespace unboundmp::android_engine
