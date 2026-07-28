#pragma once

#include <cstdint>
#include <mutex>
#include <atomic>

namespace unboundmp::persistence {

enum class DirtyComponent : uint32_t {
  kNone = 0,
  kWorldState = 1 << 0,
  kStory = 1 << 1,
  kInventory = 1 << 2,
  kParty = 1 << 3,
  kPc = 1 << 4,
  kMoney = 1 << 5,
  
  kAll = 0xFFFFFFFF
};

inline DirtyComponent operator|(DirtyComponent a, DirtyComponent b) {
    return static_cast<DirtyComponent>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline DirtyComponent operator&(DirtyComponent a, DirtyComponent b) {
    return static_cast<DirtyComponent>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline DirtyComponent operator~(DirtyComponent a) {
    return static_cast<DirtyComponent>(~static_cast<uint32_t>(a));
}

class DirtyFlagManager {
 public:
  DirtyFlagManager() = default;

  void MarkDirty(DirtyComponent component) {
    flags_.fetch_or(static_cast<uint32_t>(component), std::memory_order_relaxed);
  }

  void Clear(DirtyComponent component) {
    flags_.fetch_and(~static_cast<uint32_t>(component), std::memory_order_relaxed);
  }

  bool IsDirty(DirtyComponent component) const {
    return (flags_.load(std::memory_order_relaxed) & static_cast<uint32_t>(component)) != 0;
  }
  
  bool AnyDirty() const {
      return flags_.load(std::memory_order_relaxed) != 0;
  }
  
  uint32_t GetFlagsAndClear() {
      return flags_.exchange(0, std::memory_order_relaxed);
  }

 private:
  std::atomic<uint32_t> flags_{0};
};

}  // namespace unboundmp::persistence
