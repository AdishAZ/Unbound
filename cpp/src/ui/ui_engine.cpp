#include "ui/ui_engine.h"
#include "core/game_context.h"
#include "render/render_manager.h"
#include "render/weather_renderer.h"
#include "render/lighting_manager.h"

namespace unboundmp::ui {

UIEngine::UIEngine() {
    screen_manager_ = std::make_unique<ScreenManager>();
    theme_manager_ = std::make_unique<ThemeManager>();
    animation_manager_ = std::make_unique<AnimationManager>();
    notification_center_ = std::make_unique<NotificationCenter>();
    font_manager_ = std::make_unique<FontManager>();
    text_renderer_ = std::make_unique<TextRenderer>(font_manager_.get());
    asset_manager_ = std::make_unique<AssetManager>();
    asset_manager_->Initialize(render_context_.renderer);
    dev_overlay_ = std::make_unique<DevOverlay>();
}

UIEngine::~UIEngine() {
    Shutdown();
}

bool UIEngine::Initialize(SDL_Renderer* renderer, int width, int height) {
    if (initialized_) return true;

    render_context_.renderer = renderer;
    render_context_.screen_width = width;
    render_context_.screen_height = height;
    asset_manager_->Initialize(renderer);
    
    // Default config
    theme_manager_->SetDefaultTheme();
    LocalizationManager::GetInstance().SetLanguage("en_US"); // Try to load english

    if (!font_manager_->Initialize("../../assets/fonts/default.ttf")) {
        return false;
    }
    
    render_context_.font_manager = font_manager_.get();
    render_context_.text_renderer = text_renderer_.get();
    render_context_.theme = theme_manager_.get();
    
    if (notification_center_) {
        notification_center_->SetAnimationManager(animation_manager_.get());
    }

    initialized_ = true;
    return true;
}

void UIEngine::Shutdown() {
    if (!initialized_) return;
    
    animation_manager_->Clear();
    text_renderer_->ClearCache();
    font_manager_->Shutdown();
    
    // Clear screens
    while (screen_manager_->GetCurrentScreen() != nullptr) {
        screen_manager_->Pop();
    }

    render_context_.renderer = nullptr;
    initialized_ = false;
}

bool UIEngine::HandleInput(const SDL_Event& event) {
    if (!initialized_) return false;
    
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F3) {
        dev_overlay_->Toggle();
        return true;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F4) {
        if (game_context_ && game_context_->GetRenderManager() && game_context_->GetRenderManager()->GetWeatherRenderer()) {
            auto wr = game_context_->GetRenderManager()->GetWeatherRenderer();
            int next_weather = (static_cast<int>(wr->GetWeather()) + 1) % 5; // 0 to 4 (Ash = 4)
            wr->SetWeather(static_cast<unboundmp::render::WeatherType>(next_weather));
        }
        return true;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F5) {
        if (game_context_ && game_context_->GetRenderManager() && game_context_->GetRenderManager()->GetLightingManager()) {
            auto lm = game_context_->GetRenderManager()->GetLightingManager();
            if (lm->GetPresetName() == "Default") {
                lm->SetPreset("Morning");
                lm->SetGlobalIllumination(255, 200, 150, 40);
            } else if (lm->GetPresetName() == "Morning") {
                lm->SetPreset("Afternoon");
                lm->SetGlobalIllumination(255, 240, 220, 20);
            } else if (lm->GetPresetName() == "Afternoon") {
                lm->SetPreset("Night");
                lm->SetGlobalIllumination(20, 30, 80, 120);
            } else {
                lm->SetPreset("Default");
                lm->SetGlobalIllumination(0, 0, 0, 0);
            }
        }
        return true;
    }
    
    return screen_manager_->HandleInput(event);
}

void UIEngine::Update(float dt) {
    if (!initialized_) return;
    if (animation_manager_) {
        animation_manager_->Update(dt);
    }
    if (screen_manager_) {
        screen_manager_->Update(dt);
    }
    if (notification_center_) {
        notification_center_->Update(dt);
    }
    if (dev_overlay_ && dev_overlay_->IsVisible()) {
        dev_overlay_->Update(dt);
    }
}

void UIEngine::Render() {
    if (!initialized_) return;
    screen_manager_->Render(render_context_);
    if (dev_overlay_ && dev_overlay_->IsVisible()) {
        dev_overlay_->Render(render_context_);
    }
}

void UIEngine::Resize(int w, int h) {
    if (render_context_.screen_width == w && render_context_.screen_height == h) return;
    
    render_context_.screen_width = w;
    render_context_.screen_height = h;
    
    if (auto current = screen_manager_->GetCurrentScreen()) {
        current->OnResize(w, h);
    }
}

} // namespace unboundmp::ui
