def fix_logging(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = content.replace('#include "utils/logger.h"', '#include "core/log_manager.h"\n#include <iostream>')

    content = content.replace('Logger::Info("[UI] Text cache hits: " + std::to_string(cache_hits_));', 'LOG_INFO(UI, "[UI] Text cache hits: {}", cache_hits_);')
    content = content.replace('Logger::Info("[UI] Text cache misses: " + std::to_string(cache_misses_));', 'LOG_INFO(UI, "[UI] Text cache misses: {}", cache_misses_);')

    content = content.replace('Logger::Error("Failed to initialize SDL_ttf: " + std::string(TTF_GetError()));', 'LOG_ERROR(UI, "Failed to initialize SDL_ttf: {}", TTF_GetError());')
    content = content.replace('Logger::Info("[UI] SDL_ttf initialized");', 'LOG_INFO(UI, "[UI] SDL_ttf initialized");')
    content = content.replace('Logger::Info("[UI] SDL_ttf shutdown complete");', 'LOG_INFO(UI, "[UI] SDL_ttf shutdown complete");')
    content = content.replace('Logger::Info("[UI] Loaded font: " + path);', 'LOG_INFO(UI, "[UI] Loaded font: {}", path);')
    content = content.replace('Logger::Info("[UI] Cached font: " + std::to_string(size) + " px");', 'LOG_INFO(UI, "[UI] Cached font: {} px", size);')
    content = content.replace('Logger::Info("[UI] Loaded glyph count: " + std::to_string(TTF_GetFontFaces(font)));', 'LOG_INFO(UI, "[UI] Loaded glyph count: {}", TTF_GetFontFaces(font));')
    
    content = content.replace('Logger::Info("[UI] Requested font missing: " + request_path);', 'LOG_INFO(UI, "[UI] Requested font missing: {}", request_path);')
    content = content.replace('Logger::Info("[UI] Using fallback font: " + theme_fallback_path_);', 'LOG_INFO(UI, "[UI] Using fallback font: {}", theme_fallback_path_);')
    content = content.replace('Logger::Info("[UI] Requested font missing");', 'LOG_INFO(UI, "[UI] Requested font missing");')
    content = content.replace('Logger::Info("[UI] Using fallback font: " + default_fallback_path_);', 'LOG_INFO(UI, "[UI] Using fallback font: {}", default_fallback_path_);')
    content = content.replace('Logger::Error("[UI] Failed to load any fallback font for size " + std::to_string(pt_size));', 'LOG_ERROR(UI, "[UI] Failed to load any fallback font for size {}", pt_size);')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_logging('d:/Unbound/pokemon/cpp/src/ui/font_manager.cpp')
fix_logging('d:/Unbound/pokemon/cpp/src/ui/text_renderer.cpp')
print("fixed logging")
