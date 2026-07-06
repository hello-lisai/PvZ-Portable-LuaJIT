// MiniJson.h —— 极简 JSON 读写器（header-only）
//
// 仅满足 Mod API 配置持久化的最小需求：
//   - 支持 null / bool / int / double / string / array / object
//   - 单遍解析，无第三方依赖
//   - 不支持注释、尾逗号、单引号、科学计数法（够用即可）
//
// 用法：
//   JsonValue v;
//   if (JsonParse("{...}", v)) { ... }
//   std::string s = JsonDump(v);
//
#pragma once

#include <cstdint>
#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ModJson {

class JsonValue;
using JsonObject = std::map<std::string, JsonValue>;
using JsonArray  = std::vector<JsonValue>;

class JsonValue {
public:
    enum class Type { Null, Bool, Int, Double, String, Array, Object };

    JsonValue() : m_type(Type::Null) {}
    static JsonValue MakeBool(bool v)        { JsonValue j; j.m_type = Type::Bool;   j.m_b = v; return j; }
    static JsonValue MakeInt(int64_t v)      { JsonValue j; j.m_type = Type::Int;    j.m_i = v; return j; }
    static JsonValue MakeDouble(double v)    { JsonValue j; j.m_type = Type::Double; j.m_d = v; return j; }
    static JsonValue MakeString(std::string v){JsonValue j; j.m_type = Type::String; j.m_s = std::move(v); return j; }
    static JsonValue MakeArray(JsonArray v)  { JsonValue j; j.m_type = Type::Array;  j.m_a = std::move(v); return j; }
    static JsonValue MakeObject(JsonObject v){ JsonValue j; j.m_type = Type::Object; j.m_o = std::move(v); return j; }

    Type type() const { return m_type; }
    bool isNull()   const { return m_type == Type::Null; }
    bool isBool()   const { return m_type == Type::Bool; }
    bool isNumber() const { return m_type == Type::Int || m_type == Type::Double; }
    bool isString() const { return m_type == Type::String; }
    bool isArray()  const { return m_type == Type::Array; }
    bool isObject() const { return m_type == Type::Object; }

    bool        asBool(bool def = false)    const { return m_type == Type::Bool ? m_b : def; }
    int64_t     asInt(int64_t def = 0)      const { return m_type == Type::Int ? m_i : (m_type == Type::Double ? static_cast<int64_t>(m_d) : def); }
    double      asDouble(double def = 0.0)  const { return m_type == Type::Double ? m_d : (m_type == Type::Int ? static_cast<double>(m_i) : def); }
    const std::string& asString() const { return m_s; }
    const JsonArray&  asArray()  const { return m_a; }
    const JsonObject& asObject() const { return m_o; }

    // 对象访问：不存在返回 nullptr
    const JsonValue* find(const std::string& key) const {
        if (m_type != Type::Object) return nullptr;
        auto it = m_o.find(key);
        return it == m_o.end() ? nullptr : &it->second;
    }
    // 可写访问（不存在则插入 null）
    JsonValue& operator[](const std::string& key) {
        if (m_type != Type::Object) { m_type = Type::Object; m_o.clear(); }
        return m_o[key];
    }

private:
    Type        m_type = Type::Null;
    bool        m_b = false;
    int64_t     m_i = 0;
    double      m_d = 0.0;
    std::string m_s;
    JsonArray   m_a;
    JsonObject  m_o;
};

// ============ 解析器 ============

class JsonParser {
public:
    JsonParser(const std::string& src) : m_src(src), m_pos(0) {}

    bool parse(JsonValue& out) {
        skipWs();
        if (!parseValue(out)) return false;
        skipWs();
        return m_pos == m_src.size();  // 末尾不应有剩余内容
    }

private:
    const std::string& m_src;
    size_t m_pos;

