#pragma once
#include "ui/ui_types.h"
#include "ui/widget.h"
#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
#include "network/packet.h"

namespace unboundmp::ui {

class UIEngine; // forward

class UIScreen {
public:
    explicit UIScreen(const std::string& name);
    virtual ~UIScreen() = default;
    
    virtual void OnEnter() = 0;     // Called when screen becomes active
    virtual void OnExit() = 0;      // Called when screen is removed
    virtual void OnPause() {}       // Called when another screen covers this one
    virtual void OnResume() {}
    virtual void OnResize(int width, int height) {}      // Called when covering screen is removed
    
    virtual void Render(const RenderContext& ctx) = 0;
    virtual bool HandleInput(const SDL_Event& event) = 0;
    virtual void Update(float dt) = 0;
    
    
    const std::string& GetName() const { return name_; }
    bool IsOverlay() const { return is_overlay_; }
    void SetOverlay(bool overlay) { is_overlay_ = overlay; }
    
    void SetEngine(UIEngine* engine) { engine_ = engine; }
    
protected:
    std::string name_;
    bool is_overlay_ = false;
    UIEngine* engine_ = nullptr;
    std::vector<std::shared_ptr<Widget>> widgets_;
    
    // Focus management
    int focused_widget_index_ = -1;
    void FocusNext();
    void FocusPrev();
    void FocusWidget(int index);
};

} // namespace unboundmp::ui
