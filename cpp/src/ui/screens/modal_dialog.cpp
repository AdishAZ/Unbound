#include "ui/screens/modal_dialog.h"
#include "ui/ui_engine.h"

namespace unboundmp::ui {

ModalDialog::ModalDialog(UIEngine* engine, const std::string& title, const std::string& message) 
    : UIScreen("ModalDialog"), engine_(engine) {
    SetOverlay(true);
    
    root_layout_ = std::make_shared<AnchorLayout>();
    
    panel_ = std::make_shared<Panel>();
    panel_->SetAnchor(AnchorPoint::Center);
    panel_->SetBounds({0, 0, 300, 200});
    panel_->SetBackgroundColor(Color::DarkPanel);
    panel_->SetCornerRadius(8);
    
    auto vert = std::make_shared<VerticalLayout>();
    vert->SetBounds({0, 0, 300, 200});
    vert->SetPadding(Padding::All(20));
    vert->SetSpacing(15);
    
    auto lbl_title = std::make_shared<Label>();
    lbl_title->SetText(title);
    lbl_title->SetAlignment(Alignment::Center);
    lbl_title->SetColor(Color::DarkText);
    
    auto lbl_msg = std::make_shared<Label>();
    lbl_msg->SetText(message);
    lbl_msg->SetAlignment(Alignment::Center);
    lbl_msg->SetColor(Color::DarkSubtext);
    lbl_msg->SetHeightPolicy(SizePolicy::Expand);
    
    button_layout_ = std::make_shared<HorizontalLayout>();
    button_layout_->SetBounds({0, 0, 260, 40});
    button_layout_->SetSpacing(10);
    button_layout_->SetHeightPolicy(SizePolicy::Fixed);
    
    vert->AddChild(lbl_title);
    vert->AddChild(lbl_msg);
    vert->AddChild(button_layout_);
    
    panel_->AddChild(vert);
    root_layout_->AddChild(panel_);
}

void ModalDialog::OnEnter() {}
void ModalDialog::OnExit() {}

void ModalDialog::OnResize(int width, int height) {
    root_layout_->SetBounds({0, 0, width, height});
    root_layout_->InvalidateLayout();
}

void ModalDialog::AddButton(const std::string& text, ClickCallback on_click) {
    auto btn = std::make_shared<Button>();
    btn->SetText(text);
    btn->SetBounds({0, 0, 100, 30});
    btn->OnClick([this, on_click]() {
        if (on_click) on_click();
        engine_->GetScreens().Pop(); // Auto close
    });
    button_layout_->AddChild(btn);
}

void ModalDialog::Render(const RenderContext& ctx) {
    // Dim background
    ctx.DrawFilledRect({0, 0, ctx.screen_width, ctx.screen_height}, {0, 0, 0, 150});
    
    root_layout_->SetBounds({0, 0, ctx.screen_width, ctx.screen_height});
    root_layout_->Render(ctx);
}

bool ModalDialog::HandleInput(const SDL_Event& event) {
    root_layout_->HandleInput(event);
    return true; // Modals swallow all input below them
}

void ModalDialog::Update(float dt) {
    root_layout_->Update(dt);
}

} // namespace unboundmp::ui
