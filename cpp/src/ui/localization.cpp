#include "ui/localization.h"
#include <fstream>
#include <sstream>

namespace unboundmp::ui {

bool LocalizationManager::LoadLanguage(const std::string& filepath) {
    // Basic JSON parser stub. In real app, use nlohmann/json or similar.
    // Assuming simple key-value pairs per line for this stub
    strings_.clear();
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
        auto colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Clean quotes and spaces
            key.erase(remove(key.begin(), key.end(), '\"'), key.end());
            key.erase(remove(key.begin(), key.end(), ' '), key.end());
            
            size_t first = value.find_first_not_of(" \",");
            size_t last = value.find_last_not_of(" \",");
            if (first != std::string::npos && last != std::string::npos) {
                value = value.substr(first, (last - first + 1));
                strings_[key] = value;
            }
        }
    }
    return true;
}

std::string LocalizationManager::Get(const std::string& key) const {
    auto it = strings_.find(key);
    if (it != strings_.end()) {
        return it->second;
    }
    return key;
}

bool LocalizationManager::SetLanguage(const std::string& locale_code) {
    std::string filepath = "d:/Unbound/pokemon/cpp/data/locales/" + locale_code + ".json";
    if (LoadLanguage(filepath)) {
        current_language_ = locale_code;
        return true;
    }
    return false;
}

std::string LocalizationManager::GetCurrentLanguage() const {
    return current_language_;
}

} // namespace unboundmp::ui
