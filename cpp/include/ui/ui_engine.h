#pragma once
#include <SDL.h>
#include <memory>
#include "ui/screen_manager.h"
#include "ui/theme.h"
#include "ui/animation.h"
#include "ui/localization.h"
#include "ui/notification_center.h"
#include "ui/font_manager.h"
#include "ui/text_renderer.h"
#include "ui/asset_manager.h"
#include "ui/dev_overlay.h"
#include "network/multiplayer_client.h"

namespace unboundmp::core { class GameContext; }

namespace unboundmp::ui {

class UIEngine {
public:
    UIEngine();
    ~UIEngine();

    bool Initialize(SDL_Renderer* renderer, int width, int height);
    void Shutdown();

    void Render();
    bool HandleInput(const SDL_Event& event);
    void Update(float dt);
    
    SDL_Renderer* GetRenderer() const { return render_context_.renderer; }
    void Resize(int w, int h);

    void SetScale(float scale) { scale_ = scale; }
    float GetScale() const { return scale_; }

    ScreenManager& GetScreens() { return *screen_manager_; }
    ThemeManager& GetTheme() { return *theme_manager_; }
    LocalizationManager& GetLocalization() { return LocalizationManager::GetInstance(); }
    AnimationManager& GetAnimations() { return *animation_manager_; }
    NotificationCenter* GetNotificationCenter() { return notification_center_.get(); }
    FontManager& GetFontManager() { return *font_manager_; }
    TextRenderer& GetTextRenderer() { return *text_renderer_; }
    AssetManager& GetAssetManager() { return *asset_manager_; }
    DevOverlay& GetDevOverlay() { return *dev_overlay_; }

    const RenderContext& GetRenderContext() const { return render_context_; }

    void SetNetworkClient(network::MultiplayerClient* client) { network_client_ = client; }
    network::MultiplayerClient* GetNetworkClient() const { return network_client_; }

    void SetGameContext(core::GameContext* ctx) { 
        game_context_ = ctx; 
        if (dev_overlay_) dev_overlay_->SetGameContext(ctx);
    }
    core::GameContext* GetGameContext() const { return game_context_; }

private:
    std::unique_ptr<ScreenManager> screen_manager_;
    std::unique_ptr<ThemeManager> theme_manager_;
    std::unique_ptr<AnimationManager> animation_manager_;
    std::unique_ptr<NotificationCenter> notification_center_;
    std::unique_ptr<FontManager> font_manager_;
    std::unique_ptr<TextRenderer> text_renderer_;
    std::unique_ptr<AssetManager> asset_manager_;
    std::unique_ptr<DevOverlay> dev_overlay_;
    
    network::MultiplayerClient* network_client_ = nullptr;
    core::GameContext* game_context_ = nullptr;
    
    RenderContext render_context_;
    bool initialized_ = false;
    float scale_ = 0.6f;
};

} // namespace unboundmp::ui
