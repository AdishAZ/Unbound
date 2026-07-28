#include "ui/animation.h"
#include <algorithm>

namespace unboundmp::ui {

namespace Easing {
    float Linear(float t) { return t; }
    float EaseIn(float t) { return t * t; }
    float EaseOut(float t) { return t * (2 - t); }
    float EaseInOut(float t) { return t < .5 ? 2 * t * t : -1 + (4 - 2 * t) * t; }
}

Animation::Animation(Widget* target, float duration, std::function<float(float)> easing)
    : target_(target), duration_(duration), easing_(std::move(easing)) {}

void Animation::Update(float dt) {
    if (IsComplete()) return;
    
    elapsed_ += dt;
    float t = std::clamp(elapsed_ / duration_, 0.0f, 1.0f);
    float progress = easing_(t);
    
    Apply(progress);
    
    if (IsComplete() && completed_cb_) {
        completed_cb_();
    }
}

FadeAnimation::FadeAnimation(Widget* target, float duration, float start_alpha, float end_alpha, std::function<float(float)> easing)
    : Animation(target, duration, std::move(easing)), start_alpha_(start_alpha), end_alpha_(end_alpha) {}

void FadeAnimation::Apply(float progress) {
    // If Widget supports SetAlpha or similar
    target_->SetAlpha(start_alpha_ + (end_alpha_ - start_alpha_) * progress);
}

SlideAnimation::SlideAnimation(Widget* target, float duration, Point start_pos, Point end_pos, std::function<float(float)> easing)
    : Animation(target, duration, std::move(easing)), start_pos_(start_pos), end_pos_(end_pos) {}

void SlideAnimation::Apply(float progress) {
    target_->SetPosition(
        static_cast<int>(start_pos_.x + (end_pos_.x - start_pos_.x) * progress),
        static_cast<int>(start_pos_.y + (end_pos_.y - start_pos_.y) * progress)
    );
}

ScaleAnimation::ScaleAnimation(Widget* target, float duration, Size start_size, Size end_size, std::function<float(float)> easing)
    : Animation(target, duration, std::move(easing)), start_size_(start_size), end_size_(end_size) {}

void ScaleAnimation::Apply(float progress) {
    target_->SetSize(
        static_cast<int>(start_size_.width + (end_size_.width - start_size_.width) * progress),
        static_cast<int>(start_size_.height + (end_size_.height - start_size_.height) * progress)
    );
}

ColorAnimation::ColorAnimation(Widget* target, float duration, Color* target_color, Color start_color, Color end_color, std::function<float(float)> easing)
    : Animation(target, duration, std::move(easing)), target_color_(target_color), start_color_(start_color), end_color_(end_color) {}

void ColorAnimation::Apply(float progress) {
    if (target_color_) {
        target_color_->r = static_cast<uint8_t>(start_color_.r + (end_color_.r - start_color_.r) * progress);
        target_color_->g = static_cast<uint8_t>(start_color_.g + (end_color_.g - start_color_.g) * progress);
        target_color_->b = static_cast<uint8_t>(start_color_.b + (end_color_.b - start_color_.b) * progress);
        target_color_->a = static_cast<uint8_t>(start_color_.a + (end_color_.a - start_color_.a) * progress);
    }
}

ProgressAnimation::ProgressAnimation(Widget* target, float duration, float* target_val, float start_val, float end_val, std::function<float(float)> easing)
    : Animation(target, duration, std::move(easing)), target_val_(target_val), start_val_(start_val), end_val_(end_val) {}

void ProgressAnimation::Apply(float progress) {
    if (target_val_) {
        *target_val_ = start_val_ + (end_val_ - start_val_) * progress;
    }
}


void AnimationManager::Add(std::unique_ptr<Animation> anim) {
    animations_.push_back(std::move(anim));
}

void AnimationManager::Update(float dt) {
    for (auto& anim : animations_) {
        anim->Update(dt);
    }
    animations_.erase(std::remove_if(animations_.begin(), animations_.end(),
        [](const std::unique_ptr<Animation>& anim) { return anim->IsComplete(); }),
        animations_.end());
}

void AnimationManager::Clear() {
    animations_.clear();
}

} // namespace unboundmp::ui
