def fix_window_drag():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_drag = '''        bounds_.x = event.motion.x - drag_offset_x_;
        bounds_.y = event.motion.y - drag_offset_y_;
        return true;'''
    
    new_drag = '''        Rect new_bounds = bounds_;
        new_bounds.x = event.motion.x - drag_offset_x_;
        new_bounds.y = event.motion.y - drag_offset_y_;
        SetBounds(new_bounds);
        return true;'''

    if old_drag in content:
        content = content.replace(old_drag, new_drag)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Fixed window dragging.')
    else:
        print('Could not find window dragging code.')

fix_window_drag()
