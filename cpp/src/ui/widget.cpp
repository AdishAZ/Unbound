#include "ui/widget.h"
#include "ui/text_renderer.h"

#include <algorithm>

namespace unboundmp::ui {

Widget::Widget(const std::string& id) : id_(id) {}

void Widget::SetPosition(int x, int y) {
    bounds_.x = x;
    bounds_.y = y;
}

void Widget::SetSize(int w, int h) {
    bounds_.width = w;
    bounds_.height = h;
}

void Widget::AddChild(std::shared_ptr<Widget> child) {
    if (child) {
        child->parent_ = this;
        children_.push_back(std::move(child));
        InvalidateLayout();
    }
}

void Widget::RemoveChild(const std::string& id) {
    auto it = std::remove_if(children_.begin(), children_.end(),
        [&id](const std::shared_ptr<Widget>& child) { return child->GetId() == id; });
    if (it != children_.end()) {
        children_.erase(it, children_.end());
        InvalidateLayout();
    }
}

void Widget::ClearChildren() {
    if (!children_.empty()) {
        children_.clear();
        InvalidateLayout();
    }
}

Widget* Widget::FindChild(const std::string& id) {
    for (const auto& child : children_) {
        if (child->GetId() == id) {
            return child.get();
        }
        if (Widget* found = child->FindChild(id)) {
            return found;
        }
    }
    return nullptr;
}

void Widget::DrawText(const RenderContext& ctx, const std::string& text, int x, int y, const Color& color, const std::string& font, int size, Alignment align) {
    if (ctx.text_renderer) {
        ctx.text_renderer->DrawText(ctx, text, font, size, x, y, color, align);
    }
}

void Widget::DrawTextWrapped(const RenderContext& ctx, const std::string& text, int x, int y, int wrap_width, const Color& color, const std::string& font, int size, Alignment align) {
    if (ctx.text_renderer) {
        ctx.text_renderer->DrawTextWrapped(ctx, text, font, size, x, y, wrap_width, color, align);
    }
}

int Widget::MeasureTextWidth(const RenderContext& ctx, const std::string& text, const std::string& font, int size) {
    if (ctx.text_renderer) {
        return ctx.text_renderer->MeasureTextWidth(text, font, size);
    }
    return static_cast<int>(text.length()) * 8; // fallback
}

void Widget::MeasureText(const RenderContext& ctx, const std::string& text, int& w, int& h, const std::string& font, int size) {
    if (ctx.text_renderer) {
        ctx.text_renderer->MeasureText(text, font, size, w, h);
    } else {
        w = static_cast<int>(text.length()) * 8;
        h = 16;
    }
}

bool Widget::ContainsPoint(int x, int y) const {
    return x >= bounds_.x && x < bounds_.x + bounds_.width &&
           y >= bounds_.y && y < bounds_.y + bounds_.height;
}

} // namespace unboundmp::ui
