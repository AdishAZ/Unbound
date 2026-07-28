#pragma once

#include "render/resolution.h"

namespace unboundmp::render {

enum class ScalingMode {
  // The largest whole-number multiple of the content resolution that still
  // fits inside the window. Sharpest result - every content pixel maps to
  // an identical NxN block of window pixels, so tile edges stay crisp -
  // but leaves the most unused border space on window sizes that aren't a
  // clean multiple of the content resolution.
  kIntegerOnly,
  // The largest scale (integer or fractional) that fits inside the window
  // without exceeding it on either axis. Fills the window more fully at
  // the cost of non-pixel-aligned scaling (soft/shimmering edges).
  kFreeScale,
};

// Computes a *uniform* scale factor (the same multiplier on both axes) to
// fit a content resolution inside a window resolution. Uniformity is the
// whole point: "widescreen without stretching" means the scale ratio never
// differs per axis - only the amount of visible world (via camera.h) or
// the size of the letterbox border (via viewport.h) changes to fill extra
// space, never a per-axis distortion of the image itself.
class ResolutionScaler {
 public:
  explicit ResolutionScaler(ScalingMode mode = ScalingMode::kIntegerOnly) : mode_(mode) {}

  ScalingMode Mode() const { return mode_; }
  void SetMode(ScalingMode mode) { mode_ = mode; }

  double Compute(Resolution window, Resolution content) const;

 private:
  ScalingMode mode_;
};

}  // namespace unboundmp::render
