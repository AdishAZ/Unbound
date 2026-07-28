def edit_ui_types():
    path = 'd:/Unbound/pokemon/cpp/include/ui/ui_types.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    if 'class FontManager;' not in content:
        content = content.replace('namespace unboundmp::ui {\n\nstruct Color', 'namespace unboundmp::ui {\n\nclass FontManager;\nclass TextRenderer;\n\nstruct Color')
        
    if 'FontManager* font_manager = nullptr;' not in content:
        content = content.replace('float alpha = 1.0f;', 'float alpha = 1.0f;\n    FontManager* font_manager = nullptr;\n    TextRenderer* text_renderer = nullptr;')

    # Replace Alignment enum to have TopLeft etc since TextRenderer references it
    content = content.replace('enum class Alignment { Left, Center, Right, Top, Bottom };', 'enum class Alignment { Left, Center, Right, TopLeft, TopCenter, TopRight, BottomLeft, BottomCenter, BottomRight, Top, Bottom };')
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_ui_types()
print("ui_types.h modified")
