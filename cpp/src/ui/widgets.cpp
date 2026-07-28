#include "ui/widgets.h"
#include "ui/theme.h"
#include "ui/window_manager.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

namespace unboundmp::ui {

namespace {
    TextBox* g_active_textbox = nullptr;
}

// Container
void Container::Render(const RenderContext& ctx) {
    if (!visible_) return;
    for (const auto& child : children_) {
        child->Render(ctx);
    }
}

bool Container::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->HandleInput(event)) return true;
    }
    return false;
}

void Container::Update(float dt) {
    if (layout_dirty_) {
        PerformLayout();
        layout_dirty_ = false;
    }
    for (const auto& child : children_) {
        child->Update(dt);
    }
}

// Panel
void Panel::Render(const RenderContext& ctx) {
    if (!visible_) return;
    Color bg = bg_color_;
    if (bg.a == 0 && ctx.theme) bg = ctx.theme->GetSurface();
    
    if (bg.a > 0) {
        int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 0;
        if (corner_radius_ > 0) radius = corner_radius_;
        if (radius > 0) {
            ctx.DrawFilledRoundedRect(bounds_, bg, radius);
        } else {
            SDL_SetRenderDrawColor(ctx.renderer, bg.r, bg.g, bg.b, static_cast<Uint8>(bg.a * ctx.alpha));
            SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
            SDL_Rect r = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
            SDL_RenderFillRect(ctx.renderer, &r);
        }
    }
    
    Color border = border_color_;
    if (border.a == 0 && ctx.theme) border = ctx.theme->GetBorder();
    
    if (border_thickness_ > 0 && border.a > 0) {
        SDL_SetRenderDrawColor(ctx.renderer, border_color_.r, border_color_.g, border_color_.b, static_cast<Uint8>(border_color_.a * ctx.alpha));
        for (int i = 0; i < border_thickness_; ++i) {
            SDL_Rect r = {bounds_.x + i, bounds_.y + i, bounds_.width - 2 * i, bounds_.height - 2 * i};
            SDL_RenderDrawRect(ctx.renderer, &r);
        }
    }
    Container::Render(ctx);
}

// Window
Window::Window(const std::string& id) : Panel(id) {}

void Window::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    // Drop shadow (simple implementation)
    ctx.DrawFilledRoundedRect({bounds_.x + 4, bounds_.y + 4, bounds_.width, bounds_.height}, {0, 0, 0, 100}, 8);
    
    Panel::Render(ctx); // Renders background and borders using Theme
    
    Color title_bg = ctx.theme ? ctx.theme->GetBackground() : Color::FromHex(0x1e2227);
    Color text_color = ctx.theme ? ctx.theme->GetTextPrimary() : Color::White;
    
    // Draw title bar
    Rect title_rect = {bounds_.x, bounds_.y, bounds_.width, title_bar_height_};
    ctx.DrawFilledRoundedRect(title_rect, title_bg, 4); 
    // Fill the bottom half of the rounded rect to make it flat on bottom
    ctx.DrawFilledRect({bounds_.x, bounds_.y + 4, bounds_.width, title_bar_height_ - 4}, title_bg);
    
    // Title text
    DrawText(ctx, title_, bounds_.x + 10, bounds_.y + (title_bar_height_ - 14) / 2, text_color);
    
    if (closeable_) {
        Color close_bg = ctx.theme ? ctx.theme->GetError() : Color::FromHex(0xef4444);
        ctx.DrawFilledRoundedRect({bounds_.x + bounds_.width - 30, bounds_.y + 4, 20, 20}, close_bg, 4);
        DrawText(ctx, "X", bounds_.x + bounds_.width - 24, bounds_.y + 7, text_color);
    }
}

