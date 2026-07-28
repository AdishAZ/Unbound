#pragma once
#include "ui/widgets.h"
#include "models/item.h"

namespace unboundmp::ui {

class ItemWidget : public Widget {
public:
    explicit ItemWidget(const std::string& id = "");

    void SetItem(const models::Item& item) { item_ = item; }
    const models::Item& GetItem() const { return item_; }
    
    void SetSelected(bool selected) { selected_ = selected; }
    bool IsSelected() const { return selected_; }
    
    void OnClick(ClickCallback cb) { on_click_ = std::move(cb); }
    void OnRightClick(ClickCallback cb) { on_right_click_ = std::move(cb); }

    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    models::Item item_;
    bool selected_ = false;
    bool hover_ = false;
    bool pressed_ = false;
    
    ClickCallback on_click_;
    ClickCallback on_right_click_;
    
    // UI constants
    Color bg_color_{50, 55, 60, 255};
    Color hover_color_{70, 75, 80, 255};
    Color selected_color_{90, 150, 255, 255};
    Color border_color_{30, 30, 35, 255};
};

} // namespace unboundmp::ui