    void skipWs() {
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { ++m_pos; continue; }
            break;
        }
    }

    bool parseValue(JsonValue& out) {
        skipWs();
        if (m_pos >= m_src.size()) return false;
        char c = m_src[m_pos];
        if (c == '{') return parseObject(out);
        if (c == '[') return parseArray(out);
        if (c == '"') return parseString(out);
        if (c == 't' || c == 'f') return parseBool(out);
        if (c == 'n') return parseNull(out);
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(out);
        return false;
    }

    bool parseObject(JsonValue& out) {
        ++m_pos; // 跳过 {
        JsonObject obj;
        skipWs();
        if (peek() == '}') { ++m_pos; out = JsonValue::MakeObject(std::move(obj)); return true; }
        while (true) {
            skipWs();
            if (peek() != '"') return false;
            std::string key;
            if (!parseStringRaw(key)) return false;
            skipWs();
            if (peek() != ':') return false;
            ++m_pos;
            JsonValue v;
            if (!parseValue(v)) return false;
            obj[std::move(key)] = std::move(v);
            skipWs();
            char c = peek();
            if (c == ',') { ++m_pos; continue; }
            if (c == '}') { ++m_pos; out = JsonValue::MakeObject(std::move(obj)); return true; }
            return false;
        }
    }

    bool parseArray(JsonValue& out) {
        ++m_pos; // 跳过 [
        JsonArray arr;
        skipWs();
        if (peek() == ']') { ++m_pos; out = JsonValue::MakeArray(std::move(arr)); return true; }
        while (true) {
            JsonValue v;
            if (!parseValue(v)) return false;
            arr.push_back(std::move(v));
            skipWs();
            char c = peek();
            if (c == ',') { ++m_pos; continue; }
            if (c == ']') { ++m_pos; out = JsonValue::MakeArray(std::move(arr)); return true; }
            return false;
        }
    }

    bool parseString(JsonValue& out) {
        std::string s;
        if (!parseStringRaw(s)) return false;
        out = JsonValue::MakeString(std::move(s));
        return true;
    }

    bool parseStringRaw(std::string& out) {
        if (peek() != '"') return false;
        ++m_pos;
        out.clear();
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos++];
            if (c == '"') return true;
            if (c == '\\') {
                if (m_pos >= m_src.size()) return false;
                char esc = m_src[m_pos++];
                switch (esc) {
                case '"':  out.push_back('"');  break;
                case '\\': out.push_back('\\'); break;
                case '/':  out.push_back('/');  break;
                case 'b':  out.push_back('\b'); break;
                case 'f':  out.push_back('\f'); break;
                case 'n':  out.push_back('\n'); break;
                case 'r':  out.push_back('\r'); break;
                case 't':  out.push_back('\t'); break;
                case 'u': {
                    // 简单处理：把 \uXXXX 当作 4 个十六进制字符跳过，输出原样（够用即可）
                    if (m_pos + 4 > m_src.size()) return false;
                    out.append("\\u");
                    out.append(m_src, m_pos, 4);
                    m_pos += 4;
                    break;
                }
                default: return false;
                }
            } else {
                out.push_back(c);
            }
        }
        return false;
    }

    bool parseBool(JsonValue& out) {
        if (m_src.compare(m_pos, 4, "true") == 0) {
            m_pos += 4;
            out = JsonValue::MakeBool(true);
            return true;
        }
        if (m_src.compare(m_pos, 5, "false") == 0) {
            m_pos += 5;
            out = JsonValue::MakeBool(false);
            return true;
        }
        return false;
    }

    bool parseNull(JsonValue& out) {
        if (m_src.compare(m_pos, 4, "null") == 0) {
            m_pos += 4;
            out = JsonValue{};
            return true;
        }
        return false;
    }

    bool parseNumber(JsonValue& out) {
        size_t start = m_pos;
        if (peek() == '-') ++m_pos;
        bool isDouble = false;
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos];
            if (c >= '0' && c <= '9') { ++m_pos; continue; }
            if (c == '.') { isDouble = true; ++m_pos; continue; }
            // 不支持科学计数法，遇到 e/E 视为非法
            break;
        }
        std::string numStr = m_src.substr(start, m_pos - start);
        try {
            if (isDouble) {
                out = JsonValue::MakeDouble(std::stod(numStr));
            } else {
                out = JsonValue::MakeInt(std::stoll(numStr));
            }
        } catch (...) {
            return false;
        }
        return true;
    }

    char peek() { return m_pos < m_src.size() ? m_src[m_pos] : '\0'; }
};

inline bool JsonParse(const std::string& src, JsonValue& out) {
    JsonParser p(src);
    return p.parse(out);
}

// ============ 序列化器 ============

inline void dumpString(const std::string& s, std::string& out) {
    out.push_back('"');
    for (char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b";  break;
        case '\f': out += "\\f";  break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                // 控制字符转 \u00XX
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            } else {
                out.push_back(c);
            }
        }
    }
    out.push_back('"');
}

inline void dumpValue(const JsonValue& v, std::string& out);

inline void dumpObject(const JsonObject& obj, std::string& out) {
    out.push_back('{');
    bool first = true;
    for (auto& [k, val] : obj) {
        if (!first) out.push_back(',');
        first = false;
        dumpString(k, out);
        out.push_back(':');
        dumpValue(val, out);
    }
    out.push_back('}');
}

inline void dumpArray(const JsonArray& arr, std::string& out) {
    out.push_back('[');
    bool first = true;
    for (auto& val : arr) {
        if (!first) out.push_back(',');
        first = false;
        dumpValue(val, out);
    }
    out.push_back(']');
}

inline void dumpValue(const JsonValue& v, std::string& out) {
    switch (v.type()) {
    case JsonValue::Type::Null:   out += "null"; break;
    case JsonValue::Type::Bool:   out += v.asBool() ? "true" : "false"; break;
    case JsonValue::Type::Int: {
        out += std::to_string(v.asInt());
        break;
    }
    case JsonValue::Type::Double: {
        std::ostringstream oss;
        oss << v.asDouble();
        out += oss.str();
        break;
    }
    case JsonValue::Type::String: dumpString(v.asString(), out); break;
    case JsonValue::Type::Array:  dumpArray(v.asArray(), out); break;
    case JsonValue::Type::Object: dumpObject(v.asObject(), out); break;
    }
}

inline std::string JsonDump(const JsonValue& v) {
    std::string out;
    dumpValue(v, out);
    return out;
}

} // namespace ModJson
