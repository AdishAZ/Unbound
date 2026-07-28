#pragma once

#include "ui/ui_types.h"
#include "ui/hud_widgets.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace unboundmp::ui {

class HUDManager {
public:
    HUDManager() = default;
    ~HUDManager() = default;

    HUDManager(const HUDManager&) = delete;
    HUDManager& operator=(const HUDManager&) = delete;

    void AddWidget(const std::string& id, std::unique_ptr<HUDWidget> widget);
    void RemoveWidget(const std::string& id);
    HUDWidget* GetWidget(const std::string& id);

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    void Update(float dt, int screenWidth, int screenHeight);
    void Render(const RenderContext& ctx);

private:
    std::unordered_map<std::string, std::unique_ptr<HUDWidget>> m_widgets;
    bool m_enabled{true};
};

} // namespace unboundmp::ui
