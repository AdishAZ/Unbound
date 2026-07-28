def create_text_renderer_cpp():
    content = '''#include "ui/text_renderer.h"
#include "utils/logger.h"

namespace unboundmp::ui {

TextRenderer::TextRenderer(FontManager* font_manager) : font_manager_(font_manager) {}

std::string TextRenderer::GenerateCacheKey(const std::string& text, const std::string& font_path, int size, const Color& color, int wrap_width) const {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s_%d_%d_%d_%d_%d_%d", font_path.c_str(), size, color.r, color.g, color.b, color.a, wrap_width);
    return std::string(buf) + "_" + text;
}

CachedText* TextRenderer::GetOrRenderText(SDL_Renderer* renderer, const std::string& text, const std::string& font_path, int size, const Color& color, int wrap_width) {
    if (text.empty() || !renderer) return nullptr;

    std::string key = GenerateCacheKey(text, font_path, size, color, wrap_width);
    
    auto it = texture_cache_.find(key);
    if (it != texture_cache_.end()) {
        cache_hits_++;
        return it->second.get();
    }
    
    cache_misses_++;
    
    TTF_Font* font = font_manager_->GetFont(font_path, size);
    if (!font) return nullptr;

    SDL_Color sdl_color = {color.r, color.g, color.b, static_cast<Uint8>(color.a * 255.0f)};
    
    SDL_Surface* surface = nullptr;
    if (wrap_width > 0) {
        surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), sdl_color, wrap_width);
    } else {
        surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdl_color);
    }

    if (!surface) return nullptr;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    auto cached = std::make_unique<CachedText>();
    cached->texture = texture;
    cached->width = surface->w;
    cached->height = surface->h;
    
    SDL_FreeSurface(surface);
    
    CachedText* result = cached.get();
    texture_cache_[key] = std::move(cached);
    return result;
}

void TextRenderer::DrawText(const RenderContext& ctx, const std::string& text, const std::string& font_path, int size, int x, int y, const Color& color, Alignment align) {
    if (!ctx.renderer || text.empty()) return;
    
    CachedText* cached = GetOrRenderText(ctx.renderer, text, font_path, size, color, 0);
    if (!cached || !cached->texture) return;
    
    SDL_Rect dest = {x, y, cached->width, cached->height};
    
    // Apply alignment
    if (align == Alignment::Center || align == Alignment::TopCenter || align == Alignment::BottomCenter) {
        dest.x -= cached->width / 2;
    } else if (align == Alignment::Right || align == Alignment::TopRight || align == Alignment::BottomRight) {
        dest.x -= cached->width;
    }
    
    if (align == Alignment::Left || align == Alignment::Center || align == Alignment::Right) {
        dest.y -= cached->height / 2;
    } else if (align == Alignment::BottomLeft || align == Alignment::BottomCenter || align == Alignment::BottomRight) {
        dest.y -= cached->height;
    }

    SDL_SetTextureAlphaMod(cached->texture, static_cast<Uint8>(color.a * ctx.alpha * 255.0f));
    SDL_RenderCopy(ctx.renderer, cached->texture, nullptr, &dest);
}

void TextRenderer::DrawTextWrapped(const RenderContext& ctx, const std::string& text, const std::string& font_path, int size, int x, int y, int wrap_width, const Color& color, Alignment align) {
    if (!ctx.renderer || text.empty()) return;
    
    CachedText* cached = GetOrRenderText(ctx.renderer, text, font_path, size, color, wrap_width);
    if (!cached || !cached->texture) return;
    
    SDL_Rect dest = {x, y, cached->width, cached->height};
    
    // Standard wrapping usually is top-left aligned, but we respect the param
    if (align == Alignment::Center || align == Alignment::TopCenter || align == Alignment::BottomCenter) {
        dest.x -= cached->width / 2;
    } else if (align == Alignment::Right || align == Alignment::TopRight || align == Alignment::BottomRight) {
        dest.x -= cached->width;
    }

    if (align == Alignment::Left || align == Alignment::Center || align == Alignment::Right) {
        dest.y -= cached->height / 2;
    } else if (align == Alignment::BottomLeft || align == Alignment::BottomCenter || align == Alignment::BottomRight) {
        dest.y -= cached->height;
    }

    SDL_SetTextureAlphaMod(cached->texture, static_cast<Uint8>(color.a * ctx.alpha * 255.0f));
    SDL_RenderCopy(ctx.renderer, cached->texture, nullptr, &dest);
}

int TextRenderer::MeasureTextWidth(const std::string& text, const std::string& font_path, int size) {
    int w = 0, h = 0;
    MeasureText(text, font_path, size, w, h);
    return w;
}

void TextRenderer::MeasureText(const std::string& text, const std::string& font_path, int size, int& w, int& h) {
    w = 0;
    h = 0;
    if (text.empty()) return;
    
    TTF_Font* font = font_manager_->GetFont(font_path, size);
    if (!font) return;
    
    TTF_SizeUTF8(font, text.c_str(), &w, &h);
}

void TextRenderer::ClearCache() {
    texture_cache_.clear();
    Logger::Info("[UI] Text cache hits: " + std::to_string(cache_hits_));
    Logger::Info("[UI] Text cache misses: " + std::to_string(cache_misses_));
    cache_hits_ = 0;
    cache_misses_ = 0;
}

} // namespace unboundmp::ui
'''
    with open('d:/Unbound/pokemon/cpp/src/ui/text_renderer.cpp', 'w', encoding='utf-8') as f:
        f.write(content)

create_text_renderer_cpp()
print("text_renderer.cpp created")
