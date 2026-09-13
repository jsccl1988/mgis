// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// Minimal JSON for 04/08 contract bodies. Not a public pin surface.

#pragma once

#include <map>
#include <string>
#include <vector>

namespace sg {
namespace json {

enum class Type { kNull, kBool, kNumber, kString, kArray, kObject };

struct Value {
  Type type = Type::kNull;
  bool boolean = false;
  double number = 0;
  std::string str;
  std::vector<Value> array;
  std::map<std::string, Value> object;

  bool is_object() const { return type == Type::kObject; }
  bool is_array() const { return type == Type::kArray; }

  const Value* Get(const std::string& key) const;
  std::string String(const std::string& key, const std::string& def = {}) const;
  double Number(const std::string& key, double def = 0) const;
  bool Bool(const std::string& key, bool def = false) const;
  std::string AsText() const;
};

bool Parse(const std::string& text, Value* out, std::string* error = nullptr);

bool HasForbiddenSqlKeys(const Value& v);
bool HasForbiddenSqlKeys(const std::string& json);

}  // namespace json
}  // namespace sg
