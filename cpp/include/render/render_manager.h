#pragma once

#include <memory>
#include <vector>

#include "render/camera.h"
#include "render/camera_controller.h"
#include "render/irenderer.h"
#include "render/render_queue.h"

namespace unboundmp::render {

class ParticleSystem;
class WeatherRenderer;
class LightingManager;

// Central orchestrator for the multiplayer rendering pipeline.
// Manages camera synchronization, viewport scaling, layer ordering, and batch submission.
class RenderManager {
public:
    RenderManager();
    ~RenderManager();

    // Prevent copying to avoid reference-dangling in CameraController
    RenderManager(const RenderManager&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;

    // Allow default move construction/assignment if needed
    RenderManager(RenderManager&&) noexcept = default;
    RenderManager& operator=(RenderManager&&) noexcept = default;

    // Lifecycle management
    void Initialize();
    void Shutdown();

    // Pipeline layer registration
    void AddRenderer(std::shared_ptr<IRenderer> renderer);

    // Primary frame execution entry point
    void Render(void* native_renderer, int width, int height, float dt);

    [[nodiscard]] void* GetCurrentContext() const { return nullptr; }

    // Specialized subsystem injection
    void SetParticleSystem(std::shared_ptr<ParticleSystem> ps);
    void SetWeatherRenderer(std::shared_ptr<WeatherRenderer> wr);
    void SetLightingManager(std::shared_ptr<LightingManager> lm);

    // Specialized subsystem accessors
    [[nodiscard]] std::shared_ptr<ParticleSystem> GetParticleSystem() const { return particle_system_; }
    [[nodiscard]] std::shared_ptr<WeatherRenderer> GetWeatherRenderer() const { return weather_renderer_; }
    [[nodiscard]] std::shared_ptr<LightingManager> GetLightingManager() const { return lighting_manager_; }

    // Camera subsystem accessors
    [[nodiscard]] Camera& GetCamera() { return camera_; }
    [[nodiscard]] const Camera& GetCamera() const { return camera_; }

    [[nodiscard]] CameraController& GetCameraController() { return camera_controller_; }
    [[nodiscard]] const CameraController& GetCameraController() const { return camera_controller_; }

    // Render queue & profiling accessors
    [[nodiscard]] RenderQueue& GetQueue() { return render_queue_; }
    [[nodiscard]] const RenderQueue& GetQueue() const { return render_queue_; }

    [[nodiscard]] float GetLastFrameRenderTimeMs() const { return last_frame_render_time_ms_; }

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