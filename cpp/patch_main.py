import re

with open('d:/Unbound/pokemon/cpp/server/main.cpp', 'r') as f:
    content = f.read()

# Replace CharacterManager include
content = content.replace('#include "characters/character_manager.h"', '#include "database/character_repository.h"')
content = content.replace('auto character_manager = std::make_shared<CharacterManager>(db_pool);', 'auto character_repo = std::make_shared<CharacterRepository>(db_pool);')

# Update AuthRequest to NOT load player persistence state immediately (wait for Select)
auth_search = '''
      // Stage 8 Persistence: Load Player
      auto persistence_state = persistence::PlayerPersistence::LoadPlayer(*db_pool, acc->id);
      if (!persistence_state) {
        // Create an empty state if not found (new character scenario)
        persistence_state = PlayerDataSyncPacket();
        persistence_state->account_id = acc->id;
      }
      
      online_player.dirty_manager = std::make_shared<persistence::DirtyFlagManager>();
      online_player.persistence_state = std::make_shared<PlayerDataSyncPacket>(*persistence_state);
      
      player_registry->AddPlayer(online_player);
      
      Logger::Info("User " + req.username + " logged in (Token: " + token.substr(0, 8) + "...)");
      
      // Send PlayerDataSync
      Packet sync_p;
      sync_p.type = PacketType::kPlayerDataSync;
      sync_p.payload = persistence_state->Serialize();
      conn->SendPacket(sync_p);
'''

auth_replace = '''
      player_registry->AddPlayer(online_player);
      Logger::Info("User " + req.username + " logged in (Token: " + token.substr(0, 8) + "...)");
'''
content = content.replace(auth_search, auth_replace)

# Remove the old char fetching logic from auth
char_fetch_search = '''      // Load character if any
      auto chars = character_manager->GetCharactersForAccount(acc->id);
      if (!chars.empty()) {
        resp.character_id = chars[0].id;
      }
'''
content = content.replace(char_fetch_search, '')

# Add Handlers
handlers = '''
  dispatcher->RegisterHandler(PacketType::kCreateCharacterRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    CreateCharacterRequestPacket req = CreateCharacterRequestPacket::Deserialize(p.payload);
    CreateCharacterResponsePacket resp;
    
    auto character = character_repo->CreateCharacter(session->account.id, req.name, req.appearance);
    if (character) {
        resp.success = true;
        resp.message = "Character created successfully";
        resp.character_id = character->id;
    } else {
        resp.success = false;
        resp.message = "Failed to create character. Invalid name or name already taken.";
    }
    
    Packet out_p;
    out_p.type = PacketType::kCreateCharacterResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kDeleteCharacterRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    DeleteCharacterRequestPacket req = DeleteCharacterRequestPacket::Deserialize(p.payload);
    DeleteCharacterResponsePacket resp;
    
    if (character_repo->DeleteCharacter(session->account.id, req.character_id)) {
        resp.success = true;
        resp.message = "Character deleted.";
    } else {
        resp.success = false;
        resp.message = "Failed to delete character.";
    }
    
    Packet out_p;
    out_p.type = PacketType::kDeleteCharacterResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kCharacterListRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    CharacterListResponsePacket resp;
    auto chars = character_repo->LoadCharactersForAccount(session->account.id);
    for (const auto& c : chars) {
        CharacterListResponsePacket::CharacterEntry entry;
        entry.id = c.id;
        entry.name = c.name;
        entry.appearance = c.appearance;
        entry.map_id = c.map_id;
        entry.play_time_seconds = c.play_time_seconds;
        entry.money = c.money;
        entry.last_login = c.last_login;
        resp.characters.push_back(entry);
    }
    
    Packet out_p;
    out_p.type = PacketType::kCharacterListResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kSelectCharacterRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    SelectCharacterRequestPacket req = SelectCharacterRequestPacket::Deserialize(p.payload);
    SelectCharacterResponsePacket resp;
    
    auto character = character_repo->LoadCharacter(req.character_id);
    if (character && character->account_id == session->account.id) {
        resp.success = true;
        resp.message = "Character selected.";
        
        // Update online player state
        auto player = player_registry->GetPlayer(session->account.id);
        if (player) {
            player->character_id = character->id;
            
            // Load persistent blobs into memory
            auto persistence_state = persistence::PlayerPersistence::LoadPlayer(*db_pool, session->account.id);
            if (!persistence_state) {
                persistence_state = PlayerDataSyncPacket();
                persistence_state->account_id = session->account.id;
            }
            player->dirty_manager = std::make_shared<persistence::DirtyFlagManager>();
            player->persistence_state = std::make_shared<PlayerDataSyncPacket>(*persistence_state);
            
            // Send PlayerDataSync
            Packet sync_p;
            sync_p.type = PacketType::kPlayerDataSync;
            sync_p.payload = persistence_state->Serialize();
            conn->SendPacket(sync_p);
            
            Logger::Info("Account " + std::to_string(session->account.id) + " selected character " + std::to_string(character->id));
        }
    } else {
        resp.success = false;
        resp.message = "Character not found or ownership mismatch.";
        Logger::Warn("Failed character selection for account " + std::to_string(session->account.id) + " character " + std::to_string(req.character_id));
    }
    
    Packet out_p;
    out_p.type = PacketType::kSelectCharacterResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

'''

content = content.replace('dispatcher->RegisterHandler(PacketType::kHeartbeat', handlers + '  dispatcher->RegisterHandler(PacketType::kHeartbeat')

with open('d:/Unbound/pokemon/cpp/server/main.cpp', 'w') as f:
    f.write(content)