bool Window::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mx = event.button.x;
        int my = event.button.y;
        
        if (closeable_ && mx >= bounds_.x + bounds_.width - 30 && mx <= bounds_.x + bounds_.width - 10 &&
            my >= bounds_.y + 4 && my <= bounds_.y + 24) {
            if (on_close_) on_close_();
            return true;
        }
        
        if (mx >= bounds_.x && mx <= bounds_.x + bounds_.width &&
            my >= bounds_.y && my <= bounds_.y + title_bar_height_) {
            dragging_ = true;
            drag_offset_x_ = mx - bounds_.x;
            drag_offset_y_ = my - bounds_.y;
            if (dragging_ && WindowManager::GetInstance()) {
                WindowManager::GetInstance()->BringToFront(this);
            }
            return true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        dragging_ = false;
    } else if (event.type == SDL_MOUSEMOTION && dragging_) {
        int new_x = event.motion.x - drag_offset_x_;
        int new_y = event.motion.y - drag_offset_y_;
        SetBounds({new_x, new_y, bounds_.width, bounds_.height});
        return true;
    }
    
    // Check children (for clicks inside window)
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->HandleInput(event)) return true;
    }
    
    return false;
}

void Window::PerformLayout() {
    for (auto& child : children_) {
        if (!child->IsVisible()) continue;
        Padding p = GetPadding();
        Margin m = child->GetMargin();
        int cx = bounds_.x + p.left + m.left;
        int cy = bounds_.y + title_bar_height_ + p.top + m.top;
        int cw = bounds_.width - p.left - p.right - m.left - m.right;
        int ch = bounds_.height - title_bar_height_ - p.top - p.bottom - m.top - m.bottom;
        child->SetBounds({cx, cy, cw, ch});
    }
}

Dialog::Dialog(const std::string& title, const std::string& message) : Window("dialog") {
    SetTitle(title);
    SetBounds({0, 0, 300, 150});
    
    auto vert = std::make_shared<VerticalLayout>("dialog_vert");
    vert->SetBounds({0, 0, 300, 150});
    vert->SetPadding(Padding::All(20));
    vert->SetSpacing(20);
    
    auto msg_label = std::make_shared<Label>();
    msg_label->SetText(message);
    msg_label->SetAlignment(Alignment::Center);
    msg_label->SetBounds({0, 0, 260, 40}); // Will wrap or size correctly
    
    auto btn_layout = std::make_shared<HorizontalLayout>("dialog_btns");
    btn_layout->SetBounds({0, 0, 260, 30});
    btn_layout->SetSpacing(20);
    
    auto ok_btn = std::make_shared<Button>("dialog_ok");
    ok_btn->SetText("OK");
    ok_btn->SetBounds({0, 0, 120, 30});
    ok_btn->OnClick([this]() {
        if (on_confirm_) on_confirm_();
        if (WindowManager::GetInstance()) WindowManager::GetInstance()->ClearModal();
        SetVisible(false);
    });
    
    auto cancel_btn = std::make_shared<Button>("dialog_cancel");
    cancel_btn->SetText("Cancel");
    cancel_btn->SetBounds({0, 0, 120, 30});
    cancel_btn->OnClick([this]() {
        if (on_cancel_) on_cancel_();
        if (WindowManager::GetInstance()) WindowManager::GetInstance()->ClearModal();
        SetVisible(false);
    });
    
    btn_layout->AddChild(ok_btn);
    btn_layout->AddChild(cancel_btn);
    
    vert->AddChild(msg_label);
    vert->AddChild(btn_layout);
    
    AddChild(vert);
}

bool Dialog::HandleInput(const SDL_Event& event) {
    bool handled = Window::HandleInput(event);
    if (!IsVisible() && WindowManager::GetInstance()) { // Closed via 'X' titlebar
        WindowManager::GetInstance()->ClearModal();
    }
    return handled;
}
// Label
void Label::Render(const RenderContext& ctx) {
    if (!visible_) return;
    int tx = bounds_.x;
    int ty = bounds_.y;
    
    if (alignment_ == Alignment::Center || alignment_ == Alignment::TopCenter || alignment_ == Alignment::BottomCenter) {
        tx += bounds_.width / 2;
    } else if (alignment_ == Alignment::Right || alignment_ == Alignment::TopRight || alignment_ == Alignment::BottomRight) {
        tx += bounds_.width;
    }
    
    if (alignment_ == Alignment::Left || alignment_ == Alignment::Center || alignment_ == Alignment::Right) {
        ty += bounds_.height / 2;
    } else if (alignment_ == Alignment::BottomLeft || alignment_ == Alignment::BottomCenter || alignment_ == Alignment::BottomRight) {
        ty += bounds_.height;
    }
    
    DrawTextWrapped(ctx, text_, tx, ty, bounds_.width, color_, "default", font_size_, alignment_);
}

