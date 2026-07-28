#pragma once
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
