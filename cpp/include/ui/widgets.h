#pragma once
#include "ui/widget.h"
#include <string>
#include <functional>

namespace unboundmp::ui {

class Container : public Widget {
public:
    explicit Container(const std::string& id = "") : Widget(id) {}
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;
    virtual void PerformLayout() {}
    void SetBounds(const Rect& bounds) override {
        if (bounds_.x != bounds.x || bounds_.y != bounds.y || bounds_.width != bounds.width || bounds_.height != bounds.height) layout_dirty_ = true;
        Widget::SetBounds(bounds);
    }
    Size GetPreferredSize() const override; // Override for layout
    
    // Call when children change or resize
    void InvalidateLayout() override { layout_dirty_ = true; }

protected:
    bool layout_dirty_ = true;
};

class VerticalLayout : public Container {
public:
    explicit VerticalLayout(const std::string& id = "") : Container(id) {}
    void SetSpacing(int s) { spacing_ = s; InvalidateLayout(); }
    void Update(float dt) override;
    void PerformLayout() override;
    Size GetPreferredSize() const override;
private:
    int spacing_ = 0;
};

class HorizontalLayout : public Container {
public:
    explicit HorizontalLayout(const std::string& id = "") : Container(id) {}
    void SetSpacing(int s) { spacing_ = s; InvalidateLayout(); }
    void Update(float dt) override;
    void PerformLayout() override;
    Size GetPreferredSize() const override;
private:
    int spacing_ = 0;
};

class GridLayout : public Container {
public:
    explicit GridLayout(int cols, const std::string& id = "") : Container(id), columns_(cols) {}
    void SetSpacing(int s) { spacing_ = s; InvalidateLayout(); }
    void SetColumns(int c) { columns_ = c; InvalidateLayout(); }
    void Update(float dt) override;
    void PerformLayout() override;
    Size GetPreferredSize() const override;
private:
    int columns_ = 1;
    int spacing_ = 0;
};

class AnchorLayout : public Container {
public:
    explicit AnchorLayout(const std::string& id = "") : Container(id) {}
    void Update(float dt) override;
    void PerformLayout() override;
    Size GetPreferredSize() const override;
};

class Panel : public Container {
public:
    explicit Panel(const std::string& id = "") : Container(id) {}
    void SetBackgroundColor(const Color& c) { bg_color_ = c; }
    void SetBorderColor(const Color& c) { border_color_ = c; }
    void SetBorderThickness(int t) { border_thickness_ = t; }
    void SetCornerRadius(int r) { corner_radius_ = r; }
    
    void Render(const RenderContext& ctx) override;

protected:
    Color bg_color_{0, 0, 0, 0};
    Color border_color_{0, 0, 0, 0};
    int border_thickness_ = 0;
    int corner_radius_ = 0;
};

class Window : public Panel {
public:
    explicit Window(const std::string& id = "");
    void SetTitle(const std::string& title) { title_ = title; }
    void SetCloseable(bool c) { closeable_ = c; }
    void OnClose(ClickCallback cb) { on_close_ = std::move(cb); }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void PerformLayout() override;

private:
    std::string title_;
    bool closeable_ = true;
    ClickCallback on_close_;
    bool dragging_ = false;
    int drag_offset_x_ = 0;
    int drag_offset_y_ = 0;
    const int title_bar_height_ = 28;
};


class Dialog : public Window {
public:
    explicit Dialog(const std::string& title, const std::string& message);
    void OnConfirm(ClickCallback cb) { on_confirm_ = std::move(cb); }
    void OnCancel(ClickCallback cb) { on_cancel_ = std::move(cb); }
    
