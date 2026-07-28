#pragma once

#include "render/resolution.h"

namespace unboundmp::render {

// The well-known ratio values as plain doubles, for comparisons/tests that
// don't want to build a Resolution first.
inline constexpr double kNativeAspectRatioValue = 240.0 / 160.0;  // 1.5 (3:2)
inline constexpr double kWidescreenAspectRatioValue = 16.0 / 9.0;  // 1.777...

// Named aspect ratio classification. Widescreen support cares about which
// bucket a window falls into - not its raw pixel resolution, since a
// window can be resized freely and two very different resolutions
// (1280x853 and 1920x1280) can both be "basically the native 3:2 ratio".
enum class AspectRatioClass {
  kTallerThanNative,  // narrower than 3:2 (e.g. a portrait or split-screen window)
  kNative4x3,         // approximately the GBA-native 3:2 ratio
  kWidescreen16x9,    // wider than native, up to approximately 16:9
  kUltrawide,         // wider than 16:9 (21:9 and beyond)
};

class AspectRatio {
 public:
  AspectRatio() = default;
  explicit AspectRatio(Resolution resolution) : ratio_(resolution.AspectRatioValue()) {}
  explicit AspectRatio(double ratio) : ratio_(ratio) {}

  double Value() const { return ratio_; }

  AspectRatioClass Classify() const;

  // Whether this ratio is wide enough that the camera should widen its
  // tile window (see camera.h) to fill the extra space with more world,
  // instead of just leaving it as a pillarboxed border. True for
  // kWidescreen16x9 and kUltrawide.
  bool WantsCameraExpansion() const;

 private:
  double ratio_ = kNativeAspectRatioValue;
};

}  // namespace unboundmp::render
