#pragma once
#include "ui/widgets.h"
#include <vector>
#include <memory>

namespace unboundmp::ui {

class WindowManager {
public:
    static WindowManager* GetInstance();

public:
    WindowManager();
    ~WindowManager() = default;

    void AddWindow(std::shared_ptr<Window> window);
    void RemoveWindow(const std::string& id);
    void RemoveWindow(Window* window);
    void BringToFront(Window* window);
    void SendToBack(Window* window);

    void SetModal(std::shared_ptr<Window> window);
    void ClearModal();

    void Update(float dt);
    void Render(const RenderContext& ctx);
    bool HandleInput(const SDL_Event& event);

    void SetBounds(const Rect& bounds) { bounds_ = bounds; }
    void ShowPopup(std::shared_ptr<Widget> popup, int x, int y);
    void ClosePopup();

private:
    std::vector<std::shared_ptr<Window>> windows_;
    std::shared_ptr<Window> modal_window_;
    Rect bounds_{0, 0, 0, 0};
    
    // For popup layer (context menus, tooltips)
    std::vector<std::shared_ptr<Widget>> popups_;
};

} // namespace unboundmp::ui
