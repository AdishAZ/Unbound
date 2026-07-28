def fix_ui_engine():
    # 1. Fix UIEngine::Initialize
    path_engine = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path_engine, 'r', encoding='utf-8') as f:
        content_e = f.read()
    
    old_init = '''bool UIEngine::Initialize(SDL_Renderer* renderer, int width, int height) {
    if (initialized_) return true;

    render_context_.renderer = renderer;
    // render_context_.window_size = Size{width, height}; // Assuming Size has this'''
    
    new_init = '''bool UIEngine::Initialize(SDL_Renderer* renderer, int width, int height) {
    if (initialized_) return true;

    render_context_.renderer = renderer;
    render_context_.screen_width = width;
    render_context_.screen_height = height;'''
    
    if old_init in content_e:
        content_e = content_e.replace(old_init, new_init)
        with open(path_engine, 'w', encoding='utf-8') as f:
            f.write(content_e)

    # 2. Fix widgets.h Container::SetBounds
    path_widgets_h = 'd:/Unbound/pokemon/cpp/include/ui/widgets.h'
    with open(path_widgets_h, 'r', encoding='utf-8') as f:
        content_wh = f.read()
        
    old_setbounds = '''    void SetBounds(const Rect& bounds) override {
        if (bounds_.width != bounds.width || bounds_.height != bounds.height) layout_dirty_ = true;
        Widget::SetBounds(bounds);
    }'''
    new_setbounds = '''    void SetBounds(const Rect& bounds) override {
        if (bounds_.x != bounds.x || bounds_.y != bounds.y || bounds_.width != bounds.width || bounds_.height != bounds.height) layout_dirty_ = true;
        Widget::SetBounds(bounds);
    }'''
    if old_setbounds in content_wh:
        content_wh = content_wh.replace(old_setbounds, new_setbounds)
        with open(path_widgets_h, 'w', encoding='utf-8') as f:
            f.write(content_wh)

    # 3. Fix Panel::Render in widgets.cpp
    path_widgets_cpp = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path_widgets_cpp, 'r', encoding='utf-8') as f:
        content_wcpp = f.read()
        
    old_panel_render = '''void Panel::Render(const RenderContext& ctx) {
    if (!visible_) return;
    if (bg_color_.a > 0) {
        SDL_SetRenderDrawColor(ctx.renderer, bg_color_.r, bg_color_.g, bg_color_.b, static_cast<Uint8>(bg_color_.a * ctx.alpha));
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_Rect r = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
        SDL_RenderFillRect(ctx.renderer, &r);
    }
    if (border_thickness_ > 0 && border_color_.a > 0) {'''
    
    new_panel_render = '''void Panel::Render(const RenderContext& ctx) {
    if (!visible_) return;
    Color bg = bg_color_;
    if (bg.a == 0 && ctx.theme) bg = ctx.theme->GetSurface();
    
    if (bg.a > 0) {
        int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 0;
        if (corner_radius_ > 0) radius = corner_radius_;
        if (radius > 0) {
            ctx.DrawFilledRoundedRect(bounds_, bg, radius);
        } else {
            SDL_SetRenderDrawColor(ctx.renderer, bg.r, bg.g, bg.b, static_cast<Uint8>(bg.a * ctx.alpha));
            SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
            SDL_Rect r = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
            SDL_RenderFillRect(ctx.renderer, &r);
        }
    }
    
    Color border = border_color_;
    if (border.a == 0 && ctx.theme) border = ctx.theme->GetBorder();
    
    if (border_thickness_ > 0 && border.a > 0) {'''
    
    if old_panel_render in content_wcpp:
        content_wcpp = content_wcpp.replace(old_panel_render, new_panel_render)
        with open(path_widgets_cpp, 'w', encoding='utf-8') as f:
            f.write(content_wcpp)

    print('Applied fixes')

fix_ui_engine()
