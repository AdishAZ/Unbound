#include "emulator/game_bootstrap.h"
#include "emulator/save_loader.h"
#include "core/log_manager.h"
#include <cstring>
#include <iostream>
#include "persistence/client_save_manager.h"
#include "input/menu_input_interceptor.h"

extern "C" {
#include <mgba/core/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/sync.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/audio.h>
}

namespace unboundmp::emulator {

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

bool GameBootstrap::Initialize() {
    if (emulator_booted_) return true;

    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "GameBootstrap: Initializing emulator...");
    
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "--- BOOT SEQUENCE START ---");
    
    auto init_res = emulator_.Initialize();
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
        "1. Emulator created | Success: " + std::to_string(init_res.ok) + " | Msg: " + init_res.message);
    
    if (!init_res.ok) return false;

    std::string rom_path = "cpp/client/Pokemon Unbound.gba";
    auto rom_res = emulator_.LoadRom(rom_path);
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
        "2. ROM loaded | Path: " + rom_path + " | Success: " + std::to_string(rom_res.ok) + " | Msg: " + rom_res.message);

    if (!rom_res.ok) return false;

    MgbaEmulatorCore::RegisterLogCallback([](LogLevel level, const std::string& msg) {});

    emulator_booted_ = true;
    return true;
}

void GameBootstrap::LoadSaveState() {
    if (!emulator_booted_) return;
    
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
        std::string("WRAM before load: ") + std::to_string(emulator_.ReadU32(0x02000000)) + " " + std::to_string(emulator_.ReadU32(0x02000004)));

    // Load SRAM via ClientSaveManager BEFORE starting the core
    persistence::ClientSaveManager::GetInstance().LoadGameSave(emulator_);

    // Start emulator thread. This calls mCoreThreadStart, which calls core->reset(core)
    // and boots up the game ROM. We MUST do this before loading the savestate,
    // otherwise the savestate is wiped by the reset.
    auto start_res = emulator_.Start();
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
        "5. Emulator resumed | Success: " + std::to_string(start_res.ok) + " | Msg: " + start_res.message);

    // Now apply Savestate which will inject the state directly into the running core (thread is automatically interrupted and continued).
    persistence::ClientSaveManager::GetInstance().LoadSavestate(emulator_);

    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
        std::string("WRAM after load: ") + std::to_string(emulator_.ReadU32(0x02000000)) + " " + std::to_string(emulator_.ReadU32(0x02000004)));

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) >= 0) {
        audio_ctx_.core = emulator_.NativeCore();
        audio_ctx_.sync = emulator_.GetSync();
        SDL_AudioSpec desired{};
        desired.freq = 44100;
        desired.format = AUDIO_S16SYS;
        desired.channels = 2;
        desired.samples = 1024;
        desired.callback = audioCallback;
        desired.userdata = &audio_ctx_;
        audio_device_id_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &audio_ctx_.obtainedSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        if (audio_device_id_ != 0) {
            SDL_PauseAudioDevice(audio_device_id_, 0);
        }
    }
}

void GameBootstrap::Update(const Uint8* keys, unboundmp::input::MenuInputInterceptor* interceptor, bool is_overlay_open) {
    if (!emulator_booted_ || !emulator_.IsRunning()) return;

    InputState input;
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
                
    if (interceptor) {
        interceptor->FilterInput(input, is_overlay_open);
    }
                
    emulator_.SetInput(input);
    
    // Save state logic
    for (int i = 0; i < 5; ++i) {
        bool save_key_down = keys[SDL_SCANCODE_F1 + i];
        if (save_key_down && !f_keys_was_down_[i]) {
            emulator_.SaveState(i + 1);
        }
        f_keys_was_down_[i] = save_key_down;
        
        bool load_key_down = keys[SDL_SCANCODE_F6 + i];
        if (load_key_down && !f_keys_was_down_[i + 5]) {
            emulator_.LoadState(i + 1);
        }
        f_keys_was_down_[i + 5] = load_key_down;
    }
}

void GameBootstrap::Shutdown() {
    if (audio_device_id_ != 0) {
        SDL_PauseAudioDevice(audio_device_id_, 1);
        SDL_CloseAudioDevice(audio_device_id_);
        audio_device_id_ = 0;
    }
    
    if (emulator_booted_) {
        persistence::ClientSaveManager::GetInstance().ForceSyncSave();
        
        emulator_.Stop();
        emulator_.Shutdown();
        emulator_booted_ = false;
    }
}

} // namespace unboundmp::emulator
