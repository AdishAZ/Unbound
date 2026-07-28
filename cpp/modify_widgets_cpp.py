def modify_widgets_cpp():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'a', encoding='utf-8') as f:
        pass # just to check
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find the namespace ending
    ns_idx = content.rfind('} // namespace unboundmp::ui')
    
    new_methods = '''
Size Container::GetPreferredSize() const {
    if (children_.empty()) return {bounds_.width, bounds_.height};
    int max_w = 0, max_h = 0;
    for (const auto& child : children_) {
        if (!child->IsVisible()) continue;
        Size ps = child->GetPreferredSize();
        Margin cm = child->GetMargin();
        max_w = std::max(max_w, ps.width + cm.left + cm.right);
        max_h = std::max(max_h, ps.height + cm.top + cm.bottom);
    }
    return {max_w + padding_.left + padding_.right, max_h + padding_.top + padding_.bottom};
}

Size VerticalLayout::GetPreferredSize() const {
    int max_w = 0;
    int sum_h = 0;
    bool first = true;
    for (const auto& child : children_) {
        if (!child->IsVisible()) continue;
        Size ps = child->GetPreferredSize();
        Margin cm = child->GetMargin();
        max_w = std::max(max_w, ps.width + cm.left + cm.right);
        sum_h += ps.height + cm.top + cm.bottom;
        if (!first) sum_h += spacing_;
        first = false;
    }
    return {max_w + padding_.left + padding_.right, sum_h + padding_.top + padding_.bottom};
}

Size HorizontalLayout::GetPreferredSize() const {
    int sum_w = 0;
    int max_h = 0;
    bool first = true;
    for (const auto& child : children_) {
        if (!child->IsVisible()) continue;
        Size ps = child->GetPreferredSize();
        Margin cm = child->GetMargin();
        sum_w += ps.width + cm.left + cm.right;
        max_h = std::max(max_h, ps.height + cm.top + cm.bottom);
        if (!first) sum_w += spacing_;
        first = false;
    }
    return {sum_w + padding_.left + padding_.right, max_h + padding_.top + padding_.bottom};
}

Size GridLayout::GetPreferredSize() const {
    // simplified
    return Container::GetPreferredSize();
}

Size AnchorLayout::GetPreferredSize() const {
    return Container::GetPreferredSize();
}
'''
    content = content[:ns_idx] + new_methods + content[ns_idx:]
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

modify_widgets_cpp()
print("widgets.cpp GetPreferredSize added")
