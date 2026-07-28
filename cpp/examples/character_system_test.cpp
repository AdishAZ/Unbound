#include <iostream>
#include <thread>
#include <chrono>
#include <asio.hpp>
#include "core/log_manager.h"
#include "network/multiplayer_client.h"
#include "network/packet.h"

void Wait(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
    unboundmp::core::LogManager::Get().Initialize("logs/test_logs", 1024 * 1024, 1);
    std::string unique_name = "TestChar" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() % 10000);
    std::string unique_user = "user" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() % 10000);
    
    int port = 4000;
    asio::io_context io_context;
    
    // STEP 1: Connect, Create Account, Login, Create Character
    {
        unboundmp::network::MultiplayerClient client(io_context);
        client.Connect("127.0.0.1", port);
        
        while(!client.IsConnected()) {
            io_context.poll();
            Wait(10);
        }
        
        // CREATE ACCOUNT
        unboundmp::network::AuthRequestPacket create_acc;
        create_acc.username = unique_user;
        create_acc.password = "password123";
        unboundmp::network::Packet create_p;
        create_p.type = unboundmp::network::PacketType::kCreateAccountRequest;
        create_p.payload = create_acc.Serialize();
        client.SendPacket(create_p);
        
        for (int i = 0; i < 50; ++i) { io_context.poll(); client.ReceivePacket(); Wait(10); }
        
        // LOGIN
        unboundmp::network::AuthRequestPacket auth;
        auth.username = unique_user;
        auth.password = "password123";
        unboundmp::network::Packet p;
        p.type = unboundmp::network::PacketType::kAuthRequest;
        p.payload = auth.Serialize();
        client.SendPacket(p);
        
        std::string session_token;
        for (int i = 0; i < 50; ++i) {
            io_context.poll();
            auto p_opt = client.ReceivePacket();
            if (p_opt && p_opt->type == unboundmp::network::PacketType::kAuthResponse) {
                auto resp = unboundmp::network::AuthResponsePacket::Deserialize(p_opt->payload);
                if (resp.success) session_token = resp.token;
                break;
            }
            Wait(10);
        }
        
        if (session_token.empty()) {
            std::cerr << "Login failed in Step 1!" << std::endl;
            return 1;
        }

        // Create Character
        unboundmp::network::CreateCharacterRequestPacket create_req;
        create_req.name = unique_name;
        create_req.appearance = "hero";
        
        unboundmp::network::Packet cp;
        cp.type = unboundmp::network::PacketType::kCreateCharacterRequest;
        cp.payload = create_req.Serialize();
        cp.session_token = session_token;
        client.SendPacket(cp);
        
        bool char_created = false;
        for (int i = 0; i < 50; ++i) {
            io_context.poll();
            auto p_opt = client.ReceivePacket();
            if (p_opt && p_opt->type == unboundmp::network::PacketType::kCreateCharacterResponse) {
                auto resp = unboundmp::network::CreateCharacterResponsePacket::Deserialize(p_opt->payload);
                char_created = resp.success;
                break;
            }
            Wait(10);
        }
        
        if (!char_created) {
            std::cerr << "Failed to create character!" << std::endl;
            return 1;
        }
        client.Disconnect();
    }
    
    // STEP 2: Reconnect, Login, List, Select
    {
        unboundmp::network::MultiplayerClient client(io_context);
        client.Connect("127.0.0.1", port);
        
        while(!client.IsConnected()) { io_context.poll(); Wait(10); }
        
        // Login
        unboundmp::network::AuthRequestPacket auth_req;
        auth_req.username = unique_user;
        auth_req.password = "password123";
        unboundmp::network::Packet p;
        p.type = unboundmp::network::PacketType::kAuthRequest;
        p.payload = auth_req.Serialize();
        client.SendPacket(p);
        
        std::string session_token;
        for (int i = 0; i < 50; ++i) {
            io_context.poll();
            auto p_opt = client.ReceivePacket();
            if (p_opt && p_opt->type == unboundmp::network::PacketType::kAuthResponse) {
                auto resp = unboundmp::network::AuthResponsePacket::Deserialize(p_opt->payload);
                if (resp.success) session_token = resp.token;
                break;
            }
            Wait(10);
        }
        
        if (session_token.empty()) {
            std::cerr << "Login failed in Step 2!" << std::endl;
            return 1;
        }
        
        // List Characters
        unboundmp::network::Packet list_req;
        list_req.type = unboundmp::network::PacketType::kCharacterListRequest;
        list_req.session_token = session_token;
        client.SendPacket(list_req);
        
        uint64_t found_id = 0;
        for (int i = 0; i < 50; ++i) {
            io_context.poll();
            auto p_opt = client.ReceivePacket();
            if (p_opt && p_opt->type == unboundmp::network::PacketType::kCharacterListResponse) {
                auto resp = unboundmp::network::CharacterListResponsePacket::Deserialize(p_opt->payload);
                for (const auto& c : resp.characters) {
                    if (c.name == unique_name) found_id = c.id;
                }
                break;
            }
            Wait(10);
        }
        
        if (found_id == 0) {
            std::cerr << "Failed to find created character in list!" << std::endl;
            return 1;
        }
        
        // Select Character
        unboundmp::network::SelectCharacterRequestPacket sel_req;
        sel_req.character_id = found_id;
        unboundmp::network::Packet sel_p;
        sel_p.type = unboundmp::network::PacketType::kSelectCharacterRequest;
        sel_p.payload = sel_req.Serialize();
        sel_p.session_token = session_token;
        client.SendPacket(sel_p);
        
        bool selected = false;
        bool data_sync_received = false;
        
        for (int i = 0; i < 50; ++i) {
            io_context.poll();
            auto p_opt = client.ReceivePacket();
            if (p_opt) {
                if (p_opt->type == unboundmp::network::PacketType::kSelectCharacterResponse) {
                    auto resp = unboundmp::network::SelectCharacterResponsePacket::Deserialize(p_opt->payload);
                    selected = resp.success;
                }
                if (p_opt->type == unboundmp::network::PacketType::kPlayerDataSync) {
                    data_sync_received = true;
                }
            }
            Wait(10);
        }
        
        if (!selected || !data_sync_received) {
            std::cerr << "Failed to select character or receive sync!" << std::endl;
            return 1;
        }
        
        std::cout << "E2E Test Passed!" << std::endl;
    }
    
    return 0;
}
