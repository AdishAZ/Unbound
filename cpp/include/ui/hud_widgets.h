#pragma once

#include "ui/ui_types.h"
#include "ui/widget.h"
#include <memory>
#include <vector>

namespace unboundmp::ui {



class HUDWidget : public Widget {
public:
    HUDWidget() = default;
    virtual ~HUDWidget() = default;
    
    void SetAnchor(AnchorPoint anchor);
    void SetOffset(int x, int y);
    void UpdatePosition(int screenWidth, int screenHeight);
    
    virtual void Update(float dt) {}

protected:
    AnchorPoint m_anchor{AnchorPoint::TopLeft};
    int m_offsetX{0};
    int m_offsetY{0};
    
    void RenderPanel(const RenderContext& ctx, const std::string& text, Color textColor);
};

class FPSWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

class PingWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

class MapWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

class CoordinatesWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

class AutosaveWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

class NetworkWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

class PlayerCountWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

class EmulatorSpeedWidget : public HUDWidget {
public:
    void Render(const RenderContext& ctx) override;
};

} // namespace unboundmp::ui
