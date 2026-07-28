#pragma once
#include "render/render_layer.h"

namespace unboundmp::render {

class LightingManager : public RenderLayer {
public:
    LightingManager();
    ~LightingManager() override = default;

    void Render(const RenderContext& context) override;
    
    void SetGlobalIllumination(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        target_r_ = r; target_g_ = g; target_b_ = b; target_a_ = a;
    }
    
    void SetPreset(const std::string& name);
    void CyclePreset();
    const std::string& GetPresetName() const { return preset_name_; }

private:
    std::string preset_name_ = "Default";
    int preset_index_ = 0;
    
    float current_r_ = 0, current_g_ = 0, current_b_ = 0, current_a_ = 0;
    uint8_t target_r_ = 0, target_g_ = 0, target_b_ = 0, target_a_ = 0;
};

} // namespace unboundmp::render
