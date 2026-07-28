#pragma once
#include <string>
#include <vector>
#include <map>

namespace unboundmp::core {

struct VideoConfig {
    int window_width = 1280;
    int window_height = 720;
    bool fullscreen = false;
    bool vsync = true;
    float scale_factor = 1.0f;
    bool show_fps = false;
};

struct AudioConfig {
    float master_volume = 1.0f;
    float music_volume = 1.0f;
    float sfx_volume = 1.0f;
    bool muted = false;
};

struct ControlsConfig {
    std::map<std::string, int> key_bindings;
    float gamepad_deadzone = 0.1f;
};

struct NetworkConfig {
    std::string server_host = "localhost";
    int server_port = 7777;
    bool auto_reconnect = true;
    int reconnect_delay_ms = 3000;
    int connection_timeout_ms = 10000;
};

struct UIConfig {
    std::string theme_name = "default";
    std::string language = "en";
    int notification_duration_ms = 3000;
    bool hud_enabled = true;
};

struct DeveloperConfig {
    bool show_overlay = false;
    int overlay_key = 0;
    int log_level = 1;
};

struct GameplayConfig {
    int auto_save_interval_ms = 60000;
    bool remember_username = false;
    std::string last_username = "";
};

class ClientConfig {
public:
    int schema_version = 1;
    
    VideoConfig video;
    AudioConfig audio;
    ControlsConfig controls;
    NetworkConfig network;
    UIConfig ui;
    DeveloperConfig developer;
    GameplayConfig gameplay;

    bool Load(const std::string& filepath);
    bool Save(const std::string& filepath) const;
    void RestoreDefaults();
    void Validate();
};

} // namespace unboundmp::core
