#include "render/overlay_layout.h"

#include "render/resolution.h"

namespace unboundmp::render {

ScreenPoint OverlayLayout::Resolve(OverlayAnchor anchor, const DynamicViewport& viewport) const {
  const ViewportRect& rect = viewport.Rect();
  const auto margin = static_cast<int32_t>(config_.margin_px * viewport.Scale());

  switch (anchor) {
    case OverlayAnchor::kTopLeft:
      return ScreenPoint{rect.x + margin, rect.y + margin};
    case OverlayAnchor::kTopCenter:
      return ScreenPoint{rect.x + rect.width / 2, rect.y + margin};
    case OverlayAnchor::kTopRight:
      return ScreenPoint{rect.x + rect.width - margin, rect.y + margin};
    case OverlayAnchor::kBottomLeft:
      return ScreenPoint{rect.x + margin, rect.y + rect.height - margin};
    case OverlayAnchor::kBottomCenter:
      return ScreenPoint{rect.x + rect.width / 2, rect.y + rect.height - margin};
    case OverlayAnchor::kBottomRight:
      return ScreenPoint{rect.x + rect.width - margin, rect.y + rect.height - margin};
    case OverlayAnchor::kCenter:
      return ScreenPoint{rect.x + rect.width / 2, rect.y + rect.height / 2};
  }
  return ScreenPoint{rect.x, rect.y};  // unreachable, keeps -Wreturn-type quiet
}

ScreenPoint OverlayLayout::ProjectTile(int32_t tile_dx_from_local_player, int32_t tile_dy_from_local_player,
                                        const Camera& camera, const DynamicViewport& viewport) const {
  const CameraWindow& window = camera.Window();
  const ViewportRect& rect = viewport.Rect();
  const double tile_px = kTilePixelSize * viewport.Scale();

  const double origin_x = rect.x + window.player_column * tile_px;
  const double origin_y = rect.y + window.player_row * tile_px;

  return ScreenPoint{
      static_cast<int32_t>(origin_x + tile_dx_from_local_player * tile_px),
      static_cast<int32_t>(origin_y + tile_dy_from_local_player * tile_px),
  };
}

}  // namespace unboundmp::render
