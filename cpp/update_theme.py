content_h = '''#pragma once
#include "ui/ui_types.h"
#include <string>
#include <unordered_map>

namespace unboundmp::ui {

class ThemeManager {
public:
    ThemeManager();
    ~ThemeManager() = default;

    bool LoadTheme(const std::string& filepath);
    void SetDefaultTheme();
    void SetLightTheme();
    
    // Core Palettes
    Color GetBackground() const { return background_; }
    Color GetSurface() const { return surface_; }
    Color GetSurfaceHover() const { return surface_hover_; }
    Color GetPrimary() const { return primary_; }
    Color GetSecondary() const { return secondary_; }
    Color GetBorder() const { return border_; }
    Color GetTextPrimary() const { return text_primary_; }
    Color GetTextSecondary() const { return text_secondary_; }
    Color GetError() const { return error_; }
    Color GetSuccess() const { return success_; }
    
    // Geometry
    int GetCornerRadius() const { return corner_radius_; }
    int GetBorderThickness() const { return border_thickness_; }
    
    // Generic Lookups
    Color GetColor(const std::string& name) const;

private:
    Color background_;
    Color surface_;
    Color surface_hover_;
    Color primary_;
    Color secondary_;
    Color border_;
    Color text_primary_;
    Color text_secondary_;
    Color error_;
    Color success_;
    
    int corner_radius_;
    int border_thickness_;
    
    std::unordered_map<std::string, Color> colors_;
};

} // namespace unboundmp::ui
'''
with open('d:/Unbound/pokemon/cpp/include/ui/theme.h', 'w', encoding='utf-8') as f:
    f.write(content_h)

content_cpp = '''#include "ui/theme.h"

namespace unboundmp::ui {

ThemeManager::ThemeManager() {
    SetDefaultTheme();
}

void ThemeManager::SetDefaultTheme() {
    // Desktop MMORPG Dark Theme (PokeMMO Inspired)
    background_ = Color::FromHex(0x1e2227); // Dark gray/blue
    surface_ = Color::FromHex(0x2d323b);
    surface_hover_ = Color::FromHex(0x3a404c);
    primary_ = Color::FromHex(0x4a90e2); // Muted blue accent
    secondary_ = Color::FromHex(0x4b5363);
    border_ = Color::FromHex(0x181a1f);
    text_primary_ = Color::FromHex(0xe5e7eb);
    text_secondary_ = Color::FromHex(0x9ca3af);
    error_ = Color::FromHex(0xef4444);
    success_ = Color::FromHex(0x10b981);
    
    corner_radius_ = 4;
    border_thickness_ = 1;
}

void ThemeManager::SetLightTheme() {
    background_ = Color::FromHex(0xf3f4f6);
    surface_ = Color::FromHex(0xffffff);
    surface_hover_ = Color::FromHex(0xe5e7eb);
    primary_ = Color::FromHex(0x2563eb);
    secondary_ = Color::FromHex(0x9ca3af);
    border_ = Color::FromHex(0xd1d5db);
    text_primary_ = Color::FromHex(0x111827);
    text_secondary_ = Color::FromHex(0x4b5563);
    error_ = Color::FromHex(0xdc2626);
    success_ = Color::FromHex(0x059669);
    
    corner_radius_ = 4;
    border_thickness_ = 1;
}

bool ThemeManager::LoadTheme(const std::string& filepath) {
    // Placeholder for JSON loading
    return true; 
}

Color ThemeManager::GetColor(const std::string& name) const {
    auto it = colors_.find(name);
    if (it != colors_.end()) return it->second;
    return Color::White;
}

} // namespace unboundmp::ui
'''
with open('d:/Unbound/pokemon/cpp/src/ui/theme.cpp', 'w', encoding='utf-8') as f:
    f.write(content_cpp)

