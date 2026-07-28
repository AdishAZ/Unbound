#include "render/weather_renderer.h"
#include "render/render_queue.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace unboundmp::render {

WeatherRenderer::WeatherRenderer() : RenderLayer("WeatherRenderer") {
    current_weather_ = WeatherType::kNone;
}

void WeatherRenderer::Render(const RenderContext& context) {
    if (!context.queue || current_weather_ == WeatherType::kNone) return;
    
    time_accumulator_ += context.delta_time;
    
    int max_particles = 0;
    float spawn_rate = 0.0f; // particles per second
    float speed_y_base = 0.0f, speed_y_var = 0.0f;
    float speed_x_base = 0.0f, speed_x_var = 0.0f;
    float life_base = 0.0f;
    int size = 1;
    uint8_t r = 255, g = 255, b = 255, a = 255;
    
    if (current_weather_ == WeatherType::kRain) {
        max_particles = 500;
        spawn_rate = 300.0f;
        speed_y_base = 800.0f; speed_y_var = 200.0f;
        speed_x_base = 100.0f; speed_x_var = 50.0f;
        life_base = 1.0f;
        size = 2; // lines
        r = 150; g = 180; b = 255; a = 150;
    } else if (current_weather_ == WeatherType::kSnow) {
        max_particles = 300;
        spawn_rate = 50.0f;
        speed_y_base = 100.0f; speed_y_var = 50.0f;
        speed_x_base = 50.0f; speed_x_var = 100.0f;
        life_base = 5.0f;
        size = 3; // boxes
        r = 255; g = 255; b = 255; a = 200;
    } else if (current_weather_ == WeatherType::kAsh) {
        max_particles = 400;
        spawn_rate = 100.0f;
        speed_y_base = 80.0f; speed_y_var = 40.0f;
        speed_x_base = -60.0f; speed_x_var = 80.0f;
        life_base = 6.0f;
        size = 2; // dark grey specks
        r = 60; g = 60; b = 60; a = 180;
    }
    
    // Process Fog fullscreen overlay instead of particles
    if (current_weather_ == WeatherType::kFog) {
        DrawCommand cmd;
        cmd.sort_key.layer = RenderLayerZ::kWeather;
        cmd.dst_rect = {0, 0, context.viewport_width, context.viewport_height};
        // Pulse fog alpha
        float fog_a = 80 + 40 * std::sin(time_accumulator_ * 0.5f);
        cmd.r = 200; cmd.g = 200; cmd.b = 200; cmd.a = static_cast<uint8_t>(fog_a);
        cmd.is_filled_rect = true;
        context.queue->Enqueue(cmd);
        return;
    }
    
    // Spawn new particles
    int spawn_count = static_cast<int>(spawn_rate * context.delta_time);
    if (rand() % 100 < static_cast<int>((spawn_rate * context.delta_time - spawn_count) * 100)) spawn_count++;
    
    for (int i = 0; i < spawn_count && particles_.size() < max_particles; ++i) {
        Particle p;
        p.x = static_cast<float>(rand() % (context.viewport_width + 400)) - 200.0f;
        p.y = -50.0f; // spawn above screen
        p.vx = speed_x_base + ((rand() % 100) / 100.0f) * speed_x_var - (speed_x_var / 2.0f);
        p.vy = speed_y_base + ((rand() % 100) / 100.0f) * speed_y_var;
        p.life = p.max_life = life_base + ((rand() % 100) / 100.0f) * (life_base * 0.5f);
        particles_.push_back(p);
    }
    
    // Update and draw
    for (auto it = particles_.begin(); it != particles_.end(); ) {
        it->x += it->vx * context.delta_time;
        it->y += it->vy * context.delta_time;
        
        // Sway for snow
        if (current_weather_ == WeatherType::kSnow) {
            it->x += std::sin(time_accumulator_ * 2.0f + it->max_life) * 50.0f * context.delta_time;
        }
        
        it->life -= context.delta_time;
        
        if (it->life <= 0 || it->y > context.viewport_height + 50) {
            it = particles_.erase(it);
            continue;
        }
        
        DrawCommand cmd;
        cmd.sort_key.layer = RenderLayerZ::kWeather;
        cmd.texture = nullptr; cmd.g = g; cmd.b = b; cmd.a = a;
        
        if (current_weather_ == WeatherType::kRain) {
            cmd.is_filled_rect = false;
            cmd.src_rect = { static_cast<int>(it->x), static_cast<int>(it->y), 
                             static_cast<int>(it->x - it->vx * 0.05f), static_cast<int>(it->y - it->vy * 0.05f) };
            // Using src_rect x,y,w,h as x1,y1,x2,y2 for a line trick in RenderQueue is not standard, 
            // so we'll just draw a tall thin rect instead:
            cmd.dst_rect = { static_cast<int>(it->x), static_cast<int>(it->y), 1, static_cast<int>(it->vy * 0.05f) };
            cmd.is_filled_rect = true;
        } else {
            cmd.dst_rect = { static_cast<int>(it->x), static_cast<int>(it->y), size, size };
            cmd.is_filled_rect = true;
        }
        
        context.queue->Enqueue(cmd);
        ++it;
    }
}

} // namespace unboundmp::render
