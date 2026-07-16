#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <cctype>
#include <stdexcept>

namespace KosNitroJson {

inline std::string escape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 2);
  for (unsigned char c : s) {
    switch (c) {
      case '"':  out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (c < 0x20) {
          char buf[7];
          snprintf(buf, sizeof(buf), "\\u00%02x", c);
          out += buf;
        } else {
          out += static_cast<char>(c);
        }
    }
  }
  return out;
}

inline std::string string(const std::string& s) {
  return "\"" + escape(s) + "\"";
}

inline std::string number(int64_t n) { return std::to_string(n); }
inline std::string number(uint64_t n) { return std::to_string(n); }
inline std::string number(double n) {
  double intpart;
  if (std::modf(n, &intpart) == 0.0 && n >= 0.0 && n <= static_cast<double>(INT64_MAX)) {
    return std::to_string(static_cast<int64_t>(n));
  }
  return std::to_string(n);
}
inline std::string boolean(bool b) { return b ? "true" : "false"; }
inline std::string null() { return "null"; }

inline std::string stringArray(const std::vector<std::string>& items) {
  std::string result = "[";
  for (size_t i = 0; i < items.size(); ++i) {
    if (i > 0) result += ",";
    result += string(items[i]);
  }
  result += "]";
  return result;
}

inline std::string numberArray(const std::vector<uint64_t>& items) {
  std::string result = "[";
  for (size_t i = 0; i < items.size(); ++i) {
    if (i > 0) result += ",";
    result += number(items[i]);
  }
  result += "]";
  return result;
}

inline std::string object(const std::vector<std::pair<std::string, std::string>>& pairs) {
  std::string result = "{";
  for (size_t i = 0; i < pairs.size(); ++i) {
    if (i > 0) result += ",";
    result += "\"" + escape(pairs[i].first) + "\":" + pairs[i].second;
  }
  result += "}";
  return result;
}

inline std::string object(std::initializer_list<std::pair<std::string, std::string>> pairs) {
  return object(std::vector<std::pair<std::string, std::string>>(pairs));
}

// ---------------------------------------------------------------------------
// Parsing helpers
// ---------------------------------------------------------------------------

inline void skipWhitespace(std::string_view json, size_t& pos) {
  while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
    ++pos;
  }
}

inline size_t endOfString(std::string_view json, size_t pos) {
  // pos points at the opening quote
  if (pos >= json.size() || json[pos] != '"') return std::string_view::npos;
  ++pos;
  while (pos < json.size()) {
    char c = json[pos];
    if (c == '\\' && pos + 1 < json.size()) {
      pos += 2;
    } else if (c == '"') {
      return pos + 1;
    } else {
      ++pos;
    }
  }
  return std::string_view::npos;
}

inline std::string unescapeString(std::string_view raw) {
  // raw is the content between quotes, without quotes
  std::string out;
  out.reserve(raw.size());
  for (size_t i = 0; i < raw.size(); ++i) {
    if (raw[i] == '\\' && i + 1 < raw.size()) {
      char next = raw[i + 1];
      switch (next) {
        case '"': out += '"'; ++i; break;
        case '\\': out += '\\'; ++i; break;
        case '/': out += '/'; ++i; break;
        case 'b': out += '\b'; ++i; break;
        case 'f': out += '\f'; ++i; break;
        case 'n': out += '\n'; ++i; break;
        case 'r': out += '\r'; ++i; break;
        case 't': out += '\t'; ++i; break;
        case 'u': {
          if (i + 5 < raw.size()) {
            std::string hex(raw.substr(i + 2, 4));
            unsigned int codepoint = std::stoul(hex, nullptr, 16);
            out += static_cast<char>(codepoint);
            i += 5;
          }
          break;
        }
        default: out += raw[i]; break;
      }
    } else {
      out += raw[i];
    }
  }
  return out;
}

inline size_t endOfValue(std::string_view json, size_t pos) {
  skipWhitespace(json, pos);
  if (pos >= json.size()) return std::string_view::npos;
  char c = json[pos];
  if (c == '{') {
    int depth = 0;
    for (size_t i = pos; i < json.size(); ++i) {
      if (json[i] == '"') {
        i = endOfString(json, i);
        if (i == std::string_view::npos) return std::string_view::npos;
        --i; // compensate for loop ++i
      } else if (json[i] == '{') {
        ++depth;
      } else if (json[i] == '}') {
        --depth;
        if (depth == 0) return i + 1;
      }
    }
    return std::string_view::npos;
  } else if (c == '[') {
    int depth = 0;
    for (size_t i = pos; i < json.size(); ++i) {
      if (json[i] == '"') {
        i = endOfString(json, i);
        if (i == std::string_view::npos) return std::string_view::npos;
        --i;
      } else if (json[i] == '[') {
        ++depth;
      } else if (json[i] == ']') {
        --depth;
        if (depth == 0) return i + 1;
      }
    }
    return std::string_view::npos;
  } else if (c == '"') {
    return endOfString(json, pos);
  } else {
    size_t start = pos;
    while (pos < json.size()) {
      char ch = json[pos];
      if (ch == ',' || ch == '}' || ch == ']' || std::isspace(static_cast<unsigned char>(ch))) {
        break;
      }
      ++pos;
    }
    return pos;
  }
}

