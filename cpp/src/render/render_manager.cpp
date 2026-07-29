#include "render/render_manager.h"

#include <algorithm>
#include <ranges>
#include <SDL2/SDL.h>

#include "render/aspect_ratio.h"
#include "render/lighting_manager.h"
#include "render/particle_system.h"
#include "render/resolution.h"
#include "render/weather_renderer.h"

namespace unboundmp::render {

RenderManager::RenderManager() = default;

RenderManager::~RenderManager() {
    Shutdown();
}

void RenderManager::Initialize() {
    for (const auto& renderer : renderers_) {
        if (renderer) {
            renderer->Initialize();
        }
    }
}

void RenderManager::Shutdown() {
    for (const auto& renderer : renderers_) {
        if (renderer) {
            renderer->Shutdown();
        }
    }

    render_queue_.Clear();
    renderers_.clear();

    // Release specialized subsystem references to ensure clean RAII teardown[cite: 37]
    particle_system_.reset();
    weather_renderer_.reset();
    lighting_manager_.reset();
}

void RenderManager::AddRenderer(std::shared_ptr<IRenderer> renderer) {
    if (!renderer) {
        return;
    }

    // Prevent duplicate entries in the render pipeline when specialized setters are invoked[cite: 37]
    if (std::ranges::find(renderers_, renderer) == renderers_.end()) {
        renderers_.push_back(std::move(renderer));
    }
}

void RenderManager::SetParticleSystem(std::shared_ptr<ParticleSystem> ps) {
    particle_system_ = ps;
    AddRenderer(ps);
}

void RenderManager::SetWeatherRenderer(std::shared_ptr<WeatherRenderer> wr) {
    weather_renderer_ = wr;
    AddRenderer(wr);
}

void RenderManager::SetLightingManager(std::shared_ptr<LightingManager> lm) {
    lighting_manager_ = lm;
    AddRenderer(lm);
}

void RenderManager::Render(void* native_renderer, int width, int height, float dt) {
    // Stage 0: Input & backend validation[cite: 29, 37]
    if (!native_renderer || width <= 0 || height <= 0) {
        return;
    }

    const Uint64 start_time = SDL_GetPerformanceCounter();

    // Stage 1: Camera & Viewport Synchronization[cite: 26, 27, 30]
    camera_controller_.Update(dt);
    camera_.Recompute(AspectRatio(Resolution{width, height}));

    // Stage 2: Unified Render Context Configuration[cite: 29]
    const RenderContext context{
        &render_queue_,
        &camera_,
        width,
        height,
        dt,
        native_renderer
    };

    // Stage 3: Prune dead references and execute Command Generation Pass[cite: 29, 37]
    std::erase_if(renderers_, [](const std::shared_ptr<IRenderer>& r) {
        return !r;
    });

    for (const auto& renderer : renderers_) {
        renderer->Render(context);
    }

    // Stage 4: Hardware Flush Pass[cite: 38]
    render_queue_.Flush(static_cast<SDL_Renderer*>(native_renderer));

    // Stage 5: Profiling & Frame Metrics[cite: 37]
    const Uint64 end_time = SDL_GetPerformanceCounter();
    const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    last_frame_render_time_ms_ = (frequency > 0.0)
        ? static_cast<float>((end_time - start_time) * 1000.0 / frequency)
        : 0.0f;
}

} // namespace unboundmp::render