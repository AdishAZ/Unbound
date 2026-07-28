#include "render/render_manager.h"

#include "render/particle_system.h"
#include "render/weather_renderer.h"
#include "render/lighting_manager.h"

namespace unboundmp::render {

RenderManager::RenderManager() {}

RenderManager::~RenderManager() {
    Shutdown();
}

void RenderManager::Initialize() {
    for (auto& renderer : renderers_) {
        renderer->Initialize();
    }
}

void RenderManager::Shutdown() {
    for (auto& renderer : renderers_) {
        renderer->Shutdown();
    }
    renderers_.clear();
}

void RenderManager::AddRenderer(std::shared_ptr<IRenderer> renderer) {
    renderers_.push_back(renderer);
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
    Uint64 start_time = SDL_GetPerformanceCounter();
    
    camera_controller_.Update(dt);
    
    Resolution res{width, height};
    camera_.Recompute(AspectRatio(res));
    
    RenderContext context;
    context.camera = &camera_;
    context.queue = &render_queue_;
    context.viewport_width = width;
    context.viewport_height = height;
    context.delta_time = dt;
    context.native_renderer = native_renderer;

    for (auto& renderer : renderers_) {
        renderer->Render(context);
    }
    
    // Actually draw all queued items
    render_queue_.Flush(static_cast<SDL_Renderer*>(native_renderer));
    
    Uint64 end_time = SDL_GetPerformanceCounter();
    last_frame_render_time_ms_ = static_cast<float>(end_time - start_time) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency());
}

} // namespace unboundmp::render
