#pragma once
#include "render/render_layer.h"
#include <vector>

namespace unboundmp::render {

struct Particle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float lifetime = 1.0f;
    float max_lifetime = 1.0f;
    uint8_t r = 255, g = 255, b = 255, a = 255;
    float scale = 1.0f;
};

class ParticleSystem : public RenderLayer {
public:
    ParticleSystem();
    ~ParticleSystem() override = default;

    void Render(const RenderContext& context) override;
    
    void SpawnParticle(const Particle& p);
    void Update(float dt);
    
    size_t GetParticleCount() const { return particles_.size(); }

private:
    std::vector<Particle> particles_;
};

} // namespace unboundmp::render
