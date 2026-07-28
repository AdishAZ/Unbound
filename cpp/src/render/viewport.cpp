#include "render/viewport.h"

namespace unboundmp::render {

void DynamicViewport::Resize(Resolution window, Resolution content) {
  window_ = window;
  content_ = content;
  scale_ = scaler_.Compute(window, content);

  const auto scaled_width = static_cast<int32_t>(content.width * scale_);
  const auto scaled_height = static_cast<int32_t>(content.height * scale_);

  rect_.width = scaled_width;
  rect_.height = scaled_height;
  rect_.x = (window.width - scaled_width) / 2;
  rect_.y = (window.height - scaled_height) / 2;
}

bool DynamicViewport::HasHorizontalSlack() const {
  return rect_.x > 0;
}

}  // namespace unboundmp::render
