def fix_game_screen():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    old_resize = '''void GameScreen::OnResize(int width, int height) {
    hud_overlay_->SetBounds({0, 0, width, height});
    hud_overlay_->InvalidateLayout();
    
    // Reposition anchored/absolute elements
    for (auto& child : hud_overlay_->GetChildren()) {
        if (child->GetId() == "hud_chat") {
            child->SetBounds({10, height - 210, 300, 200});
        }
    }
}'''

    new_resize = '''void GameScreen::OnResize(int width, int height) {
    hud_overlay_->SetBounds({0, 0, width, height});
    hud_overlay_->InvalidateLayout();
    
    int chat_y = height - 210;
    
    // Reposition anchored/absolute elements
    for (auto& child : hud_overlay_->GetChildren()) {
        if (child->GetId() == "hud_chat") {
            child->SetBounds({10, chat_y, 300, 200});
            for (auto& c : child->GetChildren()) {
                if (c->GetId() == "chat_tabs") c->SetBounds({15, chat_y + 5, 290, 20});
                else if (c->GetId() == "chat_list") c->SetBounds({15, chat_y + 30, 290, 135});
                else if (c->GetId() == "chat_input") c->SetBounds({15, chat_y + 170, 250, 25});
                else if (c->GetId() == "send_btn") c->SetBounds({270, chat_y + 170, 35, 25});
            }
        } else if (child->GetId() == "party_col") {
            child->SetBounds({width - 50, 150, 40, 300});
        } else if (child->GetId() == "quick_menu") {
            child->SetBounds({width - 210, height - 40, 200, 30});
        }
    }
}'''

    if old_resize in content:
        content = content.replace(old_resize, new_resize)
    else:
        print('Could not find old OnResize')

    # Now fix BuildHUD
    old_tabs = '''auto tabs = std::make_shared<HorizontalLayout>();'''
    new_tabs = '''auto tabs = std::make_shared<HorizontalLayout>("chat_tabs");'''
    content = content.replace(old_tabs, new_tabs)
    
    old_send = '''auto send_btn = std::make_shared<Button>();'''
    new_send = '''auto send_btn = std::make_shared<Button>("send_btn");'''
    content = content.replace(old_send, new_send)
    
    old_party = '''auto party_layout = std::make_shared<VerticalLayout>();'''
    new_party = '''auto party_layout = std::make_shared<VerticalLayout>("party_col");'''
    content = content.replace(old_party, new_party)

    old_quick = '''auto quick_menu = std::make_shared<HorizontalLayout>();'''
    new_quick = '''auto quick_menu = std::make_shared<HorizontalLayout>("quick_menu");'''
    content = content.replace(old_quick, new_quick)
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Fixed game_screen.cpp')

fix_game_screen()
