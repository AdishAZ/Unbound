def modify_main():
    path = 'd:/Unbound/pokemon/cpp/client/main.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Add headers
    if '#include "ui/ui_engine.h"' not in content:
        content = content.replace('#include "render/sdl_renderer.h"', '#include "render/sdl_renderer.h"\n#include "ui/ui_engine.h"\n#include "ui/screens/login_screen.h"')

    # Add UIEngine init right after renderer.Initialize()
    old_init = '''    if (!renderer.Initialize())
    {
        std::cout << "Failed to initialize SDL renderer.\\n";
        SDL_PauseAudioDevice(audioDeviceId, 1);
        SDL_CloseAudioDevice(audioDeviceId);
        emulator.Stop();
        emulator.Shutdown();
        return 1;
    }'''

    new_init = '''    if (!renderer.Initialize())
    {
        std::cout << "Failed to initialize SDL renderer.\\n";
        SDL_PauseAudioDevice(audioDeviceId, 1);
        SDL_CloseAudioDevice(audioDeviceId);
        emulator.Stop();
        emulator.Shutdown();
        return 1;
    }

    unboundmp::ui::UIEngine ui_engine;
    if (!ui_engine.Initialize(renderer.GetNativeRenderer(), 240 * 4, 160 * 4)) {
        std::cout << "Failed to initialize UI Engine.\\n";
        return 1;
    }
    ui_engine.GetScreens().Push(std::make_unique<unboundmp::ui::LoginScreen>(&ui_engine));'''

    content = content.replace(old_init, new_init)

    # Change the event loop
    old_loop = '''    while (renderer.ProcessEvents())
    {
        // -----------------------------
        // Keyboard Input
        // -----------------------------
        InputState input;

        const Uint8* keys = SDL_GetKeyboardState(nullptr);'''

    new_loop = '''    bool running = true;
    Uint32 last_time = SDL_GetTicks();
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
            ui_engine.HandleInput(e);
        }

        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        ui_engine.Update(dt);

        // -----------------------------
        // Keyboard Input
        // -----------------------------
        InputState input;

        const Uint8* keys = SDL_GetKeyboardState(nullptr);'''

    content = content.replace(old_loop, new_loop)

    # Change the render phase
    old_render = '''        // SyncWaitFrameStart/End synchronize the framebuffer handoff
        // (prevents tearing). The actual FPS throttle is audio-driven.
        if (emulator.SyncWaitFrameStart()) {
            if (framebuffer != nullptr && pitch > 0)
            {
                renderer.Render(framebuffer, static_cast<int>(pitch));
            }
        }
        emulator.SyncWaitFrameEnd();'''

    new_render = '''        // SyncWaitFrameStart/End synchronize the framebuffer handoff
        // (prevents tearing). The actual FPS throttle is audio-driven.
        if (emulator.SyncWaitFrameStart()) {
            if (framebuffer != nullptr && pitch > 0)
            {
                renderer.Render(framebuffer, static_cast<int>(pitch));
            }
        }
        
        ui_engine.Render();
        renderer.Present();
        
        emulator.SyncWaitFrameEnd();'''

    content = content.replace(old_render, new_render)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

modify_main()
print("main.cpp modified")
