def update_button_input():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_button_input = '''bool Button::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEMOTION) {
        hovered_ = ContainsPoint(event.motion.x, event.motion.y);
        if (!hovered_) pressed_ = false;
        return hovered_;
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (hovered_) {
            pressed_ = true;
            return true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (pressed_ && hovered_) {
            pressed_ = false;
            if (on_click_) on_click_();
            return true;
        }
        pressed_ = false;
    }
    return false;
}'''

    new_button_input = '''bool Button::HandleInput(const SDL_Event& event) {
    if (!visible_ || !enabled_) return false;
    
    if (event.type == SDL_MOUSEMOTION) {
        bool was_hovered = hovered_;
        hovered_ = ContainsPoint(event.motion.x, event.motion.y);
        if (!hovered_) pressed_ = false;
        
        // Optional: Trigger hover animation
        if (hovered_ != was_hovered) {
            // we could push animation here if we had access to AnimationManager directly
        }
        return hovered_;
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (hovered_) {
            pressed_ = true;
            focused_ = true;
            return true;
        } else {
            focused_ = false;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (pressed_ && hovered_) {
            pressed_ = false;
            if (on_click_) on_click_();
            return true;
        }
        pressed_ = false;
    } else if (event.type == SDL_KEYDOWN) {
        if (focused_ && (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)) {
            pressed_ = true;
            return true;
        }
    } else if (event.type == SDL_KEYUP) {
        if (focused_ && pressed_ && (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)) {
            pressed_ = false;
            if (on_click_) on_click_();
            return true;
        }
    }
    return false;
}'''

    if 'SDLK_SPACE' not in old_button_input and 'SDLK_SPACE' not in content:
        content = content.replace(old_button_input, new_button_input)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

update_button_input()
print("widgets.cpp updated with Button input focus")