// Button
void Button::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Color c = normal_color_;
    if (!enabled_) c = disabled_color_;
    else if (pressed_) c = pressed_color_;
    else if (hovered_) c = hover_color_;
    
    SDL_SetRenderDrawColor(ctx.renderer, c.r, c.g, c.b, static_cast<Uint8>(c.a * ctx.alpha));
    SDL_Rect r = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_RenderFillRect(ctx.renderer, &r);
    
    Color text_color{255, 255, 255, 255};
    int tw = MeasureTextWidth(ctx, text_);
    DrawText(ctx, text_, bounds_.x + (bounds_.width - tw) / 2, bounds_.y + (bounds_.height - 14) / 2, text_color);
}

bool Button::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEMOTION) {
        hovered_ = ContainsPoint(event.motion.x, event.motion.y);
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (hovered_) {
            pressed_ = true;
            return true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (pressed_) {
            pressed_ = false;
            if (hovered_ && on_click_) {
                on_click_();
            }
            return true;
        }
    }
    return false;
}

// ImageWidget
void ImageWidget::Render(const RenderContext& ctx) {
    if (!visible_ || !texture_) return;
    
    SDL_Rect dst = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    
    if (scale_mode_ == ScaleMode::Fit || scale_mode_ == ScaleMode::Fill) {
        int w, h;
        SDL_QueryTexture(texture_, nullptr, nullptr, &w, &h);
        float scale_x = (float)bounds_.width / w;
        float scale_y = (float)bounds_.height / h;
        float scale = (scale_mode_ == ScaleMode::Fit) ? std::min(scale_x, scale_y) : std::max(scale_x, scale_y);
        
        dst.w = static_cast<int>(w * scale);
        dst.h = static_cast<int>(h * scale);
        dst.x = bounds_.x + (bounds_.width - dst.w) / 2;
        dst.y = bounds_.y + (bounds_.height - dst.h) / 2;
    }
    
    SDL_SetTextureAlphaMod(texture_, static_cast<Uint8>(255 * ctx.alpha));
    SDL_RenderCopy(ctx.renderer, texture_, nullptr, &dst);
}


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
// Checkbox
void Checkbox::SetChecked(bool c) {
    if (checked_ != c) {
        checked_ = c;
        if (on_changed_) on_changed_(checked_);
    }
}

void Checkbox::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    SDL_Rect box = {bounds_.x, bounds_.y + (bounds_.height - 20) / 2, 20, 20};
    SDL_SetRenderDrawColor(ctx.renderer, 255, 255, 255, static_cast<Uint8>(255 * ctx.alpha));
    SDL_RenderDrawRect(ctx.renderer, &box);
    
    if (checked_) {
        SDL_SetRenderDrawColor(ctx.renderer, 50, 150, 255, static_cast<Uint8>(255 * ctx.alpha));
        SDL_Rect fill = {box.x + 4, box.y + 4, 12, 12};
        SDL_RenderFillRect(ctx.renderer, &fill);
    }
    
    Color text_color{255, 255, 255, 255};
    DrawText(ctx, label_, box.x + 25, bounds_.y + (bounds_.height - 14) / 2, text_color);
}

bool Checkbox::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (ContainsPoint(event.button.x, event.button.y)) {
            SetChecked(!checked_);
            return true;
        }
    }
    return false;
}

// ProgressBar
void ProgressBar::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    SDL_SetRenderDrawColor(ctx.renderer, bg_color_.r, bg_color_.g, bg_color_.b, static_cast<Uint8>(bg_color_.a * ctx.alpha));
    SDL_Rect bg = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_RenderFillRect(ctx.renderer, &bg);
    
    if (progress_ > 0) {
        SDL_SetRenderDrawColor(ctx.renderer, fill_color_.r, fill_color_.g, fill_color_.b, static_cast<Uint8>(fill_color_.a * ctx.alpha));
        SDL_Rect fg = {bounds_.x, bounds_.y, static_cast<int>(bounds_.width * progress_), bounds_.height};
        SDL_RenderFillRect(ctx.renderer, &fg);
    }
    
    if (show_percentage_) {
        std::string text = std::to_string(static_cast<int>(progress_ * 100)) + "%";
        Color text_color{255, 255, 255, 255};
        int tw = MeasureTextWidth(ctx, text);
        DrawText(ctx, text, bounds_.x + (bounds_.width - tw) / 2, bounds_.y + (bounds_.height - 14) / 2, text_color);
    }
}

