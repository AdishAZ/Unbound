#include "ui/hud_widgets.h"
#include "ui/profiler.h"
#include <SDL2/SDL.h>
#include <format>
#include <iomanip>
#include <sstream>

namespace unboundmp::ui {

void HUDWidget::SetAnchor(AnchorPoint anchor) {
    m_anchor = anchor;
}

void HUDWidget::SetOffset(int x, int y) {
    m_offsetX = x;
    m_offsetY = y;
}

void HUDWidget::UpdatePosition(int screenWidth, int screenHeight) {
    int x = 0;
    int y = 0;
    
    switch (m_anchor) {
        case AnchorPoint::TopLeft:      x = 0; y = 0; break;
        case AnchorPoint::TopCenter:    x = screenWidth / 2 - bounds_.width / 2; y = 0; break;
        case AnchorPoint::TopRight:     x = screenWidth - bounds_.width; y = 0; break;
        case AnchorPoint::CenterLeft:   x = 0; y = screenHeight / 2 - bounds_.height / 2; break;
        case AnchorPoint::Center:       x = screenWidth / 2 - bounds_.width / 2; y = screenHeight / 2 - bounds_.height / 2; break;
        case AnchorPoint::CenterRight:  x = screenWidth - bounds_.width; y = screenHeight / 2 - bounds_.height / 2; break;
        case AnchorPoint::BottomLeft:   x = 0; y = screenHeight - bounds_.height; break;
        case AnchorPoint::BottomCenter: x = screenWidth / 2 - bounds_.width / 2; y = screenHeight - bounds_.height; break;
        case AnchorPoint::BottomRight:  x = screenWidth - bounds_.width; y = screenHeight - bounds_.height; break;
    }
    
    bounds_.x = x + m_offsetX;
    bounds_.y = y + m_offsetY;
}

void HUDWidget::RenderPanel(const RenderContext& ctx, const std::string& text, Color textColor) {
    int textLen = static_cast<int>(text.length());
    bounds_.width = textLen * 8 + 4; // 8px per char, 2px padding on each side
    bounds_.height = 14 + 4; // 14px height, 2px padding
    
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ctx.renderer, 20, 20, 20, 180);
    
    SDL_Rect sdlRect{bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_RenderFillRect(ctx.renderer, &sdlRect);
    
    DrawText(ctx, text, bounds_.x + 2, bounds_.y + 2, textColor);
}

void FPSWidget::Render(const RenderContext& ctx) {
    float fps = Profiler::Instance().GetFPS();
    std::string text = std::format("FPS: {:.1f}", fps);
    Color color = {255, 0, 0, 255};
    if (fps > 55.0f) color = {0, 255, 0, 255};
    else if (fps > 30.0f) color = {255, 255, 0, 255};
    
    RenderPanel(ctx, text, color);
}

void PingWidget::Render(const RenderContext& ctx) {
    float ping = Profiler::Instance().GetPingMs();
    std::string text = std::format("Ping: {:.0f}ms", ping);
    Color color = {255, 0, 0, 255};
    if (ping < 50.0f) color = {0, 255, 0, 255};
    else if (ping < 150.0f) color = {255, 255, 0, 255};
    
    RenderPanel(ctx, text, color);
}

void MapWidget::Render(const RenderContext& ctx) {
    uint32_t mapId = Profiler::Instance().GetMapId();
    std::string text = std::format("Map: {}", mapId);
    RenderPanel(ctx, text, {255, 255, 255, 255});
}

void CoordinatesWidget::Render(const RenderContext& ctx) {
    float x = Profiler::Instance().GetPlayerX();
    float y = Profiler::Instance().GetPlayerY();
    std::string text = std::format("X: {:.1f} Y: {:.1f}", x, y);
    RenderPanel(ctx, text, {255, 255, 255, 255});
}

void AutosaveWidget::Render(const RenderContext& ctx) {
    int queue = Profiler::Instance().GetAutosaveQueueSize();
    std::string status = (queue > 0) ? "Saving" : "Idle";
    std::string text = std::format("Save: {}", status);
    RenderPanel(ctx, text, {255, 255, 255, 255});
}

void NetworkWidget::Render(const RenderContext& ctx) {
    // Green dot if ping > 0 basically, simplistic check
    float ping = Profiler::Instance().GetPingMs();
    std::string text = "Net: ";
    text += (ping > 0.0f) ? "O" : "X";
    Color color = (ping > 0.0f) ? Color{0, 255, 0, 255} : Color{255, 0, 0, 255};
    RenderPanel(ctx, text, color);
}

void PlayerCountWidget::Render(const RenderContext& ctx) {
    int players = Profiler::Instance().GetPlayerCount();
    std::string text = std::format("Players: {}", players);
    RenderPanel(ctx, text, {255, 255, 255, 255});
}

void EmulatorSpeedWidget::Render(const RenderContext& ctx) {
    std::string text = "Speed: 1.0x";
    RenderPanel(ctx, text, {255, 255, 255, 255});
}

} // namespace unboundmp::ui
