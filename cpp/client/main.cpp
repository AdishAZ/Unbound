#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <thread>
#include <vector>
#include <chrono>

#include "emulator/mgba_emulator_core.h"
#include "emulator/save_loader.h"
#include "render/sdl_renderer.h"
#include "ui/ui_engine.h"
#include "core/log_manager.h"
#include "ui/screens/login_screen.h"
#include "ui/screens/character_select_screen.h"
#include "ui/screens/login_screen.h"
#include "ui/screens/game_screen.h"
#include "network/network_manager.h"
#include "network/packet.h"
#include "network/client_session_manager.h"
#include "emulator/game_bootstrap.h"
#include "gameplay/remote_player_manager.h"
#include "network/network_clock.h"
#include "render/render_manager.h"
#include "render/world_renderer.h"
#include "render/map_renderer.h"
#include "render/weather_renderer.h"
#include "render/lighting_manager.h"
#include "render/particle_system.h"
#include "game_state/game_state.h"
#include "network/client_packet_dispatcher.h"
#include "core/event_system.h"
#include "persistence/client_save_manager.h"
#include "core/game_context.h"
#include "input/menu_input_interceptor.h"
#include "ui/inventory_screen.h"
#include "gameplay/world_manager.h"
#include "ui/profiler.h"
#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/position_reader.h"
#include "memory/map_reader.h"

namespace {
    struct RollingAverage {
        std::vector<float> samples;
        size_t index = 0;
        
        RollingAverage(size_t size) { samples.resize(size, 0.0f); }
        
        void Add(float val) {
            samples[index] = val;
            index = (index + 1) % samples.size();
        }
        
        float Get() const {
            float sum = 0;
            int count = 0;
            for (float v : samples) {
                if (v > 0) {
                    sum += v;
                    count++;
                }
            }
            return count > 0 ? sum / count : 0.0f;
        }
    };
    
    RollingAverage rtt_average(10);
}

