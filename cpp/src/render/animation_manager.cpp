#include "render/animation_manager.h"
#include <algorithm>

namespace unboundmp::render {

void AnimationManager::Update(float dt) {
    for (auto& anim : animations_) {
        anim.current_time += dt;
        float progress = std::clamp(anim.current_time / anim.duration, 0.0f, 1.0f);
        
        if (anim.on_update) {
            anim.on_update(progress);
        }
        
        if (progress >= 1.0f && !anim.is_completed) {
            anim.is_completed = true;
            if (anim.on_complete) {
                anim.on_complete();
            }
        }
    }
    
    animations_.erase(std::remove_if(animations_.begin(), animations_.end(),
                                     [](const Animation& a) { return a.is_completed; }),
                      animations_.end());
}

void AnimationManager::PlayAnimation(const Animation& anim) {
    animations_.push_back(anim);
}

} // namespace unboundmp::render
