#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"

namespace unboundmp::ui {

class UIEngine;

class CharacterCreationScreen : public UIScreen {
 public:
    explicit CharacterCreationScreen(UIEngine* engine);

    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;
    void OnResize(int width, int height) override;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

 private:
    UIEngine* engine_;
    std::shared_ptr<AnchorLayout> root_layout_;
    std::shared_ptr<TextBox> name_input_;
    std::shared_ptr<Label> status_label_;
    
    std::shared_ptr<ComboBox> gender_combo_;
    std::shared_ptr<ComboBox> skin_combo_;
    std::shared_ptr<ComboBox> hat_combo_;
    std::shared_ptr<ComboBox> hair_combo_;
    std::shared_ptr<ComboBox> eyes_combo_;
    std::shared_ptr<ComboBox> face_combo_;
    std::shared_ptr<ComboBox> back_combo_;
    std::shared_ptr<ComboBox> top_combo_;
    std::shared_ptr<ComboBox> gloves_combo_;
    std::shared_ptr<ComboBox> shoes_combo_;
    std::shared_ptr<ComboBox> legs_combo_;
    std::shared_ptr<ComboBox> bike_combo_;
    std::shared_ptr<ComboBox> region_combo_;
    
    uint64_t pending_character_id_ = 0;
    std::vector<uint64_t> subscriptions_;
};

} // namespace unboundmp::ui
