import re

with open('d:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp', 'r') as f:
    content = f.read()

# Update constructor
content = content.replace(
    'GameScreen::GameScreen(UIEngine* engine) : UIScreen("GameScreen"), engine_(engine) {',
    'using namespace unboundmp::emulator;\n\nGameScreen::GameScreen(UIEngine* engine, bool load_save_state) : UIScreen("GameScreen"), engine_(engine), load_save_state_(load_save_state) {'
)

# Insert Virtual Gamepad into BuildHUD
vpad_code = """
    // Virtual Gamepad Overlay (D-Pad and A/B)
    auto vpad_panel = std::make_shared<Panel>("vpad_panel");
    vpad_panel->SetBounds({10, engine_->GetRenderContext().screen_height - 120, 150, 100});
    vpad_panel->SetBackgroundColor({0,0,0,0}); // Transparent
    
    auto add_vbtn = [&](const std::string& name, int x, int y, int w, int h, unboundmp::emulator::GbaButton btn) {
        auto b = std::make_shared<Button>();
        b->SetText(name);
        b->SetBounds({x, y, w, h});
        virtual_buttons_.push_back({b, btn});
        vpad_panel->AddChild(b);
    };
    
    // D-Pad
    add_vbtn("U", 50, 0, 30, 30, unboundmp::emulator::GbaButton::kUp);
    add_vbtn("D", 50, 60, 30, 30, unboundmp::emulator::GbaButton::kDown);
    add_vbtn("L", 20, 30, 30, 30, unboundmp::emulator::GbaButton::kLeft);
    add_vbtn("R", 80, 30, 30, 30, unboundmp::emulator::GbaButton::kRight);
    
    // Action Buttons
    auto action_panel = std::make_shared<Panel>("action_panel");
    action_panel->SetBounds({engine_->GetRenderContext().screen_width - 160, engine_->GetRenderContext().screen_height - 120, 150, 100});
    action_panel->SetBackgroundColor({0,0,0,0});
    
    auto add_abtn = [&](const std::string& name, int x, int y, int w, int h, unboundmp::emulator::GbaButton btn) {
        auto b = std::make_shared<Button>();
        b->SetText(name);
        b->SetBounds({x, y, w, h});
        virtual_buttons_.push_back({b, btn});
        action_panel->AddChild(b);
    };
    
    add_abtn("B", 20, 40, 40, 40, unboundmp::emulator::GbaButton::kB);
    add_abtn("A", 80, 20, 40, 40, unboundmp::emulator::GbaButton::kA);
    add_abtn("Sel", 40, 0, 35, 20, unboundmp::emulator::GbaButton::kSelect);
    add_abtn("St", 80, 0, 35, 20, unboundmp::emulator::GbaButton::kStart);
    
    hud_overlay_->AddChild(vpad_panel);
    hud_overlay_->AddChild(action_panel);
}
"""

content = content.replace(
    'hud_overlay_->AddChild(quick_layout);\n}',
    'hud_overlay_->AddChild(quick_layout);\n' + vpad_code
)

resize_code = """
        } else if (child->GetId() == "vpad_panel") {
            child->SetBounds({10, height - 120, 150, 100});
        } else if (child->GetId() == "action_panel") {
            child->SetBounds({width - 160, height - 120, 150, 100});
        }
    }
"""
content = content.replace(
    '        } else if (child->GetId() == "quick_menu") {\n            child->SetBounds({width - 240, height - 30, 230, 25});\n        }\n    }\n',
    '        } else if (child->GetId() == "quick_menu") {\n            child->SetBounds({width - 240, height - 30, 230, 25});\n' + resize_code
)

update_code = """void GameScreen::Update(float dt) {
    hud_overlay_->Update(dt);
    window_manager_->Update(dt);
    
    // Poll virtual buttons and update state
    virtual_input_.held_mask = 0;
    for (const auto& pair : virtual_buttons_) {
        if (pair.first->IsPressed()) {
            virtual_input_.Press(pair.second);
        }
    }
}
"""

content = content.replace(
    'void GameScreen::Update(float dt) {\n    hud_overlay_->Update(dt);\n    window_manager_->Update(dt);\n}\n',
    update_code
)

with open('d:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp', 'w') as f:
    f.write(content)
