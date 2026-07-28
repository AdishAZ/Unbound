#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include "network/packet.h"
#include "network/multiplayer_client.h"

namespace unboundmp::network {

class ClientPacketDispatcher {
public:
    using PacketCallback = std::function<void(const Packet&)>;

    static ClientPacketDispatcher& GetInstance() {
        static ClientPacketDispatcher instance;
        return instance;
    }

    // Must be called repeatedly from the main thread
    void Poll(MultiplayerClient* client);

    void Subscribe(PacketType type, PacketCallback callback);

private:
    ClientPacketDispatcher() = default;
    
    std::mutex subscribers_mutex_;
    std::unordered_map<PacketType, std::vector<PacketCallback>> subscribers_;
};

} // namespace unboundmp::network
