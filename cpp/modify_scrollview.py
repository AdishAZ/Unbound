def modify_scrollview():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_scroll = '''void ScrollView::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    // Implementation needed
    ctx.SetClipRect(bounds_);
    
    // Draw content shifted by -scroll_x_, -scroll_y_
    
    ctx.ClearClipRect();
}

bool ScrollView::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    return false;
}

void ScrollView::Update(float dt) {
    if (!visible_) return;
}'''

    new_scroll = '''void ScrollView::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    ctx.SetClipRect(bounds_);
    
    // Shift children for rendering
    for (const auto& child : children_) {
        Rect orig = child->GetBounds();
        child->SetBounds({orig.x - scroll_x_, orig.y - scroll_y_, orig.width, orig.height});
        child->Render(ctx);
        child->SetBounds(orig); // Restore
    }
    
    ctx.ClearClipRect();
    
    // Draw Scrollbar (Vertical)
    if (content_size_.height > bounds_.height) {
        int sb_width = 8;
        int sb_x = bounds_.x + bounds_.width - sb_width - 2;
        int sb_y = bounds_.y + 2;
        int sb_height = bounds_.height - 4;
        
        ctx.DrawFilledRoundedRect({sb_x, sb_y, sb_width, sb_height}, {50, 50, 50, 150}, 4);
        
        float ratio = static_cast<float>(bounds_.height) / content_size_.height;
        int thumb_height = std::max(20, static_cast<int>(sb_height * ratio));
        float scroll_ratio = static_cast<float>(scroll_y_) / (content_size_.height - bounds_.height);
        int thumb_y = sb_y + static_cast<int>(scroll_ratio * (sb_height - thumb_height));
        
        ctx.DrawFilledRoundedRect({sb_x, thumb_y, sb_width, thumb_height}, {150, 150, 150, 255}, 4);
    }
}

bool ScrollView::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEWHEEL && ContainsPoint(event.wheel.mouseX, event.wheel.mouseY)) {
        scroll_y_ -= event.wheel.y * 20; // 20px per scroll tick
        if (scroll_y_ < 0) scroll_y_ = 0;
        if (scroll_y_ > content_size_.height - bounds_.height) {
            scroll_y_ = std::max(0, content_size_.height - bounds_.height);
        }
        return true;
    }
    
    // Pass input to children with adjusted coordinates
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        Rect orig = (*it)->GetBounds();
        (*it)->SetBounds({orig.x - scroll_x_, orig.y - scroll_y_, orig.width, orig.height});
        bool handled = (*it)->HandleInput(event);
        (*it)->SetBounds(orig);
        if (handled) return true;
    }
    return false;
}

void ScrollView::Update(float dt) {
    if (!visible_) return;
    for (const auto& child : children_) {
        child->Update(dt);
    }
}'''

    content = content.replace(old_scroll, new_scroll)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

modify_scrollview()
print("widgets.cpp updated with ScrollView")
