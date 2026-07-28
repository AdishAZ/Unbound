#pragma once
#include "render/irenderer.h"
#include <string>

namespace unboundmp::render {

class RenderLayer : public IRenderer {
public:
    explicit RenderLayer(const std::string& name) : name_(name) {}
    virtual ~RenderLayer() = default;

    void Initialize() override {}
    void Shutdown() override {}
    
    // Derived classes must implement Render to push commands to context.queue
    virtual void Render(const RenderContext& context) = 0;

    const std::string& GetName() const { return name_; }

    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

protected:
    std::string name_;
    bool enabled_ = true;
};

} // namespace unboundmp::render
