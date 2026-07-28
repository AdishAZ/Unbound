#pragma once
#include "ui/ui_types.h"
#include "ui/widget.h"
#include <functional>
#include <memory>
#include <vector>

namespace unboundmp::ui {

namespace Easing {
    float Linear(float t);
    float EaseIn(float t);
    float EaseOut(float t);
    float EaseInOut(float t);
}

class Animation {
public:
    Animation(Widget* target, float duration, std::function<float(float)> easing = Easing::Linear);
    virtual ~Animation() = default;

    virtual void Update(float dt);
    bool IsComplete() const { return elapsed_ >= duration_; }

    void SetCompletedCallback(std::function<void()> cb) { completed_cb_ = std::move(cb); }

protected:
    virtual void Apply(float progress) = 0;

    Widget* target_;
    float duration_;
    float elapsed_ = 0.0f;
    std::function<float(float)> easing_;
    std::function<void()> completed_cb_;
};

class FadeAnimation : public Animation {
public:
    FadeAnimation(Widget* target, float duration, float start_alpha, float end_alpha, std::function<float(float)> easing = Easing::Linear);
protected:
    void Apply(float progress) override;
private:
    float start_alpha_;
    float end_alpha_;
};

class SlideAnimation : public Animation {
public:
    SlideAnimation(Widget* target, float duration, Point start_pos, Point end_pos, std::function<float(float)> easing = Easing::Linear);
protected:
    void Apply(float progress) override;
private:
    Point start_pos_;
    Point end_pos_;
};

class ScaleAnimation : public Animation {
public:
    ScaleAnimation(Widget* target, float duration, Size start_size, Size end_size, std::function<float(float)> easing = Easing::Linear);
protected:
    void Apply(float progress) override;
private:
    Size start_size_;
    Size end_size_;
};

class ColorAnimation : public Animation {
public:
    ColorAnimation(Widget* target, float duration, Color* target_color, Color start_color, Color end_color, std::function<float(float)> easing = Easing::Linear);
protected:
    void Apply(float progress) override;
private:
    Color* target_color_;
    Color start_color_;
    Color end_color_;
};

class ProgressAnimation : public Animation {
public:
    ProgressAnimation(Widget* target, float duration, float* target_val, float start_val, float end_val, std::function<float(float)> easing = Easing::Linear);
protected:
    void Apply(float progress) override;
private:
    float* target_val_;
    float start_val_;
    float end_val_;
};

class AnimationManager {
public:
    void Add(std::unique_ptr<Animation> anim);
    void Update(float dt);
    void Clear();
    size_t GetActiveCount() const { return animations_.size(); }

private:
    std::vector<std::unique_ptr<Animation>> animations_;
};

} // namespace unboundmp::ui
