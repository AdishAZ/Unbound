#include <sstream>
#include <set>
#include "ui/screens/game_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "ui/screens/settings_screen.h"
#include "ui/inventory_screen.h"
#include "ui/window_manager.h"
#include "core/game_context.h"
#include "game_state/game_state.h"
#include <fstream>
#include <map>
#include "core/json.h"
#include "gameplay/world_manager.h"
#include "gameplay/remote_player_manager.h"
#include "render/render_manager.h"
#include "render/camera_controller.h"
#include "render/camera.h"
namespace unboundmp::ui {

using namespace unboundmp::emulator;

GameScreen::GameScreen(UIEngine* engine, bool load_save_state) : UIScreen("GameScreen"), engine_(engine), load_save_state_(load_save_state) {
    hud_overlay_ = std::make_shared<Container>("hud_overlay");
    hud_overlay_->SetWidthPolicy(SizePolicy::Expand);
    hud_overlay_->SetHeightPolicy(SizePolicy::Expand);

    window_manager_ = std::make_unique<WindowManager>();

    BuildHUD();
}

void GameScreen::BuildHUD() {
    // Right-side Party Column
    auto party_col = std::make_shared<VerticalLayout>("party_col");
    party_col->SetSpacing(2);
    for (int i = 0; i < 6; ++i) {
        auto slot = std::make_shared<Panel>("party_slot_" + std::to_string(i));
        slot->SetBounds({0, 0, 80, 50});
        slot->SetBackgroundColor({30, 30, 30, 150});
        
        auto lbl = std::make_shared<Label>("slot_lbl");
        lbl->SetText("Empty");
        lbl->SetBounds({4, 34, 72, 14});
        slot->AddChild(lbl);
        
        auto img = std::make_shared<ImageWidget>("slot_img");
        img->SetBounds({4, -5, 72, 50});
        img->SetScaleMode(ImageWidget::ScaleMode::Fit);
        slot->AddChild(img);
        
        party_col->AddChild(slot);
    }
    hud_overlay_->AddChild(party_col);

    // Top-Left Info Panel
    auto info_panel = std::make_shared<Panel>("info_panel");
    info_panel->SetBackgroundColor({20, 20, 25, 200});
    info_panel->SetCornerRadius(4);
    
    info_label_ = std::make_shared<Label>("info_label");
    info_label_->SetText("Unknown Map\n$0");
    info_label_->SetBounds({10, 10, 140, 40});
    info_panel->AddChild(info_label_);
    hud_overlay_->AddChild(info_panel);
    
    // Quick menu (bottom)
    auto quick_layout = std::make_shared<HorizontalLayout>("quick_menu");
    quick_layout->SetSpacing(4);
    
    auto bag_btn = std::make_shared<Button>(); bag_btn->SetText("Bag"); bag_btn->SetBounds({0,0,45,20});
    bag_btn->OnClick([this]() {
        engine_->GetScreens().Push(std::make_unique<InventoryScreen>(engine_));
    });
    auto trainer_btn = std::make_shared<Button>(); trainer_btn->SetText("Card"); trainer_btn->SetBounds({0,0,50,20});
    auto friends_btn = std::make_shared<Button>(); friends_btn->SetText("Friends"); friends_btn->SetBounds({0,0,65,20});
    
    quick_layout->AddChild(bag_btn);
    quick_layout->AddChild(trainer_btn);
    quick_layout->AddChild(friends_btn);
    
    hud_overlay_->AddChild(quick_layout);
}

void GameScreen::OnEnter() {
    OnResize(engine_->GetRenderContext().screen_width, engine_->GetRenderContext().screen_height);
}
void GameScreen::OnExit() {}
void GameScreen::OnPause() {}
void GameScreen::OnResume() {}

void GameScreen::OnResize(int width, int height) {
    // Calculate the actual game display area (maintains 3:2 aspect ratio)
    float scale_x = static_cast<float>(width) / 240.0f;
    float scale_y = static_cast<float>(height) / 160.0f;
    float scale = std::min(scale_x, scale_y);
    
    int game_w = static_cast<int>(240.0f * scale);
    int game_h = static_cast<int>(160.0f * scale);
    int game_x = (width - game_w) / 2;
    int game_y = (height - game_h) / 2;

    if (hud_overlay_) {
        hud_overlay_->SetBounds({0, 0, width, height});
        
        // Reposition anchored/absolute elements relative to the SCREEN AREA
        for (auto& child : hud_overlay_->GetChildren()) {
            if (child->GetId() == "info_panel") {
                // Top left of the screen
                child->SetBounds({20, 20, 160, 60});
            }
        }
        
        if (auto quick = hud_overlay_->FindChild("quick_menu")) {
            // Bottom right of the screen
            quick->SetBounds({width - 250, height - 50, 240, 25});
        }
        
        if (auto party = hud_overlay_->FindChild("party_col")) {
            // Anchor to the right edge of the screen, not the game map
            party->SetBounds({width - 100, 20, 80, 480});
        }
    }
}

void GameScreen::Render(const RenderContext& ctx) {
    if (hud_overlay_) {
        hud_overlay_->Render(ctx);
    }
    if (window_manager_) {
        window_manager_->Render(ctx);
    }
}

bool GameScreen::HandleInput(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        engine_->GetScreens().Push(std::make_unique<SettingsScreen>(engine_));
        return true;
    }
    
