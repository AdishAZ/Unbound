def fix_widgets_h():
    path = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Remove all extra GetPreferredSize
    content = content.replace('    Size GetPreferredSize() const override;\n    Size GetPreferredSize() const override;\n    Size GetPreferredSize() const override;\n    Size GetPreferredSize() const override;', '    Size GetPreferredSize() const override;')
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_widgets_h()
print("widgets.h duplicates removed")
