#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <stdexcept>
#include <sstream>

namespace unboundmp::core {

class JsonValue;

using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;
using JsonString = std::string;
using JsonNumber = double;
using JsonBool = bool;
using JsonNull = std::nullptr_t;

class JsonValue {
public:
    using VariantType = std::variant<JsonNull, JsonBool, JsonNumber, JsonString, std::shared_ptr<JsonArray>, std::shared_ptr<JsonObject>>;

    JsonValue() : m_value(nullptr) {}
    JsonValue(JsonNull val) : m_value(val) {}
    JsonValue(bool val) : m_value(val) {}
    JsonValue(int val) : m_value(static_cast<double>(val)) {}
    JsonValue(double val) : m_value(val) {}
    JsonValue(const char* val) : m_value(std::string(val)) {}
    JsonValue(const std::string& val) : m_value(val) {}
    JsonValue(const JsonArray& val) : m_value(std::make_shared<JsonArray>(val)) {}
    JsonValue(const JsonObject& val) : m_value(std::make_shared<JsonObject>(val)) {}

    bool IsNull() const { return std::holds_alternative<JsonNull>(m_value); }
    bool IsBool() const { return std::holds_alternative<JsonBool>(m_value); }
    bool IsNumber() const { return std::holds_alternative<JsonNumber>(m_value); }
    bool IsString() const { return std::holds_alternative<JsonString>(m_value); }
    bool IsArray() const { return std::holds_alternative<std::shared_ptr<JsonArray>>(m_value); }
    bool IsObject() const { return std::holds_alternative<std::shared_ptr<JsonObject>>(m_value); }

    bool AsBool() const { return std::get<JsonBool>(m_value); }
    double AsNumber() const { return std::get<JsonNumber>(m_value); }
    int AsInt() const { return static_cast<int>(std::get<JsonNumber>(m_value)); }
    const std::string& AsString() const { return std::get<JsonString>(m_value); }
    const JsonArray& AsArray() const { return *std::get<std::shared_ptr<JsonArray>>(m_value); }
    const JsonObject& AsObject() const { return *std::get<std::shared_ptr<JsonObject>>(m_value); }

    JsonArray& AsArray() { return *std::get<std::shared_ptr<JsonArray>>(m_value); }
    JsonObject& AsObject() { return *std::get<std::shared_ptr<JsonObject>>(m_value); }

private:
    VariantType m_value;
};

class JsonParser {
public:
    static JsonValue Parse(const std::string& json);
};

class JsonWriter {
public:
    static std::string Write(const JsonValue& value, int indent = -1);
};

} // namespace unboundmp::core
