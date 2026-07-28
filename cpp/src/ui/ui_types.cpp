#include "ui/ui_types.h"
#include <algorithm>

namespace unboundmp::ui {

// Color
Color Color::FromHex(uint32_t hex) {
    if (hex > 0xFFFFFF) { // Assuming ARGB if > 0xFFFFFF
        return {
            static_cast<uint8_t>((hex >> 16) & 0xFF),
            static_cast<uint8_t>((hex >> 8) & 0xFF),
            static_cast<uint8_t>(hex & 0xFF),
            static_cast<uint8_t>((hex >> 24) & 0xFF)
        };
    }
    return {
        static_cast<uint8_t>((hex >> 16) & 0xFF),
        static_cast<uint8_t>((hex >> 8) & 0xFF),
        static_cast<uint8_t>(hex & 0xFF),
        255
    };
}

Color Color::Lerp(const Color& a, const Color& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return {
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<uint8_t>(a.a + (b.a - a.a) * t)
    };
}

const Color Color::Black = {0, 0, 0, 255};
const Color Color::White = {255, 255, 255, 255};
const Color Color::Red = {255, 0, 0, 255};
const Color Color::Green = {0, 255, 0, 255};
const Color Color::Blue = {0, 0, 255, 255};
const Color Color::Yellow = {255, 255, 0, 255};
const Color Color::Transparent = {0, 0, 0, 0};

// Dark theme colors
const Color Color::DarkBg = {18, 18, 18, 255};
const Color Color::DarkPanel = {30, 30, 30, 255};
const Color Color::DarkAccent = {88, 101, 242, 255}; // blurple-ish
const Color Color::DarkText = {240, 240, 240, 255};
const Color Color::DarkSubtext = {160, 160, 160, 255};
const Color Color::DarkSuccess = {59, 165, 93, 255};
const Color Color::DarkWarning = {250, 166, 26, 255};
const Color Color::DarkError = {237, 66, 69, 255};
const Color Color::DarkBorder = {60, 60, 60, 255};
const Color Color::DarkHover = {45, 45, 45, 255};
const Color Color::DarkPressed = {20, 20, 20, 255};
const Color Color::DarkDisabled = {80, 80, 80, 255};

// Rect
bool Rect::Contains(int px, int py) const {
    return px >= x && px < x + width && py >= y && py < y + height;
}

bool Rect::Intersects(const Rect& other) const {
    return !(x + width <= other.x || x >= other.x + other.width ||
             y + height <= other.y || y >= other.y + other.height);
}

SDL_Rect Rect::ToSDL() const {
    return {x, y, width, height};
}

// Padding
Padding Padding::All(int v) { return {v, v, v, v}; }
Padding Padding::Symmetric(int h, int v) { return {v, h, v, h}; }

// Margin
Margin Margin::All(int v) { return {v, v, v, v}; }

// RenderContext
void RenderContext::DrawFilledRect(const Rect& rect, const Color& color) const {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect sdl_rect = rect.ToSDL();
    SDL_RenderFillRect(renderer, &sdl_rect);
}

void RenderContext::DrawOutlinedRect(const Rect& rect, const Color& color, int thickness) const {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect sdl_rect = rect.ToSDL();
    
    if (thickness == 1) {
        SDL_RenderDrawRect(renderer, &sdl_rect);
    } else {
        // Top
        SDL_Rect t = {rect.x, rect.y, rect.width, thickness};
        SDL_RenderFillRect(renderer, &t);
        // Bottom
        SDL_Rect b = {rect.x, rect.y + rect.height - thickness, rect.width, thickness};
        SDL_RenderFillRect(renderer, &b);
        // Left
        SDL_Rect l = {rect.x, rect.y, thickness, rect.height};
        SDL_RenderFillRect(renderer, &l);
        // Right
        SDL_Rect r = {rect.x + rect.width - thickness, rect.y, thickness, rect.height};
        SDL_RenderFillRect(renderer, &r);
    }
}

void RenderContext::DrawLine(int x1, int y1, int x2, int y2, const Color& color) const {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void RenderContext::DrawRoundedRect(const Rect& rect, const Color& color, int radius) const {
    if (!renderer || radius <= 0) {
        DrawOutlinedRect(rect, color);
        return;
    }
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Straight edges
    SDL_RenderDrawLine(renderer, rect.x + radius, rect.y, rect.x + rect.width - radius, rect.y); // Top
    SDL_RenderDrawLine(renderer, rect.x + radius, rect.y + rect.height, rect.x + rect.width - radius, rect.y + rect.height); // Bottom
    SDL_RenderDrawLine(renderer, rect.x, rect.y + radius, rect.x, rect.y + rect.height - radius); // Left
    SDL_RenderDrawLine(renderer, rect.x + rect.width, rect.y + radius, rect.x + rect.width, rect.y + rect.height - radius); // Right
    
    // Corners approximation (4 segments)
    // Top-left
    SDL_RenderDrawLine(renderer, rect.x, rect.y + radius, rect.x + radius, rect.y);
    // Top-right
    SDL_RenderDrawLine(renderer, rect.x + rect.width - radius, rect.y, rect.x + rect.width, rect.y + radius);
    // Bottom-left
    SDL_RenderDrawLine(renderer, rect.x, rect.y + rect.height - radius, rect.x + radius, rect.y + rect.height);
    // Bottom-right
    SDL_RenderDrawLine(renderer, rect.x + rect.width - radius, rect.y + rect.height, rect.x + rect.width, rect.y + rect.height - radius);
}

void RenderContext::DrawFilledRoundedRect(const Rect& rect, const Color& color, int radius) const {
    if (!renderer || radius <= 0) {
        DrawFilledRect(rect, color);
        return;
    }
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Central main block
    SDL_Rect center = {rect.x, rect.y + radius, rect.width, rect.height - 2 * radius};
    SDL_RenderFillRect(renderer, &center);
    
    // Top block
    SDL_Rect top = {rect.x + radius, rect.y, rect.width - 2 * radius, radius};
    SDL_RenderFillRect(renderer, &top);
    
    // Bottom block
    SDL_Rect bottom = {rect.x + radius, rect.y + rect.height - radius, rect.width - 2 * radius, radius};
    SDL_RenderFillRect(renderer, &bottom);
    
    // Simple diamond corners as fill approximation
    // This is simple but fulfills requirement of approximated rounding
    int pts = 4;
    for (int i = 0; i < radius; ++i) {
        // Top Left
        SDL_RenderDrawLine(renderer, rect.x + radius - i, rect.y + i, rect.x + radius, rect.y + i);
        // Top Right
        SDL_RenderDrawLine(renderer, rect.x + rect.width - radius, rect.y + i, rect.x + rect.width - radius + i, rect.y + i);
        // Bottom Left
        SDL_RenderDrawLine(renderer, rect.x + radius - i, rect.y + rect.height - 1 - i, rect.x + radius, rect.y + rect.height - 1 - i);
        // Bottom Right
        SDL_RenderDrawLine(renderer, rect.x + rect.width - radius, rect.y + rect.height - 1 - i, rect.x + rect.width - radius + i, rect.y + rect.height - 1 - i);
    }
}

void RenderContext::SetClipRect(const Rect& rect) const {
    if (!renderer) return;
    SDL_Rect sdl_rect = rect.ToSDL();
    SDL_RenderSetClipRect(renderer, &sdl_rect);
}

void RenderContext::ClearClipRect() const {
    if (!renderer) return;
    SDL_RenderSetClipRect(renderer, nullptr);
}

} // namespace unboundmp::ui
