def create_font_manager_cpp():
    content = '''#include "ui/font_manager.h"
#include "utils/logger.h"

namespace unboundmp::ui {

FontManager::FontManager() {}

FontManager::~FontManager() {
    Shutdown();
}

bool FontManager::Initialize(const std::string& default_fallback_path) {
    if (initialized_) return true;

    if (TTF_Init() == -1) {
        Logger::Error("Failed to initialize SDL_ttf: " + std::string(TTF_GetError()));
        return false;
    }

    default_fallback_path_ = default_fallback_path;
    initialized_ = true;
    Logger::Info("[UI] SDL_ttf initialized");

    return true;
}

void FontManager::Shutdown() {
    if (!initialized_) return;

    std::lock_guard<std::mutex> lock(cache_mutex_);
    for (auto& pair : font_cache_) {
        if (pair.second) {
            TTF_CloseFont(pair.second);
        }
    }
    font_cache_.clear();

    TTF_Quit();
    initialized_ = false;
    Logger::Info("[UI] SDL_ttf shutdown complete");
}

void FontManager::SetThemeFallback(const std::string& path) {
    theme_fallback_path_ = path;
}

std::string FontManager::GenerateCacheKey(const std::string& path, int size) const {
    return path + "_" + std::to_string(size);
}

TTF_Font* FontManager::LoadInternal(const std::string& path, int size) {
    TTF_Font* font = TTF_OpenFont(path.c_str(), size);
    if (font) {
        Logger::Info("[UI] Loaded font: " + path);
        Logger::Info("[UI] Cached font: " + std::to_string(size) + " px");
        Logger::Info("[UI] Loaded glyph count: " + std::to_string(TTF_GetFontFaces(font)));
    }
    return font;
}

TTF_Font* FontManager::GetFont(const std::string& font_path, int pt_size) {
    if (!initialized_) return nullptr;

    std::string request_path = font_path;
    if (request_path.empty() || request_path == "default") {
        request_path = default_fallback_path_;
    }

    std::string cache_key = GenerateCacheKey(request_path, pt_size);

    std::lock_guard<std::mutex> lock(cache_mutex_);

    // Check cache first
    auto it = font_cache_.find(cache_key);
    if (it != font_cache_.end()) {
        return it->second;
    }

    // Try to load requested
    TTF_Font* font = LoadInternal(request_path, pt_size);

    // Try theme fallback
    if (!font && !theme_fallback_path_.empty() && request_path != theme_fallback_path_) {
        Logger::Info("[UI] Requested font missing: " + request_path);
        Logger::Info("[UI] Using fallback font: " + theme_fallback_path_);
        font = LoadInternal(theme_fallback_path_, pt_size);
    }

    // Try default fallback
    if (!font && request_path != default_fallback_path_ && theme_fallback_path_ != default_fallback_path_) {
        Logger::Info("[UI] Requested font missing");
        Logger::Info("[UI] Using fallback font: " + default_fallback_path_);
        font = LoadInternal(default_fallback_path_, pt_size);
    }

    if (font) {
        font_cache_[cache_key] = font;
    } else {
        Logger::Error("[UI] Failed to load any fallback font for size " + std::to_string(pt_size));
    }

    return font;
}

} // namespace unboundmp::ui
'''
    with open('d:/Unbound/pokemon/cpp/src/ui/font_manager.cpp', 'w', encoding='utf-8') as f:
        f.write(content)

create_font_manager_cpp()
print("font_manager.cpp created")
