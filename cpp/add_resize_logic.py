def add_resize_logic():
    path = 'd:/Unbound/pokemon/cpp/include/ui/screen.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    if 'virtual void OnResize(int width, int height) {}' not in content:
        content = content.replace('virtual void OnResume() = 0;', 'virtual void OnResume() = 0;\n    virtual void OnResize(int width, int height) {}')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

add_resize_logic()
print("screen.h updated with OnResize")
