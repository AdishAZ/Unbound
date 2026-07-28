#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include <string>
#include <functional>

namespace unboundmp::ui {

class FontManager;
class TextRenderer;
class ThemeManager;

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;
    
    static Color FromHex(uint32_t hex);
    static Color Lerp(const Color& a, const Color& b, float t);
    
    SDL_Color ToSDL() const { return {r, g, b, a}; }
    
    static const Color Black, White, Red, Green, Blue, Yellow;
    static const Color Transparent;
    // Dark theme colors
    static const Color DarkBg, DarkPanel, DarkAccent, DarkText, DarkSubtext;
    static const Color DarkSuccess, DarkWarning, DarkError;
    static const Color DarkBorder, DarkHover, DarkPressed, DarkDisabled;
};

struct Point { int x = 0, y = 0; };
struct Size { int width = 0, height = 0; };
struct Rect {
    int x = 0, y = 0, width = 0, height = 0;
    bool Contains(int px, int py) const;
    bool Intersects(const Rect& other) const;
    SDL_Rect ToSDL() const;
};

struct Padding {
    int top = 0, right = 0, bottom = 0, left = 0;
    static Padding All(int v);
    static Padding Symmetric(int h, int v);
};

struct Margin {
    int top = 0, right = 0, bottom = 0, left = 0;
    static Margin All(int v);
};

enum class Alignment { Left, Center, Right, TopLeft, TopCenter, TopRight, BottomLeft, BottomCenter, BottomRight, Top, Bottom };
enum class AnchorPoint { TopLeft, TopCenter, TopRight, CenterLeft, Center, CenterRight, BottomLeft, BottomCenter, BottomRight };
enum class LayoutDirection { Horizontal, Vertical };

enum class SizePolicy { Fixed, Expand, WrapContent };

// Wrap an SDL_Renderer* with helper drawing methods
struct RenderContext {
    SDL_Renderer* renderer = nullptr;
    int screen_width = 0;
    int screen_height = 0;
    float alpha = 1.0f;
    FontManager* font_manager = nullptr;
    TextRenderer* text_renderer = nullptr;
    ThemeManager* theme = nullptr;
    
    void DrawFilledRect(const Rect& rect, const Color& color) const;
    void DrawOutlinedRect(const Rect& rect, const Color& color, int thickness = 1) const;
    void DrawLine(int x1, int y1, int x2, int y2, const Color& color) const;
    void DrawRoundedRect(const Rect& rect, const Color& color, int radius) const;
    void DrawFilledRoundedRect(const Rect& rect, const Color& color, int radius) const;
    void SetClipRect(const Rect& rect) const;
    void ClearClipRect() const;
};

using ClickCallback = std::function<void()>;
using ValueCallback = std::function<void(float)>;
using TextCallback = std::function<void(const std::string&)>;
using BoolCallback = std::function<void(bool)>;
using IndexCallback = std::function<void(int)>;

} // namespace unboundmp::ui
