def fix_container_update():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    old_update = '''void Container::Update(float dt) {
    for (const auto& child : children_) {
        child->Update(dt);
    }
}'''
    new_update = '''void Container::Update(float dt) {
    if (layout_dirty_) {
        PerformLayout();
        layout_dirty_ = false;
    }
    for (const auto& child : children_) {
        child->Update(dt);
    }
}'''
    if old_update in content:
        content = content.replace(old_update, new_update)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Fixed Container::Update')
    else:
        print('Could not find old Container::Update')

fix_container_update()
