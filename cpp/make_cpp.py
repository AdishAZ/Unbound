content = '''#include "ui/screens/create_account_screen.h"
#include "ui/screens/login_screen.h"
#include "ui/ui_engine.h"

namespace unboundmp::ui {

CreateAccountScreen::CreateAccountScreen(UIEngine* engine) : UIScreen("CreateAccountScreen"), engine_(engine) {
    root_layout_ = std::make_shared<AnchorLayout>();
    root_layout_->SetWidthPolicy(SizePolicy::Expand);
    root_layout_->SetHeightPolicy(SizePolicy::Expand);
    
    auto bg_img = std::make_shared<ImageWidget>("bg");
    bg_img->SetWidthPolicy(SizePolicy::Expand);
    bg_img->SetHeightPolicy(SizePolicy::Expand);
    SDL_Texture* tex = engine->GetAssetManager().LoadTexture("login_bg", "assets/ui/login_bg.bmp");
    if (tex) bg_img->SetTexture(tex);
    root_layout_->AddChild(bg_img);

    auto panel = std::make_shared<Panel>("create_panel");
    panel->SetAnchor(AnchorPoint::Center);
    panel->SetBounds({0, 0, 400, 450});
    panel->SetBackgroundColor(Color::DarkPanel);
    panel->SetCornerRadius(8);
    
    auto vert = std::make_shared<VerticalLayout>();
    vert->SetBounds({0, 0, 400, 450});
    vert->SetPadding(Padding::All(20));
    vert->SetSpacing(15);
    
    auto title = std::make_shared<Label>();
    title->SetText("Create Account");
    title->SetAlignment(Alignment::Center);
    title->SetBounds({0, 0, 360, 30});
    
    auto u_label = std::make_shared<Label>();
    u_label->SetText("Username");
    u_label->SetBounds({0, 0, 360, 20});
    
    username_input_ = std::make_shared<TextBox>();
    username_input_->SetBounds({0, 0, 360, 35});
    
    auto p_label = std::make_shared<Label>();
    p_label->SetText("Password");
    p_label->SetBounds({0, 0, 360, 20});
    
    password_input_ = std::make_shared<TextBox>();
    password_input_->SetBounds({0, 0, 360, 35});
    password_input_->SetPassword(true);
    
    auto cp_label = std::make_shared<Label>();
    cp_label->SetText("Confirm Password");
    cp_label->SetBounds({0, 0, 360, 20});
    
    confirm_password_input_ = std::make_shared<TextBox>();
    confirm_password_input_->SetBounds({0, 0, 360, 35});
    confirm_password_input_->SetPassword(true);
    
    auto h_layout = std::make_shared<HorizontalLayout>();
    h_layout->SetBounds({0, 0, 360, 40});
    h_layout->SetSpacing(20);
    
    auto back_btn = std::make_shared<Button>();
    back_btn->SetText("Back to Login");
    back_btn->SetBounds({0, 0, 160, 40});
    back_btn->OnClick([this]() {
        engine_->GetScreens().Replace(std::make_unique<LoginScreen>(engine_));
    });
    
    auto submit_btn = std::make_shared<Button>();
    submit_btn->SetText("Create");
    submit_btn->SetBounds({0, 0, 160, 40});
    submit_btn->OnClick([this]() {
        if (username_input_->GetText().empty() || password_input_->GetText().empty()) {
            status_label_->SetText("Fields cannot be empty.");
            return;
        }
        if (password_input_->GetText() != confirm_password_input_->GetText()) {
            status_label_->SetText("Passwords do not match.");
            return;
        }
        status_label_->SetText("Account created successfully!");
        status_label_->SetColor({50, 255, 50, 255});
    });
    
    h_layout->AddChild(back_btn);
    h_layout->AddChild(submit_btn);
    
    status_label_ = std::make_shared<Label>();
    status_label_->SetBounds({0, 0, 360, 20});
    status_label_->SetAlignment(Alignment::Center);
    status_label_->SetColor(Color::DarkError);
    
    vert->AddChild(title);
    vert->AddChild(u_label);
    vert->AddChild(username_input_);
    vert->AddChild(p_label);
    vert->AddChild(password_input_);
    vert->AddChild(cp_label);
    vert->AddChild(confirm_password_input_);
    vert->AddChild(h_layout);
    vert->AddChild(status_label_);
    
    panel->AddChild(vert);
    root_layout_->AddChild(panel);
}

void CreateAccountScreen::OnEnter() {}
void CreateAccountScreen::OnExit() {}
void CreateAccountScreen::OnPause() {}
void CreateAccountScreen::OnResume() {}

void CreateAccountScreen::OnResize(int width, int height) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, width, height});
        root_layout_->InvalidateLayout();
    }
}

void CreateAccountScreen::Render(const RenderContext& ctx) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, ctx.screen_width, ctx.screen_height});
        root_layout_->Render(ctx);
    }
}

bool CreateAccountScreen::HandleInput(const SDL_Event& event) {
    if (root_layout_) return root_layout_->HandleInput(event);
    return false;
}

void CreateAccountScreen::Update(float dt) {
    if (root_layout_) root_layout_->Update(dt);
}

} // namespace unboundmp::ui
'''
with open('d:/Unbound/pokemon/cpp/src/ui/screens/create_account_screen.cpp', 'w') as f:
    f.write(content)