// Slider
void Slider::SetValue(float val) {
    float new_val = std::clamp(val, min_, max_);
    if (value_ != new_val) {
        value_ = new_val;
        if (on_changed_) on_changed_(value_);
    }
}

void Slider::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    // Draw track
    SDL_SetRenderDrawColor(ctx.renderer, 100, 100, 100, static_cast<Uint8>(255 * ctx.alpha));
    SDL_Rect track = {bounds_.x, bounds_.y + bounds_.height / 2 - 1, bounds_.width, 2};
    SDL_RenderFillRect(ctx.renderer, &track);
    
    // Draw thumb
    float t = (max_ > min_) ? (value_ - min_) / (max_ - min_) : 0.0f;
    int thumb_x = bounds_.x + static_cast<int>(t * (bounds_.width - 14));
    SDL_SetRenderDrawColor(ctx.renderer, 200, 200, 200, static_cast<Uint8>(255 * ctx.alpha));
    SDL_Rect thumb = {thumb_x, bounds_.y + (bounds_.height - 20) / 2, 14, 20};
    SDL_RenderFillRect(ctx.renderer, &thumb);
}

bool Slider::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (ContainsPoint(event.button.x, event.button.y)) {
            dragging_ = true;
            float t = static_cast<float>(event.button.x - bounds_.x) / bounds_.width;
            SetValue(min_ + t * (max_ - min_));
            return true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        dragging_ = false;
    } else if (event.type == SDL_MOUSEMOTION && dragging_) {
        float t = static_cast<float>(event.motion.x - bounds_.x) / bounds_.width;
        SetValue(min_ + t * (max_ - min_));
        return true;
    }
    return false;
}

// TextBox
void TextBox::Update(float dt) {
    if (g_active_textbox != this) {
        focused_ = false;
    }
    
    if (focused_) {
        blink_timer_ += dt;
        if (blink_timer_ >= 0.5f) {
            blink_timer_ -= 0.5f;
            cursor_visible_ = !cursor_visible_;
        }
    } else {
        cursor_visible_ = false;
        blink_timer_ = 0.0f;
    }
}

void TextBox::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 30, static_cast<Uint8>(255 * ctx.alpha));
    SDL_Rect bg = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_RenderFillRect(ctx.renderer, &bg);
    
    SDL_SetRenderDrawColor(ctx.renderer, focused_ ? 100 : 50, focused_ ? 200 : 50, focused_ ? 255 : 50, static_cast<Uint8>(255 * ctx.alpha));
    SDL_RenderDrawRect(ctx.renderer, &bg);
    
    Color text_color{255, 255, 255, 255};
    std::string display_text = text_;
    if (password_) {
        display_text = std::string(text_.length(), '*');
    }
    
    if (text_.empty() && !focused_) {
        text_color = {150, 150, 150, 255};
        display_text = placeholder_;
    }
    
    DrawText(ctx, display_text, bounds_.x + 5, bounds_.y + (bounds_.height - 14) / 2, text_color);
    
    if (focused_ && cursor_visible_) {
        int cx = bounds_.x + 5 + MeasureTextWidth(ctx, display_text.substr(0, cursor_pos_));
        SDL_SetRenderDrawColor(ctx.renderer, 255, 255, 255, static_cast<Uint8>(255 * ctx.alpha));
        SDL_Rect cursor = {cx, bounds_.y + (bounds_.height - 14) / 2, 2, 14};
        SDL_RenderFillRect(ctx.renderer, &cursor);
    }
}

