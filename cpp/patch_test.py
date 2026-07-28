with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'r') as f:
    content = f.read()

content = content.replace('unboundmp_server\n)', ')')
with open('d:/Unbound/pokemon/cpp/CMakeLists.txt', 'w') as f:
    f.write(content)

content_cpp = '''#include "network/network_manager.h"
#include "network/packet.h"
#include "core/log_manager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>
#include <cstdlib>

using namespace unboundmp;

void Wait(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main(int argc, char* argv[]) {
    core::LogManager::Get().Initialize("logs/test_logs", 1024 * 1024, 1);
    std::string unique_name = "TestChar" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() % 10000);
    
    // Assume server is running on 8080 or pass via args
    int port = 8080;
    
    // STEP 1: Connect, Create Character
    {
        network::NetworkManager client_manager;
        client_manager.ConnectClient("127.0.0.1", port);
        Wait(500); // Wait for connection
        
        auto client = client_manager.GetClient();
        if (!client || !client->IsConnected()) {
            std::cerr << "Failed to connect to server.\\n";
            return 1;
        }
        
        // Auth Request
        network::AuthRequestPacket auth_req;
        auth_req.username = "testuser";
        auth_req.password = "testpass";
        network::Packet p;
        p.type = network::PacketType::kAuthRequest;
        p.payload = auth_req.Serialize();
        client->SendPacket(p);
        
        Wait(500); // Wait for auth
        while (auto opt_p = client->ReceivePacket()) {}
        
        // Create Character
        network::CreateCharacterRequestPacket create_req;
        create_req.name = unique_name;
        create_req.appearance = "default";
        p.type = network::PacketType::kCreateCharacterRequest;
        p.payload = create_req.Serialize();
        client->SendPacket(p);
        
        Wait(500); // Wait for creation
        
        bool create_success = false;
        while (auto opt_p = client->ReceivePacket()) {
            if (opt_p->type == network::PacketType::kCreateCharacterResponse) {
                auto resp = network::CreateCharacterResponsePacket::Deserialize(opt_p->payload);
                create_success = resp.success;
                if (!create_success) std::cerr << "Failed: " << resp.message << "\\n";
            }
        }
        if (!create_success) return 1;
        
        std::cout << "Character " << unique_name << " created successfully.\\n";
        
        client_manager.DisconnectClient();
        client_manager.Shutdown();
    }
    
    Wait(500);
    
    // STEP 2: Reconnect, List, Select
    {
        network::NetworkManager client_manager;
        client_manager.ConnectClient("127.0.0.1", port);
        Wait(500);
        
        auto client = client_manager.GetClient();
        
        // Auth Request
        network::AuthRequestPacket auth_req;
        auth_req.username = "testuser";
        auth_req.password = "testpass";
        network::Packet p;
        p.type = network::PacketType::kAuthRequest;
        p.payload = auth_req.Serialize();
        client->SendPacket(p);
        
        Wait(500);
        while (auto opt_p = client->ReceivePacket()) {} // Drain
        
        // List Characters
        network::CharacterListRequestPacket list_req;
        p.type = network::PacketType::kCharacterListRequest;
        p.payload = list_req.Serialize();
        client->SendPacket(p);
        
        Wait(500);
        
        bool found = false;
        uint64_t char_id = 0;
        while (auto opt_p = client->ReceivePacket()) {
            if (opt_p->type == network::PacketType::kCharacterListResponse) {
                auto resp = network::CharacterListResponsePacket::Deserialize(opt_p->payload);
                for (const auto& c : resp.characters) {
                    if (c.name == unique_name) {
                        found = true;
                        char_id = c.id;
                    }
                }
            }
        }
        if (!found) {
            std::cerr << "Character " << unique_name << " not found after restart.\\n";
            return 1;
        }
        
        std::cout << "Character " << unique_name << " persisted and found!\\n";
        
        // Select Character
        network::SelectCharacterRequestPacket select_req;
        select_req.character_id = char_id;
        p.type = network::PacketType::kSelectCharacterRequest;
        p.payload = select_req.Serialize();
        client->SendPacket(p);
        
        Wait(500);
        
        bool selected = false;
        while (auto opt_p = client->ReceivePacket()) {
            if (opt_p->type == network::PacketType::kSelectCharacterResponse) {
                auto resp = network::SelectCharacterResponsePacket::Deserialize(opt_p->payload);
                selected = resp.success;
                if (!selected) std::cerr << "Select Failed: " << resp.message << "\\n";
            }
        }
        if (!selected) return 1;
        
        std::cout << "Character " << unique_name << " selected successfully!\\n";
        
        client_manager.DisconnectClient();
        client_manager.Shutdown();
    }
    
    std::cout << "All E2E tests passed.\\n";
    return 0;
}
'''
with open('d:/Unbound/pokemon/cpp/examples/character_system_test.cpp', 'w') as f:
    f.write(content_cpp)

