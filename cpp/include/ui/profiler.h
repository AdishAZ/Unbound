#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>

namespace unboundmp::ui {

class Profiler {
public:
    static Profiler& Instance();
    
    // Frame timing
    void BeginFrame();
    void EndFrame();
    float GetFPS() const;
    float GetFrameTimeMs() const;
    const std::array<float, 60>& GetFrameTimeHistory() const;
    
    // Memory
    void UpdateMemoryStats();
    size_t GetWorkingSetMB() const;
    size_t GetPrivateBytesMB() const;
    
    // Network stats
    void SetPingMs(float ping);
    void SetAverageRTT(float rtt);
    void SetPacketLoss(float loss_percent);
    void SetBandwidthKBps(float in_kbps, float out_kbps);
    void SetPacketRate(int in_pps, int out_pps);
    void SetPacketQueueSize(int size);
    
    float GetPingMs() const;
    float GetAverageRTT() const;
    float GetPacketLoss() const;
    float GetBandwidthInKBps() const;
    float GetBandwidthOutKBps() const;
    int GetPacketRateIn() const;
    int GetPacketRateOut() const;
    int GetPacketQueueSize() const;
    
    // Game stats
    void SetEntityCount(int count);
    void SetDirtyFlagCount(int count);
    void SetAutosaveQueueSize(int size);
    void SetServerTick(uint64_t tick);
    void SetClientTick(uint64_t tick);
    void SetPredictionErrors(int count);
    void SetInterpolationDelay(float ms);
    void SetPlayerCount(int count);
    void SetMapId(uint32_t id);
    void SetPlayerPosition(float x, float y);
    void SetDirection(uint8_t dir);
    
    int GetEntityCount() const;
    int GetDirtyFlagCount() const;
    int GetAutosaveQueueSize() const;
    uint64_t GetServerTick() const;
    uint64_t GetClientTick() const;
    int GetPredictionErrors() const;
    float GetInterpolationDelay() const;
    int GetPlayerCount() const;
    uint32_t GetMapId() const;
    float GetPlayerX() const;
    float GetPlayerY() const;
    uint8_t GetDirection() const;
    
private:
    Profiler() = default;
    ~Profiler() = default;
    
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;
    
    // Frame timing
    std::chrono::high_resolution_clock::time_point m_frameStart;
    std::atomic<float> m_fps{0.0f};
    std::atomic<float> m_frameTimeMs{0.0f};
    
    std::array<float, 60> m_frameTimeHistory{};
    size_t m_historyIndex = 0;
    mutable std::mutex m_historyMutex;
    
    // Memory
    std::atomic<size_t> m_workingSetMB{0};
    std::atomic<size_t> m_privateBytesMB{0};
    
    // Network
    std::atomic<float> m_pingMs{0.0f};
    std::atomic<float> m_averageRTT{0.0f};
    std::atomic<float> m_packetLoss{0.0f};
    std::atomic<float> m_bandwidthInKBps{0.0f};
    std::atomic<float> m_bandwidthOutKBps{0.0f};
    std::atomic<int> m_packetRateIn{0};
    std::atomic<int> m_packetRateOut{0};
    std::atomic<int> m_packetQueueSize{0};
    
    // Game stats
    std::atomic<int> m_entityCount{0};
    std::atomic<int> m_dirtyFlagCount{0};
    std::atomic<int> m_autosaveQueueSize{0};
    std::atomic<uint64_t> m_serverTick{0};
    std::atomic<uint64_t> m_clientTick{0};
    std::atomic<int> m_predictionErrors{0};
    std::atomic<float> m_interpolationDelay{0.0f};
    std::atomic<int> m_playerCount{0};
    std::atomic<uint32_t> m_mapId{0};
    std::atomic<float> m_playerX{0.0f};
    std::atomic<float> m_playerY{0.0f};
    std::atomic<uint8_t> m_direction{0};
};

} // namespace unboundmp::ui
