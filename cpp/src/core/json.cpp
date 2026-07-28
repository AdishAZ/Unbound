#include "core/json.h"
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace unboundmp::core {

namespace {

class ParserImpl {
    const std::string& m_json;
    size_t m_pos;

    void SkipWhitespace() {
        while (m_pos < m_json.length() && std::isspace(static_cast<unsigned char>(m_json[m_pos]))) {
            m_pos++;
        }
    }

    char Peek() const { return m_pos < m_json.length() ? m_json[m_pos] : '\0'; }
    char Next() { return m_pos < m_json.length() ? m_json[m_pos++] : '\0'; }

    void Expect(char c) {
        SkipWhitespace();
        if (Next() != c) {
            throw std::runtime_error(std::string("Expected '") + c + "' at position " + std::to_string(m_pos));
        }
    }

    std::string ParseString() {
        Expect('"');
        std::string result;
        while (m_pos < m_json.length() && m_json[m_pos] != '"') {
            if (m_json[m_pos] == '\\') {
                m_pos++;
                if (m_pos >= m_json.length()) throw std::runtime_error("Unexpected end of string escape");
                char c = m_json[m_pos++];
                switch (c) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += c; break; // Simplified unicode/escape handling
                }
            } else {
                result += m_json[m_pos++];
            }
        }
        Expect('"');
        return result;
    }

    JsonValue ParseNumber() {
        size_t start = m_pos;
        if (m_pos < m_json.length() && m_json[m_pos] == '-') m_pos++;
        while (m_pos < m_json.length() && std::isdigit(static_cast<unsigned char>(m_json[m_pos]))) m_pos++;
        if (m_pos < m_json.length() && m_json[m_pos] == '.') {
            m_pos++;
            while (m_pos < m_json.length() && std::isdigit(static_cast<unsigned char>(m_json[m_pos]))) m_pos++;
        }
        if (m_pos < m_json.length() && (m_json[m_pos] == 'e' || m_json[m_pos] == 'E')) {
            m_pos++;
            if (m_pos < m_json.length() && (m_json[m_pos] == '+' || m_json[m_pos] == '-')) m_pos++;
            while (m_pos < m_json.length() && std::isdigit(static_cast<unsigned char>(m_json[m_pos]))) m_pos++;
        }
        std::string numStr = m_json.substr(start, m_pos - start);
        return JsonValue(std::stod(numStr));
    }

    JsonValue ParseObject() {
        JsonObject obj;
        Expect('{');
        SkipWhitespace();
        if (Peek() == '}') {
            Next();
            return JsonValue(obj);
        }
        while (true) {
            SkipWhitespace();
            std::string key = ParseString();
            Expect(':');
            JsonValue val = ParseValue();
            obj[key] = val;
            SkipWhitespace();
            if (Peek() == ',') {
                Next();
            } else if (Peek() == '}') {
                Next();
                break;
            } else {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
        }
        return JsonValue(obj);
    }

    JsonValue ParseArray() {
        JsonArray arr;
        Expect('[');
        SkipWhitespace();
        if (Peek() == ']') {
            Next();
            return JsonValue(arr);
        }
        while (true) {
            arr.push_back(ParseValue());
            SkipWhitespace();
            if (Peek() == ',') {
                Next();
            } else if (Peek() == ']') {
                Next();
                break;
            } else {
                throw std::runtime_error("Expected ',' or ']' in array");
            }
        }
        return JsonValue(arr);
    }

    JsonValue ParseValue() {
        SkipWhitespace();
        char c = Peek();
        if (c == '"') return ParseString();
        if (c == '{') return ParseObject();
        if (c == '[') return ParseArray();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return ParseNumber();
        
        if (m_json.compare(m_pos, 4, "true") == 0) { m_pos += 4; return JsonValue(true); }
        if (m_json.compare(m_pos, 5, "false") == 0) { m_pos += 5; return JsonValue(false); }
        if (m_json.compare(m_pos, 4, "null") == 0) { m_pos += 4; return JsonValue(nullptr); }
        
        throw std::runtime_error(std::string("Unexpected character '") + c + "' at position " + std::to_string(m_pos));
    }

public:
    ParserImpl(const std::string& json) : m_json(json), m_pos(0) {}
    JsonValue Parse() {
        JsonValue val = ParseValue();
        SkipWhitespace();
        if (m_pos < m_json.length()) {
            throw std::runtime_error("Trailing characters after JSON value");
        }
        return val;
    }
};

void EscapeString(std::ostream& os, const std::string& str) {
    os << '"';
    for (char c : str) {
        switch (c) {
            case '"': os << "\\\""; break;
            case '\\': os << "\\\\"; break;
            case '\b': os << "\\b"; break;
            case '\f': os << "\\f"; break;
            case '\n': os << "\\n"; break;
            case '\r': os << "\\r"; break;
            case '\t': os << "\\t"; break;
            default: os << c; break;
        }
    }
    os << '"';
}

void WriteImpl(std::ostream& os, const JsonValue& val, int indentLevel, int indentSize) {
    if (val.IsNull()) { os << "null"; }
    else if (val.IsBool()) { os << (val.AsBool() ? "true" : "false"); }
    else if (val.IsNumber()) {
        double d = val.AsNumber();
        if (d == static_cast<int>(d)) { os << static_cast<int>(d); }
        else { os << d; }
    }
    else if (val.IsString()) { EscapeString(os, val.AsString()); }
    else if (val.IsArray()) {
        const auto& arr = val.AsArray();
        if (arr.empty()) { os << "[]"; return; }
        os << "[";
        if (indentSize >= 0) os << "\n";
        for (size_t i = 0; i < arr.size(); ++i) {
            if (indentSize >= 0) os << std::string((indentLevel + 1) * indentSize, ' ');
            WriteImpl(os, arr[i], indentLevel + 1, indentSize);
            if (i < arr.size() - 1) os << ",";
            if (indentSize >= 0) os << "\n";
        }
        if (indentSize >= 0) os << std::string(indentLevel * indentSize, ' ');
        os << "]";
    }
    else if (val.IsObject()) {
        const auto& obj = val.AsObject();
        if (obj.empty()) { os << "{}"; return; }
        os << "{";
        if (indentSize >= 0) os << "\n";
        size_t count = 0;
        for (const auto& [key, value] : obj) {
            if (indentSize >= 0) os << std::string((indentLevel + 1) * indentSize, ' ');
            EscapeString(os, key);
            os << (indentSize >= 0 ? ": " : ":");
            WriteImpl(os, value, indentLevel + 1, indentSize);
            if (++count < obj.size()) os << ",";
            if (indentSize >= 0) os << "\n";
        }
        if (indentSize >= 0) os << std::string(indentLevel * indentSize, ' ');
        os << "}";
    }
}

} // namespace

JsonValue JsonParser::Parse(const std::string& json) {
    ParserImpl parser(json);
    return parser.Parse();
}

std::string JsonWriter::Write(const JsonValue& value, int indent) {
    std::ostringstream os;
    WriteImpl(os, value, 0, indent);
    return os.str();
}

} // namespace unboundmp::core
