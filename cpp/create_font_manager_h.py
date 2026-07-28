def create_font_manager_h():
    content = '''#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace unboundmp::ui {

class FontManager {
public:
    FontManager();
    ~FontManager();

    bool Initialize(const std::string& default_fallback_path);
    void Shutdown();

    // Gets a font loaded at the specified size. Falls back automatically if missing.
    TTF_Font* GetFont(const std::string& font_path, int pt_size);

    // Explicit fallback config
    void SetThemeFallback(const std::string& path);
    const std::string& GetDefaultFallback() const { return default_fallback_path_; }

private:
    std::string GenerateCacheKey(const std::string& path, int size) const;
    TTF_Font* LoadInternal(const std::string& path, int size);
    
    std::string default_fallback_path_;
    std::string theme_fallback_path_;
    
    std::unordered_map<std::string, TTF_Font*> font_cache_;
    std::mutex cache_mutex_;
    
    bool initialized_ = false;
};

} // namespace unboundmp::ui
'''
    with open('d:/Unbound/pokemon/cpp/include/ui/font_manager.h', 'w', encoding='utf-8') as f:
        f.write(content)

create_font_manager_h()
print("font_manager.h created")
