#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "emulator/emulator_core.h"

struct mCore;
struct mCoreThread;
struct mCoreSync;

namespace unboundmp::emulator {

class MgbaEmulatorCore final : public IEmulatorCore {
 public:
  MgbaEmulatorCore();
  ~MgbaEmulatorCore() override;

  MgbaEmulatorCore(const MgbaEmulatorCore&) = delete;
  MgbaEmulatorCore& operator=(const MgbaEmulatorCore&) = delete;

  EmulatorResult Initialize() override;
  EmulatorResult LoadRom(const std::string& rom_path) override;
  EmulatorResult LoadSave(const std::string& save_path) override;
  EmulatorResult Start() override;
  EmulatorResult Pause() override;
  EmulatorResult Resume() override;
  EmulatorResult Reset() override;
  EmulatorResult Stop() override;
  void Shutdown() override;
  EmulatorState State() const override;

  // --- Stage 2: Save/Load State ---
  EmulatorResult SaveState(int slot);
  EmulatorResult LoadState(int slot);

  // --- Stage 2: Frame Callbacks ---
  using FrameCallback = std::function<void()>;
  size_t RegisterFrameCallback(FrameCallback cb);
  void UnregisterFrameCallback(size_t id);

  void SetInput(const InputState& state) override;

  uint8_t ReadU8(uint32_t address) override;
  uint16_t ReadU16(uint32_t address) override;
  uint32_t ReadU32(uint32_t address) override;

  void WriteU8(uint32_t address, uint8_t value) override;
  void WriteU16(uint32_t address, uint16_t value) override;
  void WriteU32(uint32_t address, uint32_t value) override;

  std::optional<std::string> GameTitle() const override;
  std::optional<std::string> GameCode() const override;

  // --- Stage 3: Status API ---
  bool IsRunning() const override;
  bool IsPaused() const override;
  bool IsLoaded() const override;
  uint32_t GetFrameCount() const override;
  float GetFPS() const override;
  float GetCurrentSpeed() const override;

  // --- Stage 3: ROM Info API ---
  uint32_t RomCrc32() const override;
  size_t RomSize() const override;
  std::optional<std::string> SaveType() const override;

  // --- Stage 3: Save Management ---
  bool HasSaveState(int slot) const override;
  EmulatorResult DeleteSaveState(int slot) override;
  std::vector<int> ListSaveStates() const override;
  std::optional<SaveStateMetadata> GetSaveStateMetadata(int slot) const override;

  // --- Stage 3: Event System ---
  size_t RegisterEventCallback(EventCallback cb) override;
  void UnregisterEventCallback(size_t id) override;

  // --- Stage 3: Logging ---
  static void RegisterLogCallback(LogCallback cb);

  const void* GetVideoBuffer() const override;
  size_t GetVideoPitch() const override;

  // Frame-pacing, mirroring mgba's own SDL frontend
  // (src/platform/sdl/sw-sdl2.c mSDLSWRunloop): the caller's render loop
  // should call SyncWaitFrameStart() each iteration, render only if it
  // returns true (a real video frame is ready and the emulator thread is
  // blocked waiting, so the buffer is safe to read without tearing), then
  // unconditionally call SyncWaitFrameEnd() to let the emulator thread
  // continue. This is what throttles the emulator thread to real time -
  // Start() enables core_->opts.videoSync so the thread blocks here every
  // frame instead of running uncapped.
  bool SyncWaitFrameStart();
  void SyncWaitFrameEnd();

  // Returns the sync object from the running thread (needed for audio
  // callback to call mCoreSyncLockAudio/mCoreSyncConsumeAudio).
  // Returns nullptr if the thread is not running.
  mCoreSync* GetSync();

  mCore* NativeCore() { return core_; }

 private:
  mCore* core_ = nullptr;
  mCoreThread* thread_ = nullptr;

  std::atomic<EmulatorState> state_{EmulatorState::kUninitialized};

  mutable std::mutex lifecycle_mutex_;

  bool rom_loaded_ = false;

  // Video framebuffer information
  unsigned video_width_ = 0;
  unsigned video_height_ = 0;

  // Pitch in bytes
  size_t video_pitch_ = 0;

  // Framebuffer owned by the frontend and supplied to mGBA.
  std::vector<uint32_t> video_buffer_;

  // Frame callbacks
  std::vector<std::pair<size_t, FrameCallback>> frame_callbacks_;
  size_t next_callback_id_ = 0;
  mutable std::mutex callback_mutex_;

  // Static thunk for mGBA thread frame callback
  static void OnFrameEnded(mCoreThread* thread);

  // Stage 3: FPS Tracking
  std::chrono::steady_clock::time_point last_fps_time_;
  uint32_t last_fps_frame_count_ = 0;
  float current_fps_ = 0.0f;

  // Stage 3: Events
  std::vector<std::pair<size_t, EventCallback>> event_callbacks_;
  size_t next_event_id_ = 0;
  mutable std::mutex event_mutex_;

  void FireEvent(EmulatorEvent event);
};

}  // namespace unboundmp::emulator