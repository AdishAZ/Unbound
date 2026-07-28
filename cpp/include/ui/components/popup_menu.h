#pragma once

#include "ui/widget.h"
#include "ui/ui_types.h"
#include <string>
#include <vector>
#include <functional>

namespace unboundmp::ui {



class PopupMenu : public Widget {
public:
    PopupMenu();
    
    void AddItem(const std::string& text, std::function<void()> callback);
    void ClearItems();
    
    void ShowAnchored(Widget* anchor, AnchorPoint point);
    void Hide();
    bool IsVisible() const;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;

private:
    struct MenuItem {
        std::string text;
        std::function<void()> callback;
        Rect bounds;
    };
    
    std::vector<MenuItem> items_;
    bool visible_ = false;
    int hover_index_ = -1;
    
    void UpdateLayout();
};

} // namespace unboundmp::ui