inline size_t findValue(std::string_view json, std::string_view key) {
  size_t i = 0;
  skipWhitespace(json, i);
  if (i >= json.size() || json[i] != '{') return std::string_view::npos;
  ++i;
  while (true) {
    skipWhitespace(json, i);
    if (i >= json.size()) return std::string_view::npos;
    if (json[i] == '}') return std::string_view::npos;
    if (json[i] != '"') return std::string_view::npos;
    ++i;
    size_t keyStart = i;
    while (i < json.size() && json[i] != '"') {
      if (json[i] == '\\' && i + 1 < json.size()) i += 2;
      else ++i;
    }
    std::string_view foundKey(json.data() + keyStart, i - keyStart);
    if (i < json.size() && json[i] == '"') ++i;
    skipWhitespace(json, i);
    if (i >= json.size() || json[i] != ':') return std::string_view::npos;
    ++i;
    skipWhitespace(json, i);
    if (foundKey == key) return i;
    size_t end = endOfValue(json, i);
    if (end == std::string_view::npos) return std::string_view::npos;
    i = end;
    skipWhitespace(json, i);
    if (i < json.size() && json[i] == ',') { ++i; continue; }
    if (i < json.size() && json[i] == '}') return std::string_view::npos;
  }
}

inline bool hasKey(std::string_view json, std::string_view key) {
  return findValue(json, key) != std::string_view::npos;
}

inline std::string extractString(std::string_view value) {
  size_t i = 0;
  skipWhitespace(value, i);
  if (i >= value.size() || value[i] != '"') return "";
  ++i;
  size_t start = i;
  while (i < value.size() && value[i] != '"') {
    if (value[i] == '\\' && i + 1 < value.size()) i += 2;
    else ++i;
  }
  return unescapeString(value.substr(start, i - start));
}

inline std::string_view extractValue(std::string_view json, std::string_view key) {
  size_t pos = findValue(json, key);
  if (pos == std::string_view::npos) return {};
  size_t end = endOfValue(json, pos);
  if (end == std::string_view::npos) return {};
  return json.substr(pos, end - pos);
}

inline std::string getString(std::string_view json, std::string_view key) {
  return extractString(extractValue(json, key));
}

inline std::string getString(std::string_view json, std::string_view key, const std::string& defaultValue) {
  if (!hasKey(json, key)) return defaultValue;
  return getString(json, key);
}

inline std::string_view extractObject(std::string_view value) {
  size_t i = 0;
  skipWhitespace(value, i);
  if (i >= value.size() || value[i] != '{') return {};
  size_t end = endOfValue(value, i);
  if (end == std::string_view::npos) return {};
  return value.substr(i, end - i);
}

inline std::string_view getObject(std::string_view json, std::string_view key) {
  return extractObject(extractValue(json, key));
}

inline std::string_view extractArray(std::string_view value) {
  size_t i = 0;
  skipWhitespace(value, i);
  if (i >= value.size() || value[i] != '[') return {};
  size_t end = endOfValue(value, i);
  if (end == std::string_view::npos) return {};
  return value.substr(i, end - i);
}

inline std::string_view getArray(std::string_view json, std::string_view key) {
  return extractArray(extractValue(json, key));
}

inline std::string extractRawPrimitive(std::string_view value) {
  size_t i = 0;
  skipWhitespace(value, i);
  size_t end = endOfValue(value, i);
  return std::string(value.substr(i, end - i));
}

inline double getNumber(std::string_view json, std::string_view key) {
  std::string raw = extractRawPrimitive(extractValue(json, key));
  return std::stod(raw);
}

inline int64_t getInt64(std::string_view json, std::string_view key) {
  std::string raw = extractRawPrimitive(extractValue(json, key));
  return std::stoll(raw);
}

inline uint64_t getUInt64(std::string_view json, std::string_view key) {
  std::string raw = extractRawPrimitive(extractValue(json, key));
  return std::stoull(raw);
}

inline bool getBool(std::string_view json, std::string_view key) {
  std::string raw = extractRawPrimitive(extractValue(json, key));
  return raw == "true";
}

inline std::vector<double> parseNumberArray(std::string_view array) {
  std::vector<double> result;
  size_t i = 0;
  skipWhitespace(array, i);
  if (i >= array.size() || array[i] != '[') return result;
  ++i;
  while (true) {
    skipWhitespace(array, i);
    if (i < array.size() && array[i] == ']') break;
    size_t start = i;
    while (i < array.size() && array[i] != ',' && array[i] != ']') ++i;
    std::string raw(array.substr(start, i - start));
    if (!raw.empty()) {
      result.push_back(static_cast<double>(std::stoull(raw)));
    }
    skipWhitespace(array, i);
    if (i < array.size() && array[i] == ',') { ++i; continue; }
    if (i < array.size() && array[i] == ']') break;
  }
  return result;
}

inline std::vector<std::string> parseStringArray(std::string_view array) {
  std::vector<std::string> result;
  size_t i = 0;
  skipWhitespace(array, i);
  if (i >= array.size() || array[i] != '[') return result;
  ++i;
  while (true) {
    skipWhitespace(array, i);
    if (i < array.size() && array[i] == ']') break;
    if (array[i] != '"') break;
    ++i;
    size_t start = i;
    while (i < array.size() && array[i] != '"') {
      if (array[i] == '\\' && i + 1 < array.size()) i += 2;
      else ++i;
    }
    result.push_back(unescapeString(array.substr(start, i - start)));
    if (i < array.size() && array[i] == '"') ++i;
    skipWhitespace(array, i);
    if (i < array.size() && array[i] == ',') { ++i; continue; }
    if (i < array.size() && array[i] == ']') break;
  }
  return result;
}

} // namespace KosNitroJson
