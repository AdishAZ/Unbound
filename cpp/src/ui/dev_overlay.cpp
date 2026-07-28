#include "ui/dev_overlay.h"
#include "ui/profiler.h"
#include "ui/widget.h"
#include "core/game_context.h"
#include "render/render_manager.h"
#include "render/particle_system.h"
#include "render/weather_renderer.h"
#include "render/lighting_manager.h"
#include "render/camera.h"
#include "gameplay/world_manager.h"
#include "gameplay/remote_player_manager.h"
#include "gameplay/entity_manager.h"
#include "network/multiplayer_client.h"
#include "network/client_session_manager.h"
#include "game_state/game_state.h"
#include <format>
#include <algorithm>

#ifdef _WIN32
#undef DrawText
#endif

namespace unboundmp::ui {

void DevOverlay::Toggle() {
    m_visible = !m_visible;
}

bool DevOverlay::IsVisible() const {
    return m_visible;
}

void DevOverlay::Update(float dt) {
    if (!m_visible) return;
    Profiler::Instance().UpdateMemoryStats();
}

void DevOverlay::RenderSectionHeader(const RenderContext& ctx, const std::string& title, int x, int& y) {
    // Header background
    SDL_SetRenderDrawColor(ctx.renderer, 100, 100, 100, 255);
    SDL_Rect headerRect{x, y, 320, 24};
    SDL_RenderFillRect(ctx.renderer, &headerRect);
    
    Widget::DrawText(ctx, title, x + 10, y + 5, {255, 255, 255, 255});
    
    y += 32;
}

void DevOverlay::RenderTextRow(const RenderContext& ctx, const std::string& label, const std::string& value, int x, int& y) {
    Widget::DrawText(ctx, label + ":", x + 10, y, {200, 200, 200, 255});
    Widget::DrawText(ctx, value, x + 160, y, {255, 255, 255, 255});
    y += 20;
}

void DevOverlay::RenderSparkline(const RenderContext& ctx, int x, int& y) {
    const auto& history = Profiler::Instance().GetFrameTimeHistory();
    
    int width = 320;
    int height = 50;
    
    SDL_SetRenderDrawColor(ctx.renderer, 50, 50, 50, 255);
    SDL_Rect bgRect{x, y, width, height};
    SDL_RenderFillRect(ctx.renderer, &bgRect);
    
    SDL_SetRenderDrawColor(ctx.renderer, 0, 255, 0, 255);
    
    float maxTime = 33.3f; // 30 FPS baseline for scaling
    for (float t : history) {
        if (t > maxTime) maxTime = t;
    }
    
    int stepX = width / 60;
    for (size_t i = 1; i < history.size(); ++i) {
        int x1 = x + static_cast<int>((i - 1) * stepX);
        int y1 = y + height - static_cast<int>((history[i - 1] / maxTime) * height);
        int x2 = x + static_cast<int>(i * stepX);
        int y2 = y + height - static_cast<int>((history[i] / maxTime) * height);
        
        // Clamp Y
        y1 = std::max(y, std::min(y + height, y1));
        y2 = std::max(y, std::min(y + height, y2));
        
        SDL_RenderDrawLine(ctx.renderer, x1, y1, x2, y2);
    }
    
    y += height + 16;
}

void DevOverlay::Render(const RenderContext& ctx) {
    if (!m_visible) return;
    
    auto& profiler = Profiler::Instance();
    
    int panelWidth = 340;
    int x = 0;
    int y = 0;
    
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ctx.renderer, 20, 20, 20, 210);
    
    int windowHeight = ctx.screen_height > 0 ? ctx.screen_height : 720;
    
    // Draw initial column background
    SDL_Rect panelRect{x, y, panelWidth, windowHeight};
    SDL_RenderFillRect(ctx.renderer, &panelRect);
    
    int contentX = x + 10;
    int contentY = y + 10;
    
    auto checkWrap = [&](int requiredHeight) {
        if (contentY + requiredHeight > windowHeight - 20) {
            contentX += panelWidth;
            contentY = y + 10;
            
            // Draw background for new column
            SDL_SetRenderDrawColor(ctx.renderer, 20, 20, 20, 210);
            SDL_Rect colRect{contentX - 10, y, panelWidth, windowHeight};
            SDL_RenderFillRect(ctx.renderer, &colRect);
        }
    };
    
    // PERFORMANCE
    checkWrap(160);
    RenderSectionHeader(ctx, "Performance", contentX, contentY);
    RenderTextRow(ctx, "FPS", std::format("{:.1f}", profiler.GetFPS()), contentX, contentY);
    RenderTextRow(ctx, "Frame Time", std::format("{:.1f} ms", profiler.GetFrameTimeMs()), contentX, contentY);
    RenderTextRow(ctx, "Memory", std::format("{} MB", profiler.GetWorkingSetMB()), contentX, contentY);
    contentY += 10;
    RenderSparkline(ctx, contentX, contentY);
    contentY += 10;
    
    // NETWORK
    checkWrap(220);
    RenderSectionHeader(ctx, "Network", contentX, contentY);
    
    bool is_connected = false;
    bool is_authenticated = false;
    
    if (game_context_) {
        if (auto client = game_context_->GetNetworkClient()) {
            is_connected = client->IsConnected();
        }
        is_authenticated = game_context_->GetSessionManager().IsAuthenticated();
    }
    
