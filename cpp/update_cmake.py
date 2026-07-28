def update_cmake():
    path = 'd:/Unbound/pokemon/cpp/CMakeLists.txt'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_lines = '''    src/ui/screens/character_select_screen.cpp
    src/ui/screens/loading_screen.cpp'''
    
    new_lines = '''    src/ui/screens/character_select_screen.cpp
    src/ui/screens/character_creation_screen.cpp
    src/ui/screens/loading_screen.cpp'''
    
    if old_lines in content:
        content = content.replace(old_lines, new_lines)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Added character_creation_screen to CMakeLists.txt')

update_cmake()
