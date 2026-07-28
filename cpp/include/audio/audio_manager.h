#pragma once

#include <SDL.h>
#include <cstdint>

namespace unboundmp::audio {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    bool Initialize();
    void Shutdown();
    
    // Volume controls (0.0 - 1.0)
    void SetMasterVolume(float volume);
    void SetMusicVolume(float volume);
    void SetSfxVolume(float volume);
    float GetMasterVolume() const;
    float GetMusicVolume() const;
    float GetSfxVolume() const;
    
    // Mute
    void SetMuted(bool muted);
    bool IsMuted() const;
    void SetMusicMuted(bool muted);
    void SetSfxMuted(bool muted);
    
    // Scale the emulator's audio output buffer
    void ProcessAudioBuffer(int16_t* buffer, int sample_count);
    
    // Notification sounds (procedural sine wave beeps)
    void PlayConnectSound();
    void PlayDisconnectSound();
    void PlayAutosaveSound();
    void PlayErrorSound();
    void PlayNotificationSound();
    
private:
    float master_volume_ = 1.0f;
    float music_volume_ = 1.0f;
    float sfx_volume_ = 1.0f;
    bool muted_ = false;
    bool music_muted_ = false;
    bool sfx_muted_ = false;
    
    // For procedural audio
    SDL_AudioDeviceID sfx_device_ = 0;
    void GenerateTone(float frequency, float duration_ms, float volume);
};

} // namespace unboundmp::audio
