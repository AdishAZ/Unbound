#include "audio/audio_manager.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace unboundmp::audio {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Initialize() {
    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);
    desired.freq = 44100;
    desired.format = AUDIO_F32SYS;
    desired.channels = 1;
    desired.samples = 2048;
    desired.callback = nullptr; // Push model

    sfx_device_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (sfx_device_ == 0) {
        return false;
    }
    
    SDL_PauseAudioDevice(sfx_device_, 0);
    return true;
}

void AudioManager::Shutdown() {
    if (sfx_device_ != 0) {
        SDL_CloseAudioDevice(sfx_device_);
        sfx_device_ = 0;
    }
}

void AudioManager::SetMasterVolume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
}

void AudioManager::SetMusicVolume(float volume) {
    music_volume_ = std::clamp(volume, 0.0f, 1.0f);
}

void AudioManager::SetSfxVolume(float volume) {
    sfx_volume_ = std::clamp(volume, 0.0f, 1.0f);
}

float AudioManager::GetMasterVolume() const {
    return master_volume_;
}

float AudioManager::GetMusicVolume() const {
    return music_volume_;
}

float AudioManager::GetSfxVolume() const {
    return sfx_volume_;
}

void AudioManager::SetMuted(bool muted) {
    muted_ = muted;
}

bool AudioManager::IsMuted() const {
    return muted_;
}

void AudioManager::SetMusicMuted(bool muted) {
    music_muted_ = muted;
}

void AudioManager::SetSfxMuted(bool muted) {
    sfx_muted_ = muted;
}

void AudioManager::ProcessAudioBuffer(int16_t* buffer, int sample_count) {
    if (muted_ || music_muted_ || master_volume_ == 0.0f || music_volume_ == 0.0f) {
        std::fill(buffer, buffer + sample_count, 0);
        return;
    }
    
    float scale = master_volume_ * music_volume_;
    for (int i = 0; i < sample_count; ++i) {
        float sample = buffer[i] * scale;
        buffer[i] = static_cast<int16_t>(std::clamp(sample, -32768.0f, 32767.0f));
    }
}

void AudioManager::GenerateTone(float frequency, float duration_ms, float volume) {
    if (muted_ || sfx_muted_ || sfx_device_ == 0 || master_volume_ == 0.0f || sfx_volume_ == 0.0f || volume == 0.0f) {
        return;
    }

    float final_volume = master_volume_ * sfx_volume_ * volume;
    int sample_rate = 44100;
    int num_samples = static_cast<int>((duration_ms / 1000.0f) * sample_rate);
    std::vector<float> buffer(num_samples);

    for (int i = 0; i < num_samples; ++i) {
        float time = static_cast<float>(i) / sample_rate;
        buffer[i] = final_volume * std::sin(2.0f * static_cast<float>(M_PI) * frequency * time);
    }

    SDL_QueueAudio(sfx_device_, buffer.data(), static_cast<Uint32>(num_samples * sizeof(float)));
}

void AudioManager::PlayConnectSound() {
    GenerateTone(880.0f, 100.0f, 0.5f);
}

void AudioManager::PlayDisconnectSound() {
    GenerateTone(440.0f, 200.0f, 0.5f);
}

void AudioManager::PlayAutosaveSound() {
    GenerateTone(660.0f, 50.0f, 0.3f);
}

void AudioManager::PlayErrorSound() {
    GenerateTone(220.0f, 300.0f, 0.8f);
}

void AudioManager::PlayNotificationSound() {
    GenerateTone(1000.0f, 80.0f, 0.5f);
}

} // namespace unboundmp::audio
