// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "json_lite.h"

#include <cctype>
#include <cmath>

namespace sg {
namespace json {
namespace {

class Parser {
 public:
  explicit Parser(const std::string& t) : t_(t) {}

  bool ParseValue(Value* out) {
    Skip();
    if (pos_ >= t_.size()) {
      err_ = "empty";
      return false;
    }
    const char c = t_[pos_];
    if (c == '{') {
      return ParseObject(out);
    }
    if (c == '[') {
      return ParseArray(out);
    }
    if (c == '"') {
      return ParseString(out);
    }
    if (c == 't' || c == 'f') {
      return ParseBool(out);
    }
    if (c == 'n') {
      return ParseNull(out);
    }
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
      return ParseNumber(out);
    }
    err_ = "unexpected token";
    return false;
  }

  const std::string& error() const { return err_; }

 private:
  void Skip() {
    while (pos_ < t_.size() &&
           std::isspace(static_cast<unsigned char>(t_[pos_]))) {
      ++pos_;
    }
  }

  bool Expect(char c) {
    Skip();
    if (pos_ >= t_.size() || t_[pos_] != c) {
      err_ = std::string("expected ") + c;
      return false;
    }
    ++pos_;
    return true;
  }

  bool ParseObject(Value* out) {
    if (!Expect('{')) {
      return false;
    }
    out->type = Type::kObject;
    Skip();
    if (pos_ < t_.size() && t_[pos_] == '}') {
      ++pos_;
      return true;
    }
    for (;;) {
      Value key;
      if (!ParseValue(&key) || key.type != Type::kString) {
        err_ = "object key";
        return false;
      }
      if (!Expect(':')) {
        return false;
      }
      Value val;
      if (!ParseValue(&val)) {
        return false;
      }
      out->object.emplace(key.str, std::move(val));
      Skip();
      if (pos_ < t_.size() && t_[pos_] == ',') {
        ++pos_;
        continue;
      }
      return Expect('}');
    }
  }

  bool ParseArray(Value* out) {
    if (!Expect('[')) {
      return false;
    }
    out->type = Type::kArray;
    Skip();
    if (pos_ < t_.size() && t_[pos_] == ']') {
      ++pos_;
      return true;
    }
    for (;;) {
      Value item;
      if (!ParseValue(&item)) {
        return false;
      }
      out->array.push_back(std::move(item));
      Skip();
      if (pos_ < t_.size() && t_[pos_] == ',') {
        ++pos_;
        continue;
      }
      return Expect(']');
    }
  }

  bool ParseString(Value* out) {
    if (!Expect('"')) {
      return false;
    }
    std::string s;
    while (pos_ < t_.size()) {
      char c = t_[pos_++];
      if (c == '"') {
        out->type = Type::kString;
        out->str = std::move(s);
        return true;
      }
      if (c == '\\' && pos_ < t_.size()) {
        const char e = t_[pos_++];
        switch (e) {
          case '"':
          case '\\':
          case '/':
            s += e;
            break;
          case 'n':
            s += '\n';
            break;
          case 'r':
            s += '\r';
            break;
          case 't':
            s += '\t';
            break;
          default:
            s += e;
            break;
        }
        continue;
      }
      s += c;
    }
    err_ = "unterminated string";
    return false;
  }

  bool ParseNumber(Value* out) {
    Skip();
    const std::size_t start = pos_;
    if (pos_ < t_.size() && t_[pos_] == '-') {
      ++pos_;
    }
    while (pos_ < t_.size() &&
           (std::isdigit(static_cast<unsigned char>(t_[pos_])) ||
            t_[pos_] == '.' || t_[pos_] == 'e' || t_[pos_] == 'E' ||
            t_[pos_] == '+' || t_[pos_] == '-')) {
      ++pos_;
    }
    try {
      out->number = std::stod(t_.substr(start, pos_ - start));
    } catch (...) {
      err_ = "bad number";
      return false;
    }
    out->type = Type::kNumber;
    return true;
  }

  bool ParseBool(Value* out) {
    Skip();
    if (t_.compare(pos_, 4, "true") == 0) {
      pos_ += 4;
      out->type = Type::kBool;
      out->boolean = true;
      return true;
    }
    if (t_.compare(pos_, 5, "false") == 0) {
      pos_ += 5;
      out->type = Type::kBool;
      out->boolean = false;
      return true;
    }
    err_ = "bad bool";
    return false;
  }

  bool ParseNull(Value* out) {
    Skip();
    if (t_.compare(pos_, 4, "null") == 0) {
      pos_ += 4;
      out->type = Type::kNull;
      return true;
    }
    err_ = "bad null";
    return false;
  }

  const std::string& t_;
  std::size_t pos_ = 0;
  std::string err_;
};

bool WalkForbidden(const Value& v) {
  if (v.is_object()) {
    for (const auto& kv : v.object) {
      if (kv.first == "sql" || kv.first == "raw_sql" ||
          kv.first == "raw_query") {
        return true;
      }
      if (WalkForbidden(kv.second)) {
        return true;
      }
    }
  } else if (v.is_array()) {
    for (const auto& item : v.array) {
      if (WalkForbidden(item)) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace

const Value* Value::Get(const std::string& key) const {
  if (!is_object()) {
    return nullptr;
  }
  const auto it = object.find(key);
  if (it == object.end()) {
    return nullptr;
  }
  return &it->second;
}

std::string Value::String(const std::string& key, const std::string& def) const {
  const Value* v = Get(key);
  if (!v) {
    return def;
  }
  if (v->type == Type::kString) {
    return v->str;
  }
  if (v->type == Type::kNumber) {
    return v->AsText();
  }
  return def;
}

double Value::Number(const std::string& key, double def) const {
  const Value* v = Get(key);
  if (!v) {
    return def;
  }
  if (v->type == Type::kNumber) {
    return v->number;
  }
  if (v->type == Type::kString) {
    try {
      return std::stod(v->str);
    } catch (...) {
      return def;
    }
  }
  return def;
}

bool Value::Bool(const std::string& key, bool def) const {
  const Value* v = Get(key);
  if (!v || v->type != Type::kBool) {
    return def;
  }
  return v->boolean;
}

std::string Value::AsText() const {
  if (type == Type::kString) {
    return str;
  }
  if (type == Type::kNumber) {
    if (std::floor(number) == number && std::fabs(number) < 1e15) {
      return std::to_string(static_cast<long long>(number));
    }
    return std::to_string(number);
  }
  if (type == Type::kBool) {
    return boolean ? "true" : "false";
  }
  return {};
}

bool Parse(const std::string& text, Value* out, std::string* error) {
  if (!out) {
    return false;
  }
  *out = Value{};
  Parser p(text);
  if (!p.ParseValue(out)) {
    if (error) {
      *error = p.error();
    }
    return false;
  }
  return true;
}

bool HasForbiddenSqlKeys(const Value& v) {
  return WalkForbidden(v);
}

bool HasForbiddenSqlKeys(const std::string& json) {
  Value v;
  if (!Parse(json, &v, nullptr)) {
    return json.find("\"sql\"") != std::string::npos ||
           json.find("\"raw_sql\"") != std::string::npos ||
           json.find("\"raw_query\"") != std::string::npos;
  }
  return WalkForbidden(v);
}

}  // namespace json
}  // namespace sg
