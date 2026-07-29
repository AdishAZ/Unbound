#pragma once

#include <SDL2/SDL.h>

namespace unboundmp::render {

struct SDLTextureDeleter {
    void operator()(SDL_Texture* texture) const noexcept {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
};

} // namespace unboundmp::render