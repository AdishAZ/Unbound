#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace unboundmp::emulator {

// GBA face/shoulder buttons. Mirrors mgba's GBAKey enum (mgba/internal/gba/input.h)
// bit-for-bit so InputState::ToKeyMask() below can be handed straight to a
// backend's setKeys()/addKeys()/clearKeys() without translation. Not an
// invented mapping - GBA_KEY_A=0 .. GBA_KEY_L=9 is part of libmgba's public
// API, which itself mirrors the real GBA's button bit order.
enum class GbaButton : uint32_t {
  kA = 0,
  kB = 1,
  kSelect = 2,
  kStart = 3,
  kRight = 4,
  kLeft = 5,
  kUp = 6,
  kDown = 7,
  kR = 8,
  kL = 9,
};

// Bitmask helper for forwarding held-button state to the core in one call.
struct InputState {
  uint32_t held_mask = 0;

  static constexpr uint32_t BitFor(GbaButton button) {
    return 1u << static_cast<uint32_t>(button);
  }

  void Press(GbaButton button) { held_mask |= BitFor(button); }
  void Release(GbaButton button) { held_mask &= ~BitFor(button); }
  bool IsHeld(GbaButton button) const { return (held_mask & BitFor(button)) != 0; }
  void Clear() { held_mask = 0; }
};

enum class EmulatorState {
  kUninitialized,  // no core created yet
  kNoRom,          // core created, no ROM loaded
  kStopped,        // ROM loaded, thread not started / not running
  kRunning,
  kPaused,
  kCrashed,
};

// Result of a lifecycle operation. `ok == false` means `message` explains
// why; callers should surface `message` to logs/UI rather than guessing.
struct EmulatorResult {
  bool ok = false;
  std::string message;

  static EmulatorResult Success() { return {true, ""}; }
  static EmulatorResult Failure(std::string msg) { return {false, std::move(msg)}; }
  explicit operator bool() const { return ok; }
};

struct SaveStateMetadata {
  int slot;
  size_t size;
};

enum class EmulatorEvent {
  kStarted,
  kStopped,
  kPaused,
  kResumed,
  kSaveStateCreated,
  kSaveStateLoaded,
  kSoftReset
};

enum class LogLevel {
  kInfo,
  kWarning,
  kError,
  kDebug
};

using EventCallback = std::function<void(EmulatorEvent)>;
using LogCallback = std::function<void(LogLevel, const std::string&)>;

// Backend-agnostic contract for "an emulator we can load a ROM into, run,
// pause/resume/reset, forward input to, and read memory from". The only
// concrete implementation right now is MgbaEmulatorCore (mgba_emulator_core.h),
// but keeping this abstract lets the memory-abstraction layer and the rest
// of the client depend on an interface instead of libmgba directly, and
// makes it possible to substitute a fake/mock core in unit tests.
//
// IMPORTANT: this class never reads, writes, or otherwise transforms ROM
// file bytes itself - it only ever hands a path/blob to the backend's
// loader unmodified. No patch application, no header rewriting. Loading a
// save file is likewise a pass-through to the backend's save-loading path.
class IEmulatorCore {
 public:
  virtual ~IEmulatorCore() = default;
  virtual const void* GetVideoBuffer() const = 0;
  virtual size_t GetVideoPitch() const = 0;

  // --- Lifecycle -------------------------------------------------------
  // Allocates the underlying core (e.g. GBACoreCreate() + core->init()).
  // Must succeed before LoadRom() may be called.
  virtual EmulatorResult Initialize() = 0;

  // Loads the ROM at `rom_path` completely unmodified (opened read-only by
  // the backend). Does not start emulation - call Start() afterwards.
  virtual EmulatorResult LoadRom(const std::string& rom_path) = 0;

  // Loads (or creates, if absent) the save file at `save_path`. Must be
  // called after LoadRom() and before Start(). If never called, the core
  // runs with only in-memory/battery-backed save state that is never
  // persisted to disk.
  virtual EmulatorResult LoadSave(const std::string& save_path) = 0;

  // Starts emulation on a background thread. Returns once the thread has
  // been created; the core may still be finishing its own startup.
  virtual EmulatorResult Start() = 0;