int main()
{
    unboundmp::core::LogManager::Get().Initialize("logs", 5 * 1024 * 1024, 3);
    
    SDLRenderer renderer;
    if (!renderer.Initialize(1280, 720, 1))
    {
        std::cout << "Failed to initialize SDL renderer.\n";
        return 1;
    }

    unboundmp::ui::UIEngine ui_engine;
    int init_w, init_h;
    SDL_GetWindowSize(renderer.GetWindow(), &init_w, &init_h);
    if (!ui_engine.Initialize(renderer.GetNativeRenderer(), init_w, init_h)) {
        std::cout << "Failed to initialize UI Engine.\n";
        return 1;
    }
    
    unboundmp::network::NetworkManager network_manager;
    network_manager.ConnectClient("127.0.0.1", 4000);
    ui_engine.SetNetworkClient(network_manager.GetClient());

    // Subscribe to Pong for RTT
    unboundmp::network::ClientPacketDispatcher::GetInstance().Subscribe(
        unboundmp::network::PacketType::kPong,
        [](const unboundmp::network::Packet& p) {
            if (p.payload.size() == sizeof(uint64_t)) {
                uint64_t sent_time = 0;
                std::memcpy(&sent_time, p.payload.data(), sizeof(uint64_t));
                uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                float rtt = static_cast<float>(now - sent_time);
                
                rtt_average.Add(rtt);
                unboundmp::ui::Profiler::Instance().SetAverageRTT(rtt_average.Get());
            }
        });

    ui_engine.GetScreens().Push(std::make_unique<unboundmp::ui::LoginScreen>(&ui_engine));

    bool is_paused = false;

    bool running = true;
    Uint32 last_time = SDL_GetTicks();
    
    auto& bootstrap = unboundmp::emulator::GameBootstrap::GetInstance();
    auto& session_mgr = unboundmp::network::ClientSessionManager::GetInstance();
    session_mgr.Initialize();
    
    unboundmp::input::MenuInputInterceptor menu_interceptor;
    
    // Setup AddressTable for reading player memory
    unboundmp::memory::AddressTable address_table;
    std::string config_err;
    if (auto loaded = unboundmp::memory::AddressTable::LoadFromFile("config/address_table.cfg", config_err)) {
        address_table = *loaded;
    } else {
        // Fallback to Unbound standard addresses if config missing
        std::cout << "[Client] Using fallback address table configuration." << std::endl;
        
        address_table.Set("map_header_wram", {0x02036DFC, unboundmp::memory::ValueWidth::kU32, false});
        address_table.Set("player_map_bank", {0x02031DBC, unboundmp::memory::ValueWidth::kU8, false});
        address_table.Set("player_map_number", {0x02031DBD, unboundmp::memory::ValueWidth::kU8, false});
        address_table.Set("player_x", {0x02036E48, unboundmp::memory::ValueWidth::kU16, true});
        address_table.Set("player_y", {0x02036E4A, unboundmp::memory::ValueWidth::kU16, true});
        address_table.Set("player_facing", {0x02036E50, unboundmp::memory::ValueWidth::kU8, false});
        address_table.Set("save_block_1_ptr", {0x03005008, unboundmp::memory::ValueWidth::kU32, false});
        address_table.Set("party_count", {0x02024029, unboundmp::memory::ValueWidth::kU8, false});
        address_table.Set("party_base", {0x02024284, unboundmp::memory::ValueWidth::kU32, false});
    }

    unboundmp::core::GameContext game_context;
    game_context.SetUIEngine(&ui_engine);
    game_context.SetNetworkClient(network_manager.GetClient());
    ui_engine.SetGameContext(&game_context);
    
    // (Packet processing will happen in the main loop instead of via dispatcher)
    
    game_context.Initialize(address_table);
    
    auto render_manager = std::make_unique<unboundmp::render::RenderManager>();
    
    auto weather_renderer = std::make_shared<unboundmp::render::WeatherRenderer>();
    auto lighting_manager = std::make_shared<unboundmp::render::LightingManager>();
    auto particle_system = std::make_shared<unboundmp::render::ParticleSystem>();
    
    render_manager->AddRenderer(game_context.GetWorldManager()->GetWorldRenderer());
    render_manager->SetParticleSystem(particle_system);
    render_manager->SetWeatherRenderer(weather_renderer);
    render_manager->SetLightingManager(lighting_manager);
    
    render_manager->Initialize();
    game_context.SetRenderManager(render_manager.get());
    
    enum class RunState {
        kRunning,
        kSavingToLogin,
        kSavingToExit,
        kDisconnectingToLogin,
        kDisconnectingToExit,
        kWaitingForLogoutToLogin,
        kWaitingForLogoutToExit,
        kTeardownToLogin,
        kTeardownToExit
    };
    
    std::atomic<RunState> run_state = RunState::kRunning;

    unboundmp::persistence::ClientSaveManager::GetInstance().Initialize();

    unboundmp::core::EventSystem::GetInstance().Subscribe(unboundmp::core::EventType::kReturnToLoginRequested, [&](const unboundmp::core::Event& e) {
        if (run_state.load() != RunState::kRunning) return;
        
        // Safely pause emulator first before enqueuing save
        if (bootstrap.IsBooted() && bootstrap.GetEmulator().IsRunning()) {
            bootstrap.GetEmulator().Pause();
        }
        
        run_state.store(RunState::kSavingToLogin);
        unboundmp::persistence::ClientSaveManager::GetInstance().RequestSave(unboundmp::persistence::SaveType::kExitSave, [&](bool success) {
            run_state.store(RunState::kDisconnectingToLogin);
        });
    });
    
    unboundmp::core::EventSystem::GetInstance().Subscribe(unboundmp::core::EventType::kSaveAndExitRequested, [&](const unboundmp::core::Event& e) {
        if (run_state.load() != RunState::kRunning) return;
        
        // Safely pause emulator first before enqueuing save
        if (bootstrap.IsBooted() && bootstrap.GetEmulator().IsRunning()) {
            bootstrap.GetEmulator().Pause();
        }
        
        run_state.store(RunState::kSavingToExit);
        unboundmp::persistence::ClientSaveManager::GetInstance().RequestSave(unboundmp::persistence::SaveType::kExitSave, [&](bool success) {
            run_state.store(RunState::kDisconnectingToExit);
        });
    });

    unboundmp::core::EventSystem::GetInstance().Subscribe(unboundmp::core::EventType::kLogoutResponseReceived, [&](const unboundmp::core::Event& e) {
        if (run_state.load() == RunState::kWaitingForLogoutToLogin) {
            run_state.store(RunState::kTeardownToLogin);
        } else if (run_state.load() == RunState::kWaitingForLogoutToExit) {
            run_state.store(RunState::kTeardownToExit);
        }
    });

    unboundmp::network::ClientPacketDispatcher::GetInstance().Subscribe(
        unboundmp::network::PacketType::kLogoutResponse,
        [](const unboundmp::network::Packet& p) {
            unboundmp::core::EventSystem::GetInstance().Publish(unboundmp::core::EventType::kLogoutResponseReceived, unboundmp::core::EmptyEvent());
        }
    );

    float heartbeat_timer = 0.0f;
    float emulator_accumulator = 0.0f;
    const float GBA_FRAME_TIME = 1.0f / 59.7275f;
    
    while (running)
    {
        unboundmp::ui::Profiler::Instance().BeginFrame();
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F4) {
                    if (auto rm = game_context.GetRenderManager()) {
                        if (auto wr = rm->GetWeatherRenderer()) {
                            wr->CycleWeather();
                        }
                    }
                } else if (event.key.keysym.sym == SDLK_F5) {
                    if (auto rm = game_context.GetRenderManager()) {
                        if (auto lm = rm->GetLightingManager()) {
                            lm->CyclePreset();
                        }
                    }
                } else if (event.key.keysym.sym == SDLK_F11) {
                    renderer.ToggleFullscreen();
                }
            }
            ui_engine.HandleInput(event);
        }

        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        game_context.GetNetworkClock().Update(dt);
        if (game_context.GetWorldManager()) {
            game_context.GetWorldManager()->Update(dt);
        }
        
        game_context.Update(dt);
        
        // Process incoming packets
        if (auto cli = network_manager.GetClient()) {
            cli->UpdateStats(dt);
            unboundmp::network::ClientPacketDispatcher::GetInstance().Poll(cli);
            
            unboundmp::ui::Profiler::Instance().SetPingMs(static_cast<float>(cli->GetPing()));
            unboundmp::ui::Profiler::Instance().SetBandwidthKBps(cli->GetBandwidthInKBps(), cli->GetBandwidthOutKBps());
            unboundmp::ui::Profiler::Instance().SetPacketRate(cli->GetPacketRateIn(), cli->GetPacketRateOut());
            unboundmp::ui::Profiler::Instance().SetPacketQueueSize(cli->GetQueueSize());
        } else {
            unboundmp::ui::Profiler::Instance().SetPingMs(0.0f);
            unboundmp::ui::Profiler::Instance().SetBandwidthKBps(0.0f, 0.0f);
            unboundmp::ui::Profiler::Instance().SetPacketRate(0, 0);
            unboundmp::ui::Profiler::Instance().SetPacketQueueSize(0);
        }
        
        // Process UI events triggered by packets
        unboundmp::core::EventSystem::GetInstance().ProcessEvents();

        ui_engine.Update(dt);
        
        RunState state = run_state.load();
        if (state == RunState::kDisconnectingToLogin || state == RunState::kDisconnectingToExit) {
            unboundmp::network::Packet p;
            p.type = unboundmp::network::PacketType::kLogoutRequest;
            if (session_mgr.HasActiveSession()) p.session_token = session_mgr.GetSession()->session_token;
            if (auto client = network_manager.GetClient()) client->SendPacket(p);
            
            // Move to waiting state so we don't spam logout packets
            run_state.store(state == RunState::kDisconnectingToLogin ? RunState::kWaitingForLogoutToLogin : RunState::kWaitingForLogoutToExit);
        } else if (state == RunState::kTeardownToLogin || state == RunState::kTeardownToExit) {
            // Teardown is triggered here from main loop to ensure thread safety
            if (game_context.GetWorldManager()) {
                game_context.GetWorldManager()->GetMapRenderer()->SetFramebuffer(nullptr, 0);
            }
            bootstrap.Shutdown(); // This stops emulator thread and joins it
            
            if (state == RunState::kTeardownToLogin) {
                // Return to login screen safely
                while (ui_engine.GetScreens().GetCurrentScreen()) {
                    ui_engine.GetScreens().Pop();
                }
                ui_engine.GetScreens().Push(std::make_unique<unboundmp::ui::LoginScreen>(&ui_engine));
                run_state.store(RunState::kRunning);
            } else {
                running = false;
            }
        }
        
        // Check for menu open request
        if (menu_interceptor.ConsumeMenuOpenRequest()) {
            if (session_mgr.IsAuthenticated() && bootstrap.IsBooted()) {
                // Check if we are already in the inventory screen to avoid pushing multiple times
                bool inventory_open = false;
                auto* current = ui_engine.GetScreens().GetCurrentScreen();
                if (current && current->GetName() == "InventoryScreen") {
                    inventory_open = true;
                }
                
                if (!inventory_open) {
                    ui_engine.GetScreens().Push(std::make_unique<unboundmp::ui::InventoryScreen>(&ui_engine));
                }
            }
        }
        
        // Heartbeat & Ping
        if (session_mgr.IsAuthenticated()) {
            heartbeat_timer += dt;
            if (heartbeat_timer >= 1.0f) {
                heartbeat_timer = 0.0f;
                auto client = network_manager.GetClient();
                if (client) {
                    // Send Ping
                    unboundmp::network::Packet ping;
                    ping.type = unboundmp::network::PacketType::kPing;
                    ping.session_token = session_mgr.GetSession()->session_token;
                    
                    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                    ping.payload.resize(sizeof(uint64_t));
                    std::memcpy(ping.payload.data(), &now, sizeof(uint64_t));
                    client->SendPacket(ping);
                    
                    // Send Heartbeat
                    unboundmp::network::Packet hb;
                    hb.type = unboundmp::network::PacketType::kHeartbeat;
                    hb.session_token = session_mgr.GetSession()->session_token;
                    client->SendPacket(hb);
                }
                session_mgr.UpdateHeartbeat();
            }
        }
        
        // If emulator is running, handle its logic
        if (bootstrap.IsBooted() && bootstrap.GetEmulator().IsRunning()) {
            const Uint8* keys = SDL_GetKeyboardState(nullptr);
            
            // Read memory values for Profiler
            unboundmp::memory::MemoryApi mem_api(bootstrap.GetEmulator());
            std::cout
                << "X=" << mem_api.ReadU16(0x02036E48)
                << " Y=" << mem_api.ReadU16(0x02036E4A)
                << std::endl;
            unboundmp::memory::PositionReader pos_reader(mem_api, address_table);
            unboundmp::memory::MapReader map_reader(mem_api, address_table);
            
            auto facing_symbol = address_table.Get("player_facing");
            if (facing_symbol) {
                uint8_t facing = mem_api.ReadWidth(facing_symbol->address, facing_symbol->width);
                unboundmp::ui::Profiler::Instance().SetDirection(facing);
            }
            
            if (game_context.GetGameState()) {
                const auto& player = game_context.GetGameState()->GetLocalPlayer();
                float logical_x = static_cast<float>(player.position.x);
                float logical_y = static_cast<float>(player.position.y);
                unboundmp::ui::Profiler::Instance().SetPlayerPosition(logical_x, logical_y);
            }
            if (auto map = map_reader.Read(); map.ok()) {
                unboundmp::ui::Profiler::Instance().SetMapId(map.value->number | (map.value->bank << 16));
            }
            

            bool is_overlay_open = false;
            auto* current = ui_engine.GetScreens().GetCurrentScreen();
            if (current && current->GetName() == "InventoryScreen") {
                is_overlay_open = true;
            }
            
            bootstrap.Update(keys, &menu_interceptor, is_overlay_open);
            
            emulator_accumulator += dt;
            
            // Advance emulator state 59.7275 times per second
            while (emulator_accumulator >= GBA_FRAME_TIME) {
                bool frame_ready = bootstrap.GetEmulator().SyncWaitFrameStart();
                if (frame_ready) {
                    const void* framebuffer = bootstrap.GetEmulator().GetVideoBuffer();
                    size_t pitch = bootstrap.GetEmulator().GetVideoPitch();
                    if (framebuffer != nullptr && pitch > 0) {
                        if (game_context.GetWorldManager()) {
                            game_context.GetWorldManager()->GetMapRenderer()->SetFramebuffer(framebuffer, static_cast<int>(pitch));
                        }
                    }
                    bootstrap.GetEmulator().SyncWaitFrameEnd();
                }
                emulator_accumulator -= GBA_FRAME_TIME;
            }
            
            SDL_SetRenderDrawColor(renderer.GetNativeRenderer(), 0, 0, 0, 255);
            SDL_RenderClear(renderer.GetNativeRenderer());
            
            // Move the camera in a circle to prove interpolation/panning works
            // static float time_acc = 0.0f;
            // time_acc += dt;
            // float cam_x = std::sin(time_acc * 0.5f) * 10.0f;
            // float cam_y = std::cos(time_acc * 0.5f) * 10.0f;
            int w, h;
            SDL_GetWindowSize(renderer.GetWindow(), &w, &h);

            // Resize UI engine if window size changed
            if (ui_engine.GetRenderContext().screen_width != w || ui_engine.GetRenderContext().screen_height != h) {
                ui_engine.Resize(w, h);
            }

            if (render_manager) {
                render_manager->Render(renderer.GetNativeRenderer(), w, h, dt);
            }
            
            ui_engine.Update(dt);
            ui_engine.Render();
            renderer.Present();
        } else {
            // Emulation not booted, just render UI over black background
            SDL_SetRenderDrawColor(renderer.GetNativeRenderer(), 0, 0, 0, 255);
            SDL_RenderClear(renderer.GetNativeRenderer());
            ui_engine.Render();
            renderer.Present();
        }
        
        unboundmp::ui::Profiler::Instance().EndFrame();
    }

    render_manager->Shutdown();
    bootstrap.Shutdown();
    
    unboundmp::persistence::ClientSaveManager::GetInstance().Shutdown();
    
    network_manager.DisconnectClient();
    network_manager.Shutdown();
    renderer.Shutdown();
    return 0;
}
