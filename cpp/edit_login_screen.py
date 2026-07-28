def edit_login_screen():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = content.replace('            return true;\n        }\n        state_ = LoginState::Connecting;', '            return;\n        }\n        state_ = LoginState::Connecting;')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_login_screen()
print("login_screen.cpp fixed")
