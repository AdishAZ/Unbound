#include "render/lighting_manager.h"
#include "render/render_queue.h"

namespace unboundmp::render {

LightingManager::LightingManager() : RenderLayer("LightingManager") {
    SetPreset("Default");
    current_r_ = target_r_;
    current_g_ = target_g_;
    current_b_ = target_b_;
    current_a_ = target_a_;
}

void LightingManager::SetPreset(const std::string& name) {
    preset_name_ = name;
    if (name == "Default") {
        preset_index_ = 0;
        SetGlobalIllumination(0, 0, 0, 0);
    } else if (name == "Morning") {
        preset_index_ = 1;
        SetGlobalIllumination(255, 200, 150, 40);
    } else if (name == "Afternoon") {
        preset_index_ = 2;
        SetGlobalIllumination(255, 240, 200, 20);
    } else if (name == "Night") {
        preset_index_ = 3;
        SetGlobalIllumination(20, 30, 60, 100);
    }
}

void LightingManager::CyclePreset() {
    int next = (preset_index_ + 1) % 4;
    switch (next) {
        case 0: SetPreset("Default"); break;
        case 1: SetPreset("Morning"); break;
        case 2: SetPreset("Afternoon"); break;
        case 3: SetPreset("Night"); break;
    }
}

void LightingManager::Render(const RenderContext& context) {
    if (!context.queue) return;
    
    float dt = context.delta_time;
    float speed = 200.0f * dt;
    
    auto approach = [speed](float current, float target) -> float {
        if (current < target) return std::min(current + speed, target);
        if (current > target) return std::max(current - speed, target);
        return current;
    };
    
    current_r_ = approach(current_r_, static_cast<float>(target_r_));
    current_g_ = approach(current_g_, static_cast<float>(target_g_));
    current_b_ = approach(current_b_, static_cast<float>(target_b_));
    current_a_ = approach(current_a_, static_cast<float>(target_a_));
    
    if (current_a_ <= 0.5f) return;
    
    DrawCommand cmd;
    cmd.sort_key.layer = RenderLayerZ::kLighting;
    cmd.dst_rect = {0, 0, context.viewport_width, context.viewport_height};
    cmd.r = static_cast<uint8_t>(current_r_);
    cmd.g = static_cast<uint8_t>(current_g_);
    cmd.b = static_cast<uint8_t>(current_b_);
    cmd.a = static_cast<uint8_t>(current_a_);
    cmd.is_filled_rect = true;
    
    context.queue->Enqueue(cmd);
}

} // namespace unboundmp::render
