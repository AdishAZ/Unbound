#pragma once

#include "ui/widget.h"
#include <string>
#include <vector>
#include <functional>

namespace unboundmp::ui {

class ContextMenu : public Widget {
public:
    ContextMenu();
    
    void AddItem(const std::string& text, std::function<void()> callback);
    void AddSeparator();
    void ClearItems();
    
    void Show(int x, int y);
    void Hide();
    bool IsVisible() const;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    struct MenuItem {
        std::string text;
        std::function<void()> callback;
        bool is_separator = false;
        Rect bounds;
    };
    
    std::vector<MenuItem> items_;
    bool visible_ = false;
    int hover_index_ = -1;
    
    void UpdateLayout();
};

} // namespace unboundmp::ui
