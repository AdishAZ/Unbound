def append_layouts():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'a', encoding='utf-8') as f:
        f.write('''

// VerticalLayout
void VerticalLayout::Update(float dt) {
    Container::Update(dt);
}

void VerticalLayout::PerformLayout() {
    int current_y = bounds_.y + padding_.top;
    for (auto& child : children_) {
        if (!child->IsVisible()) continue;
        
        Rect cb = child->GetBounds();
        Padding cp = child->GetPadding();
        Margin cm = child->GetMargin();
        
        int w = cb.width;
        int h = cb.height;
        
        if (child->GetWidthPolicy() == SizePolicy::Expand) {
            w = bounds_.width - padding_.left - padding_.right - cm.left - cm.right;
        }
        
        child->SetBounds({bounds_.x + padding_.left + cm.left, current_y + cm.top, w, h});
        current_y += h + cm.top + cm.bottom + spacing_;
    }
}

// HorizontalLayout
void HorizontalLayout::Update(float dt) {
    Container::Update(dt);
}

void HorizontalLayout::PerformLayout() {
    int current_x = bounds_.x + padding_.left;
    for (auto& child : children_) {
        if (!child->IsVisible()) continue;
        
        Rect cb = child->GetBounds();
        Padding cp = child->GetPadding();
        Margin cm = child->GetMargin();
        
        int w = cb.width;
        int h = cb.height;
        
        if (child->GetHeightPolicy() == SizePolicy::Expand) {
            h = bounds_.height - padding_.top - padding_.bottom - cm.top - cm.bottom;
        }
        
        child->SetBounds({current_x + cm.left, bounds_.y + padding_.top + cm.top, w, h});
        current_x += w + cm.left + cm.right + spacing_;
    }
}

// GridLayout
void GridLayout::Update(float dt) {
    Container::Update(dt);
}

void GridLayout::PerformLayout() {
    if (columns_ <= 0) return;
    
    int cell_w = (bounds_.width - padding_.left - padding_.right - spacing_ * (columns_ - 1)) / columns_;
    int col = 0;
    int current_x = bounds_.x + padding_.left;
    int current_y = bounds_.y + padding_.top;
    int max_row_h = 0;
    
    for (auto& child : children_) {
        if (!child->IsVisible()) continue;
        
        Margin cm = child->GetMargin();
        int w = child->GetWidthPolicy() == SizePolicy::Expand ? cell_w - cm.left - cm.right : child->GetBounds().width;
        int h = child->GetBounds().height;
        
        child->SetBounds({current_x + cm.left, current_y + cm.top, w, h});
        
        max_row_h = std::max(max_row_h, h + cm.top + cm.bottom);
        current_x += cell_w + spacing_;
        col++;
        
        if (col >= columns_) {
            col = 0;
            current_x = bounds_.x + padding_.left;
            current_y += max_row_h + spacing_;
            max_row_h = 0;
        }
    }
}

// AnchorLayout
void AnchorLayout::Update(float dt) {
    Container::Update(dt);
}

void AnchorLayout::PerformLayout() {
    for (auto& child : children_) {
        if (!child->IsVisible()) continue;
        
        Rect cb = child->GetBounds();
        Margin cm = child->GetMargin();
        AnchorPoint anchor = child->GetAnchor();
        
        int x = cb.x;
        int y = cb.y;
        
        if (anchor == AnchorPoint::Center) {
            x = bounds_.x + (bounds_.width - cb.width) / 2;
            y = bounds_.y + (bounds_.height - cb.height) / 2;
        } else if (anchor == AnchorPoint::TopCenter) {
            x = bounds_.x + (bounds_.width - cb.width) / 2;
            y = bounds_.y + padding_.top + cm.top;
        } else if (anchor == AnchorPoint::BottomCenter) {
            x = bounds_.x + (bounds_.width - cb.width) / 2;
            y = bounds_.y + bounds_.height - cb.height - padding_.bottom - cm.bottom;
        } else if (anchor == AnchorPoint::TopLeft) {
            x = bounds_.x + padding_.left + cm.left;
            y = bounds_.y + padding_.top + cm.top;
        }
        
        child->SetBounds({x, y, cb.width, cb.height});
    }
}
''')

append_layouts()
print("widgets.cpp updated with Layout implementations")
