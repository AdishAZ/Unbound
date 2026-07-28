def update_main():
    path = 'd:/Unbound/pokemon/cpp/client/main.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # We need to restructure main.cpp
    # 1. Remove initial emulator init
    # 2. Add state machine inside loop
    
    # Let's just rewrite the whole file cleanly.
    new_main = '''#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <fstream>
#include <cstring>

#include "emulator/mgba_emulator_core.h"
#include "emulator/save_loader.h"
#include "render/sdl_renderer.h"
#include "ui/ui_engine.h"
#include "core/log_manager.h"
#include "ui/screens/login_screen.h"

extern "C" {
#include <mgba/core/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/sync.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/audio.h>
}

using namespace unboundmp::emulator;

struct AudioContext {
    mCore* core = nullptr;
    mCoreSync* sync = nullptr;
    SDL_AudioSpec obtainedSpec{};
};

static void audioCallback(void* userdata, Uint8* stream, int len) {
    auto* ctx = static_cast<AudioContext*>(userdata);
    if (!ctx || !ctx->core) {
        std::memset(stream, 0, len);
        return;
    }

    blip_t* left = ctx->core->getAudioChannel(ctx->core, 0);
    blip_t* right = ctx->core->getAudioChannel(ctx->core, 1);
    int32_t clockRate = ctx->core->frequency(ctx->core);

    double fauxClock = 1.0;
    if (ctx->sync) {
        if (ctx->sync->fpsTarget > 0) {
            fauxClock = GBAAudioCalculateRatio(1, ctx->sync->fpsTarget, 1);
        }
        mCoreSyncLockAudio(ctx->sync);
    }

    blip_set_rates(left,  clockRate, ctx->obtainedSpec.freq * fauxClock);
    blip_set_rates(right, clockRate, ctx->obtainedSpec.freq * fauxClock);

    int samples = len / (2 * ctx->obtainedSpec.channels);
    int available = blip_samples_avail(left);
    if (available > samples) {
        available = samples;
    }

    blip_read_samples(left, reinterpret_cast<short*>(stream), available,
                      ctx->obtainedSpec.channels == 2);
    if (ctx->obtainedSpec.channels == 2) {
        blip_read_samples(right, reinterpret_cast<short*>(stream) + 1,
                          available, 1);
    }

    if (ctx->sync) {
        mCoreSyncConsumeAudio(ctx->sync);
    }

    if (available < samples) {
        std::memset(reinterpret_cast<short*>(stream) +
                    ctx->obtainedSpec.channels * available,
                    0,
                    (samples - available) * ctx->obtainedSpec.channels * sizeof(short));
    }
}

int main()
{
    unboundmp::core::LogManager::Get().Initialize("logs", 5 * 1024 * 1024, 3);
    
    SDLRenderer renderer;
    if (!renderer.Initialize())
    {
        std::cout << "Failed to initialize SDL renderer.\\n";
        return 1;
    }

    unboundmp::ui::UIEngine ui_engine;
    if (!ui_engine.Initialize(renderer.GetNativeRenderer(), 240 * 4, 160 * 4)) {
        std::cout << "Failed to initialize UI Engine.\\n";
        return 1;
    }
    
    ui_engine.GetScreens().Push(std::make_unique<unboundmp::ui::LoginScreen>(&ui_engine));

    MgbaEmulatorCore emulator;
    bool emulator_booted = false;
    SDL_AudioDeviceID audioDeviceId = 0;
    AudioContext audioCtx;
    
    bool is_paused = false;
    bool p_key_was_down = false;
    bool f_keys_was_down[10] = {false};
    bool t_key_was_down = false;
    bool i_key_was_down = false;

    bool running = true;
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
        
        auto current_screen = ui_engine.GetScreens().GetCurrentScreen();
        bool in_game = current_screen && current_screen->GetName() == "GameScreen";
        
        // BOOT SEQUENCE
        if (in_game && !emulator_booted) {
            std::cout << "Booting Emulator...\\n";
            emulator_booted = true;
            
            auto result = emulator.Initialize();
            if (result) {
                result = emulator.LoadRom("cpp/client/Pokemon Unbound.gba");
                if (result) {
                    MgbaEmulatorCore::RegisterLogCallback([](LogLevel level, const std::string& msg) {
                        // Suppress logs for brevity
                    });
                    
                    // Priority 1: Load Save State 1 (to bypass title screen)
                    result = emulator.LoadState(1);
                    if (!result) {
                        std::cout << "No Save State 1 found. Loading battery save.\\n";
                        SaveLoader::LoadDefault(emulator, "cpp/client/Pokemon Unbound.gba");
                    } else {
                        std::cout << "Save State 1 loaded! Bypassing title screen.\\n";
                    }
                    
                    emulator.Start();
                    
                    if (SDL_InitSubSystem(SDL_INIT_AUDIO) >= 0) {
                        audioCtx.core = emulator.NativeCore();
                        audioCtx.sync = emulator.GetSync();
                        SDL_AudioSpec desired{};
                        desired.freq = 44100;
                        desired.format = AUDIO_S16SYS;
                        desired.channels = 2;
                        desired.samples = 1024;
                        desired.callback = audioCallback;
                        desired.userdata = &audioCtx;
                        audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &audioCtx.obtainedSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
                        if (audioDeviceId != 0) {
                            SDL_PauseAudioDevice(audioDeviceId, 0);
                        }
                    }
                }
            }
        }
        
        // If emulator is running, handle its logic
        if (emulator_booted && emulator.IsRunning()) {
            InputState input;
            const Uint8* keys = SDL_GetKeyboardState(nullptr);
            if (keys[SDL_SCANCODE_W]) input.Press(GbaButton::kUp);
            if (keys[SDL_SCANCODE_S]) input.Press(GbaButton::kDown);
            if (keys[SDL_SCANCODE_A]) input.Press(GbaButton::kLeft);
            if (keys[SDL_SCANCODE_D]) input.Press(GbaButton::kRight);
            if (keys[SDL_SCANCODE_E]) input.Press(GbaButton::kA);
            if (keys[SDL_SCANCODE_Q]) input.Press(GbaButton::kB);
            if (keys[SDL_SCANCODE_RETURN]) input.Press(GbaButton::kStart);
            if (keys[SDL_SCANCODE_BACKSPACE]) input.Press(GbaButton::kSelect);
            if (keys[SDL_SCANCODE_R]) input.Press(GbaButton::kL);
            if (keys[SDL_SCANCODE_F]) input.Press(GbaButton::kR);
            emulator.SetInput(input);
            
            // Save state logic
            for (int i = 0; i < 5; ++i) {
                bool save_key_down = keys[SDL_SCANCODE_F1 + i];
                if (save_key_down && !f_keys_was_down[i]) {
                    emulator.SaveState(i + 1);
                }
                f_keys_was_down[i] = save_key_down;
                
                bool load_key_down = keys[SDL_SCANCODE_F6 + i];
                if (load_key_down && !f_keys_was_down[i + 5]) {
                    emulator.LoadState(i + 1);
                }
                f_keys_was_down[i + 5] = load_key_down;
            }
            
            if (emulator.SyncWaitFrameStart()) {
                const void* framebuffer = emulator.GetVideoBuffer();
                size_t pitch = emulator.GetVideoPitch();
                if (framebuffer != nullptr && pitch > 0) {
                    renderer.Render(framebuffer, static_cast<int>(pitch));
                }
            }
            
            ui_engine.Render();
            renderer.Present();
            emulator.SyncWaitFrameEnd();
        } else {
            // Emulation not booted, just render UI over black background
            SDL_SetRenderDrawColor(renderer.GetNativeRenderer(), 0, 0, 0, 255);
            SDL_RenderClear(renderer.GetNativeRenderer());
            ui_engine.Render();
            renderer.Present();
        }
    }

    if (audioDeviceId != 0) {
        SDL_PauseAudioDevice(audioDeviceId, 1);
        SDL_CloseAudioDevice(audioDeviceId);
    }
    
    if (emulator_booted) {
        emulator.Stop();
        emulator.Shutdown();
    }
    renderer.Shutdown();
    return 0;
}
'''
    with open(path, 'w', encoding='utf-8') as f:
        f.write(new_main)
    print('Updated main.cpp')

update_main()
