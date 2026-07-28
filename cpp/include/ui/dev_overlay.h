#pragma once

#include "ui/ui_types.h"
#include <SDL2/SDL.h>

namespace unboundmp::core { class GameContext; }

namespace unboundmp::ui {

class DevOverlay {
public:
    DevOverlay() = default;
    ~DevOverlay() = default;
    
    void Toggle();
    bool IsVisible() const;
    
    void Update(float dt);
    void Render(const RenderContext& ctx);
    
    void SetGameContext(core::GameContext* game_context) { game_context_ = game_context; }

private:
    bool m_visible{false};
    core::GameContext* game_context_{nullptr};
    
    void RenderSectionHeader(const RenderContext& ctx, const std::string& title, int x, int& y);
    void RenderTextRow(const RenderContext& ctx, const std::string& label, const std::string& value, int x, int& y);
    void RenderSparkline(const RenderContext& ctx, int x, int& y);
};

} // namespace unboundmp::ui
