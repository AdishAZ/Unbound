#include "ui/components/player_card.h"
#include "ui/components/avatar_renderer.h"

namespace unboundmp::ui {

PlayerCard::PlayerCard() {
    bounds_.width = 200;
    bounds_.height = 40;
}

void PlayerCard::SetPlayerData(uint64_t id, const std::string& name, Status status) {
    id_ = id;
    name_ = name;
    status_ = status;
}

void PlayerCard::Render(const RenderContext& ctx) {
    // Draw background
    SDL_SetRenderDrawColor(ctx.renderer, 40, 40, 40, 200);
    SDL_Rect bg_rect{bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_RenderFillRect(ctx.renderer, &bg_rect);

    // Draw avatar
    Rect avatar_bounds{bounds_.x + 4, bounds_.y + 4, 32, 32};
    AvatarRenderer::Render(ctx, avatar_bounds, id_);

    // Draw name text
    Color text_color = {255, 255, 255, 255};
    DrawText(ctx, name_, bounds_.x + 44, bounds_.y + 12, text_color);

    // Draw status dot
    Color status_color;
    switch (status_) {
        case Status::Online: status_color = {0, 255, 0, 255}; break;
        case Status::Away:   status_color = {255, 255, 0, 255}; break;
        case Status::Offline:
        default:             status_color = {128, 128, 128, 255}; break;
    }

    SDL_SetRenderDrawColor(ctx.renderer, status_color.r, status_color.g, status_color.b, status_color.a);
    SDL_Rect status_rect{bounds_.x + bounds_.width - 12, bounds_.y + 16, 8, 8};
    SDL_RenderFillRect(ctx.renderer, &status_rect);
}

} // namespace unboundmp::ui
