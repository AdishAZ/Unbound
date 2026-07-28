#pragma once

#include "ui/ui_types.h"
#include <cstdint>

namespace unboundmp::ui {

class AvatarRenderer {
public:
    static void Render(const RenderContext& ctx, Rect bounds, uint64_t player_id);
};

} // namespace unboundmp::ui
