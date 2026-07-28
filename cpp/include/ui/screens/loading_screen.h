#pragma once
#include "ui/screen.h"
#include "ui/widgets.h"
#include "ui/ui_types.h"
#include <memory>

namespace unboundmp::ui {
class UIEngine;

class LoadingScreen : public UIScreen {
public:
    explicit LoadingScreen(UIEngine* engine);
    ~LoadingScreen() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    UIEngine* engine_;
    float timer_;
    int step_;

    std::shared_ptr<Container> root_;
    std::shared_ptr<Label> title_label_;
    std::shared_ptr<ProgressBar> loading_bar_;
    std::shared_ptr<Label> status_label_;
    
    std::vector<uint64_t> subscriptions_;
};
} // namespace unboundmp::ui
