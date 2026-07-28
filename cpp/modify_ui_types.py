def modify_ui_types():
    path = 'd:/Unbound/pokemon/cpp/include/ui/ui_types.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    if 'enum class SizePolicy' not in content:
        old_enums = 'enum class Alignment { Left, Center, Right, TopLeft, TopCenter, TopRight, BottomLeft, BottomCenter, BottomRight, Top, Bottom };\nenum class AnchorPoint { TopLeft, TopCenter, TopRight, CenterLeft, Center, CenterRight, BottomLeft, BottomCenter, BottomRight };\nenum class LayoutDirection { Horizontal, Vertical };'
        new_enums = old_enums + '\n\nenum class SizePolicy { Fixed, Expand, WrapContent };'
        content = content.replace(old_enums, new_enums)
        
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

modify_ui_types()
print("ui_types.h updated")
