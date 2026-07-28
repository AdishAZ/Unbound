#include "render/particle_system.h"
#include "render/render_queue.h"
#include "render/camera.h"

namespace unboundmp::render {

ParticleSystem::ParticleSystem() : RenderLayer("ParticleSystem") {}

void ParticleSystem::SpawnParticle(const Particle& p) {
    particles_.push_back(p);
}

void ParticleSystem::Update(float dt) {
    for (auto it = particles_.begin(); it != particles_.end(); ) {
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        it->lifetime -= dt;
        
        if (it->lifetime <= 0.0f) {
            it = particles_.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::Render(const RenderContext& context) {
    if (!context.queue || !context.camera) return;
    
    // Stub: Emit a particle every few frames to prove it works
    if (rand() % 5 == 0) {
        Particle p;
        p.x = context.camera->GetX() + (rand() % 40 - 20);
        p.y = context.camera->GetY() + (rand() % 30 - 15);
        p.vy = 2.0f; // falling down
        p.vx = (rand() % 10 - 5) * 0.1f;
        p.r = 150; p.g = 150; p.b = 255; p.a = 200; // Raindrop color
        p.lifetime = 3.0f; p.max_lifetime = 3.0f;
        p.scale = 0.5f;
        SpawnParticle(p);
    }
    
    Update(context.delta_time);

    for (const auto& p : particles_) {
        float screen_x, screen_y;
        context.camera->WorldToScreen(p.x, p.y, context.viewport_width, context.viewport_height, screen_x, screen_y);
        
        int size = static_cast<int>(4 * p.scale);
        
        DrawCommand cmd;
        cmd.sort_key.layer = RenderLayerZ::kWeather;
        cmd.sort_key.sort_y = p.y;
        cmd.dst_rect = { static_cast<int>(screen_x) - size/2, static_cast<int>(screen_y) - size/2, size, size };
        cmd.r = p.r; cmd.g = p.g; cmd.b = p.b; 
        cmd.a = static_cast<uint8_t>((p.lifetime / p.max_lifetime) * p.a);
        cmd.is_filled_rect = true;
        
        context.queue->Enqueue(cmd);
    }
}

} // namespace unboundmp::render
