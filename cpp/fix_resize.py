def fix_resize():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_resize = '''void GameScreen::OnResize(int width, int height) {
    hud_overlay_->SetBounds({0, 0, width, height});
    hud_overlay_->InvalidateLayout();
    
    // Reposition anchored/absolute elements
    auto chat = hud_overlay_->GetChild("hud_chat");
    if (chat) chat->SetBounds({10, height - 210, 300, 200});'''

    new_resize = '''void GameScreen::OnResize(int width, int height) {
    hud_overlay_->SetBounds({0, 0, width, height});
    hud_overlay_->InvalidateLayout();
    
    // Reposition anchored/absolute elements
    for (auto& child : hud_overlay_->GetChildren()) {
        if (child->GetId() == "hud_chat") {
            child->SetBounds({10, height - 210, 300, 200});
        }
    }'''
    
    if old_resize in content:
        content = content.replace(old_resize, new_resize)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Fixed GameScreen::OnResize')

fix_resize()
