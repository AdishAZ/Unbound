#include "network/network_clock.h"

namespace unboundmp::network {

void NetworkClock::Sync(uint64_t server_tick, uint64_t server_time_ms) {
    last_server_tick_ = server_tick;
    last_server_time_ms_ = server_time_ms;
    local_sync_time_ = std::chrono::steady_clock::now();
    
    // Reset our interpolation tracker to the new tick boundary
    current_tick_ = server_tick;
    tick_accumulator_ = 0.0f;
}

void NetworkClock::Update(float dt) {
    tick_accumulator_ += dt;
    while (tick_accumulator_ >= tick_duration_) {
        tick_accumulator_ -= tick_duration_;
        current_tick_++;
    }
}

uint64_t NetworkClock::GetServerTimeMs() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - local_sync_time_).count();
    return last_server_time_ms_ + elapsed;
}

} // namespace unboundmp::network
