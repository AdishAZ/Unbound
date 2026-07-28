#include "render/resolution_scaling.h"

#include <algorithm>
#include <cmath>

namespace unboundmp::render {

double ResolutionScaler::Compute(Resolution window, Resolution content) const {
  if (content.width <= 0 || content.height <= 0 || window.width <= 0 || window.height <= 0) {
    return 1.0;
  }

  const double scale_x = static_cast<double>(window.width) / static_cast<double>(content.width);
  const double scale_y = static_cast<double>(window.height) / static_cast<double>(content.height);
  const double free_scale = std::min(scale_x, scale_y);

  if (mode_ == ScalingMode::kFreeScale) {
    return std::max(free_scale, 0.0);
  }

  // kIntegerOnly: floor to the largest whole multiple that still fits. If
  // even 1x doesn't fit (a window smaller than the content), fall back to
  // the fractional value rather than returning 0/invisible content - a
  // shrunk-but-visible frame is the more useful failure mode for a window
  // the user just made too small.
  const double floored = std::floor(free_scale);
  return floored >= 1.0 ? floored : free_scale;
}

}  // namespace unboundmp::render
