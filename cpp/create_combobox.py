def create_combobox():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()
    
    if 'class ComboBox : public Widget' not in content_h:
        combobox_h = '''
class ComboBox : public Widget {
public:
    explicit ComboBox(const std::string& id = "");
    void AddItem(const std::string& item) { items_.push_back(item); }
    void RemoveItem(int index);
    void ClearItems() { items_.clear(); selected_index_ = -1; }
    void SetSelectedIndex(int index) { selected_index_ = index; }
    int GetSelectedIndex() const { return selected_index_; }
    void OnSelectionChanged(IndexCallback cb) { on_selection_changed_ = std::move(cb); }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override {}

private:
    std::vector<std::string> items_;
    int selected_index_ = -1;
    IndexCallback on_selection_changed_;
    bool is_open_ = false;
};
'''
        content_h = content_h.replace('class Checkbox', combobox_h + '\nclass Checkbox')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)
            
    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    if 'ComboBox::ComboBox' not in content_cpp:
        # Need to include window_manager.h for popup
        if '#include "ui/window_manager.h"' not in content_cpp:
            content_cpp = content_cpp.replace('#include "ui/theme.h"', '#include "ui/theme.h"\n#include "ui/window_manager.h"')
            
        combobox_cpp = '''
ComboBox::ComboBox(const std::string& id) : Widget(id) {
    SetWidthPolicy(SizePolicy::Fixed);
    SetHeightPolicy(SizePolicy::Fixed);
    bounds_.width = 150;
    bounds_.height = 30;
}

void ComboBox::RemoveItem(int index) {
    if (index >= 0 && index < (int)items_.size()) {
        items_.erase(items_.begin() + index);
        if (selected_index_ == index) selected_index_ = -1;
        else if (selected_index_ > index) selected_index_--;
    }
}

void ComboBox::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Color bg = ctx.theme ? ctx.theme->GetSurface() : Color::DarkPanel;
    Color border = is_open_ ? (ctx.theme ? ctx.theme->GetPrimary() : Color::DarkAccent) : (ctx.theme ? ctx.theme->GetBorder() : Color::DarkBorder);
    int thick = is_open_ ? 2 : 1;
    int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 4;
    
    ctx.DrawFilledRoundedRect(bounds_, bg, radius);
    ctx.DrawOutlinedRect(bounds_, border, thick);
    
    std::string text = (selected_index_ >= 0 && selected_index_ < (int)items_.size()) ? items_[selected_index_] : "";
    Color text_color = ctx.theme ? ctx.theme->GetTextPrimary() : Color::White;
    DrawText(ctx, text, bounds_.x + 8, bounds_.y + (bounds_.height - 14) / 2, text_color);
    
    // Draw dropdown arrow (simple triangle)
    int arrow_x = bounds_.x + bounds_.width - 15;
    int arrow_y = bounds_.y + bounds_.height / 2 - 2;
    ctx.DrawLine(arrow_x, arrow_y, arrow_x + 4, arrow_y + 4, text_color);
    ctx.DrawLine(arrow_x + 4, arrow_y + 4, arrow_x + 8, arrow_y, text_color);
}

bool ComboBox::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (ContainsPoint(event.button.x, event.button.y)) {
            is_open_ = !is_open_;
            if (is_open_ && WindowManager::GetInstance()) {
                auto list = std::make_shared<ListView>(id_ + "_popup");
                for (const auto& item : items_) list->AddItem(item);
                list->SetBounds({0, 0, bounds_.width, (int)items_.size() * 30});
                
                // When an item is selected from popup list
                list->OnSelectionChanged([this](int idx) {
                    SetSelectedIndex(idx);
                    if (on_selection_changed_) on_selection_changed_(idx);
                    if (WindowManager::GetInstance()) WindowManager::GetInstance()->ClosePopup();
                    is_open_ = false;
                });
                
                WindowManager::GetInstance()->ShowPopup(list, bounds_.x, bounds_.y + bounds_.height);
            }
            return true;
        }
    }
    return false;
}
'''
        idx = content_cpp.find('// Checkbox')
        if idx == -1: idx = content_cpp.find('void Checkbox::SetChecked')
        content_cpp = content_cpp[:idx] + combobox_cpp + content_cpp[idx:]
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)
        print('ComboBox created')

create_combobox()
