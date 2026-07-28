def modify_bounds_change():
    path_widget_h = 'd:/Unbound/pokemon/cpp/include/ui/widget.h'
    with open(path_widget_h, 'r', encoding='utf-8') as f:
        content_w = f.read()
    
    content_w = content_w.replace('void SetBounds(const Rect& bounds) { bounds_ = bounds; }', 'virtual void SetBounds(const Rect& bounds) { bounds_ = bounds; }')
    
    with open(path_widget_h, 'w', encoding='utf-8') as f:
        f.write(content_w)

    path_widgets_h = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path_widgets_h, 'r', encoding='utf-8') as f:
        content_ws = f.read()
    
    if 'void SetBounds(const Rect& bounds) override' not in content_ws:
        content_ws = content_ws.replace('virtual void PerformLayout() {}', 'virtual void PerformLayout() {}\n    void SetBounds(const Rect& bounds) override {\n        if (bounds_.width != bounds.width || bounds_.height != bounds.height) layout_dirty_ = true;\n        Widget::SetBounds(bounds);\n    }')
        with open(path_widgets_h, 'w', encoding='utf-8') as f:
            f.write(content_ws)

modify_bounds_change()
print("SetBounds made virtual and overridden in Container")
