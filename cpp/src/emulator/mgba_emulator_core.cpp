#include "emulator/mgba_emulator_core.h"
#include <iostream> 
#include <cstring>
#include "core/log_manager.h"

// Real libmgba public headers (verified present/matching in
// libmgba-dev 0.10.2, Ubuntu 24.04 "noble"). See cpp/CMakeLists.txt for how
// these are located.
#include <mgba/core/interface.h>
#include <mgba/core/core.h>
#include <mgba/core/sync.h>
#include <mgba/core/thread.h>
#include <mgba/core/serialize.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>
#include <mgba/gba/core.h>

namespace unboundmp::emulator {

namespace {

// GBA cartridge header field sizes, from the hardware header layout
// (GBATek "GBA Cartridge Header"): game title is 12 bytes at 0x0A0, game
// code is 4 bytes at 0x0AC. libmgba's getGameTitle()/getGameCode() write a
// null-terminated copy into a caller-provided buffer without taking a
// length, so buffers here are sized generously above the hardware max
// rather than exactly at it, purely as a safety margin against any
// mgba-internal formatting (e.g. appended punctuation) - not a claim about
// Unbound's specific header contents.
constexpr size_t kTitleBufferSize = 32;
constexpr size_t kCodeBufferSize = 16;

std::vector<LogCallback> g_log_callbacks;
std::mutex g_log_mutex;

void StaticLogProxy(struct mLogger* logger, int category, enum mLogLevel level, const char* format, va_list args) {
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), format, args);
  std::string msg(buffer);

  LogLevel out_level = LogLevel::kInfo;
  if (level & mLOG_FATAL || level & mLOG_ERROR || level & mLOG_GAME_ERROR) out_level = LogLevel::kError;
  else if (level & mLOG_WARN) out_level = LogLevel::kWarning;
  else if (level & mLOG_DEBUG) out_level = LogLevel::kDebug;

  std::lock_guard<std::mutex> lock(g_log_mutex);
  for (auto& cb : g_log_callbacks) {
    cb(out_level, msg);
  }
}

mLogger g_mgba_logger = {
  StaticLogProxy,
  nullptr
};

}  // namespace

MgbaEmulatorCore::MgbaEmulatorCore() = default;

MgbaEmulatorCore::~MgbaEmulatorCore() {
  Shutdown();
}

