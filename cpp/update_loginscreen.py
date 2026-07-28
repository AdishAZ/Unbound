def update_loginscreen():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Add server selection combo box
    old_welcome = '''    auto welcome_label = std::make_shared<Label>();
    welcome_label->SetText("Welcome Back");
    welcome_label->SetAlignment(Alignment::Center);
    welcome_label->SetColor(Color::DarkSubtext);
    welcome_label->SetBounds({0, 0, 360, 20});'''
    
    new_server = '''    auto server_label = std::make_shared<Label>();
    server_label->SetText("Select Server");
    server_label->SetBounds({0, 0, 360, 20});
    
    auto server_combo = std::make_shared<ComboBox>("server_select");
    server_combo->SetBounds({0, 0, 360, 35});
    server_combo->AddItem("Global [NA]");
    server_combo->AddItem("Global [EU]");
    server_combo->AddItem("Test Realm");
    server_combo->SetSelectedIndex(0);'''

    if old_welcome in content:
        content = content.replace(old_welcome, new_server)
        
    old_add_welcome = '''    vert_layout->AddChild(title_label_);
    vert_layout->AddChild(welcome_label);'''
    
    new_add_server = '''    vert_layout->AddChild(title_label_);
    vert_layout->AddChild(server_label);
    vert_layout->AddChild(server_combo);'''

    if old_add_welcome in content:
        content = content.replace(old_add_welcome, new_add_server)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('LoginScreen updated with ComboBox')

update_loginscreen()
