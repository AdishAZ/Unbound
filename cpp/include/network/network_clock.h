#pragma once
#include <cstdint>
#include <chrono>
#include <atomic>

namespace unboundmp::network {

// Synchronizes client time with server time and provides interpolation metrics.
class NetworkClock {
public:
    NetworkClock() = default;

    // Sync with the server snapshot time
    void Sync(uint64_t server_tick, uint64_t server_time_ms);

    // Call every frame with delta time in seconds
    void Update(float dt);

    // Current estimated server tick
    uint64_t GetCurrentTick() const { return current_tick_; }

    // How far into the current tick we are [0.0, 1.0]
    float GetInterpolationFactor() const { return tick_accumulator_ / tick_duration_; }
    
    // Server time estimated
    uint64_t GetServerTimeMs() const;

private:
    std::atomic<uint64_t> last_server_tick_{0};
    std::atomic<uint64_t> last_server_time_ms_{0};
    std::chrono::steady_clock::time_point local_sync_time_;
    
    uint64_t current_tick_ = 0;
    float tick_accumulator_ = 0.0f;
    float tick_duration_ = 0.05f; // 50ms per tick (20 TPS)
};

} // namespace unboundmp::network
