#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"
#include "network/packet.h"

namespace unboundmp::ui {

class UIEngine;

class CharacterSelectScreen : public UIScreen {
 public:
    explicit CharacterSelectScreen(UIEngine* engine);
    
    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;
    void OnResize(int width, int height) override;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

    void BuildCharacterList();

 private:
    void BuildUI(const std::vector<network::CharacterEntry>& characters);

    std::shared_ptr<Button> delete_btn_;
    std::shared_ptr<Label> status_label_;
    std::vector<unboundmp::network::CharacterEntry> characters_;
    std::vector<uint64_t> subscriptions_;

    UIEngine* engine_;
    std::shared_ptr<AnchorLayout> root_layout_;
    std::shared_ptr<VerticalLayout> chars_layout_;
    
    enum class State {
        Fetching,
        Loaded
    };
    State state_ = State::Fetching;
    uint64_t pending_character_id_ = 0;
    std::string pending_character_name_;
};

} // namespace unboundmp::ui
