def update_animation_cpp():
    path = 'd:/Unbound/pokemon/cpp/src/ui/animation.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Fix FadeAnimation
    content = content.replace(
        '// target_->SetAlpha(start_alpha_ + (end_alpha_ - start_alpha_) * progress);',
        'target_->SetAlpha(start_alpha_ + (end_alpha_ - start_alpha_) * progress);'
    )
    
    # Fix SlideAnimation
    old_slide = '''    // target_->SetPosition(Point{
    //     static_cast<int>(start_pos_.x + (end_pos_.x - start_pos_.x) * progress),
    //     static_cast<int>(start_pos_.y + (end_pos_.y - start_pos_.y) * progress)
    // });'''
    new_slide = '''    target_->SetPosition(
        static_cast<int>(start_pos_.x + (end_pos_.x - start_pos_.x) * progress),
        static_cast<int>(start_pos_.y + (end_pos_.y - start_pos_.y) * progress)
    );'''
    content = content.replace(old_slide, new_slide)
    
    # Fix ScaleAnimation
    old_scale = '''    // target_->SetSize(Size{
    //     static_cast<int>(start_size_.w + (end_size_.w - start_size_.w) * progress),
    //     static_cast<int>(start_size_.h + (end_size_.h - start_size_.h) * progress)
    // });'''
    new_scale = '''    target_->SetSize(
        static_cast<int>(start_size_.width + (end_size_.width - start_size_.width) * progress),
        static_cast<int>(start_size_.height + (end_size_.height - start_size_.height) * progress)
    );'''
    content = content.replace(old_scale, new_scale)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

update_animation_cpp()
print("animation.cpp updated")
