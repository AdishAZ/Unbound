def edit_cmake_sources():
    path = 'd:/Unbound/pokemon/cpp/CMakeLists.txt'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find the unboundmp_ui sources
    if 'src/ui/font_manager.cpp' not in content:
        content = content.replace('src/ui/theme.cpp\n      src/ui/ui_engine.cpp\n      src/ui/ui_types.cpp', 'src/ui/theme.cpp\n      src/ui/ui_engine.cpp\n      src/ui/ui_types.cpp\n      src/ui/font_manager.cpp\n      src/ui/text_renderer.cpp')
        
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_cmake_sources()
print("CMakeLists.txt modified with new sources")
