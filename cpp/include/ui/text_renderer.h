#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "ui/font_manager.h"
#include "ui/ui_types.h"

namespace unboundmp::ui {

// A cached texture for a specific string+font+color combination
struct CachedText {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    
    ~CachedText() {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    
    // Non-copyable due to resource management
    CachedText() = default;
    CachedText(const CachedText&) = delete;
    CachedText& operator=(const CachedText&) = delete;
    
    // Movable
    CachedText(CachedText&& other) noexcept : texture(other.texture), width(other.width), height(other.height) {
        other.texture = nullptr;
    }
    CachedText& operator=(CachedText&& other) noexcept {
        if (this != &other) {
            if (texture) SDL_DestroyTexture(texture);
            texture = other.texture;
            width = other.width;
            height = other.height;
            other.texture = nullptr;
        }
        return *this;
    }
};

class TextRenderer {
public:
    TextRenderer(FontManager* font_manager);
    ~TextRenderer() = default;

    // Renders and caches a single line of text
    void DrawText(const RenderContext& ctx, const std::string& text, const std::string& font_path, int size, int x, int y, const Color& color, Alignment align = Alignment::Left);
    
    // Renders and caches wrapped text
    void DrawTextWrapped(const RenderContext& ctx, const std::string& text, const std::string& font_path, int size, int x, int y, int wrap_width, const Color& color, Alignment align = Alignment::Left);

    // Get dimensions of text as it would be rendered
    int MeasureTextWidth(const std::string& text, const std::string& font_path, int size);
    void MeasureText(const std::string& text, const std::string& font_path, int size, int& w, int& h);
    
    void ClearCache();

private:
    std::string GenerateCacheKey(const std::string& text, const std::string& font_path, int size, const Color& color, int wrap_width = 0) const;
    CachedText* GetOrRenderText(SDL_Renderer* renderer, const std::string& text, const std::string& font_path, int size, const Color& color, int wrap_width = 0);

    FontManager* font_manager_;
    std::unordered_map<std::string, std::unique_ptr<CachedText>> texture_cache_;
    
    int cache_hits_ = 0;
    int cache_misses_ = 0;
};

} // namespace unboundmp::ui
