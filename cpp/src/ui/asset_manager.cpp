#include "ui/asset_manager.h"
#include <stdexcept>
#include <iostream>

namespace unboundmp::ui {

void AssetManager::Initialize(SDL_Renderer* renderer) {
    renderer_ = renderer;
}

void AssetManager::Shutdown() {
    textures_.clear();
    total_memory_ = 0;
    renderer_ = nullptr;
}

SDL_Texture* AssetManager::LoadTexture(const std::string& id, const std::string& path) {
    if (textures_.find(id) != textures_.end()) {
        textures_[id].ref_count++;
        return textures_[id].texture.get();
    }

    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load BMP: " << path << " error: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surface);
    int width = surface->w;
    int height = surface->h;
    SDL_FreeSurface(surface);

    if (!tex) {
        std::cerr << "Failed to create texture from BMP: " << path << " error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    AssetEntry entry;
    entry.texture.reset(tex);
    entry.ref_count = 1;
    entry.width = width;
    entry.height = height;

    textures_[id] = std::move(entry);
    total_memory_ += static_cast<size_t>(width * height * 4); // Estimate 4 bytes per pixel

    return textures_[id].texture.get();
}

SDL_Texture* AssetManager::CreateColorTexture(const std::string& id, int w, int h, const Color& color) {
    if (textures_.find(id) != textures_.end()) {
        textures_[id].ref_count++;
        return textures_[id].texture.get();
    }

    SDL_Texture* tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!tex) {
        std::cerr << "Failed to create color texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_SetRenderTarget(renderer_, tex);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer_);
    SDL_SetRenderTarget(renderer_, nullptr);

    AssetEntry entry;
    entry.texture.reset(tex);
    entry.ref_count = 1;
    entry.width = w;
    entry.height = h;

    textures_[id] = std::move(entry);
    total_memory_ += static_cast<size_t>(w * h * 4);

    return textures_[id].texture.get();
}

SDL_Texture* AssetManager::GetTexture(const std::string& id) {
    auto it = textures_.find(id);
    if (it != textures_.end()) {
        return it->second.texture.get();
    }
    return nullptr;
}

void AssetManager::ReleaseTexture(const std::string& id) {
    auto it = textures_.find(id);
    if (it != textures_.end()) {
        it->second.ref_count--;
    }
}

void AssetManager::ClearUnused() {
    for (auto it = textures_.begin(); it != textures_.end(); ) {
        if (it->second.ref_count <= 0) {
            total_memory_ -= static_cast<size_t>(it->second.width * it->second.height * 4);
            it = textures_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t AssetManager::GetCacheSize() const {
    return textures_.size();
}

size_t AssetManager::GetTextureCount() const {
    size_t count = 0;
    for (const auto& [id, entry] : textures_) {
        if (entry.ref_count > 0) {
            count++;
        }
    }
    return count;
}

size_t AssetManager::GetTotalTextureMemory() const {
    return total_memory_;
}

// Future placeholders
void* AssetManager::LoadFont(const std::string& id, const std::string& path) { return nullptr; }
void* AssetManager::LoadIcon(const std::string& id, const std::string& path) { return nullptr; }
void* AssetManager::LoadPlayerSprite(const std::string& id, const std::string& path) { return nullptr; }
void* AssetManager::LoadItemIcon(const std::string& id, const std::string& path) { return nullptr; }
void* AssetManager::LoadPokemonSprite(const std::string& id, const std::string& path) { 
    return LoadTexture(id, path); 
}
void* AssetManager::LoadAudio(const std::string& id, const std::string& path) { return nullptr; }

} // namespace unboundmp::ui
