def modify_widgets_h():
    path = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Container
    content = content.replace('virtual void PerformLayout() {}', 'virtual void PerformLayout() {}\n    Size GetPreferredSize() const override;')
    
    # VerticalLayout
    content = content.replace('void PerformLayout() override;', 'void PerformLayout() override;\n    Size GetPreferredSize() const override;')
    
    # HorizontalLayout
    content = content.replace('void PerformLayout() override;', 'void PerformLayout() override;\n    Size GetPreferredSize() const override;')
    
    # GridLayout
    content = content.replace('void PerformLayout() override;', 'void PerformLayout() override;\n    Size GetPreferredSize() const override;')
    
    # AnchorLayout
    content = content.replace('void PerformLayout() override;', 'void PerformLayout() override;\n    Size GetPreferredSize() const override;')
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

modify_widgets_h()
print("widgets.h updated")