bool TextBox::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (ContainsPoint(event.button.x, event.button.y)) {
            g_active_textbox = this;
            focused_ = true;
            return true;
        } else {
            focused_ = false;
            if (g_active_textbox == this) g_active_textbox = nullptr;
        }
    }
    
    if (!focused_) return false;
    
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_BACKSPACE && cursor_pos_ > 0) {
            text_.erase(cursor_pos_ - 1, 1);
            cursor_pos_--;
            if (on_text_changed_) on_text_changed_(text_);
        } else if (event.key.keysym.sym == SDLK_DELETE && cursor_pos_ < text_.length()) {
            text_.erase(cursor_pos_, 1);
            if (on_text_changed_) on_text_changed_(text_);
        } else if (event.key.keysym.sym == SDLK_LEFT && cursor_pos_ > 0) {
            cursor_pos_--;
        } else if (event.key.keysym.sym == SDLK_RIGHT && cursor_pos_ < text_.length()) {
            cursor_pos_++;
        } else if (event.key.keysym.sym == SDLK_HOME) {
            cursor_pos_ = 0;
        } else if (event.key.keysym.sym == SDLK_END) {
            cursor_pos_ = static_cast<int>(text_.length());
        }
        return true;
    } else if (event.type == SDL_TEXTINPUT) {
        text_.insert(cursor_pos_, event.text.text);
        cursor_pos_ += std::strlen(event.text.text);
        if (on_text_changed_) on_text_changed_(text_);
        return true;
    }
    
    return false;
}

// ListView
void ListView::RemoveItem(int index) {
    if (index >= 0 && index < items_.size()) {
        items_.erase(items_.begin() + index);
        if (selected_index_ == index) {
            selected_index_ = -1;
            if (on_selection_changed_) on_selection_changed_(-1);
        } else if (selected_index_ > index) {
            selected_index_--;
        }
    }
}

void ListView::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Color bg = ctx.theme ? ctx.theme->GetSurface() : Color::DarkPanel;
    Color border = ctx.theme ? ctx.theme->GetBorder() : Color::DarkBorder;
    Color sel_color = ctx.theme ? ctx.theme->GetPrimary() : Color::DarkAccent;
    Color text_color = ctx.theme ? ctx.theme->GetTextPrimary() : Color::White;
    int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 4;
    
    ctx.DrawFilledRoundedRect(bounds_, bg, radius);
    ctx.DrawOutlinedRect(bounds_, border, 1);
    
    ctx.SetClipRect(bounds_);
    
    for (size_t i = 0; i < items_.size(); ++i) {
        int item_y = bounds_.y + static_cast<int>(i) * item_height_ - scroll_y_;
        if (item_y + item_height_ < bounds_.y || item_y > bounds_.y + bounds_.height) continue;
        
        if (static_cast<int>(i) == selected_index_) {
            ctx.DrawFilledRect({bounds_.x, item_y, bounds_.width, item_height_}, sel_color);
        } else {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            if (mx >= bounds_.x && mx <= bounds_.x + bounds_.width && my >= item_y && my <= item_y + item_height_) {
                Color hover_bg = ctx.theme ? ctx.theme->GetSurfaceHover() : Color::FromHex(0x3a404c);
                ctx.DrawFilledRect({bounds_.x, item_y, bounds_.width, item_height_}, hover_bg);
            }
        }
        
        DrawText(ctx, items_[i], bounds_.x + 10, item_y + (item_height_ - 14) / 2, text_color);
    }
    
    ctx.ClearClipRect();
    
    int total_height = static_cast<int>(items_.size()) * item_height_;
    if (total_height > bounds_.height) {
        Color sb_bg = ctx.theme ? ctx.theme->GetSecondary() : Color::FromHex(0x4b5363);
        float scroll_ratio = static_cast<float>(bounds_.height) / total_height;
        int sb_height = std::max(20, static_cast<int>(bounds_.height * scroll_ratio));
        float pos_ratio = static_cast<float>(scroll_y_) / (total_height - bounds_.height);
        int sb_y = bounds_.y + static_cast<int>(pos_ratio * (bounds_.height - sb_height));
        ctx.DrawFilledRoundedRect({bounds_.x + bounds_.width - 10, sb_y, 8, sb_height}, sb_bg, 4);
    }
}

