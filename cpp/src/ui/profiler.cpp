#include "ui/profiler.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")
#endif

namespace unboundmp::ui {

Profiler& Profiler::Instance() {
    static Profiler instance;
    return instance;
}

void Profiler::BeginFrame() {
    m_frameStart = std::chrono::high_resolution_clock::now();
}

void Profiler::EndFrame() {
    auto frameEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> frameDuration = frameEnd - m_frameStart;
    float currentFrameTimeMs = frameDuration.count();
    
    m_frameTimeMs.store(currentFrameTimeMs, std::memory_order_relaxed);
    
    if (currentFrameTimeMs > 0.0f) {
        m_fps.store(1000.0f / currentFrameTimeMs, std::memory_order_relaxed);
    }
    
    std::lock_guard<std::mutex> lock(m_historyMutex);
    m_frameTimeHistory[m_historyIndex] = currentFrameTimeMs;
    m_historyIndex = (m_historyIndex + 1) % m_frameTimeHistory.size();
}

float Profiler::GetFPS() const {
    return m_fps.load(std::memory_order_relaxed);
}

float Profiler::GetFrameTimeMs() const {
    return m_frameTimeMs.load(std::memory_order_relaxed);
}

const std::array<float, 60>& Profiler::GetFrameTimeHistory() const {
    std::lock_guard<std::mutex> lock(m_historyMutex);
    return m_frameTimeHistory;
}

void Profiler::UpdateMemoryStats() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        m_workingSetMB.store(pmc.WorkingSetSize / (1024 * 1024), std::memory_order_relaxed);
        m_privateBytesMB.store(pmc.PrivateUsage / (1024 * 1024), std::memory_order_relaxed);
    }
#endif
}

size_t Profiler::GetWorkingSetMB() const { return m_workingSetMB.load(std::memory_order_relaxed); }
size_t Profiler::GetPrivateBytesMB() const { return m_privateBytesMB.load(std::memory_order_relaxed); }

void Profiler::SetPingMs(float ping) { m_pingMs.store(ping, std::memory_order_relaxed); }
void Profiler::SetAverageRTT(float rtt) { m_averageRTT.store(rtt, std::memory_order_relaxed); }
void Profiler::SetPacketLoss(float loss_percent) { m_packetLoss.store(loss_percent, std::memory_order_relaxed); }
void Profiler::SetBandwidthKBps(float in_kbps, float out_kbps) {
    m_bandwidthInKBps.store(in_kbps, std::memory_order_relaxed);
    m_bandwidthOutKBps.store(out_kbps, std::memory_order_relaxed);
}
void Profiler::SetPacketRate(int in_pps, int out_pps) {
    m_packetRateIn.store(in_pps, std::memory_order_relaxed);
    m_packetRateOut.store(out_pps, std::memory_order_relaxed);
}
void Profiler::SetPacketQueueSize(int size) { m_packetQueueSize.store(size, std::memory_order_relaxed); }

float Profiler::GetPingMs() const { return m_pingMs.load(std::memory_order_relaxed); }
float Profiler::GetAverageRTT() const { return m_averageRTT.load(std::memory_order_relaxed); }
float Profiler::GetPacketLoss() const { return m_packetLoss.load(std::memory_order_relaxed); }
float Profiler::GetBandwidthInKBps() const { return m_bandwidthInKBps.load(std::memory_order_relaxed); }
float Profiler::GetBandwidthOutKBps() const { return m_bandwidthOutKBps.load(std::memory_order_relaxed); }
int Profiler::GetPacketRateIn() const { return m_packetRateIn.load(std::memory_order_relaxed); }
int Profiler::GetPacketRateOut() const { return m_packetRateOut.load(std::memory_order_relaxed); }
int Profiler::GetPacketQueueSize() const { return m_packetQueueSize.load(std::memory_order_relaxed); }

void Profiler::SetEntityCount(int count) { m_entityCount.store(count, std::memory_order_relaxed); }
void Profiler::SetDirtyFlagCount(int count) { m_dirtyFlagCount.store(count, std::memory_order_relaxed); }
void Profiler::SetAutosaveQueueSize(int size) { m_autosaveQueueSize.store(size, std::memory_order_relaxed); }
void Profiler::SetServerTick(uint64_t tick) { m_serverTick.store(tick, std::memory_order_relaxed); }
void Profiler::SetClientTick(uint64_t tick) { m_clientTick.store(tick, std::memory_order_relaxed); }
void Profiler::SetPredictionErrors(int count) { m_predictionErrors.store(count, std::memory_order_relaxed); }
void Profiler::SetInterpolationDelay(float ms) { m_interpolationDelay.store(ms, std::memory_order_relaxed); }
void Profiler::SetPlayerCount(int count) { m_playerCount.store(count, std::memory_order_relaxed); }
void Profiler::SetMapId(uint32_t id) { m_mapId.store(id, std::memory_order_relaxed); }
void Profiler::SetPlayerPosition(float x, float y) {
    m_playerX.store(x, std::memory_order_relaxed);
    m_playerY.store(y, std::memory_order_relaxed);
}
void Profiler::SetDirection(uint8_t dir) { m_direction.store(dir, std::memory_order_relaxed); }

int Profiler::GetEntityCount() const { return m_entityCount.load(std::memory_order_relaxed); }
int Profiler::GetDirtyFlagCount() const { return m_dirtyFlagCount.load(std::memory_order_relaxed); }
int Profiler::GetAutosaveQueueSize() const { return m_autosaveQueueSize.load(std::memory_order_relaxed); }
uint64_t Profiler::GetServerTick() const { return m_serverTick.load(std::memory_order_relaxed); }
uint64_t Profiler::GetClientTick() const { return m_clientTick.load(std::memory_order_relaxed); }
int Profiler::GetPredictionErrors() const { return m_predictionErrors.load(std::memory_order_relaxed); }
float Profiler::GetInterpolationDelay() const { return m_interpolationDelay.load(std::memory_order_relaxed); }
int Profiler::GetPlayerCount() const { return m_playerCount.load(std::memory_order_relaxed); }
uint32_t Profiler::GetMapId() const { return m_mapId.load(std::memory_order_relaxed); }
float Profiler::GetPlayerX() const { return m_playerX.load(std::memory_order_relaxed); }
float Profiler::GetPlayerY() const { return m_playerY.load(std::memory_order_relaxed); }
uint8_t Profiler::GetDirection() const { return m_direction.load(std::memory_order_relaxed); }

} // namespace unboundmp::ui
