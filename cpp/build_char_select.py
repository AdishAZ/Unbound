def build_char_select():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_init = '''CharacterSelectScreen::CharacterSelectScreen(UIEngine* engine) : UIScreen("CharacterSelectScreen"), engine_(engine) {
    auto anchor = std::make_shared<AnchorLayout>();
    anchor->SetWidthPolicy(SizePolicy::Expand);
    anchor->SetHeightPolicy(SizePolicy::Expand);
    
    auto panel = std::make_shared<Panel>();
    panel->SetBounds({0, 0, 800, 600});
    
    auto title = std::make_shared<Label>();
    title->SetText("Select Character");
    title->SetBounds({0, 50, 800, 40});
    title->SetAlignment(Alignment::Center);
    panel->AddChild(title);
    
    auto chars = std::make_shared<HorizontalLayout>();
    chars->SetBounds({100, 150, 600, 300});
    chars->SetSpacing(20);
    
    for(int i=0; i<3; i++) {
        auto card = std::make_shared<Panel>();
        card->SetBounds({0, 0, 180, 250});
        card->SetBorderThickness(2);
        card->SetCornerRadius(8);
        
        auto name = std::make_shared<Label>();
        name->SetText("Player " + std::to_string(i+1));
        name->SetBounds({0, 200, 180, 30});
        name->SetAlignment(Alignment::Center);
        
        auto btn = std::make_shared<Button>();
        btn->SetText("Play");
        btn->SetBounds({40, 100, 100, 40});
        btn->OnClick([this]() {
            engine_->GetScreens().Replace(std::make_unique<GameScreen>(engine_));
        });
        
        card->AddChild(name);
        card->AddChild(btn);
        chars->AddChild(card);
    }
    
    panel->AddChild(chars);
    anchor->AddChild(panel);
    root_layout_ = anchor;
}'''

    old_init_start = 'CharacterSelectScreen::CharacterSelectScreen(UIEngine* engine)'
    old_init_end = 'void CharacterSelectScreen::OnEnter()'
    
    idx1 = content.find(old_init_start)
    idx2 = content.find(old_init_end)
    
    if idx1 != -1 and idx2 != -1:
        content = content[:idx1] + new_init + '\n\n' + content[idx2:]
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('CharacterSelectScreen rebuilt')

build_char_select()