    // Override to also clear modal state
    bool HandleInput(const SDL_Event& event) override;

private:
    ClickCallback on_confirm_;
    ClickCallback on_cancel_;
};

class Label : public Widget {
public:
    explicit Label(const std::string& id = "") : Widget(id) {}
    void SetText(const std::string& text) { text_ = text; }
    void SetColor(const Color& c) { color_ = c; }
    void SetAlignment(Alignment align) { alignment_ = align; }
    void SetFontSize(int size) { font_size_ = size; }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override { return false; }
    void Update(float dt) override {}

private:
    std::string text_;
    Color color_{255, 255, 255, 255};
    Alignment alignment_ = Alignment::Left;
    int font_size_ = 16;
};

class Button : public Widget {
public:
    explicit Button(const std::string& id = "") : Widget(id) {}
    void SetText(const std::string& text) { text_ = text; }
    void OnClick(ClickCallback cb) { on_click_ = std::move(cb); }
    bool IsPressed() const { return pressed_; }
    void SetFontSize(int size) { font_size_ = size; }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override {}

private:
    std::string text_;
    ClickCallback on_click_;
    bool pressed_ = false;
    Color normal_color_{100, 100, 100, 255};
    Color hover_color_{130, 130, 130, 255};
    Color pressed_color_{70, 70, 70, 255};
    Color disabled_color_{50, 50, 50, 255};
    int font_size_ = 16;
};

class ImageWidget : public Widget {
public:
    enum class ScaleMode { Fit, Fill, Stretch };
    explicit ImageWidget(const std::string& id = "") : Widget(id) {}
    void SetTexture(SDL_Texture* tex) { texture_ = tex; }
    void SetScaleMode(ScaleMode mode) { scale_mode_ = mode; }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override { return false; }
    void Update(float dt) override {}

private:
    SDL_Texture* texture_ = nullptr;
    ScaleMode scale_mode_ = ScaleMode::Fit;
};


class ComboBox : public Widget {
public:
    explicit ComboBox(const std::string& id = "");
    void AddItem(const std::string& item) { items_.push_back(item); }
    void RemoveItem(int index);
    void ClearItems() { items_.clear(); selected_index_ = -1; }
    void SetSelectedIndex(int index) { selected_index_ = index; }
    int GetSelectedIndex() const { return selected_index_; }
    std::string GetSelectedItem() const {
        if (selected_index_ >= 0 && selected_index_ < items_.size()) return items_[selected_index_];
        return "";
    }
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

class Checkbox : public Widget {
public:
    explicit Checkbox(const std::string& id = "") : Widget(id) {}
    void SetChecked(bool c);
    bool IsChecked() const { return checked_; }
    void SetLabel(const std::string& label) { label_ = label; }
    void OnChanged(BoolCallback cb) { on_changed_ = std::move(cb); }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override {}

private:
    bool checked_ = false;
    std::string label_;
    BoolCallback on_changed_;
};

class ProgressBar : public Widget {
public:
    explicit ProgressBar(const std::string& id = "") : Widget(id) {}
    void SetProgress(float p) { progress_ = p; }
    void SetFillColor(const Color& c) { fill_color_ = c; }
    void SetBackgroundColor(const Color& c) { bg_color_ = c; }
    void SetShowPercentage(bool show) { show_percentage_ = show; }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override { return false; }
    void Update(float dt) override {}

private:
    float progress_ = 0.0f;
    Color fill_color_{0, 255, 0, 255};
    Color bg_color_{50, 50, 50, 255};
    bool show_percentage_ = false;
};

class Slider : public Widget {
public:
    explicit Slider(const std::string& id = "") : Widget(id) {}
    void SetRange(float min, float max) { min_ = min; max_ = max; }
    void SetValue(float val);
    float GetValue() const { return value_; }
    void OnChanged(ValueCallback cb) { on_changed_ = std::move(cb); }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override {}

private:
    float min_ = 0.0f;
    float max_ = 100.0f;
    float value_ = 0.0f;
    bool dragging_ = false;
    ValueCallback on_changed_;
};

class TextBox : public Widget {
public:
    explicit TextBox(const std::string& id = "") : Widget(id) {}
    void SetText(const std::string& text) { text_ = text; }
    std::string GetText() const { return text_; }
    void SetPlaceholder(const std::string& p) { placeholder_ = p; }
    void OnTextChanged(TextCallback cb) { on_text_changed_ = std::move(cb); }
    void SetPassword(bool pwd) { password_ = pwd; }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    std::string text_;
    std::string placeholder_;
    TextCallback on_text_changed_;
    bool password_ = false;
    int cursor_pos_ = 0;
    float blink_timer_ = 0.0f;
    bool cursor_visible_ = false;
};

class ListView : public Widget {
public:
    explicit ListView(const std::string& id = "") : Widget(id) {}
    void AddItem(const std::string& item) { items_.push_back(item); }
    void RemoveItem(int index);
    void ClearItems() { items_.clear(); }
    int GetSelectedIndex() const { return selected_index_; }
    void OnSelectionChanged(IndexCallback cb) { on_selection_changed_ = std::move(cb); }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override {}

private:
    std::vector<std::string> items_;
    int selected_index_ = -1;
    int scroll_y_ = 0;
    IndexCallback on_selection_changed_;
    const int item_height_ = 30;
};


class TabControl : public Container {
public:
    explicit TabControl(const std::string& id = "");
    void AddTab(const std::string& title, std::shared_ptr<Widget> content);
    void SelectTab(int index);
    
    void Update(float dt) override;
    void PerformLayout() override;
    Size GetPreferredSize() const override;

private:
    std::shared_ptr<HorizontalLayout> tab_bar_;
    std::shared_ptr<Container> content_area_;
    int active_tab_ = -1;
    std::vector<std::shared_ptr<Widget>> tab_contents_;
};

class ScrollView : public Widget {
public:
    explicit ScrollView(const std::string& id = "") : Widget(id) {}
    void SetContentSize(const Size& size) { content_size_ = size; }
    void SetScrollOffset(int x, int y) { scroll_x_ = x; scroll_y_ = y; }
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    Size content_size_{0, 0};
    int scroll_x_ = 0;
    int scroll_y_ = 0;
};

} // namespace unboundmp::ui
