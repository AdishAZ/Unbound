def fix_compile_errors():
    # Fix screen.h
    path = 'd:/Unbound/pokemon/cpp/include/ui/screen.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    if 'virtual void OnResize(int width, int height) {}' not in content:
        # Check what the actual text is
        content = content.replace('virtual void OnResume() = 0;', 'virtual void OnResume() = 0;\n    virtual void OnResize(int width, int height) {}')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

    # Fix ui_engine.cpp renderer variable name
    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()
        
    content_cpp = content_cpp.replace('asset_manager_->Initialize(renderer);', 'asset_manager_->Initialize(render_context_.renderer);')
    with open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(content_cpp)

fix_compile_errors()
print("Compilation fixes applied")
