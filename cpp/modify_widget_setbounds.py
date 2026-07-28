def modify_widget_setbounds():
    path = 'd:/Unbound/pokemon/cpp/include/ui/widget.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_set = 'virtual void SetBounds(const Rect& bounds) { bounds_ = bounds; }'
    new_set = '''virtual void SetBounds(const Rect& bounds) {
        int dx = bounds.x - bounds_.x;
        int dy = bounds.y - bounds_.y;
        bounds_ = bounds;
        if (dx != 0 || dy != 0) {
            for (auto& child : children_) {
                Rect cb = child->GetBounds();
                cb.x += dx;
                cb.y += dy;
                child->SetBounds(cb);
            }
        }
    }'''

    if old_set in content:
        content = content.replace(old_set, new_set)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Widget::SetBounds updated')
    else:
        print('old SetBounds not found')

modify_widget_setbounds()
