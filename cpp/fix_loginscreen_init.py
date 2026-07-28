def fix_loginscreen_init():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_init = '''    auto anchor_layout = std::make_shared<AnchorLayout>();
    anchor_layout->SetWidthPolicy(SizePolicy::Expand);
    anchor_layout->SetHeightPolicy(SizePolicy::Expand);'''
    
    new_init = '''    auto anchor_layout = std::make_shared<AnchorLayout>();
    anchor_layout->SetBounds({0, 0, engine->GetRenderContext().screen_width, engine->GetRenderContext().screen_height});
    anchor_layout->SetWidthPolicy(SizePolicy::Expand);
    anchor_layout->SetHeightPolicy(SizePolicy::Expand);'''

    if old_init in content:
        content = content.replace(old_init, new_init)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Fixed LoginScreen init bounds')
    else:
        print('Could not find LoginScreen init')

fix_loginscreen_init()
