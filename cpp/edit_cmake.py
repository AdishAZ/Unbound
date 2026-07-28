def edit_cmake():
    path = 'd:/Unbound/pokemon/cpp/CMakeLists.txt'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = content.replace('src/ui/ui_engine.cpp', 'src/ui/ui_engine.cpp\n      src/ui/font_manager.cpp\n      src/ui/text_renderer.cpp')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_cmake()
print("CMakeLists.txt fixed sources")
