def update_textbox_input():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_textbox_input = '''bool TextBox::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        focused_ = ContainsPoint(event.button.x, event.button.y);
        if (focused_) {
            SDL_StartTextInput();
            return true;
        } else {
            SDL_StopTextInput();
        }
    } else if (focused_) {
        if (event.type == SDL_TEXTINPUT) {
            text_ += event.text.text;
            cursor_pos_ = text_.length();
            if (on_text_changed_) on_text_changed_(text_);
            return true;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_BACKSPACE && !text_.empty()) {
                text_.pop_back();
                cursor_pos_ = text_.length();
                if (on_text_changed_) on_text_changed_(text_);
                return true;
            }
        }
    }
    return false;
}'''

    new_textbox_input = '''bool TextBox::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        bool was_focused = focused_;
        focused_ = ContainsPoint(event.button.x, event.button.y);
        if (focused_ && !was_focused) {
            SDL_StartTextInput();
            return true;
        } else if (!focused_ && was_focused) {
            SDL_StopTextInput();
        }
    } else if (focused_) {
        if (event.type == SDL_TEXTINPUT) {
            text_ += event.text.text;
            cursor_pos_ = text_.length();
            if (on_text_changed_) on_text_changed_(text_);
            return true;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_BACKSPACE && !text_.empty()) {
                text_.pop_back();
                cursor_pos_ = text_.length();
                if (on_text_changed_) on_text_changed_(text_);
                return true;
            }
        }
    }
    return false;
}'''

    content = content.replace(old_textbox_input, new_textbox_input)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

update_textbox_input()
print("widgets.cpp updated with TextBox input")
