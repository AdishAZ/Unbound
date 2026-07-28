#pragma once
#include "ui/screen.h"
#include <vector>
#include <memory>

namespace unboundmp::ui {

class ScreenManager {
public:
    ScreenManager() = default;
    ~ScreenManager() = default;

    void Push(std::unique_ptr<UIScreen> screen);
    void Pop();
    void Replace(std::unique_ptr<UIScreen> screen);
    void Overlay(std::unique_ptr<UIScreen> screen);

    void Render(const RenderContext& ctx);
    bool HandleInput(const SDL_Event& event);
    void Update(float dt);
    
    UIScreen* GetCurrentScreen() const;

private:
    std::vector<std::unique_ptr<UIScreen>> screens_;
};

} // namespace unboundmp::ui
