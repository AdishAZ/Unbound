#pragma once
#include <vector>
#include "render/render_layer.h"

namespace unboundmp::render {

enum class WeatherType {
    kNone,
    kRain,
    kSnow,
    kFog,
    kAsh
};

class WeatherRenderer : public RenderLayer {
public:
    WeatherRenderer();
    ~WeatherRenderer() override = default;

    void Render(const RenderContext& context) override;
    
    void CycleWeather() {
        int next = static_cast<int>(current_weather_) + 1;
        if (next > static_cast<int>(WeatherType::kAsh)) {
            next = 0;
        }
        current_weather_ = static_cast<WeatherType>(next);
    }
    
    void SetWeather(WeatherType type) { current_weather_ = type; }
    WeatherType GetWeather() const { return current_weather_; }
    const char* GetWeatherName() const {
        switch (current_weather_) {
            case WeatherType::kNone: return "None";
            case WeatherType::kRain: return "Rain";
            case WeatherType::kSnow: return "Snow";
            case WeatherType::kFog: return "Fog";
            case WeatherType::kAsh: return "Ash";
            default: return "Unknown";
        }
    }

private:
    struct Particle {
        float x, y;
        float vx, vy;
        float life;
        float max_life;
    };
    std::vector<Particle> particles_;
    WeatherType current_weather_ = WeatherType::kNone;
    float time_accumulator_ = 0.0f;
};

} // namespace unboundmp::render
