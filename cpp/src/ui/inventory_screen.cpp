#include "ui/inventory_screen.h"
#include "ui/ui_engine.h"
#include "ui/item_widget.h"
#include "core/game_context.h"
#include "gameplay/inventory_manager.h"
#include "network/network_manager.h"
#include "network/client_session_manager.h"
#include <algorithm>

namespace unboundmp::ui {

InventoryScreen::InventoryScreen(UIEngine* engine)
    : UIScreen("InventoryScreen"), engine_(engine), game_context_(engine->GetGameContext()) {
    
    BuildUI();

    if (game_context_ && game_context_->GetInventoryManager()) {
        game_context_->GetInventoryManager()->SetOnInventoryUpdatedCallback([this]() {
            RefreshInventory();
        });
        
        // Request sync
        unboundmp::network::Packet req;
        req.type = unboundmp::network::PacketType::kInventoryRequest;
        if (auto session = unboundmp::network::ClientSessionManager::GetInstance().GetSession()) {
            req.session_token = session->session_token;
        }
        if (engine_->GetNetworkClient()) {
            engine_->GetNetworkClient()->SendPacket(req);
        }
    }
}

InventoryScreen::~InventoryScreen() {
    if (game_context_ && game_context_->GetInventoryManager()) {
        game_context_->GetInventoryManager()->SetOnInventoryUpdatedCallback(nullptr);
    }
}

void InventoryScreen::BuildUI() {
    int sw = engine_->GetRenderContext().screen_width;
    int sh = engine_->GetRenderContext().screen_height;

    main_window_ = std::make_shared<Window>("InventoryWindow");
    main_window_->SetTitle("Bag");
    main_window_->SetBounds({ (sw - 600) / 2, (sh - 480) / 2, 600, 480 });
    main_window_->SetBackgroundColor({ 30, 34, 40, 240 }); // Translucent dark
    main_window_->SetCornerRadius(10);
    main_window_->OnClose([this]() {
        engine_->GetScreens().Pop();
    });

    auto root_layout = std::make_shared<HorizontalLayout>("RootLayout");
    root_layout->SetBounds({ 0, 30, 600, 450 }); // Below title bar
    
    // Left Sidebar
    sidebar_layout_ = std::make_shared<VerticalLayout>("Sidebar");
    sidebar_layout_->SetBounds({ 0, 0, 150, 450 });
    sidebar_layout_->SetPadding(Padding::All(10));
    sidebar_layout_->SetSpacing(5);
    
    std::vector<std::string> categories = {
        "Items", "Medicine", "Poké Balls", "TMs/HMs",
        "Berries", "Battle", "Key Items", "Mail"
    };

    for (size_t i = 0; i < categories.size(); ++i) {
        auto btn = std::make_shared<Button>("Cat_" + categories[i]);
        btn->SetText(categories[i]);
        btn->SetBounds({0, 0, 130, 30});
        btn->OnClick([this, i]() {
            current_category_ = i;
            RefreshInventory();
        });
        category_buttons_.push_back(btn);
        sidebar_layout_->AddChild(btn);
    }
    
    // Main Content (Right side)
    auto content_layout = std::make_shared<VerticalLayout>("ContentLayout");
    content_layout->SetBounds({ 150, 0, 450, 450 });
    content_layout->SetPadding(Padding::All(10));
    content_layout->SetSpacing(10);
    
    // Top bar (Search)
    auto top_bar = std::make_shared<HorizontalLayout>("TopBar");
    top_bar->SetBounds({0, 0, 430, 30});
    
    search_box_ = std::make_shared<TextBox>("SearchBox");
    search_box_->SetPlaceholder("Search items...");
    search_box_->SetBounds({ 0, 0, 200, 28 });
    search_box_->OnTextChanged([this](const std::string& text) {
        current_search_ = text;
        std::transform(current_search_.begin(), current_search_.end(), current_search_.begin(), ::tolower);
        RefreshInventory();
    });
    top_bar->AddChild(search_box_);
    content_layout->AddChild(top_bar);
    
    // Grid in ScrollView
    auto scroll = std::make_shared<ScrollView>("GridScroll");
    scroll->SetBounds({ 0, 0, 430, 280 });
    
    item_grid_ = std::make_shared<GridLayout>(1, "ItemGrid"); // 1 col of wide cards
    item_grid_->SetSpacing(4);
    scroll->AddChild(item_grid_);
    content_layout->AddChild(scroll);
    
    // Bottom Details Panel
    details_panel_ = std::make_shared<Panel>("DetailsPanel");
    details_panel_->SetBounds({ 0, 0, 430, 100 });
    details_panel_->SetBackgroundColor({ 20, 25, 30, 200 });
    details_panel_->SetCornerRadius(8);
    // Details panel is manually drawn in Render or populated with widgets. We'll populate dynamically in OnItemClick.
    content_layout->AddChild(details_panel_);
    
    root_layout->AddChild(sidebar_layout_);
    root_layout->AddChild(content_layout);
    
    main_window_->AddChild(root_layout);
    
    RefreshInventory();
}

void InventoryScreen::RefreshInventory() {
    RebuildGridContent();
}

void InventoryScreen::RebuildGridContent() {
    if (!game_context_ || !game_context_->GetInventoryManager()) return;
    
    item_grid_->ClearChildren();
    
    const auto& inv = game_context_->GetInventoryManager()->GetInventory();
    bool has_items = false;
    
    for (const auto& item : inv.items) {
        if (item.quantity == 0) continue;
        
        // Map ItemType to tab_index
        int expected_tab = 0;
        switch (item.type) {
            case models::ItemType::kGeneral: expected_tab = 0; break;
            case models::ItemType::kMedicine: expected_tab = 1; break;
            case models::ItemType::kPokeball: expected_tab = 2; break;
            case models::ItemType::kTMHM: expected_tab = 3; break;
            case models::ItemType::kBerry: expected_tab = 4; break;
            case models::ItemType::kBattleItem: expected_tab = 5; break;
            case models::ItemType::kKeyItem: expected_tab = 6; break;
            case models::ItemType::kMail: expected_tab = 7; break;
        }
        
        if (expected_tab != current_category_) continue;
        
        if (!current_search_.empty()) {
            std::string name_lower = item.name;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
            if (name_lower.find(current_search_) == std::string::npos) {
                continue;
            }
        }
        
        auto widget = std::make_shared<ItemWidget>();
        widget->SetItem(item);
        
        widget->OnClick([this, item]() {
            OnItemClick(item);
        });
        
        item_grid_->AddChild(widget);
        has_items = true;
    }
    
    if (!has_items) {
        auto empty_label = std::make_shared<Label>();
        empty_label->SetText("No items in this category.");
        empty_label->SetBounds({10, 10, 300, 30});
        item_grid_->AddChild(empty_label);
    }
    
    item_grid_->InvalidateLayout();
}

void InventoryScreen::OnItemClick(const models::Item& item) {
    has_selection_ = true;
    selected_item_ = item;
    
    details_panel_->ClearChildren();
    
    // Rebuild details panel UI
    auto icon_img = std::make_shared<Panel>("DetIcon");
    icon_img->SetBounds({ 10, 10, 64, 64 });
    icon_img->SetBackgroundColor({40, 200, 40, 255}); // Green placeholder
    
    auto name_lbl = std::make_shared<Label>();
    name_lbl->SetText(item.name + " x" + std::to_string(item.quantity));
    name_lbl->SetBounds({ 84, 10, 200, 24 });
    
    auto desc_lbl = std::make_shared<Label>();
    desc_lbl->SetText("No description available for " + item.name + "."); 
    desc_lbl->SetBounds({ 84, 34, 200, 40 });
    
    // Actions
    auto use_btn = std::make_shared<Button>("DetUse");
    use_btn->SetText("Use");
    use_btn->SetBounds({ 290, 10, 60, 24 });
    use_btn->OnClick([this, item]() {
        if (game_context_->GetInventoryManager()) {
            game_context_->GetInventoryManager()->RequestUseItem(engine_->GetNetworkClient(), item.slot_index, 0);
        }
    });
    
    auto drop_btn = std::make_shared<Button>("DetDrop");
    drop_btn->SetText("Drop");
    drop_btn->SetBounds({ 360, 10, 60, 24 });
    drop_btn->OnClick([this, item]() {
        if (game_context_->GetInventoryManager()) {
            game_context_->GetInventoryManager()->RequestDropItem(engine_->GetNetworkClient(), item.slot_index, 1);
        }
        has_selection_ = false;
        details_panel_->ClearChildren();
        RefreshInventory();
    });
    
    details_panel_->AddChild(icon_img);
    details_panel_->AddChild(name_lbl);
    details_panel_->AddChild(desc_lbl);
    details_panel_->AddChild(use_btn);
    details_panel_->AddChild(drop_btn);
}

void InventoryScreen::Update(float dt) {
    main_window_->Update(dt);
}

void InventoryScreen::Render(const RenderContext& ctx) {
    main_window_->Render(ctx);
}

bool InventoryScreen::HandleInput(const SDL_Event& event) {
    if (main_window_->HandleInput(event)) return true;
    
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE || 
            event.key.keysym.sym == SDLK_RETURN || 
            event.key.keysym.sym == SDLK_b) {
            engine_->GetScreens().Pop();
            return true;
        }
    }
    
    return true; // We consume all input so it doesn't fall through to game
}

void InventoryScreen::OnEnter() {}
void InventoryScreen::OnExit() {}

} // namespace unboundmp::ui
