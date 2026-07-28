import re

with open('d:/Unbound/pokemon/cpp/client/main.cpp', 'r') as f:
    content = f.read()

# Add network includes
includes = '''#include "ui/screens/login_screen.h"
#include "ui/screens/game_screen.h"
#include "network/network_manager.h"
#include "network/packet.h"'''
content = content.replace('#include "ui/screens/login_screen.h"\n#include "ui/screens/game_screen.h"', includes)

# Setup network manager
setup_search = '''    ui_engine.GetScreens().Push(std::make_unique<unboundmp::ui::LoginScreen>(&ui_engine));

    MgbaEmulatorCore emulator;'''
setup_replace = '''    unboundmp::network::NetworkManager network_manager;
    network_manager.ConnectClient("127.0.0.1", 8080);
    ui_engine.SetNetworkClient(network_manager.GetClient());

    ui_engine.GetScreens().Push(std::make_unique<unboundmp::ui::LoginScreen>(&ui_engine));

    MgbaEmulatorCore emulator;'''
content = content.replace(setup_search, setup_replace)

# Poll packets
poll_search = '''        ui_engine.Update(dt);
        
        auto current_screen = ui_engine.GetScreens().GetCurrentScreen();'''
poll_replace = '''        // Process packets
        if (network_manager.GetClient()) {
            while (auto opt_packet = network_manager.GetClient()->ReceivePacket()) {
                // UI screens can process these, but since the UI is synchronous and screens are separate, 
                // typically screens subscribe to a dispatcher. 
                // For now, we will handle global things here if needed, but UI screens will poll the client themselves in Update() 
                // or we can just let UI screens use a custom dispatcher. 
                // Actually, if we pull the packet here, the UI screen won't see it!
                // So we SHOULD NOT poll here unless we have a dispatcher.
                // Let's create a very basic callback or just let UI screens pull them? 
                // If we pull here, we must dispatch.
            }
        }
        
        ui_engine.Update(dt);
        
        auto current_screen = ui_engine.GetScreens().GetCurrentScreen();'''
# Wait, if I pull packets here, UI screens can't see them. The UI screens shouldn't pull either because they might steal packets meant for others.
# Let's not poll in main.cpp, let the screens poll? No, MultiplayerClient::ReceivePacket pulls one packet from the thread-safe queue.
# If I don't poll here, I can let the current screen poll.
content = content.replace(poll_search, poll_search)

# Disconnect
shutdown_search = '''    if (emulator_booted) {
        emulator.Stop();
        emulator.Shutdown();
    }
    renderer.Shutdown();'''
shutdown_replace = '''    if (emulator_booted) {
        emulator.Stop();
        emulator.Shutdown();
    }
    network_manager.DisconnectClient();
    network_manager.Shutdown();
    renderer.Shutdown();'''
content = content.replace(shutdown_search, shutdown_replace)

with open('d:/Unbound/pokemon/cpp/client/main.cpp', 'w') as f:
    f.write(content)

