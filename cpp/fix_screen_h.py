def fix_screen_h():
    path = 'd:/Unbound/pokemon/cpp/include/ui/screen.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'virtual void OnResize(int width, int height)' not in content:
        content = content.replace('virtual void OnResume() {}', 'virtual void OnResume() {}\n    virtual void OnResize(int width, int height) {}')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

fix_screen_h()
print("screen.h fixed")
