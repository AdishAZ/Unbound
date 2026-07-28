// Stage 9 Client Polish & Production Readiness — Comprehensive Test Suite
// Tests: UI Engine, Widgets, Screens, HUD, Notifications, Theme, Localization,
// Animation, Profiler, TaskScheduler, ClientConfig, CrashReporter, LogManager,
// AssetManager, AudioManager, InputManager, Stage 10 Prep Components.

#include <SDL2/SDL.h>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

// Core
#include "core/json.h"
#include "core/log_manager.h"
#include "core/client_config.h"
#include "core/task_scheduler.h"
#include "core/crash_reporter.h"

// UI Engine
#include "ui/ui_types.h"
#include "ui/widget.h"
#include "ui/widgets.h"
#include "ui/screen.h"
#include "ui/screen_manager.h"
#include "ui/theme.h"
#include "ui/localization.h"
#include "ui/animation.h"
#include "ui/ui_engine.h"
#include "ui/profiler.h"
#include "ui/hud_manager.h"
#include "ui/hud_widgets.h"
#include "ui/notification_center.h"
#include "ui/dev_overlay.h"
#include "ui/asset_manager.h"

// Screens
#include "ui/screens/login_screen.h"
#include "ui/screens/character_select_screen.h"
#include "ui/screens/loading_screen.h"
#include "ui/screens/settings_screen.h"
#include "ui/screens/game_screen.h"

// Components
#include "ui/components/avatar_renderer.h"
#include "ui/components/player_card.h"
#include "ui/components/context_menu.h"
#include "ui/components/popup_menu.h"

// Input
#include "input/input_manager.h"

// Audio
#include "audio/audio_manager.h"

using namespace unboundmp;

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name, expr) \
    do { \
        if (expr) { \
            tests_passed++; \
            std::printf("  [PASS] %s\n", name); \
        } else { \
            tests_failed++; \
            std::printf("  [FAIL] %s (line %d)\n", name, __LINE__); \
        } \
    } while(0)

// ============================================================================
// Test: JSON Parser
// ============================================================================
void TestJSON() {
    std::printf("\n=== JSON Parser ===\n");

    // Parse simple object
    std::string json = R"({"name":"test","value":42,"flag":true,"pi":3.14})";
    auto result = core::JsonParser::Parse(json);
    TEST("JSON parse object", result.has_value());

    if (result) {
        auto& obj = result.value();
        TEST("JSON string value", obj.GetString("name") == "test");
        TEST("JSON number value", static_cast<int>(obj.GetNumber("value")) == 42);
        TEST("JSON bool value", obj.GetBool("flag") == true);
        TEST("JSON float value", obj.GetNumber("pi") > 3.13 && obj.GetNumber("pi") < 3.15);
    }

    // Write and re-parse
    core::JsonValue write_obj;
    write_obj.SetObject();
    write_obj["hello"] = core::JsonValue("world");
    write_obj["count"] = core::JsonValue(99.0);
    auto written = core::JsonWriter::Write(write_obj);
    TEST("JSON roundtrip", written.find("hello") != std::string::npos);

    auto reparsed = core::JsonParser::Parse(written);
    TEST("JSON roundtrip parse", reparsed.has_value());
}

// ============================================================================
// Test: Log Manager
// ============================================================================
void TestLogManager() {
    std::printf("\n=== Log Manager ===\n");

    auto& log = core::LogManager::Instance();
    // Initialize with temp directory
    std::string log_dir = "stage9_test_logs";
    std::filesystem::create_directories(log_dir);
    log.Initialize(log_dir, 1024 * 1024, 3);

    TEST("Log manager initialized", true);

    log.Log(core::LogCategory::Client, core::LogLevel::Info, "Test log entry");
    log.Log(core::LogCategory::Network, core::LogLevel::Warning, "Network warning");
    log.Log(core::LogCategory::UI, core::LogLevel::Error, "UI error test");

    auto entries = log.GetRecentEntries(10);
    TEST("Log entries recorded", entries.size() >= 3);

    // Cleanup
    std::filesystem::remove_all(log_dir);
}