    if (is_authenticated && is_connected) {
        RenderTextRow(ctx, "Status", "Connected", contentX, contentY);
        RenderTextRow(ctx, "Ping", std::format("{:.0f} ms", profiler.GetPingMs()), contentX, contentY);
        RenderTextRow(ctx, "Avg RTT", std::format("{:.0f} ms", profiler.GetAverageRTT()), contentX, contentY);
        RenderTextRow(ctx, "Packet Loss", std::format("{:.1f} %", profiler.GetPacketLoss()), contentX, contentY);
        RenderTextRow(ctx, "Bandwidth In", std::format("{:.1f} KB/s", profiler.GetBandwidthInKBps()), contentX, contentY);
        RenderTextRow(ctx, "Bandwidth Out", std::format("{:.1f} KB/s", profiler.GetBandwidthOutKBps()), contentX, contentY);
        RenderTextRow(ctx, "Packet Rate", std::format("{}/{} pps", profiler.GetPacketRateIn(), profiler.GetPacketRateOut()), contentX, contentY);
        RenderTextRow(ctx, "Queue Size", std::to_string(profiler.GetPacketQueueSize()), contentX, contentY);
    } else {
        std::string status = "Disconnected";
        if (is_connected && !is_authenticated) {
            status = "Authenticating...";
        }
        RenderTextRow(ctx, "Status", status, contentX, contentY);
    }
    
    contentY += 16;
    
    // RENDER METRICS
    checkWrap(240);
    RenderSectionHeader(ctx, "Rendering Pipeline", contentX, contentY);
    if (game_context_) {
        if (auto rm = game_context_->GetRenderManager()) {
            RenderTextRow(ctx, "Draw Commands", std::to_string(rm->GetQueue().GetCommandCount()), contentX, contentY);
            RenderTextRow(ctx, "Batches", std::to_string(rm->GetQueue().GetBatchCount()), contentX, contentY);
            RenderTextRow(ctx, "Render Sort Time", std::format("{:.2f} ms", rm->GetQueue().GetSortTimeMs()), contentX, contentY);
            RenderTextRow(ctx, "Frame R-Time", std::format("{:.2f} ms", rm->GetLastFrameRenderTimeMs()), contentX, contentY);
            
            auto cam = rm->GetCamera();
            RenderTextRow(ctx, "Camera Pos", std::format("X:{:.0f} Y:{:.0f}", cam.GetX(), cam.GetY()), contentX, contentY);
            RenderTextRow(ctx, "Camera Zoom", std::format("{:.2f}x", cam.GetZoom()), contentX, contentY);
            
            if (auto ps = rm->GetParticleSystem()) {
                RenderTextRow(ctx, "Particles", std::to_string(ps->GetParticleCount()), contentX, contentY);
            }
            if (auto wr = rm->GetWeatherRenderer()) {
                RenderTextRow(ctx, "Weather", wr->GetWeatherName(), contentX, contentY);
            }
            if (auto lm = rm->GetLightingManager()) {
                RenderTextRow(ctx, "Lighting", lm->GetPresetName(), contentX, contentY);
            }
        }
        if (auto wm = game_context_->GetWorldManager()) {
            int rendered = wm->GetRemotePlayerManager()->GetPlayerCount() + 1;
            RenderTextRow(ctx, "Rendered Players", std::to_string(rendered), contentX, contentY);
        }
    }
    
    contentY += 16;
    
    contentY += 16;
    
    // GAME STATE
    checkWrap(300);
    RenderSectionHeader(ctx, "Game State", contentX, contentY);
    RenderTextRow(ctx, "Server Tick", std::to_string(profiler.GetServerTick()), contentX, contentY);
    RenderTextRow(ctx, "Client Tick", std::to_string(profiler.GetClientTick()), contentX, contentY);
    RenderTextRow(ctx, "Pred Errors", std::to_string(profiler.GetPredictionErrors()), contentX, contentY);
    RenderTextRow(ctx, "Interp Delay", std::format("{:.1f} ms", profiler.GetInterpolationDelay()), contentX, contentY);
    RenderTextRow(ctx, "Remote Players", std::to_string(profiler.GetEntityCount()), contentX, contentY);
    RenderTextRow(ctx, "Total Players", std::to_string(profiler.GetPlayerCount()), contentX, contentY);
    RenderTextRow(ctx, "Map Name", "Unknown", contentX, contentY);
    RenderTextRow(ctx, "Map ID", std::to_string(profiler.GetMapId()), contentX, contentY);
    RenderTextRow(ctx, "Tile Position", std::format("({:.0f}, {:.0f})", profiler.GetPlayerX(), profiler.GetPlayerY()), contentX, contentY);
    RenderTextRow(ctx, "World Position", std::format("({:.0f}, {:.0f})", profiler.GetPlayerX() * 16.0f, profiler.GetPlayerY() * 16.0f), contentX, contentY);
    
    std::string facing = "Unknown";
    switch (profiler.GetDirection()) {
        case 1: facing = "South"; break;
        case 2: facing = "North"; break;
        case 3: facing = "West"; break;
        case 4: facing = "East"; break;
        default: facing = "South"; break;
    }
    RenderTextRow(ctx, "Facing", facing, contentX, contentY);
    
    contentY += 16;
    
    // PERSISTENCE
    checkWrap(100);
    RenderSectionHeader(ctx, "Persistence", contentX, contentY);
    RenderTextRow(ctx, "Dirty Flags", std::to_string(profiler.GetDirtyFlagCount()), contentX, contentY);
    RenderTextRow(ctx, "Autosave Queue", std::to_string(profiler.GetAutosaveQueueSize()), contentX, contentY);
    
    contentY += 16;
    
    // RENDER HINTS (At the bottom of whichever column we end up in)
    checkWrap(40);
    Widget::DrawText(ctx, "F3: Toggle Debug  F4: Cycle Weather  F5: Cycle Lighting", contentX, contentY, {255, 255, 0, 255});
}

} // namespace unboundmp::ui