bool ListView::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (ContainsPoint(event.button.x, event.button.y)) {
            int click_y = event.button.y - bounds_.y + scroll_y_;
            int clicked_index = click_y / item_height_;
            if (clicked_index >= 0 && clicked_index < items_.size()) {
                selected_index_ = clicked_index;
                if (on_selection_changed_) on_selection_changed_(selected_index_);
            }
            return true;
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        if (ContainsPoint(event.wheel.mouseX, event.wheel.mouseY) || true) { // SDL doesn't give mouse pos in wheel event reliably, assuming hover
            scroll_y_ -= event.wheel.y * 20;
            int total_height = static_cast<int>(items_.size()) * item_height_;
            scroll_y_ = std::clamp(scroll_y_, 0, std::max(0, total_height - bounds_.height));
            return true;
        }
    } else if (event.type == SDL_KEYDOWN && focused_) {
        if (event.key.keysym.sym == SDLK_UP && selected_index_ > 0) {
            selected_index_--;
            if (on_selection_changed_) on_selection_changed_(selected_index_);
            return true;
        } else if (event.key.keysym.sym == SDLK_DOWN && selected_index_ < static_cast<int>(items_.size()) - 1) {
            selected_index_++;
            if (on_selection_changed_) on_selection_changed_(selected_index_);
            return true;
        }
    }
    
    return false;
}

// ScrollView
void ScrollView::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    SDL_Rect clip = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_Rect prev_clip;
    SDL_RenderGetClipRect(ctx.renderer, &prev_clip);
    SDL_RenderSetClipRect(ctx.renderer, &clip);
    
    // Create a modified context with offset for children
    for (const auto& child : children_) {
        // Temp shift child bounds (in real UI we'd apply transform to context or move bounds)
        Rect cb = child->GetBounds();
        child->SetPosition(cb.x - scroll_x_, cb.y - scroll_y_);
        child->Render(ctx);
        child->SetBounds(cb); // restore
    }
    
    SDL_RenderSetClipRect(ctx.renderer, SDL_RectEmpty(&prev_clip) ? nullptr : &prev_clip);
    
    // Draw scrollbars if needed
    SDL_SetRenderDrawColor(ctx.renderer, 100, 100, 100, 150);
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    if (content_size_.height > bounds_.height) {
        float h_ratio = static_cast<float>(bounds_.height) / content_size_.height;
        int sh = std::max(20, static_cast<int>(bounds_.height * h_ratio));
        float p = static_cast<float>(scroll_y_) / (content_size_.height - bounds_.height);
        SDL_Rect sb = {bounds_.x + bounds_.width - 8, bounds_.y + static_cast<int>(p * (bounds_.height - sh)), 8, sh};
        SDL_RenderFillRect(ctx.renderer, &sb);
    }
    if (content_size_.width > bounds_.width) {
        float w_ratio = static_cast<float>(bounds_.width) / content_size_.width;
        int sw = std::max(20, static_cast<int>(bounds_.width * w_ratio));
        float p = static_cast<float>(scroll_x_) / (content_size_.width - bounds_.width);
        SDL_Rect sb = {bounds_.x + static_cast<int>(p * (bounds_.width - sw)), bounds_.y + bounds_.height - 8, sw, 8};
        SDL_RenderFillRect(ctx.renderer, &sb);
    }
}

bool ScrollView::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEWHEEL) {
        if (event.wheel.y != 0) {
            scroll_y_ -= event.wheel.y * 30;
            scroll_y_ = std::clamp(scroll_y_, 0, std::max(0, content_size_.height - bounds_.height));
            return true;
        }
        if (event.wheel.x != 0) {
            scroll_x_ += event.wheel.x * 30;
            scroll_x_ = std::clamp(scroll_x_, 0, std::max(0, content_size_.width - bounds_.width));
            return true;
        }
    }
    
    // Translate event coords for children
    SDL_Event offset_event = event;
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        offset_event.button.x += scroll_x_;
        offset_event.button.y += scroll_y_;
    } else if (event.type == SDL_MOUSEMOTION) {
        offset_event.motion.x += scroll_x_;
        offset_event.motion.y += scroll_y_;
    }
    
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->HandleInput(offset_event)) return true;
    }
    
    return false;
}

