#pragma once
#include "ui/ui_types.h"
#include <memory>
#include <vector>
#include <string>
#include <SDL2/SDL.h>

namespace unboundmp::ui {

class Widget {
public:
    explicit Widget(const std::string& id = "");
    virtual ~Widget() = default;
    
    // Core interface
    virtual void Render(const RenderContext& ctx) = 0;
    virtual bool HandleInput(const SDL_Event& event) = 0;
    virtual void Update(float dt) = 0;
    
    // Bounds management
    const Rect& GetBounds() const { return bounds_; }
    virtual void SetBounds(const Rect& bounds) {
        int dx = bounds.x - bounds_.x;
        int dy = bounds.y - bounds_.y;
        bounds_ = bounds;
        if (dx != 0 || dy != 0) {
            for (auto& child : children_) {
                Rect cb = child->GetBounds();
                cb.x += dx;
                cb.y += dy;
                child->SetBounds(cb);
            }
        }
    }
    virtual void PerformLayout() {}
    virtual void InvalidateLayout() {}
    virtual Size GetPreferredSize() const { return {bounds_.width, bounds_.height}; }
    
    void SetPosition(int x, int y);
    void SetSize(int w, int h);
    
    // Visibility & state
    bool IsVisible() const { return visible_; }
    void SetVisible(bool v) { visible_ = v; }
    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool e) { enabled_ = e; }
    bool IsFocused() const { return focused_; }
    void SetFocused(bool f) { focused_ = f; }
    bool IsHovered() const { return hovered_; }
    
    // Identity
    const std::string& GetId() const { return id_; }
    
    // Hierarchy
    Widget* GetParent() const { return parent_; }
    const std::vector<std::shared_ptr<Widget>>& GetChildren() const { return children_; }
    void AddChild(std::shared_ptr<Widget> child);
    void RemoveChild(const std::string& id);
    void ClearChildren();
    Widget* FindChild(const std::string& id);
    
    // Focus chain
    void SetTabIndex(int index) { tab_index_ = index; }
    int GetTabIndex() const { return tab_index_; }
    
    // Alpha for animations
    float GetAlpha() const { return alpha_; }
    void SetAlpha(float a) { alpha_ = a; }
    
public:
    // Layout properties
    Padding GetPadding() const { return padding_; }
    void SetPadding(const Padding& p) { padding_ = p; }
    Margin GetMargin() const { return margin_; }
    void SetMargin(const Margin& m) { margin_ = m; }
    SizePolicy GetWidthPolicy() const { return width_policy_; }
    void SetWidthPolicy(SizePolicy p) { width_policy_ = p; }
    SizePolicy GetHeightPolicy() const { return height_policy_; }
    void SetHeightPolicy(SizePolicy p) { height_policy_ = p; }
    AnchorPoint GetAnchor() const { return anchor_; }
    void SetAnchor(AnchorPoint a) { anchor_ = a; }
    Alignment GetAlignment() const { return alignment_; }
    void SetAlignment(Alignment a) { alignment_ = a; }
    
    virtual void OnThemeChanged() {}

protected:
    std::string id_;
    Rect bounds_{};
    bool visible_ = true;
    bool enabled_ = true;
    bool focused_ = false;
    bool hovered_ = false;
    float alpha_ = 1.0f;
    int tab_index_ = 0;
    Widget* parent_ = nullptr;
    std::vector<std::shared_ptr<Widget>> children_;
    
    Padding padding_;
    Margin margin_;
    SizePolicy width_policy_ = SizePolicy::Fixed;
    SizePolicy height_policy_ = SizePolicy::Fixed;
    AnchorPoint anchor_ = AnchorPoint::TopLeft;
    Alignment alignment_ = Alignment::TopLeft;
    
public:
    static void DrawText(const RenderContext& ctx, const std::string& text, int x, int y, const Color& color, const std::string& font = "default", int size = 16, Alignment align = Alignment::TopLeft);
    static void DrawTextWrapped(const RenderContext& ctx, const std::string& text, int x, int y, int wrap_width, const Color& color, const std::string& font = "default", int size = 16, Alignment align = Alignment::TopLeft);
    static int MeasureTextWidth(const RenderContext& ctx, const std::string& text, const std::string& font = "default", int size = 16);
    static void MeasureText(const RenderContext& ctx, const std::string& text, int& w, int& h, const std::string& font = "default", int size = 16);
    // Helper for hit testing
    bool ContainsPoint(int x, int y) const;
};

} // namespace unboundmp::ui
