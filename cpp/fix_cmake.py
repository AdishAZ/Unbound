def fix_cmake():
    path = 'd:/Unbound/pokemon/cpp/CMakeLists.txt'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = content.replace('SDL2::SDL2_ttf', 'SDL2_ttf::SDL2_ttf')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_cmake()
print("CMakeLists.txt fixed")
