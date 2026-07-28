#include "render/aspect_ratio.h"

namespace unboundmp::render {

namespace {
// Tolerance for floating point resolution math (e.g. a 1280x853 window
// computes to ~1.5006, not exactly 1.5) - not a design threshold, just
// enough slack to absorb integer-pixel rounding.
constexpr double kEpsilon = 0.01;
}  // namespace

AspectRatioClass AspectRatio::Classify() const {
  if (ratio_ < kNativeAspectRatioValue - kEpsilon) {
    return AspectRatioClass::kTallerThanNative;
  }
  if (ratio_ <= kNativeAspectRatioValue + kEpsilon) {
    return AspectRatioClass::kNative4x3;
  }
  if (ratio_ <= kWidescreenAspectRatioValue + kEpsilon) {
    return AspectRatioClass::kWidescreen16x9;
  }
  return AspectRatioClass::kUltrawide;
}

bool AspectRatio::WantsCameraExpansion() const {
  const AspectRatioClass cls = Classify();
  return cls == AspectRatioClass::kWidescreen16x9 || cls == AspectRatioClass::kUltrawide;
}

}  // namespace unboundmp::render