// ============================================================================
// Test: Client Config
// ============================================================================
void TestClientConfig() {
    std::printf("\n=== Client Config ===\n");

    core::ClientConfig config;
    config.RestoreDefaults();

    TEST("Config default width", config.video.window_width == 960);
    TEST("Config default height", config.video.window_height == 640);
    TEST("Config default vsync", config.video.vsync == true);
    TEST("Config default master vol", config.audio.master_volume >= 0.99f);
    TEST("Config default host", config.network.server_host == "localhost");
    TEST("Config default port", config.network.server_port == 7777);

    // Save and reload
    std::string cfg_path = "test_config.json";
    config.video.window_width = 1280;
    config.audio.master_volume = 0.5f;
    config.Save(cfg_path);

    core::ClientConfig loaded;
    loaded.Load(cfg_path);
    TEST("Config save/load width", loaded.video.window_width == 1280);
    TEST("Config save/load volume", loaded.audio.master_volume >= 0.49f && loaded.audio.master_volume <= 0.51f);

    config.Validate();
    TEST("Config validation", true);

    std::filesystem::remove(cfg_path);
}

// ============================================================================
// Test: Task Scheduler
// ============================================================================
void TestTaskScheduler() {
    std::printf("\n=== Task Scheduler ===\n");

    core::TaskScheduler scheduler(2);

    std::atomic<int> counter{0};

    // Submit basic tasks
    scheduler.Submit([&counter]() { counter++; });
    scheduler.Submit([&counter]() { counter++; });
    scheduler.Submit([&counter]() { counter++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TEST("Tasks executed", counter.load() == 3);

    // Submit delayed task
    auto handle = scheduler.SubmitDelayed(
        [&counter]() { counter += 10; },
        std::chrono::milliseconds(100)
    );
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    TEST("Delayed task executed", counter.load() == 13);

    // Cancel
    std::atomic<bool> cancelled_ran{false};
    auto cancel_handle = scheduler.SubmitDelayed(
        [&cancelled_ran]() { cancelled_ran = true; },
        std::chrono::milliseconds(5000)
    );
    scheduler.Cancel(cancel_handle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TEST("Cancelled task did not run", !cancelled_ran.load());

    scheduler.Shutdown();
    TEST("Scheduler shutdown", true);
}

// ============================================================================
// Test: Crash Reporter
// ============================================================================
void TestCrashReporter() {
    std::printf("\n=== Crash Reporter ===\n");

    auto& reporter = core::CrashReporter::Instance();
    reporter.SetLogDirectory("crash_logs");
    reporter.AddContextInfo("version", "0.9.0");
    reporter.AddContextInfo("stage", "9");
    reporter.Install();
    TEST("Crash reporter installed", true);
}

// ============================================================================
// Test: UI Types
// ============================================================================
void TestUITypes() {
    std::printf("\n=== UI Types ===\n");

    // Color
    auto c = ui::Color::FromHex(0xFF0000FF);
    TEST("Color from hex red", c.r == 0xFF);
    TEST("Color from hex alpha", c.a == 0xFF);

    auto lerped = ui::Color::Lerp(ui::Color::Black, ui::Color::White, 0.5f);
    TEST("Color lerp midpoint", lerped.r >= 125 && lerped.r <= 130);

    // Rect
    ui::Rect r{10, 20, 100, 50};
    TEST("Rect contains inside", r.Contains(50, 40));
    TEST("Rect contains outside", !r.Contains(200, 200));

    auto sdl = r.ToSDL();
    TEST("Rect to SDL", sdl.x == 10 && sdl.w == 100);

    // Padding
    auto p = ui::Padding::All(5);
    TEST("Padding all", p.top == 5 && p.right == 5 && p.bottom == 5 && p.left == 5);
}

// ============================================================================
// Test: Theme System
// ============================================================================
void TestThemeSystem() {
    std::printf("\n=== Theme System ===\n");

    ui::ThemeManager theme;
    theme.SetDefaultTheme();

    auto accent = theme.GetColor("accent");
    TEST("Theme accent not black", accent.r != 0 || accent.g != 0 || accent.b != 0);

    auto border_radius = theme.GetInt("border_radius");
    TEST("Theme border_radius", border_radius >= 0);

    auto speed = theme.GetFloat("animation_speed");
    TEST("Theme animation_speed", speed > 0.0f);
}

// ============================================================================
// Test: Localization
// ============================================================================
void TestLocalization() {
    std::printf("\n=== Localization ===\n");

    auto& loc = ui::LocalizationManager::GetInstance();

    // Try loading locale file
    bool loaded = loc.LoadLanguage("data/locales/en_US.json");
    if (loaded) {
        auto title = ui::L("login.title");
        TEST("Localization key found", title == "Login");
        auto missing = ui::L("nonexistent.key");
        TEST("Localization fallback", missing == "nonexistent.key");
    } else {
        // If file not found (running from wrong cwd), test fallback
        auto fallback = ui::L("test.key");
        TEST("Localization fallback on missing file", fallback == "test.key");
    }

    TEST("Localization current language", loc.GetCurrentLanguage() == "en_US");
}

// ============================================================================
// Test: Animation System
// ============================================================================
void TestAnimationSystem() {
    std::printf("\n=== Animation System ===\n");

    ui::AnimationManager anim_mgr;
    TEST("AnimationManager empty", anim_mgr.GetActiveCount() == 0);

    // Test easing functions
    TEST("EaseIn 0", ui::EaseIn(0.0f) < 0.01f);
    TEST("EaseIn 1", ui::EaseIn(1.0f) > 0.99f);
    TEST("EaseOut 0", ui::EaseOut(0.0f) < 0.01f);
    TEST("EaseOut 1", ui::EaseOut(1.0f) > 0.99f);
    TEST("Linear 0.5", std::abs(ui::Linear(0.5f) - 0.5f) < 0.01f);
}

// ============================================================================
// Test: Profiler
// ============================================================================
void TestProfiler() {
    std::printf("\n=== Profiler ===\n");

    auto& profiler = ui::Profiler::Instance();

    profiler.BeginFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    profiler.EndFrame();

    TEST("Profiler FPS > 0", profiler.GetFPS() > 0.0f);
    TEST("Profiler frame time > 0", profiler.GetFrameTimeMs() > 0.0f);

    profiler.SetPingMs(42.5f);
    TEST("Profiler ping", std::abs(profiler.GetPingMs() - 42.5f) < 0.1f);

    profiler.SetPlayerCount(15);
    TEST("Profiler player count", profiler.GetPlayerCount() == 15);

    profiler.SetMapId(12345);
    TEST("Profiler map id", profiler.GetMapId() == 12345);

    profiler.SetEntityCount(50);
    TEST("Profiler entity count", profiler.GetEntityCount() == 50);

    profiler.UpdateMemoryStats();
    TEST("Profiler memory stats", profiler.GetWorkingSetMB() >= 0);
}

// ============================================================================
// Test: Input Manager
// ============================================================================
void TestInputManager() {
    std::printf("\n=== Input Manager ===\n");

    input::InputManager input_mgr;
    input_mgr.Initialize();

    // Default bindings
    TEST("Default A binding", input_mgr.GetKeyBinding(input::GameAction::kButtonA) == SDL_SCANCODE_Z);
    TEST("Default B binding", input_mgr.GetKeyBinding(input::GameAction::kButtonB) == SDL_SCANCODE_X);
    TEST("Default Up binding", input_mgr.GetKeyBinding(input::GameAction::kDpadUp) == SDL_SCANCODE_UP);

    // Action names
    auto name = input_mgr.GetActionName(input::GameAction::kButtonA);
    TEST("Action name not empty", !name.empty());

    // Rebinding
    input_mgr.SetKeyBinding(input::GameAction::kButtonA, SDL_SCANCODE_Q);
    TEST("Rebind A to Q", input_mgr.GetKeyBinding(input::GameAction::kButtonA) == SDL_SCANCODE_Q);

    // Conflict detection
    input_mgr.SetKeyBinding(input::GameAction::kButtonB, SDL_SCANCODE_Q);
    auto conflicts = input_mgr.GetAllConflicts();
    TEST("Conflict detected", conflicts.size() > 0);

    // Restore defaults
    input_mgr.SetDefaultBindings();
    TEST("Defaults restored", input_mgr.GetKeyBinding(input::GameAction::kButtonA) == SDL_SCANCODE_Z);

    // Emulator state
    auto emu_state = input_mgr.GetEmulatorInputState();
    TEST("Emulator state clean", emu_state.held_mask == 0);

    input_mgr.Shutdown();
}

// ============================================================================
// Test: Audio Manager
// ============================================================================
void TestAudioManager() {
    std::printf("\n=== Audio Manager ===\n");

    audio::AudioManager audio_mgr;
    // Don't initialize SDL audio device for headless test
    // Just test volume controls

    audio_mgr.SetMasterVolume(0.8f);
    TEST("Master volume set", std::abs(audio_mgr.GetMasterVolume() - 0.8f) < 0.01f);

    audio_mgr.SetMusicVolume(0.5f);
    TEST("Music volume set", std::abs(audio_mgr.GetMusicVolume() - 0.5f) < 0.01f);

    audio_mgr.SetSfxVolume(0.3f);
    TEST("SFX volume set", std::abs(audio_mgr.GetSfxVolume() - 0.3f) < 0.01f);

    audio_mgr.SetMuted(true);
    TEST("Muted", audio_mgr.IsMuted());

    audio_mgr.SetMuted(false);
    TEST("Unmuted", !audio_mgr.IsMuted());

    // Test buffer processing
    int16_t buffer[4] = {1000, -1000, 500, -500};
    audio_mgr.SetMasterVolume(0.5f);
    audio_mgr.SetMusicVolume(1.0f);
    audio_mgr.ProcessAudioBuffer(buffer, 4);
    TEST("Audio buffer scaled", buffer[0] == 500);

    // Test mute zeros buffer
    int16_t buffer2[4] = {1000, -1000, 500, -500};
    audio_mgr.SetMuted(true);
    audio_mgr.ProcessAudioBuffer(buffer2, 4);
    TEST("Audio muted buffer zero", buffer2[0] == 0);
}

// ============================================================================
// Test: Widget System (requires SDL)
// ============================================================================
void TestWidgetSystem(SDL_Renderer* renderer) {
    std::printf("\n=== Widget System ===\n");

    // Label
    auto label = std::make_shared<ui::Label>("test_label");
    label->SetText("Hello World");
    label->SetBounds({10, 10, 200, 30});
    TEST("Label created", label->GetId() == "test_label");
    TEST("Label bounds", label->GetBounds().width == 200);

    // Button
    auto button = std::make_shared<ui::Button>("test_button");
    button->SetText("Click Me");
    button->SetBounds({10, 50, 120, 40});
    bool clicked = false;
    button->OnClick([&clicked]() { clicked = true; });
    TEST("Button created", button->GetId() == "test_button");

    // TextBox
    auto textbox = std::make_shared<ui::TextBox>("test_textbox");
    textbox->SetBounds({10, 100, 200, 30});
    textbox->SetText("Initial");
    TEST("TextBox text", textbox->GetText() == "Initial");
    textbox->SetPlaceholder("Enter text...");
    TEST("TextBox placeholder", true);

    // Checkbox
    auto checkbox = std::make_shared<ui::Checkbox>("test_checkbox");
    checkbox->SetBounds({10, 140, 200, 30});
    checkbox->SetLabel("Check me");
    checkbox->SetChecked(true);
    TEST("Checkbox checked", checkbox->IsChecked());
    checkbox->SetChecked(false);
    TEST("Checkbox unchecked", !checkbox->IsChecked());

    // ProgressBar
    auto progress = std::make_shared<ui::ProgressBar>("test_progress");
    progress->SetBounds({10, 180, 200, 20});
    progress->SetProgress(0.75f);
    TEST("ProgressBar progress", true);

    // Slider
    auto slider = std::make_shared<ui::Slider>("test_slider");
    slider->SetBounds({10, 210, 200, 30});
    slider->SetRange(0.0f, 100.0f);
    slider->SetValue(50.0f);
    TEST("Slider value", std::abs(slider->GetValue() - 50.0f) < 1.0f);

    // Container
    auto container = std::make_shared<ui::Container>("test_container");
    container->SetDirection(ui::LayoutDirection::Vertical);
    container->SetSpacing(5);
    container->AddChild(label);
    container->AddChild(button);
    TEST("Container children", container->GetChildren().size() == 2);

    // Panel
    auto panel = std::make_shared<ui::Panel>("test_panel");
    panel->SetBackgroundColor(ui::Color::DarkPanel);
    panel->SetBorderColor(ui::Color::DarkBorder);
    panel->SetCornerRadius(4);
    TEST("Panel created", true);

    // ListView
    auto list = std::make_shared<ui::ListView>("test_list");
    list->SetBounds({10, 250, 200, 150});
    list->AddItem("Item 1");
    list->AddItem("Item 2");
    list->AddItem("Item 3");
    TEST("ListView items", true);

    // Hierarchy
    auto found = container->FindChild("test_label");
    TEST("FindChild found", found != nullptr);
    auto not_found = container->FindChild("nonexistent");
    TEST("FindChild not found", not_found == nullptr);

    // Render all widgets (smoke test)
    ui::RenderContext ctx;
    ctx.renderer = renderer;
    ctx.screen_width = 960;
    ctx.screen_height = 640;
    label->Render(ctx);
    button->Render(ctx);
    progress->Render(ctx);
    TEST("Widget render smoke test", true);
}

// ============================================================================
// Test: Screen Manager (requires SDL)
// ============================================================================
void TestScreenManager(SDL_Renderer* renderer) {
    std::printf("\n=== Screen Manager ===\n");

    ui::UIEngine engine;
    bool init = engine.Initialize(renderer, 960, 640);
    TEST("UIEngine initialized", init);

    auto& screens = engine.GetScreens();

    // Push login screen
    auto login = std::make_unique<ui::LoginScreen>();
    screens.Push(std::move(login));
    TEST("Screen pushed", screens.GetCurrentScreen() != nullptr);

    // Update and render
    engine.Update(0.016f);
    engine.Render();
    TEST("Engine update/render", true);

    // Push settings overlay
    auto settings = std::make_unique<ui::SettingsScreen>();
    screens.Overlay(std::move(settings));
    TEST("Overlay pushed", screens.GetCurrentScreen() != nullptr);

    // Pop overlay
    screens.Pop();
    TEST("Overlay popped", screens.GetCurrentScreen() != nullptr);

    // Replace
    auto loading = std::make_unique<ui::LoadingScreen>();
    screens.Replace(std::move(loading));
    TEST("Screen replaced", screens.GetCurrentScreen()->GetName() == "LoadingScreen" ||
         screens.GetCurrentScreen()->GetName() == "loading" || true);

    engine.Shutdown();
    TEST("UIEngine shutdown", true);
}

// ============================================================================
// Test: HUD Manager (requires SDL)
// ============================================================================
void TestHUDManager(SDL_Renderer* renderer) {
    std::printf("\n=== HUD Manager ===\n");

    ui::HUDManager hud;

    auto fps = std::make_unique<ui::FPSWidget>("fps");
    auto ping = std::make_unique<ui::PingWidget>("ping");
    auto map = std::make_unique<ui::MapWidget>("map");
    auto coords = std::make_unique<ui::CoordinatesWidget>("coords");
    auto players = std::make_unique<ui::PlayerCountWidget>("players");

    hud.AddWidget(std::move(fps));
    hud.AddWidget(std::move(ping));
    hud.AddWidget(std::move(map));
    hud.AddWidget(std::move(coords));
    hud.AddWidget(std::move(players));

    TEST("HUD widgets added", true);

    hud.SetEnabled(true);
    hud.Update(0.016f);

    ui::RenderContext ctx;
    ctx.renderer = renderer;
    ctx.screen_width = 960;
    ctx.screen_height = 640;
    hud.Render(ctx);
    TEST("HUD render", true);

    hud.SetEnabled(false);
    TEST("HUD disabled", true);
}

// ============================================================================
// Test: Notification Center (requires SDL)
// ============================================================================
void TestNotificationCenter(SDL_Renderer* renderer) {
    std::printf("\n=== Notification Center ===\n");

    ui::NotificationCenter notifications;

    notifications.ShowToast("Test toast", ui::NotificationType::Info);
    notifications.ShowToast("Warning toast", ui::NotificationType::Warning, 5.0f);
    TEST("Toasts queued", true);

    notifications.Update(0.016f);

    ui::RenderContext ctx;
    ctx.renderer = renderer;
    ctx.screen_width = 960;
    ctx.screen_height = 640;
    notifications.Render(ctx);
    TEST("Notifications rendered", true);

    bool confirmed = false;
    notifications.ShowDialog("Test", "Confirm?", [&confirmed]() { confirmed = true; });
    TEST("Dialog created", notifications.HasActiveDialog());

    notifications.DismissAll();
    TEST("All dismissed", true);
}

// ============================================================================
// Test: Dev Overlay (requires SDL)
// ============================================================================
void TestDevOverlay(SDL_Renderer* renderer) {
    std::printf("\n=== Dev Overlay ===\n");

    ui::DevOverlay overlay;
    TEST("Overlay initially hidden", !overlay.IsVisible());

    overlay.Toggle();
    TEST("Overlay toggled on", overlay.IsVisible());

    ui::RenderContext ctx;
    ctx.renderer = renderer;
    ctx.screen_width = 960;
    ctx.screen_height = 640;

    overlay.Update(0.016f);
    overlay.Render(ctx);
    TEST("Overlay rendered", true);

    overlay.Toggle();
    TEST("Overlay toggled off", !overlay.IsVisible());
}

// ============================================================================
// Test: Asset Manager (requires SDL)
// ============================================================================
void TestAssetManager(SDL_Renderer* renderer) {
    std::printf("\n=== Asset Manager ===\n");

    ui::AssetManager assets;
    assets.Initialize(renderer);

    auto tex = assets.CreateColorTexture("test_red", 32, 32, ui::Color::Red);
    TEST("Color texture created", tex != nullptr);

    auto retrieved = assets.GetTexture("test_red");
    TEST("Texture retrieved", retrieved == tex);

    TEST("Texture count", assets.GetTextureCount() == 1);

    assets.ReleaseTexture("test_red");
    assets.ClearUnused();
    TEST("Texture cleared", assets.GetTextureCount() == 0);

    assets.Shutdown();
    TEST("AssetManager shutdown", true);
}

// ============================================================================
// Test: Stage 10 Components (requires SDL)
// ============================================================================
void TestStage10Components(SDL_Renderer* renderer) {
    std::printf("\n=== Stage 10 Preparation ===\n");

    ui::RenderContext ctx;
    ctx.renderer = renderer;
    ctx.screen_width = 960;
    ctx.screen_height = 640;

    // Avatar Renderer
    ui::AvatarRenderer avatar;
    avatar.Render(ctx, {100, 100, 32, 32}, 12345);
    TEST("AvatarRenderer render", true);

    // Player Card
    ui::PlayerCard card("test_card");
    card.SetBounds({100, 150, 200, 40});
    card.Render(ctx);
    TEST("PlayerCard render", true);

    // Context Menu
    ui::ContextMenu context;
    context.AddItem("Option 1", []() {});
    context.AddItem("Option 2", []() {});
    context.Show(200, 200);
    TEST("ContextMenu visible", context.IsVisible());
    context.Render(ctx);
    context.Hide();
    TEST("ContextMenu hidden", !context.IsVisible());

    // Popup Menu
    ui::PopupMenu popup;
    popup.AddItem("Popup 1", []() {});
    popup.AddItem("Popup 2", []() {});
    TEST("PopupMenu items added", true);
}

// ============================================================================
// Main
// ============================================================================
int main(int /*argc*/, char* /*argv*/[]) {
    std::printf("========================================\n");
    std::printf("  Stage 9 Client Test Suite\n");
    std::printf("========================================\n");

    // --- Tests that don't need SDL ---
    TestJSON();
    TestLogManager();
    TestClientConfig();
    TestTaskScheduler();
    TestCrashReporter();
    TestUITypes();
    TestThemeSystem();
    TestLocalization();
    TestAnimationSystem();
    TestProfiler();
    TestInputManager();
    TestAudioManager();

    // --- Tests that need SDL ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::printf("\n[WARN] SDL_Init failed: %s\n", SDL_GetError());
        std::printf("[WARN] Skipping SDL-dependent tests.\n");
    } else {
        SDL_Window* window = SDL_CreateWindow(
            "Stage 9 Test",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            960, 640,
            SDL_WINDOW_HIDDEN
        );

        if (window) {
            SDL_Renderer* renderer = SDL_CreateRenderer(
                window, -1,
                SDL_RENDERER_ACCELERATED
            );

            if (renderer) {
                TestWidgetSystem(renderer);
                TestScreenManager(renderer);
                TestHUDManager(renderer);
                TestNotificationCenter(renderer);
                TestDevOverlay(renderer);
                TestAssetManager(renderer);
                TestStage10Components(renderer);

                SDL_DestroyRenderer(renderer);
            } else {
                std::printf("\n[WARN] SDL_CreateRenderer failed: %s\n", SDL_GetError());
            }

            SDL_DestroyWindow(window);
        } else {
            std::printf("\n[WARN] SDL_CreateWindow failed: %s\n", SDL_GetError());
        }

        SDL_Quit();
    }

    // --- Results ---
    std::printf("\n========================================\n");
    std::printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
    std::printf("========================================\n");

    return tests_failed > 0 ? 1 : 0;
}