  // Pauses emulation. Safe to call from any thread. No-op if already paused.
  virtual EmulatorResult Pause() = 0;

  // Resumes a paused core. No-op if already running.
  virtual EmulatorResult Resume() = 0;

  // Resets the running game (equivalent to a hardware reset), preserving
  // the loaded ROM/save. Does not touch ROM/save files on disk.
  virtual EmulatorResult Reset() = 0;

  // Stops emulation and joins the background thread. Safe to call from any
  // thread except the emulator thread itself. After this, Start() may be
  // called again to relaunch with the same ROM/save.
  virtual EmulatorResult Stop() = 0;

  // Tears down the core entirely (inverse of Initialize()). The instance
  // is not usable again after this without a fresh Initialize().
  virtual void Shutdown() = 0;

  virtual EmulatorState State() const = 0;

  // --- Input -------------------------------------------------------------
  // Forwards the full held-button state to the core. Called every frame
  // (or whenever input changes) from the client's input-polling code, not
  // from network code - remote players' input never reaches this method;
  // see docs/REVERSE_ENGINEERING.md and the network layer for how remote
  // player state is applied instead (memory writes, once that's designed -
  // out of scope for this milestone).
  virtual void SetInput(const InputState& state) = 0;

  // --- Raw memory access (used by the memory abstraction layer) ---------
  // These map directly to a backend's bus read functions and are
  // intentionally free of any game-specific interpretation - see
  // memory/memory_api.h for the layer that gives these addresses meaning.
  virtual uint8_t ReadU8(uint32_t address) = 0;
  virtual uint16_t ReadU16(uint32_t address) = 0;
  virtual uint32_t ReadU32(uint32_t address) = 0;

  // Bulk read of `length` bytes starting at `address` into `out`, which
  // must have at least `length` bytes of space. Backends that only expose
  // word-sized reads should implement this by reading through ReadU8/16/32
  // as appropriate; this default is provided as a fallback.
  virtual void ReadBytes(uint32_t address, uint8_t* out, size_t length) {
    for (size_t i = 0; i < length; ++i) {
      out[i] = ReadU8(address + static_cast<uint32_t>(i));
    }
  }

  // --- Stage 8: Memory Writing ---
  // These map directly to a backend's bus write functions. Only used by the
  // client when restoring state from the server upon login or receiving
  // authoritative delta syncs.
  virtual void WriteU8(uint32_t address, uint8_t value) = 0;
  virtual void WriteU16(uint32_t address, uint16_t value) = 0;
  virtual void WriteU32(uint32_t address, uint32_t value) = 0;

  virtual void WriteBytes(uint32_t address, const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
      WriteU8(address + static_cast<uint32_t>(i), data[i]);
    }
  }

  // Returns the game title/code the core parsed out of the ROM header, if
  // a ROM is loaded. Useful for sanity-checking "is this actually Unbound"
  // without touching ROM bytes ourselves - the backend already parsed the
  // header as part of loading.
  virtual std::optional<std::string> GameTitle() const = 0;
  virtual std::optional<std::string> GameCode() const = 0;

  // --- Stage 3: Status API ---
  virtual bool IsRunning() const = 0;
  virtual bool IsPaused() const = 0;
  virtual bool IsLoaded() const = 0;
  virtual uint32_t GetFrameCount() const = 0;
  virtual float GetFPS() const = 0;
  virtual float GetCurrentSpeed() const = 0;

  // --- Stage 3: ROM Info API ---
  virtual uint32_t RomCrc32() const = 0;
  virtual size_t RomSize() const = 0;
  virtual std::optional<std::string> SaveType() const = 0;

  // --- Stage 3: Save Management ---
  virtual bool HasSaveState(int slot) const = 0;
  virtual EmulatorResult DeleteSaveState(int slot) = 0;
  virtual std::vector<int> ListSaveStates() const = 0;
  virtual std::optional<SaveStateMetadata> GetSaveStateMetadata(int slot) const = 0;

  // --- Stage 3: Event System ---
  virtual size_t RegisterEventCallback(EventCallback cb) = 0;
  virtual void UnregisterEventCallback(size_t id) = 0;
};

}  // namespace unboundmp::emulator
