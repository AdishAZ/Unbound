#pragma once
#include <functional>
#include <vector>

namespace unboundmp::render {

struct Animation {
    float duration = 1.0f;
    float current_time = 0.0f;
    std::function<void(float)> on_update; // passes 0.0 to 1.0 progress
    std::function<void()> on_complete;
    bool is_completed = false;
};

class AnimationManager {
public:
    AnimationManager() = default;
    ~AnimationManager() = default;
    
    void Update(float dt);
    
    void PlayAnimation(const Animation& anim);

private:
    std::vector<Animation> animations_;
};

} // namespace unboundmp::render
