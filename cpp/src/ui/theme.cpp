#include "ui/theme.h"

namespace unboundmp::ui {

ThemeManager::ThemeManager() {
    SetDefaultTheme();
}

void ThemeManager::SetDefaultTheme() {
    // Desktop MMORPG Dark Theme (PokeMMO Inspired)
    // Using ARGB format (0xAARRGGBB) for translucent UI (80% opacity = 0xCC)
    background_ = Color::FromHex(0xCC1e2227); // Dark gray/blue with 80% alpha
    surface_ = Color::FromHex(0xCC2d323b);
    surface_hover_ = Color::FromHex(0xCC3a404c);
    // Dark MMORPG Theme (PokeMMO style)
    background_ = Color{45, 53, 63, 255};      // Dark grey/blue for window backgrounds
    surface_ = Color{62, 73, 87, 255};         // Slightly lighter for panels/buttons
    surface_hover_ = Color{75, 88, 104, 255};  // Hover state
    primary_ = Color{90, 130, 170, 255};       // Selection highlight / accent
    secondary_ = Color{40, 48, 58, 255};       // Darker for title bars / depressed states
    border_ = Color{75, 95, 115, 255};         // Thin blue/grey borders
    
    text_primary_ = Color{255, 255, 255, 255}; // White text
    text_secondary_ = Color{200, 200, 200, 255};
    error_ = Color{220, 60, 60, 255};
    success_ = Color{60, 200, 60, 255};
    
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
    return false; // TODO: Implement JSON loading
}

Color ThemeManager::GetColor(const std::string& name) const {
    auto it = colors_.find(name);
    if (it != colors_.end()) return it->second;
    return Color::White;
}

} // namespace unboundmp::ui
