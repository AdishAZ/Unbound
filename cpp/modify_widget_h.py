def modify_widget_h():
    path = 'd:/Unbound/pokemon/cpp/include/ui/widget.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'virtual Size GetPreferredSize() const' not in content:
        content = content.replace('void SetSize(int w, int h);', 'void SetSize(int w, int h);\n    virtual Size GetPreferredSize() const { return {bounds_.width, bounds_.height}; }')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

modify_widget_h()
print("widget.h updated")
