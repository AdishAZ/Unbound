def update_login():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    old_layout = '''    auto anchor_layout = std::make_shared<AnchorLayout>();
    anchor_layout->SetBounds({0, 0, engine->GetRenderContext().screen_width, engine->GetRenderContext().screen_height});
    anchor_layout->SetWidthPolicy(SizePolicy::Expand);
    anchor_layout->SetHeightPolicy(SizePolicy::Expand);
    
    panel_ = std::make_shared<Panel>("login_panel");'''
    
    new_layout = '''    auto anchor_layout = std::make_shared<AnchorLayout>();
    anchor_layout->SetBounds({0, 0, engine->GetRenderContext().screen_width, engine->GetRenderContext().screen_height});
    anchor_layout->SetWidthPolicy(SizePolicy::Expand);
    anchor_layout->SetHeightPolicy(SizePolicy::Expand);
    
    auto bg_img = std::make_shared<ImageWidget>("bg");
    bg_img->SetWidthPolicy(SizePolicy::Expand);
    bg_img->SetHeightPolicy(SizePolicy::Expand);
    SDL_Texture* tex = engine->GetAssetManager().LoadTexture("login_bg", "assets/ui/login_bg.bmp");
    if (tex) bg_img->SetTexture(tex, true);
    anchor_layout->AddChild(bg_img);
    
    panel_ = std::make_shared<Panel>("login_panel");'''
    
    if old_layout in content:
        content = content.replace(old_layout, new_layout)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Updated LoginScreen')
    else:
        print('Could not find anchor_layout in LoginScreen')

update_login()
