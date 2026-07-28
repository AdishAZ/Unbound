#include "ui/hud_manager.h"

namespace unboundmp::ui {

void HUDManager::AddWidget(const std::string& id, std::unique_ptr<HUDWidget> widget) {
    m_widgets[id] = std::move(widget);
}

void HUDManager::RemoveWidget(const std::string& id) {
    m_widgets.erase(id);
}

HUDWidget* HUDManager::GetWidget(const std::string& id) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.get();
    }
    return nullptr;
}

void HUDManager::SetEnabled(bool enabled) {
    m_enabled = enabled;
}

bool HUDManager::IsEnabled() const {
    return m_enabled;
}

void HUDManager::Update(float dt, int screenWidth, int screenHeight) {
    if (!m_enabled) return;
    
    for (auto& [id, widget] : m_widgets) {
        widget->UpdatePosition(screenWidth, screenHeight);
        widget->Update(dt);
    }
}

void HUDManager::Render(const RenderContext& ctx) {
    if (!m_enabled) return;
    
    for (const auto& [id, widget] : m_widgets) {
        widget->Render(ctx);
    }
}

} // namespace unboundmp::ui
