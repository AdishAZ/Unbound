def edit_widget_cpp():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widget.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    if '#include "ui/text_renderer.h"' not in content:
        content = content.replace('#include "ui/widget.h"', '#include "ui/widget.h"\n#include "ui/text_renderer.h"\n#include "utils/logger.h"')

    old_impl = '''void Widget::DrawText(const RenderContext& ctx, const std::string& text, int x, int y, const Color& color, int char_width, int char_height) {
    if (!ctx.renderer || color.a == 0) return;
    
    SDL_SetRenderDrawColor(ctx.renderer, color.r, color.g, color.b, static_cast<Uint8>(color.a * 1.0f));
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    
    int cx = x;
    for (char c : text) {
        if (c > ' ' && c <= '~') { // printable characters
            SDL_Rect r = {cx + 2, y + 2, char_width - 4, char_height - 4};
            SDL_RenderFillRect(ctx.renderer, &r);
        }
        cx += char_width;
    }
}

int Widget::MeasureTextWidth(const std::string& text, int char_width) {
    return static_cast<int>(text.length()) * char_width;
}'''

    new_impl = '''void Widget::DrawText(const RenderContext& ctx, const std::string& text, int x, int y, const Color& color, const std::string& font, int size, Alignment align) {
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
}'''

    content = content.replace(old_impl, new_impl)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_widget_cpp()
print("widget.cpp modified")
