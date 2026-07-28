#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"
#include "ui/components/context_menu.h"
#include "models/item.h"
#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <vector>

namespace unboundmp::ui {
    class UIEngine;
    class ItemWidget;
}

namespace unboundmp::core {
    class GameContext;
}

namespace unboundmp::ui {

class InventoryScreen : public UIScreen {
public:
    explicit InventoryScreen(UIEngine* engine);
    ~InventoryScreen() override;

    void Update(float dt) override;
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    
    void OnEnter() override;
    void OnExit() override;

private:
    void BuildUI();
    void RefreshInventory();
    void RebuildGridContent();
    void OnItemClick(const models::Item& item);

    UIEngine* engine_;
    core::GameContext* game_context_;
    
    std::shared_ptr<Window> main_window_;
    std::shared_ptr<TextBox> search_box_;
    std::shared_ptr<VerticalLayout> sidebar_layout_;
    std::shared_ptr<GridLayout> item_grid_;
    std::shared_ptr<Panel> details_panel_;
    
    std::vector<std::shared_ptr<Button>> category_buttons_;

    std::string current_search_;
    int current_category_ = 0; // Index of category
    models::Item selected_item_;
    bool has_selection_ = false;
};

} // namespace unboundmp::ui
