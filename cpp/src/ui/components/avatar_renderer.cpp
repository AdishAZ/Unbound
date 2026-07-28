#include "ui/components/avatar_renderer.h"
#include <cmath>

namespace unboundmp::ui {

static void HSLtoRGB(float h, float s, float l, uint8_t& r, uint8_t& g, uint8_t& b) {
    auto hue2rgb = [](float p, float q, float t) {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f/6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f/2.0f) return q;
        if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6.0f;
        return p;
    };

    if (s == 0.0f) {
        r = g = b = static_cast<uint8_t>(l * 255.0f);
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        r = static_cast<uint8_t>(hue2rgb(p, q, h + 1.0f/3.0f) * 255.0f);
        g = static_cast<uint8_t>(hue2rgb(p, q, h) * 255.0f);
        b = static_cast<uint8_t>(hue2rgb(p, q, h - 1.0f/3.0f) * 255.0f);
    }
}

void AvatarRenderer::Render(const RenderContext& ctx, Rect bounds, uint64_t player_id) {
    // Generate color from player_id hash
    float hue = static_cast<float>((player_id * 2654435761ULL) % 360) / 360.0f;
    uint8_t r, g, b;
    HSLtoRGB(hue, 0.7f, 0.5f, r, g, b); // Fixed saturation and lightness

    SDL_SetRenderDrawColor(ctx.renderer, r, g, b, 255);
    SDL_Rect sdl_rect{bounds.x, bounds.y, bounds.width, bounds.height};
    SDL_RenderFillRect(ctx.renderer, &sdl_rect);

    // Draw border
    SDL_SetRenderDrawColor(ctx.renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(ctx.renderer, &sdl_rect);
}

} // namespace unboundmp::ui