EmulatorResult MgbaEmulatorCore::Initialize() {
  std::lock_guard<std::mutex> lock(lifecycle_mutex_);

  if (state_.load() != EmulatorState::kUninitialized) {
    return EmulatorResult::Failure("Initialize() called more than once");
  }

  core_ = GBACoreCreate();
  if (core_ == nullptr) {
    return EmulatorResult::Failure("GBACoreCreate() returned null");
  }

  if (!core_->init(core_)) {
    core_ = nullptr;  // core->init() releases the core itself on failure
    return EmulatorResult::Failure("mCore::init() failed");
  }

  mCoreInitConfig(core_, "unboundmp");
  core_->desiredVideoDimensions(
    core_,
    &video_width_,
    &video_height_);

  video_buffer_.resize(video_width_ * video_height_);

  video_pitch_ = video_width_ * sizeof(uint32_t);

  core_->setVideoBuffer(
    core_,
    reinterpret_cast<color_t*>(video_buffer_.data()),
    video_width_);

  state_.store(EmulatorState::kNoRom);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::LoadRom(const std::string& rom_path) {
  std::lock_guard<std::mutex> lock(lifecycle_mutex_);

  if (state_.load() != EmulatorState::kNoRom) {
    return EmulatorResult::Failure(
        "LoadRom() requires state kNoRom (call Initialize() first, and "
        "don't call LoadRom() twice)");
  }

  // mCoreLoadFile opens rom_path read-only via VFileOpen internally and
  // hands the resulting VFile to core_->loadROM(). The ROM bytes are never
  // touched, copied-and-modified, or written back by us - this is a
  // straight pass-through of the path to libmgba's own loader.
  if (!mCoreLoadFile(core_, rom_path.c_str())) {
    return EmulatorResult::Failure("mCoreLoadFile() failed for: " + rom_path);
  }

  rom_loaded_ = true;
  state_.store(EmulatorState::kStopped);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::LoadSave(const std::string& save_path) {
  std::lock_guard<std::mutex> lock(lifecycle_mutex_);

  if (!rom_loaded_) {
    return EmulatorResult::Failure("LoadSave() requires a ROM to already be loaded");
  }

  // temporary=false: writes made during play are persisted back to
  // save_path (normal play save, as opposed to a scratch/throwaway save).
  if (!mCoreLoadSaveFile(core_, save_path.c_str(), false)) {
    return EmulatorResult::Failure("mCoreLoadSaveFile() failed for: " + save_path);
  }

  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::Start() {
  std::lock_guard<std::mutex> lock(lifecycle_mutex_);

  const EmulatorState current = state_.load();
  if (current != EmulatorState::kStopped) {
    return EmulatorResult::Failure(
        "Start() requires state kStopped (ROM must be loaded, and the core "
        "must not already be running)");
  }

  if (thread_ == nullptr) {
    thread_ = new mCoreThread();
    std::memset(thread_, 0, sizeof(mCoreThread));
  }
  thread_->core = core_;
  thread_->userData = this;
  thread_->frameCallback = &MgbaEmulatorCore::OnFrameEnded;

  core_->opts.videoSync = true; // Let's force videoSync true just to test if this was the issue!
  core_->opts.audioSync = false; // Disable audioSync for testing! If this works, audio callback is the culprit.
  core_->opts.fpsTarget = 60.0f;
  core_->opts.audioBuffers = 1024;

  core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "MgbaEmulatorCore: Starting mCoreThread...");

  if (!mCoreThreadStart(thread_)) {
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Error, "MgbaEmulatorCore: mCoreThreadStart() failed!");
    return EmulatorResult::Failure("mCoreThreadStart() failed");
  }
  
  core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "MgbaEmulatorCore: mCoreThreadStart() succeeded!");

  state_.store(EmulatorState::kRunning);
  
  last_fps_time_ = std::chrono::steady_clock::now();
  last_fps_frame_count_ = 0;
  
  FireEvent(EmulatorEvent::kStarted);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::SaveState(int slot) {
  if (core_ == nullptr) {
    return EmulatorResult::Failure("Cannot save state: core is not initialized");
  }
  if (!rom_loaded_) {
    return EmulatorResult::Failure("Cannot save state: no ROM loaded");
  }

  // Interrupt thread if running to ensure save is thread-safe
  bool was_running = (state_.load() == EmulatorState::kRunning);
  if (was_running && thread_ != nullptr) {
    mCoreThreadInterrupt(thread_);
  }

  bool success = mCoreSaveState(core_, slot, SAVESTATE_ALL);

  if (was_running && thread_ != nullptr) {
    mCoreThreadContinue(thread_);
  }

  if (!success) {
    return EmulatorResult::Failure("mCoreSaveState() failed for slot " + std::to_string(slot));
  }
  
  FireEvent(EmulatorEvent::kSaveStateCreated);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::LoadState(int slot) {
  if (core_ == nullptr) {
    return EmulatorResult::Failure("Cannot load state: core is not initialized");
  }
  if (!rom_loaded_) {
    return EmulatorResult::Failure("Cannot load state: no ROM loaded");
  }

  // Interrupt thread if running to ensure load is thread-safe
  bool was_running = (state_.load() == EmulatorState::kRunning);
  if (was_running && thread_ != nullptr) {
    mCoreThreadInterrupt(thread_);
  }

  bool success = mCoreLoadState(core_, slot, SAVESTATE_ALL);

  if (was_running && thread_ != nullptr) {
    mCoreThreadContinue(thread_);
  }

  if (!success) {
    return EmulatorResult::Failure("mCoreLoadState() failed for slot " + std::to_string(slot) + " (slot may be empty/invalid)");
  }
  
  FireEvent(EmulatorEvent::kSaveStateLoaded);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::Pause() {
  const EmulatorState current = state_.load();
  if (current == EmulatorState::kPaused) {
    return EmulatorResult::Success();  // already paused, no-op
  }
  if (current != EmulatorState::kRunning) {
    return EmulatorResult::Failure("Pause() requires state kRunning");
  }

  mCoreThreadPause(thread_);
  state_.store(EmulatorState::kPaused);
  FireEvent(EmulatorEvent::kPaused);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::Resume() {
  const EmulatorState current = state_.load();
  if (current == EmulatorState::kRunning) {
    return EmulatorResult::Success();  // already running, no-op
  }
  if (current != EmulatorState::kPaused) {
    return EmulatorResult::Failure("Resume() requires state kPaused");
  }

  mCoreThreadUnpause(thread_);
  state_.store(EmulatorState::kRunning);
  last_fps_time_ = std::chrono::steady_clock::now();
  FireEvent(EmulatorEvent::kResumed);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::Reset() {
  const EmulatorState current = state_.load();
  if (current != EmulatorState::kRunning && current != EmulatorState::kPaused) {
    return EmulatorResult::Failure("Reset() requires the core to be running or paused");
  }

  // mCoreThreadReset (rather than calling core_->reset() directly)
  // correctly synchronizes the reset with the core's own thread instead of
  // racing it.
  mCoreThreadReset(thread_);
  FireEvent(EmulatorEvent::kSoftReset);
  return EmulatorResult::Success();
}

EmulatorResult MgbaEmulatorCore::Stop() {
  std::lock_guard<std::mutex> lock(lifecycle_mutex_);

  const EmulatorState current = state_.load();
  if (current != EmulatorState::kRunning && current != EmulatorState::kPaused) {
    return EmulatorResult::Success();  // already stopped, no-op
  }

  if (current == EmulatorState::kPaused) {
    // mCoreThreadEnd must be issued to a running thread; unpause first so
    // the end request is actually observed rather than sitting behind a
    // paused core forever.
    mCoreThreadUnpause(thread_);
  }

  mCoreThreadEnd(thread_);
  mCoreThreadJoin(thread_);

  state_.store(EmulatorState::kStopped);
  FireEvent(EmulatorEvent::kStopped);
  return EmulatorResult::Success();
}

void MgbaEmulatorCore::Shutdown() {
  std::lock_guard<std::mutex> lock(lifecycle_mutex_);

  const EmulatorState current = state_.load();
  if (current == EmulatorState::kRunning || current == EmulatorState::kPaused) {
    if (current == EmulatorState::kPaused) {
      mCoreThreadUnpause(thread_);
    }
    mCoreThreadEnd(thread_);
    mCoreThreadJoin(thread_);
  }

  if (thread_ != nullptr) {
    delete thread_;
    thread_ = nullptr;
  }

  if (core_ != nullptr) {
    core_->deinit(core_);
    core_ = nullptr;
  }

  rom_loaded_ = false;
  state_.store(EmulatorState::kUninitialized);
}

EmulatorState MgbaEmulatorCore::State() const {
  return state_.load();
}

void MgbaEmulatorCore::SetInput(const InputState& input_state) {
  if (core_ == nullptr) {
    return;
  }
  // Deliberately not guarded by lifecycle_mutex_: this is called every
  // frame from client input-polling code while the core runs on its own
  // thread, same as mgba's own Qt/SDL frontends call setKeys() from the UI
  // thread. Guarding this with the lifecycle mutex would serialize every
  // input update behind whatever lifecycle operation (load/reset/stop) is
  // in flight, which is unnecessary - setKeys() just writes a bitmask the
  // core reads on its own next input poll.
  core_->setKeys(core_, input_state.held_mask);
}

bool MgbaEmulatorCore::SyncWaitFrameStart() {
  if (thread_ == nullptr) {
    return false;
  }
  return mCoreSyncWaitFrameStart(&thread_->impl->sync);
}

void MgbaEmulatorCore::SyncWaitFrameEnd() {
  if (thread_ == nullptr) {
    return;
  }
  mCoreSyncWaitFrameEnd(&thread_->impl->sync);
}

mCoreSync* MgbaEmulatorCore::GetSync() {
  if (thread_ == nullptr || thread_->impl == nullptr) {
    return nullptr;
  }
  return &thread_->impl->sync;
}

uint8_t MgbaEmulatorCore::ReadU8(uint32_t address) {
  if (core_ == nullptr) {
    return 0;
  }
  return static_cast<uint8_t>(core_->busRead8(core_, address));
}

uint16_t MgbaEmulatorCore::ReadU16(uint32_t address) {
  if (core_ == nullptr) {
    return 0;
  }
  return static_cast<uint16_t>(core_->busRead16(core_, address));
}

uint32_t MgbaEmulatorCore::ReadU32(uint32_t address) {
  if (core_ == nullptr) {
    return 0;
  }
  return core_->busRead32(core_, address);
}

void MgbaEmulatorCore::WriteU8(uint32_t address, uint8_t value) {
  if (core_) core_->busWrite8(core_, address, value);
}

void MgbaEmulatorCore::WriteU16(uint32_t address, uint16_t value) {
  if (core_) core_->busWrite16(core_, address, value);
}

void MgbaEmulatorCore::WriteU32(uint32_t address, uint32_t value) {
  if (core_) core_->busWrite32(core_, address, value);
}

std::optional<std::string> MgbaEmulatorCore::GameTitle() const {
  if (core_ == nullptr || !rom_loaded_) {
    return std::nullopt;
  }
  char buffer[kTitleBufferSize] = {0};
  core_->getGameTitle(core_, buffer);
  return std::string(buffer);
}

std::optional<std::string> MgbaEmulatorCore::GameCode() const {
  if (core_ == nullptr || !rom_loaded_) {
    return std::nullopt;
  }
  char buffer[kCodeBufferSize] = {0};
  core_->getGameCode(core_, buffer);
  return std::string(buffer);
}
const void* MgbaEmulatorCore::GetVideoBuffer() const {
    if (video_buffer_.empty()) {
        return nullptr;
    }

    return video_buffer_.data();
}

size_t MgbaEmulatorCore::GetVideoPitch() const {
    return video_pitch_;
}

size_t MgbaEmulatorCore::RegisterFrameCallback(FrameCallback cb) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  size_t id = ++next_callback_id_;
  frame_callbacks_.emplace_back(id, std::move(cb));
  return id;
}

void MgbaEmulatorCore::UnregisterFrameCallback(size_t id) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  for (auto it = frame_callbacks_.begin(); it != frame_callbacks_.end(); ++it) {
    if (it->first == id) {
      frame_callbacks_.erase(it);
      break;
    }
  }
}

void MgbaEmulatorCore::OnFrameEnded(mCoreThread* thread) {
  auto* self = static_cast<MgbaEmulatorCore*>(thread->userData);
  if (!self) return;
  
  static int frame_log_count = 0;
  if (frame_log_count++ % 60 == 0) {
      core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "MgbaEmulatorCore: OnFrameEnded called!");
  }

  // Compute FPS over 60-frame intervals
  uint32_t current_frames = self->core_->frameCounter(self->core_);
  if (current_frames - self->last_fps_frame_count_ >= 60) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - self->last_fps_time_;
    if (elapsed.count() > 0.0f) {
      self->current_fps_ = (current_frames - self->last_fps_frame_count_) / elapsed.count();
    }
    self->last_fps_time_ = now;
    self->last_fps_frame_count_ = current_frames;
  }

  std::lock_guard<std::mutex> lock(self->callback_mutex_);
  for (const auto& [id, cb] : self->frame_callbacks_) {
    cb();
  }
}

// --- Stage 3: Status API ---
bool MgbaEmulatorCore::IsRunning() const {
  return state_.load() == EmulatorState::kRunning;
}

bool MgbaEmulatorCore::IsPaused() const {
  return state_.load() == EmulatorState::kPaused;
}

bool MgbaEmulatorCore::IsLoaded() const {
  return rom_loaded_;
}

uint32_t MgbaEmulatorCore::GetFrameCount() const {
  if (!core_) return 0;
  return core_->frameCounter(core_);
}

float MgbaEmulatorCore::GetFPS() const {
  return current_fps_;
}

float MgbaEmulatorCore::GetCurrentSpeed() const {
  if (!core_) return 0.0f;
  float target_fps = core_->opts.fpsTarget > 0 ? core_->opts.fpsTarget : 60.0f;
  return current_fps_ / target_fps;
}

// --- Stage 3: ROM Info API ---
uint32_t MgbaEmulatorCore::RomCrc32() const {
  if (!core_ || !rom_loaded_) return 0;
  uint32_t crc32 = 0;
  core_->checksum(core_, &crc32, mCHECKSUM_CRC32);
  return crc32;
}

size_t MgbaEmulatorCore::RomSize() const {
  if (!core_ || !rom_loaded_) return 0;
  return core_->romSize(core_);
}

std::optional<std::string> MgbaEmulatorCore::SaveType() const {
  // mGBA abstracts hardware details like save type behind the generic mCore interface.
  // There is no official mCore API to retrieve it without relying on platform-specific (GBA) internals.
  return std::nullopt;
}

// --- Stage 3: Save Management ---
bool MgbaEmulatorCore::HasSaveState(int slot) const {
  if (!core_) return false;
  VFile* vf = mCoreGetState(core_, slot, false);
  if (vf) {
    vf->close(vf);
    return true;
  }
  return false;
}

EmulatorResult MgbaEmulatorCore::DeleteSaveState(int slot) {
  if (!core_) return EmulatorResult::Failure("Core not initialized");
  mCoreDeleteState(core_, slot);
  return EmulatorResult::Success();
}

std::vector<int> MgbaEmulatorCore::ListSaveStates() const {
  std::vector<int> available_slots;
  if (!core_) return available_slots;
  
  for (int i = 1; i <= 9; ++i) {
    if (HasSaveState(i)) {
      available_slots.push_back(i);
    }
  }
  return available_slots;
}

std::optional<SaveStateMetadata> MgbaEmulatorCore::GetSaveStateMetadata(int slot) const {
  if (!core_) return std::nullopt;
  VFile* vf = mCoreGetState(core_, slot, false);
  if (vf) {
    size_t size = vf->size(vf);
    vf->close(vf);
    return SaveStateMetadata{slot, size};
  }
  return std::nullopt;
}

// --- Stage 3: Event System ---
size_t MgbaEmulatorCore::RegisterEventCallback(EventCallback cb) {
  std::lock_guard<std::mutex> lock(event_mutex_);
  size_t id = ++next_event_id_;
  event_callbacks_.emplace_back(id, std::move(cb));
  return id;
}

void MgbaEmulatorCore::UnregisterEventCallback(size_t id) {
  std::lock_guard<std::mutex> lock(event_mutex_);
  for (auto it = event_callbacks_.begin(); it != event_callbacks_.end(); ++it) {
    if (it->first == id) {
      event_callbacks_.erase(it);
      break;
    }
  }
}

void MgbaEmulatorCore::FireEvent(EmulatorEvent event) {
  std::lock_guard<std::mutex> lock(event_mutex_);
  for (const auto& [id, cb] : event_callbacks_) {
    cb(event);
  }
}

// --- Stage 3: Logging ---
void MgbaEmulatorCore::RegisterLogCallback(LogCallback cb) {
  std::lock_guard<std::mutex> lock(g_log_mutex);
  if (g_log_callbacks.empty()) {
    mLogSetDefaultLogger(&g_mgba_logger);
  }
  g_log_callbacks.push_back(std::move(cb));
}

}  // namespace unboundmp::emulator
