import re

def fix_file(path, replacements):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    for old, new in replacements:
        content = content.replace(old, new)
        
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp', [
    (': UIScreen("LoginScreen"), engine_(engine), state_(LoginState::Idle), state_timer_(0.0f) : UIScreen("LoginScreen") {', ': UIScreen("LoginScreen"), engine_(engine), state_(LoginState::Idle), state_timer_(0.0f) {')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp', [
    (': UIScreen("CharacterSelectScreen"), engine_(engine), state_(CharacterSelectState::Loading) : UIScreen("CharacterSelectScreen") {', ': UIScreen("CharacterSelectScreen"), engine_(engine), state_(CharacterSelectState::Loading) {')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/loading_screen.cpp', [
    (': UIScreen("LoadingScreen"), engine_(engine), step_(0), timer_(0.0f) : UIScreen("LoadingScreen") {', ': UIScreen("LoadingScreen"), engine_(engine), step_(0), timer_(0.0f) {')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/settings_screen.cpp', [
    (': UIScreen("SettingsScreen"), engine_(engine) : UIScreen("SettingsScreen") {', ': UIScreen("SettingsScreen"), engine_(engine) {')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp', [
    (': UIScreen("GameScreen"), engine_(engine) : UIScreen("GameScreen") {', ': UIScreen("GameScreen"), engine_(engine) {')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/components/popup_menu.cpp', [
    ('ui/components/popup_menu.height', 'ui/components/popup_menu.h')
])
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/context_menu.cpp', [
    ('ui/components/context_menu.height', 'ui/components/context_menu.h')
])
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/player_card.cpp', [
    ('ui/components/player_card.height', 'ui/components/player_card.h')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/components/avatar_renderer.cpp', [
    ('bounds.w', 'bounds.width'),
    ('bounds.h', 'bounds.height')
])

