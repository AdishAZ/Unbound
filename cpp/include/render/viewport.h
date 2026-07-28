#pragma once

#include <cstdint>

#include "render/resolution.h"
#include "render/resolution_scaling.h"

namespace unboundmp::render {

// The rectangle, in window pixel coordinates, that content should be drawn
// into so it fills as much of the window as possible at a uniform scale
// without stretching. Any leftover space (`window` minus this rect) is the
// letterbox/pillarbox border that the actual renderer (a future milestone)
// should clear to black - this struct only computes the geometry, it
// doesn't draw anything.
struct ViewportRect {
  int32_t x = 0;
  int32_t y = 0;
  int32_t width = 0;
  int32_t height = 0;
};

// Tracks "how big is the window right now, and where inside it does game
// content go" as the window resizes. This is the "dynamic" half of
// "dynamic viewport": nothing here is fixed at 240x160 or any particular
// widescreen resolution - Resize() recomputes everything from whatever the
// window actually is at the moment it's called, including on every resize
// event, entering/leaving fullscreen, or a display DPI change.
class DynamicViewport {
 public:
  explicit DynamicViewport(ResolutionScaler scaler = ResolutionScaler{}) : scaler_(scaler) {}

  // Recomputes the viewport for a new window size. `content` defaults to
  // the GBA-native resolution, but a caller using camera.h's expanded tile
  // window should pass the resulting expanded content resolution instead
  // (see camera.h's CameraWindow -> pixel size conversion) so the viewport
  // and camera never disagree about how much content there is to place.
  void Resize(Resolution window, Resolution content = kGbaNativeResolution);

  Resolution Window() const { return window_; }
  Resolution Content() const { return content_; }
  double Scale() const { return scale_; }
  const ViewportRect& Rect() const { return rect_; }

  // True when the window is wider (relative to its height) than the
  // content being drawn - i.e. there's horizontal letterbox space a
  // widescreen-aware camera (see camera.h) could fill with extra world
  // instead of leaving it black. False once the camera has already
  // expanded to consume that slack.
  bool HasHorizontalSlack() const;

 private:
  ResolutionScaler scaler_;
  Resolution window_{};
  Resolution content_ = kGbaNativeResolution;
  double scale_ = 1.0;
  ViewportRect rect_{};
};

}  // namespace unboundmp::render