void ScrollView::Update(float dt) {
    for (const auto& child : children_) {
        child->Update(dt);
    }
}




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
    
    // First pass: calculate total fixed width and count expanding children
    int total_fixed_width = 0;
    int expand_count = 0;
    for (auto& child : children_) {
        if (!child->IsVisible()) continue;
        Margin cm = child->GetMargin();
        if (child->GetWidthPolicy() == SizePolicy::Expand) {
            expand_count++;
            total_fixed_width += cm.left + cm.right + spacing_;
        } else {
            total_fixed_width += child->GetBounds().width + cm.left + cm.right + spacing_;
        }
    }
    // Remove last spacing
    if (!children_.empty() && total_fixed_width > 0) total_fixed_width -= spacing_;
    
    int available_width = bounds_.width - padding_.left - padding_.right;
    int expand_width = 0;
    if (expand_count > 0 && available_width > total_fixed_width) {
        expand_width = (available_width - total_fixed_width) / expand_count;
    }
    
    for (auto& child : children_) {
        if (!child->IsVisible()) continue;
        
        Rect cb = child->GetBounds();
        Margin cm = child->GetMargin();
        
        int w = cb.width;
        int h = cb.height;
        
        if (child->GetWidthPolicy() == SizePolicy::Expand) {
            w = expand_width;
        }
        
        if (child->GetHeightPolicy() == SizePolicy::Expand) {
            h = bounds_.height - padding_.top - padding_.bottom - cm.top - cm.bottom;
        }
        
        child->SetBounds({current_x + cm.left, bounds_.y + padding_.top + cm.top, w, h});
        current_x += w + cm.left + cm.right + spacing_;
    }
}


TabControl::TabControl(const std::string& id) : Container(id) {
    tab_bar_ = std::make_shared<HorizontalLayout>(id + "_bar");
    tab_bar_->SetHeightPolicy(SizePolicy::Fixed);
    tab_bar_->SetBounds({0, 0, 0, 30});
    
    content_area_ = std::make_shared<Panel>(id + "_content");
    content_area_->SetWidthPolicy(SizePolicy::Expand);
    content_area_->SetHeightPolicy(SizePolicy::Expand);
    
    AddChild(tab_bar_);
    AddChild(content_area_);
}

void TabControl::AddTab(const std::string& title, std::shared_ptr<Widget> content) {
    int index = (int)tab_contents_.size();
    
    auto btn = std::make_shared<Button>(id_ + "_btn_" + std::to_string(index));
    btn->SetText(title);
    btn->SetWidthPolicy(SizePolicy::Fixed);
    btn->SetBounds({0, 0, 100, 30});
    
    btn->OnClick([this, index]() {
        SelectTab(index);
    });
    
    tab_bar_->AddChild(btn);
    content_area_->AddChild(content);
    tab_contents_.push_back(content);
    
    if (active_tab_ == -1) {
        SelectTab(0);
    } else {
        content->SetVisible(false);
    }
}

void TabControl::SelectTab(int index) {
    if (index < 0 || index >= (int)tab_contents_.size()) return;
    active_tab_ = index;
    for (int i = 0; i < (int)tab_contents_.size(); ++i) {
        tab_contents_[i]->SetVisible(i == index);
    }
    InvalidateLayout();
}

void TabControl::Update(float dt) {
    Container::Update(dt);
}

void TabControl::PerformLayout() {
    int w = bounds_.width;
    int h = bounds_.height;
    
    tab_bar_->SetBounds({bounds_.x, bounds_.y, w, 30});
    content_area_->SetBounds({bounds_.x, bounds_.y + 30, w, h - 30});
    
    // Size contents to fill content area
    for (auto& content : tab_contents_) {
        content->SetBounds({bounds_.x, bounds_.y + 30, w, h - 30});
    }
}

Size TabControl::GetPreferredSize() const {
    int max_w = 0;
    int max_h = 30; // bar height
    for (auto& content : tab_contents_) {
        Size ps = content->GetPreferredSize();
        max_w = std::max(max_w, ps.width);
        max_h = std::max(max_h, 30 + ps.height);
    }
    return {max_w, max_h};
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
} // namespace unboundmp::ui
