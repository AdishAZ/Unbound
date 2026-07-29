#pragma once

#include "render/irenderer.h"
#include "render/camera.h"
#include "render/camera_controller.h"
#include "render/render_queue.h"
#include <memory>
#include <vector>

namespace unboundmp::render {

class ParticleSystem;
class WeatherRenderer;
class LightingManager;

// Orchestrates the overall rendering pipeline
class RenderManager {
public:
    RenderManager();
    ~RenderManager();

    void Initialize();
    void Shutdown();

    void AddRenderer(std::shared_ptr<IRenderer> renderer);

    // Main render entry point
    void Render(void* native_renderer, int width, int height, float dt);
    void* GetCurrentContext() const { return nullptr; } // Stub if needed

    void SetParticleSystem(std::shared_ptr<ParticleSystem> ps);
    void SetWeatherRenderer(std::shared_ptr<WeatherRenderer> wr);
    void SetLightingManager(std::shared_ptr<LightingManager> lm);
    
    std::shared_ptr<ParticleSystem> GetParticleSystem() const { return particle_system_; }
    std::shared_ptr<WeatherRenderer> GetWeatherRenderer() const { return weather_renderer_; }
    std::shared_ptr<LightingManager> GetLightingManager() const { return lighting_manager_; }
    
    Camera& GetCamera() { return camera_; }
    CameraController& GetCameraController() { return camera_controller_; }

    RenderQueue& GetQueue() { return render_queue_; }
    float GetLastFrameRenderTimeMs() const { return last_frame_render_time_ms_; }

private:
    Camera camera_;
    CameraController camera_controller_{camera_};
    RenderQueue render_queue_;
    std::vector<std::shared_ptr<IRenderer>> renderers_;
    
    std::shared_ptr<ParticleSystem> particle_system_;
    std::shared_ptr<WeatherRenderer> weather_renderer_;
    std::shared_ptr<LightingManager> lighting_manager_;
    
    float last_frame_render_time_ms_ = 0.0f;
};

} // namespace unboundmp::render
