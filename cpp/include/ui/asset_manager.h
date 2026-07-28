#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <SDL.h>
#include "ui/ui_types.h"

namespace unboundmp::ui {

struct SDLTextureDeleter { 
    void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); } 
};
using SDLTexturePtr = std::unique_ptr<SDL_Texture, SDLTextureDeleter>;

struct AssetEntry {
    SDLTexturePtr texture;
    int ref_count = 0;
    int width = 0;
    int height = 0;
};

class AssetManager {
public:
    void Initialize(SDL_Renderer* renderer);
    void Shutdown();
    
    // Texture management
    SDL_Texture* LoadTexture(const std::string& id, const std::string& path);
    SDL_Texture* CreateColorTexture(const std::string& id, int w, int h, const Color& color);
    SDL_Texture* GetTexture(const std::string& id);
    void ReleaseTexture(const std::string& id);
    
    // Cache management
    void ClearUnused(); // removes entries with ref_count <= 0
    size_t GetCacheSize() const;
    size_t GetTextureCount() const;
    
    // Future placeholders for broadened responsibilities
    void* LoadFont(const std::string& id, const std::string& path);
    void* LoadIcon(const std::string& id, const std::string& path);
    void* LoadPlayerSprite(const std::string& id, const std::string& path);
    void* LoadItemIcon(const std::string& id, const std::string& path);
    void* LoadPokemonSprite(const std::string& id, const std::string& path);
    void* LoadAudio(const std::string& id, const std::string& path);
    
    // Stats
    size_t GetTotalTextureMemory() const; // estimated bytes
    
private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, AssetEntry> textures_;
    size_t total_memory_ = 0;
};

} // namespace unboundmp::ui
