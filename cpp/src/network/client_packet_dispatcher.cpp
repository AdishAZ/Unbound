#include "network/client_packet_dispatcher.h"
#include "core/log_manager.h"

namespace unboundmp::network {

void ClientPacketDispatcher::Subscribe(PacketType type, PacketCallback callback) {
    std::lock_guard<std::mutex> lock(subscribers_mutex_);
    subscribers_[type].push_back(std::move(callback));
}

void ClientPacketDispatcher::Poll(MultiplayerClient* client) {
    if (!client) return;

    while (auto opt_p = client->ReceivePacket()) {
        const auto& p = *opt_p;
        
        std::vector<PacketCallback> callbacks;
        {
            std::lock_guard<std::mutex> lock(subscribers_mutex_);
            if (subscribers_.find(p.type) != subscribers_.end()) {
                callbacks = subscribers_[p.type];
            }
        }
        
        if (callbacks.empty()) {
            LOG_INFO(Network, "ClientPacketDispatcher: Unhandled packet type {}", static_cast<int>(p.type));
        } else {
            for (const auto& cb : callbacks) {
                cb(p);
            }
        }
    }
}

} // namespace unboundmp::network
