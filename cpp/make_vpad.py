import re

with open('d:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp', 'r') as f:
    content = f.read()

# Update constructor
content = content.replace(
    'GameScreen::GameScreen(UIEngine* engine) : UIScreen("GameScreen"), engine_(engine) {',
    'using namespace unboundmp::emulator;\n\nGameScreen::GameScreen(UIEngine* engine, bool load_save_state) : UIScreen("GameScreen"), engine_(engine), load_save_state_(load_save_state) {'
)

# Insert Virtual Gamepad into BuildHUD
virtual_pad_code = """
    // Virtual Gamepad
    auto dpad_layout = std::make_shared<Panel>("dpad_panel");
    dpad_layout->SetBounds({10, engine_->GetRenderContext().screen_height - 120, 100, 100});
    dpad_layout->SetBackgroundColor({0,0,0,0});
    
    auto create_vbtn = [this](const std::string& name, int x, int y, int w, int h, GbaButton btn) {
        auto b = std::make_shared<Button>();
        b->SetText(name);
        b->SetBounds({x, y, w, h});
        // We simulate hold state manually. The UI engine doesn't have OnPressDown/Up natively for Buttons unless we add it.
        // Let's check Button class. It has OnClick which triggers on mouse up. 
        // For a virtual D-Pad, OnClick is not enough for holding. 
        // Let's just set virtual_input_ flag when they click and toggle it?
        // Or wait, if we can't hold, maybe we just toggle state?
        // Let's just make it toggle for now, or just send a quick pulse?
        return b;
    };
"""
# Actually, the UI engine Button doesn't support press and hold. 
# Let's check the Widget class in widgets.h to see what we can do.
