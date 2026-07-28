#pragma once
#include <string>
#include <unordered_map>

namespace unboundmp::ui {

class LocalizationManager {
public:
    static LocalizationManager& GetInstance() {
        static LocalizationManager instance;
        return instance;
    }

    bool LoadLanguage(const std::string& filepath);
    std::string Get(const std::string& key) const;
    bool SetLanguage(const std::string& locale_code);
    std::string GetCurrentLanguage() const;

private:
    LocalizationManager() = default;
    ~LocalizationManager() = default;

    std::unordered_map<std::string, std::string> strings_;
    std::string current_language_ = "en_US";
};

inline std::string L(const std::string& key) {
    return LocalizationManager::GetInstance().Get(key);
}

} // namespace unboundmp::ui
