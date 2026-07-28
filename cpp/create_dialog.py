def create_dialog():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if 'class Dialog' not in content_h:
        dialog_h = '''
class Dialog : public Window {
public:
    explicit Dialog(const std::string& title, const std::string& message);
    void OnConfirm(ClickCallback cb) { on_confirm_ = std::move(cb); }
    void OnCancel(ClickCallback cb) { on_cancel_ = std::move(cb); }
    
    // Override to also clear modal state
    bool HandleInput(const SDL_Event& event) override;

private:
    ClickCallback on_confirm_;
    ClickCallback on_cancel_;
};
'''
        content_h = content_h.replace('class Label', dialog_h + '\nclass Label')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    if 'Dialog::Dialog' not in content_cpp:
        dialog_cpp = '''
Dialog::Dialog(const std::string& title, const std::string& message) : Window("dialog") {
    SetTitle(title);
    SetBounds({0, 0, 300, 150});
    
    auto vert = std::make_shared<VerticalLayout>("dialog_vert");
    vert->SetBounds({0, 0, 300, 150});
    vert->SetPadding(Padding::All(20));
    vert->SetSpacing(20);
    
    auto msg_label = std::make_shared<Label>();
    msg_label->SetText(message);
    msg_label->SetAlignment(Alignment::Center);
    msg_label->SetBounds({0, 0, 260, 40}); // Will wrap or size correctly
    
    auto btn_layout = std::make_shared<HorizontalLayout>("dialog_btns");
    btn_layout->SetBounds({0, 0, 260, 30});
    btn_layout->SetSpacing(20);
    
    auto ok_btn = std::make_shared<Button>("dialog_ok");
    ok_btn->SetText("OK");
    ok_btn->SetBounds({0, 0, 120, 30});
    ok_btn->OnClick([this]() {
        if (on_confirm_) on_confirm_();
        if (WindowManager::GetInstance()) WindowManager::GetInstance()->ClearModal();
        SetVisible(false);
    });
    
    auto cancel_btn = std::make_shared<Button>("dialog_cancel");
    cancel_btn->SetText("Cancel");
    cancel_btn->SetBounds({0, 0, 120, 30});
    cancel_btn->OnClick([this]() {
        if (on_cancel_) on_cancel_();
        if (WindowManager::GetInstance()) WindowManager::GetInstance()->ClearModal();
        SetVisible(false);
    });
    
    btn_layout->AddChild(ok_btn);
    btn_layout->AddChild(cancel_btn);
    
    vert->AddChild(msg_label);
    vert->AddChild(btn_layout);
    
    AddChild(vert);
}

bool Dialog::HandleInput(const SDL_Event& event) {
    bool handled = Window::HandleInput(event);
    if (!IsVisible() && WindowManager::GetInstance()) { // Closed via 'X' titlebar
        WindowManager::GetInstance()->ClearModal();
    }
    return handled;
}
'''
        idx = content_cpp.find('// Label')
        if idx == -1: idx = content_cpp.find('void Label::Render')
        content_cpp = content_cpp[:idx] + dialog_cpp + content_cpp[idx:]
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)
        print('Dialog created')

create_dialog()
