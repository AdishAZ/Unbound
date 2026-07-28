#pragma once
#include "emulator/mgba_emulator_core.h"
#include <SDL2/SDL.h>
#include <string>
#include <memory>

extern "C" {
struct mCore;
struct mCoreSync;
}

namespace unboundmp::input { class MenuInputInterceptor; }

namespace unboundmp::emulator {

struct AudioContext {
    mCore* core = nullptr;
    mCoreSync* sync = nullptr;
    SDL_AudioSpec obtainedSpec{};
};

class GameBootstrap {
public:
    static GameBootstrap& GetInstance() {
        static GameBootstrap instance;
        return instance;
    }

    bool Initialize();
    void LoadSaveState();
    void Update(const Uint8* keyboard_state, unboundmp::input::MenuInputInterceptor* interceptor, bool is_overlay_open);
    bool IsBooted() const { return emulator_booted_; }
    void Shutdown();
    
    MgbaEmulatorCore& GetEmulator() { return emulator_; }

private:
    GameBootstrap() = default;
    ~GameBootstrap() = default;

    GameBootstrap(const GameBootstrap&) = delete;
    GameBootstrap& operator=(const GameBootstrap&) = delete;

    MgbaEmulatorCore emulator_;
    bool emulator_booted_ = false;
    
    SDL_AudioDeviceID audio_device_id_ = 0;
    AudioContext audio_ctx_;
    
    bool f_keys_was_down_[10] = {false};
};

} // namespace unboundmp::emulator
