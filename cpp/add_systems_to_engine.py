def add_systems_to_engine():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/ui_engine.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if '#include "ui/asset_manager.h"' not in content_h:
        content_h = content_h.replace('#include "ui/text_renderer.h"', '#include "ui/text_renderer.h"\n#include "ui/asset_manager.h"\n#include "ui/dev_overlay.h"')
        content_h = content_h.replace('TextRenderer& GetTextRenderer() { return *text_renderer_; }', 'TextRenderer& GetTextRenderer() { return *text_renderer_; }\n    AssetManager& GetAssetManager() { return *asset_manager_; }\n    DevOverlay& GetDevOverlay() { return *dev_overlay_; }')
        content_h = content_h.replace('std::unique_ptr<TextRenderer> text_renderer_;', 'std::unique_ptr<TextRenderer> text_renderer_;\n    std::unique_ptr<AssetManager> asset_manager_;\n    std::unique_ptr<DevOverlay> dev_overlay_;')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    if 'asset_manager_ = std::make_unique<AssetManager>();' not in content_cpp:
        old_init = '''    text_renderer_ = std::make_unique<TextRenderer>(font_manager_.get());'''
        new_init = '''    text_renderer_ = std::make_unique<TextRenderer>(font_manager_.get());\n    asset_manager_ = std::make_unique<AssetManager>();\n    asset_manager_->Initialize(renderer);\n    dev_overlay_ = std::make_unique<DevOverlay>();'''
        content_cpp = content_cpp.replace(old_init, new_init)

        old_render = '''    if (notification_center_) {
        notification_center_->Render(render_context_);
    }'''
        new_render = '''    if (notification_center_) {
        notification_center_->Render(render_context_);
    }
    if (dev_overlay_->IsVisible()) {
        dev_overlay_->Render(render_context_);
    }'''
        content_cpp = content_cpp.replace(old_render, new_render)
        
        old_update = '''    if (notification_center_) {
        notification_center_->Update(dt);
    }'''
        new_update = '''    if (notification_center_) {
        notification_center_->Update(dt);
    }
    if (dev_overlay_->IsVisible()) {
        dev_overlay_->Update(dt);
    }'''
        content_cpp = content_cpp.replace(old_update, new_update)

        old_handle = '''    if (event.type == SDL_WINDOWEVENT) {'''
        new_handle = '''    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKQUOTE) {
        dev_overlay_->Toggle();
        return true;
    }
    
    if (event.type == SDL_WINDOWEVENT) {'''
        content_cpp = content_cpp.replace(old_handle, new_handle)
        
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)

add_systems_to_engine()
print("UIEngine updated with AssetManager and DevOverlay")