    if (window_manager_->HandleInput(event)) return true;
    if (hud_overlay_->HandleInput(event)) return true;
    
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_i) {
        engine_->GetScreens().Push(std::make_unique<InventoryScreen>(engine_));
        return true;
    }
    
    return false;
}

void GameScreen::Update(float dt) {
    hud_overlay_->Update(dt);
    window_manager_->Update(dt);
    
    if (engine_ && engine_->GetGameContext() && engine_->GetGameContext()->GetGameState()) {
        auto game_state = engine_->GetGameContext()->GetGameState();
        auto world_mgr = engine_->GetGameContext()->GetWorldManager();
        
        if (world_mgr && world_mgr->GetRemotePlayerManager()) {
            auto remote_mgr = world_mgr->GetRemotePlayerManager();
            auto players = remote_mgr->GetAllPlayers();
            
            // Manage UI labels for remote players
            auto players_container_ptr = hud_overlay_->FindChild("remote_players");
            if (!players_container_ptr) {
                auto new_container = std::make_shared<unboundmp::ui::Container>("remote_players");
                new_container->SetBounds({0, 0, engine_->GetRenderContext().screen_width, engine_->GetRenderContext().screen_height});
                hud_overlay_->AddChild(new_container);
                players_container_ptr = new_container.get();
            }
            
            auto players_container = static_cast<unboundmp::ui::Container*>(players_container_ptr);
            
            // Set bounds to match screen
            players_container->SetBounds({0, 0, engine_->GetRenderContext().screen_width, engine_->GetRenderContext().screen_height});
            
            // Keep track of which accounts are visible
            std::set<uint64_t> visible_accounts;
            
            for (const auto& p : players) {
                visible_accounts.insert(p.account_id);
                
                std::string label_id = "name_" + std::to_string(p.account_id);
                auto lbl = players_container->FindChild(label_id);
                if (!lbl) {
                    auto new_lbl = std::make_shared<unboundmp::ui::Label>(label_id);
                    new_lbl->SetText("Player " + std::to_string(p.account_id));
                    new_lbl->SetColor({255, 255, 255, 255});
                    players_container->AddChild(new_lbl);
                    lbl = new_lbl.get();
                }
                
                // Calculate screen position
                float world_px = p.current_x * 16.0f;
                float world_py = p.current_y * 16.0f;
                
                auto& cam = engine_->GetGameContext()->GetRenderManager()->GetCamera();
                float cam_x = cam.GetX() * 16.0f;
                float cam_y = cam.GetY() * 16.0f;
                
                float screen_x = (world_px - cam_x) + (240.0f / 2.0f);
                float screen_y = (world_py - cam_y) + (160.0f / 2.0f);
                
                // Scale from 240x160 to actual window size
                float scale_x = static_cast<float>(engine_->GetRenderContext().screen_width) / 240.0f;
                float scale_y = static_cast<float>(engine_->GetRenderContext().screen_height) / 160.0f;
                float scale = std::min(scale_x, scale_y);
                
                float final_x = (screen_x * scale) + (engine_->GetRenderContext().screen_width - (240.0f * scale)) / 2.0f;
                float final_y = (screen_y * scale) + (engine_->GetRenderContext().screen_height - (160.0f * scale)) / 2.0f;
                
                lbl->SetBounds({static_cast<int>(final_x) - 40, static_cast<int>(final_y) - 30, 80, 20});
            }
            
            // Remove labels for disconnected players
            std::vector<std::string> to_remove;
            for (auto& child : players_container->GetChildren()) {
                if (child->GetId().substr(0, 5) == "name_") {
                    uint64_t acc_id = std::stoull(child->GetId().substr(5));
                    if (visible_accounts.find(acc_id) == visible_accounts.end()) {
                        to_remove.push_back(child->GetId());
                    }
                }
            }
            for (const auto& id : to_remove) {
                players_container->RemoveChild(id);
            }
        }
        
        if (info_label_) {
            const auto& player = game_state->GetLocalPlayer();
            const auto& inventory = game_state->GetInventory();
            
            static std::map<std::string, std::string> map_names;
            static bool map_names_loaded = false;
            if (!map_names_loaded) {
                map_names_loaded = true;
                std::ifstream f("assets/maps/map_names.json");
                if (f.is_open()) {
                    std::stringstream buffer;
                    buffer << f.rdbuf();
                    auto json_val = core::JsonParser::Parse(buffer.str());
                    if (json_val.IsObject()) {
                        for (const auto& [k, v] : json_val.AsObject()) {
                            map_names[k] = v.AsString();
                        }
                    }
                }
            }

            std::string map_key = std::to_string(player.map.bank) + "-" + std::to_string(player.map.number);
            std::string location = "Map " + map_key;
            if (map_names.count(map_key)) {
                location = map_names[map_key];
            }

            info_label_->SetText(location + "\n$" + std::to_string(inventory.money));
        }

        const auto& party = game_state->GetParty();
        if (auto party_col = hud_overlay_->FindChild("party_col")) {
            for (uint32_t i = 0; i < 6; i++) {
                if (auto slot = dynamic_cast<Panel*>(party_col->FindChild("party_slot_" + std::to_string(i)))) {
                    auto lbl = dynamic_cast<Label*>(slot->FindChild("slot_lbl"));
                    auto img = dynamic_cast<ImageWidget*>(slot->FindChild("slot_img"));
                    if (i < party.parsed_slots.size()) {
                        const auto& p_data = party.parsed_slots[i];
                        
                        // DEBUG LOG
                        static int log_ticks = 0;
                        if (log_ticks++ % 60 == 0) {
                            std::ofstream debug_log("party_debug.log", std::ios::app);
                            uint16_t raw_species = 0;
                            std::memcpy(&raw_species, party.slots[i].data() + 32, 2);
                            debug_log << "Slot " << i << ": Decrypted=" << p_data.species_id 
                                      << " Raw=" << raw_species 
                                      << " HP=" << p_data.current_hp << "/" << p_data.max_hp << "\n";
                        }
                        
                        int hp_percent = 0;
                        if (p_data.max_hp > 0) {
                            hp_percent = (p_data.current_hp * 100) / p_data.max_hp;
                        }
                        
                        if (lbl) {
                            Rect sb = slot->GetBounds();
                            lbl->SetBounds({sb.x + 5, sb.y + 52, 90, 15});
                            lbl->SetText(std::to_string(p_data.current_hp) + "/" + std::to_string(p_data.max_hp) + " (" + std::to_string(hp_percent) + "%)");
                        }
                        
                        slot->SetBackgroundColor({30, 30, 30, 150});
                        
                        if (img) {
                            Rect sb = slot->GetBounds();
                            img->SetBounds({sb.x + 5, sb.y - 10, 90, 70}); // Make image bounds taller to enlarge sprite
                            
                            if (p_data.species_id != 0) {
                                // In Unbound, the raw species ID without encryption holds the index!
                                // Read it from the raw memory block
                                uint16_t raw_species = 0;
                                std::memcpy(&raw_species, party.slots[i].data() + 32, 2);
                                
                                if (raw_species != 0 && raw_species <= 1025) {
                                    SDL_Texture* tex = (SDL_Texture*)engine_->GetAssetManager().LoadPokemonSprite(
                                        "unbound_" + std::to_string(raw_species), 
                                        "assets/unbound_pokemon/" + std::to_string(raw_species) + ".bmp");
                                    img->SetTexture(tex);
                                } else {
                                    img->SetTexture(nullptr);
                                }
                            } else {
                                img->SetTexture(nullptr);
                            }
                        }
                    } else {
                        slot->SetBackgroundColor({30, 30, 30, 150});
                        if (lbl) {
                            Rect sb = slot->GetBounds();
                            lbl->SetBounds({sb.x + 5, sb.y + 52, 90, 15});
                            lbl->SetText("Empty");
                        }
                        if (img) {
                            img->SetTexture(nullptr);
                        }
                    }
                }
            }
        }
    }
}

} // namespace unboundmp::ui
