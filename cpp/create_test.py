content = '''#include "network/network_manager.h"
#include "network/packet.h"
#include "database/database.h"
#include "core/log_manager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

using namespace unboundmp;

void Wait(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
    core::LogManager::Get().Initialize("logs/test_logs", 1024 * 1024, 1);
    
    // Clear old test db rows if any (hard without raw query, so we use a unique name)
    std::string unique_name = "TestChar" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() % 10000);
    
    // STEP 1: Boot server and client, create character
    {
        network::NetworkManager server_manager;
        server_manager.StartServer(8081);
        Wait(100);
        
        network::NetworkManager client_manager;
        client_manager.ConnectClient("127.0.0.1", 8081);
        Wait(100);
        
        auto client = client_manager.GetClient();
        assert(client->IsConnected());
        
        // Auth Request
        network::AuthRequestPacket auth_req;
        auth_req.username = "testuser";
        auth_req.password = "testpass";
        network::Packet p;
        p.type = network::PacketType::kAuthRequest;
        p.payload = auth_req.Serialize();
        client->SendPacket(p);
        
        Wait(500); // Wait for auth
        
        // Drain auth response
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
            }
        }
        assert(create_success);
        
        std::cout << "Character " << unique_name << " created successfully.\\n";
        
        client_manager.DisconnectClient();
        client_manager.Shutdown();
        
        server_manager.StopServer();
        server_manager.Shutdown();
    }
    
    Wait(500);
    
    // STEP 2: Boot server and client again, verify persistence
    {
        network::NetworkManager server_manager;
        server_manager.StartServer(8081);
        Wait(100);
        
        network::NetworkManager client_manager;
        client_manager.ConnectClient("127.0.0.1", 8081);
        Wait(100);
        
        auto client = client_manager.GetClient();
        assert(client->IsConnected());
        
        // Auth Request again
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
        assert(found);
        std::cout << "Character " << unique_name << " persisted and found in list!\\n";
        
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
            }
        }
        assert(selected);
        std::cout << "Character " << unique_name << " selected successfully!\\n";
        
        client_manager.DisconnectClient();
        client_manager.Shutdown();
        
        server_manager.StopServer();
        server_manager.Shutdown();
    }
    
    std::cout << "All E2E tests passed.\\n";
    return 0;
}
'''
with open('d:/Unbound/pokemon/cpp/examples/character_system_test.cpp', 'w') as f:
    f.write(content)
