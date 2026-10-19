// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_PIPE_JSON_MINI_H_
#define CONTENT_BROWSER_PIPE_JSON_MINI_H_

#include <string>
#include <vector>

namespace content {

// Small JSON value for named-pipe envelopes. Objects keep insertion order.
// Integer tokens keep an exact integer so HWND and sizes round-trip.
class PipeJson {
 public:
  enum Type { kNull, kBool, kNumber, kString, kObject, kArray };

  PipeJson();

  static PipeJson Null();
  static PipeJson Bool(bool value);
  static PipeJson Number(double value);
  static PipeJson Integer(long long value);
  static PipeJson String(const std::string& value);
  static PipeJson Object();
  static PipeJson Array();

  Type type() const { return type_; }
  bool bool_value() const { return bool_; }
  double number_value() const { return number_; }
  long long int_value() const { return int_; }
  bool is_int() const { return is_int_; }
  const std::string& string_value() const { return string_; }
  size_t object_size() const { return object_.size(); }
  size_t array_size() const { return array_.size(); }

  void set(const std::string& key, const PipeJson& value);
  void push(const PipeJson& value);
  const PipeJson* find(const std::string& key) const;

  std::string stringify() const;

 private:
  Type type_;
  bool bool_;
  bool is_int_;
  double number_;
  long long int_;
  std::string string_;
  std::vector<std::pair<std::string, PipeJson> > object_;
  std::vector<PipeJson> array_;
};

// Parses one JSON value. Trailing whitespace is allowed. Returns false when
// the text is not a single value.
bool parse_pipe_json(const std::string& text, PipeJson* out);

}  // namespace content

#endif  // CONTENT_BROWSER_PIPE_JSON_MINI_H_
