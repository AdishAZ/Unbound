// Standalone smoke test for the Milestone 10 widescreen render foundation.
// Walks through a window resize (4:3 -> 16:9 -> ultrawide -> back to 4:3),
// printing the resulting viewport rect, camera tile window, and a couple
// of resolved overlay positions at each step. No SDL2/OpenGL, no window,
// no actual pixels drawn: this only proves the aspect-ratio/viewport/
// camera/overlay math behaves sensibly on its own, the same way
// memory_probe_example.cpp proves the memory readers behave sensibly
// without a real emulator core attached.
#include <iomanip>
#include <iostream>
#include <string>

#include "render/aspect_ratio.h"
#include "render/camera.h"
#include "render/overlay_layout.h"
#include "render/resolution.h"
#include "render/viewport.h"

namespace {

using unboundmp::render::AspectRatio;
using unboundmp::render::Camera;
using unboundmp::render::CameraWindow;
using unboundmp::render::DynamicViewport;
using unboundmp::render::OverlayAnchor;
using unboundmp::render::OverlayLayout;
using unboundmp::render::Resolution;
using unboundmp::render::ScreenPoint;
using unboundmp::render::ViewportRect;

void PrintStep(const std::string& label, Resolution window, DynamicViewport& viewport, Camera& camera,
               const OverlayLayout& overlay) {
  const AspectRatio window_ratio(window);
  const CameraWindow& cam = camera.Recompute(window_ratio);

  // The camera's expanded tile window becomes the viewport's "content" -
  // this is the link between "how much world is visible" (camera.h) and
  // "where does that get drawn in the window" (viewport.h).
  const Resolution content{cam.columns * unboundmp::render::kTilePixelSize,
                            cam.rows * unboundmp::render::kTilePixelSize};
  viewport.Resize(window, content);

  const ViewportRect& rect = viewport.Rect();
  const ScreenPoint top_right = overlay.Resolve(OverlayAnchor::kTopRight, viewport);
  // A remote player 4 tiles east, 0 tiles north of the local player.
  const ScreenPoint remote = overlay.ProjectTile(4, 0, camera, viewport);

  std::cout << "-- " << label << " (" << window.width << "x" << window.height << ") --\n"
            << "  aspect ratio      : " << std::fixed << std::setprecision(3) << window_ratio.Value() << "\n"
            << "  camera tiles      : " << cam.columns << "x" << cam.rows << " (player at column "
            << cam.player_column << ")\n"
            << "  viewport scale    : " << viewport.Scale() << "x\n"
            << "  viewport rect     : x=" << rect.x << " y=" << rect.y << " w=" << rect.width
            << " h=" << rect.height << "\n"
            << "  pillarbox/letterbox slack? " << (viewport.HasHorizontalSlack() ? "yes" : "no") << "\n"
            << "  HUD top-right     : (" << top_right.x << ", " << top_right.y << ")\n"
            << "  remote player (+4,0) marker : (" << remote.x << ", " << remote.y << ")\n\n";
}

}  // namespace

int main() {
  DynamicViewport viewport;
  Camera camera;
  OverlayLayout overlay;

  PrintStep("Native 4:3 window", Resolution{960, 640}, viewport, camera, overlay);
  PrintStep("16:9 widescreen window", Resolution{1280, 720}, viewport, camera, overlay);
  PrintStep("21:9 ultrawide window", Resolution{2560, 1080}, viewport, camera, overlay);

  // Camera expansion disabled: widescreen should now fall back to
  // pillarboxing the native 15x10 window instead of widening it.
  unboundmp::render::CameraConfig no_expansion;
  no_expansion.allow_expansion = false;
  Camera fixed_camera(no_expansion);
  PrintStep("16:9 window, expansion OFF", Resolution{1280, 720}, viewport, fixed_camera, overlay);

  // Map-edge clamp: player standing 2 tiles from the left edge of a
  // 40-tile-wide map shouldn't have the camera request columns that don't
  // exist off the map's edge.
  const AspectRatio widescreen(Resolution{1280, 720});
  const CameraWindow& clamped = camera.RecomputeClamped(widescreen, /*player_tile_x=*/2, /*map_width_tiles=*/40);
  std::cout << "-- Map-edge clamp (player at tile x=2, map width=40) --\n"
            << "  camera tiles      : " << clamped.columns << "x" << clamped.rows << " (player at column "
            << clamped.player_column << ")\n"
            << "  tiles left/right of player : " << clamped.TilesLeftOfPlayer() << " / "
            << clamped.TilesRightOfPlayer() << "\n";

  return 0;
}
