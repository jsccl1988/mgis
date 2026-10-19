// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/pipe/json_mini.h"

#include <stdio.h>
#include <stdlib.h>

namespace content {
namespace {

std::string Quote(const std::string& text) {
  std::string out;
  out.push_back('"');
  for (size_t i = 0; i < text.size(); ++i) {
    unsigned char c = static_cast<unsigned char>(text[i]);
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (c < 0x20) {
          char buf[8];
          snprintf(buf, sizeof(buf), "\\u%04x", c);
          out += buf;
        } else {
          out.push_back(static_cast<char>(c));
        }
        break;
    }
  }
  out.push_back('"');
  return out;
}

void AppendUtf8(std::string* out, unsigned code) {
  if (code <= 0x7f) {
    out->push_back(static_cast<char>(code));
  } else if (code <= 0x7ff) {
    out->push_back(static_cast<char>(0xc0 | (code >> 6)));
    out->push_back(static_cast<char>(0x80 | (code & 0x3f)));
  } else {
    out->push_back(static_cast<char>(0xe0 | (code >> 12)));
    out->push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3f)));
    out->push_back(static_cast<char>(0x80 | (code & 0x3f)));
  }
}

class Parser {
 public:
  explicit Parser(const std::string& text)
      : p_(text.data()), end_(text.data() + text.size()), failed_(false) {}

  bool failed() const { return failed_; }

  bool consumed() {
    skip();
    return !failed_ && p_ == end_;
  }

  PipeJson parse_value() {
    skip();
    if (failed_ || p_ >= end_) {
      fail();
      return PipeJson::Null();
    }
    char c = *p_;
    if (c == '{')
      return parse_object();
    if (c == '[')
      return parse_array();
    if (c == '"')
      return parse_string();
    if (c == 't' || c == 'f')
      return parse_bool();
    if (c == 'n')
      return parse_null();
    if (c == '-' || (c >= '0' && c <= '9'))
      return parse_number();
    fail();
    return PipeJson::Null();
  }

 private:
  void fail() { failed_ = true; }

  void skip() {
    while (p_ < end_ &&
           (*p_ == ' ' || *p_ == '\t' || *p_ == '\n' || *p_ == '\r'))
      ++p_;
  }

  PipeJson parse_object() {
    ++p_;
    PipeJson object = PipeJson::Object();
    skip();
    if (p_ < end_ && *p_ == '}') {
      ++p_;
      return object;
    }
    while (!failed_ && p_ < end_) {
      skip();
      if (p_ >= end_ || *p_ != '"') {
        fail();
        return PipeJson::Null();
      }
      PipeJson key = parse_string();
      if (failed_)
        return PipeJson::Null();
      skip();
      if (p_ >= end_ || *p_ != ':') {
        fail();
        return PipeJson::Null();
      }
      ++p_;
      PipeJson value = parse_value();
      if (failed_)
        return PipeJson::Null();
      object.set(key.string_value(), value);
      skip();
      if (p_ < end_ && *p_ == ',') {
        ++p_;
        continue;
      }
      if (p_ < end_ && *p_ == '}') {
        ++p_;
        return object;
      }
      fail();
      return PipeJson::Null();
    }
    fail();
    return PipeJson::Null();
  }

  PipeJson parse_array() {
    ++p_;
    PipeJson array = PipeJson::Array();
    skip();
    if (p_ < end_ && *p_ == ']') {
      ++p_;
      return array;
    }
    while (!failed_ && p_ < end_) {
      array.push(parse_value());
      if (failed_)
        return PipeJson::Null();
      skip();
      if (p_ < end_ && *p_ == ',') {
        ++p_;
        continue;
      }
      if (p_ < end_ && *p_ == ']') {
        ++p_;
        return array;
      }
      fail();
      return PipeJson::Null();
    }
    fail();
    return PipeJson::Null();
  }

  PipeJson parse_string() {
    ++p_;
    std::string text;
    while (p_ < end_) {
      unsigned char c = static_cast<unsigned char>(*p_);
      ++p_;
      if (c == '"')
        return PipeJson::String(text);
      if (c == '\\') {
        if (p_ >= end_) {
          fail();
          return PipeJson::Null();
        }
        char esc = *p_;
        ++p_;
        switch (esc) {
          case '"':
          case '\\':
          case '/':
            text.push_back(esc);
            break;
          case 'b':
            text.push_back('\b');
            break;
          case 'f':
            text.push_back('\f');
            break;
          case 'n':
            text.push_back('\n');
            break;
          case 'r':
            text.push_back('\r');
            break;
          case 't':
            text.push_back('\t');
            break;
          case 'u': {
            if (p_ + 4 > end_) {
              fail();
              return PipeJson::Null();
            }
            unsigned code = 0;
            for (int i = 0; i < 4; ++i) {
              char h = *p_;
              ++p_;
              unsigned digit = 0;
              if (h >= '0' && h <= '9')
                digit = static_cast<unsigned>(h - '0');
              else if (h >= 'a' && h <= 'f')
                digit = static_cast<unsigned>(h - 'a' + 10);
              else if (h >= 'A' && h <= 'F')
                digit = static_cast<unsigned>(h - 'A' + 10);
              else {
                fail();
                return PipeJson::Null();
              }
              code = (code << 4) | digit;
            }
            AppendUtf8(&text, code);
            break;
          }
          default:
            fail();
            return PipeJson::Null();
        }
        continue;
      }
      if (c < 0x20) {
        fail();
        return PipeJson::Null();
      }
      text.push_back(static_cast<char>(c));
    }
    fail();
    return PipeJson::Null();
  }

  PipeJson parse_bool() {
    if (p_ + 4 <= end_ && p_[0] == 't' && p_[1] == 'r' && p_[2] == 'u' &&
        p_[3] == 'e') {
      p_ += 4;
      return PipeJson::Bool(true);
    }
    if (p_ + 5 <= end_ && p_[0] == 'f' && p_[1] == 'a' && p_[2] == 'l' &&
        p_[3] == 's' && p_[4] == 'e') {
      p_ += 5;
      return PipeJson::Bool(false);
    }
    fail();
    return PipeJson::Null();
  }

  PipeJson parse_null() {
    if (p_ + 4 <= end_ && p_[0] == 'n' && p_[1] == 'u' && p_[2] == 'l' &&
        p_[3] == 'l') {
      p_ += 4;
      return PipeJson::Null();
    }
    fail();
    return PipeJson::Null();
  }

  PipeJson parse_number() {
    const char* start = p_;
    if (*p_ == '-')
      ++p_;
    if (p_ >= end_ || *p_ < '0' || *p_ > '9') {
      fail();
      return PipeJson::Null();
    }
    if (*p_ == '0') {
      ++p_;
    } else {
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9')
        ++p_;
    }
    bool is_int = true;
    if (p_ < end_ && *p_ == '.') {
      is_int = false;
      ++p_;
      if (p_ >= end_ || *p_ < '0' || *p_ > '9') {
        fail();
        return PipeJson::Null();
      }
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9')
        ++p_;
    }
    if (p_ < end_ && (*p_ == 'e' || *p_ == 'E')) {
      is_int = false;
      ++p_;
      if (p_ < end_ && (*p_ == '+' || *p_ == '-'))
        ++p_;
      if (p_ >= end_ || *p_ < '0' || *p_ > '9') {
        fail();
        return PipeJson::Null();
      }
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9')
        ++p_;
    }
    std::string token(start, p_ - start);
    char* endp = NULL;
    if (is_int) {
      long long value = strtoll(token.c_str(), &endp, 10);
      if (endp != token.c_str() + token.size()) {
        fail();
        return PipeJson::Null();
      }
      return PipeJson::Integer(value);
    }
    double value = strtod(token.c_str(), &endp);
    if (endp != token.c_str() + token.size()) {
      fail();
      return PipeJson::Null();
    }
    return PipeJson::Number(value);
  }

  const char* p_;
  const char* end_;
  bool failed_;
};

}  // namespace

PipeJson::PipeJson()
    : type_(kNull), bool_(false), is_int_(false), number_(0.0), int_(0) {}

PipeJson PipeJson::Null() { return PipeJson(); }

PipeJson PipeJson::Bool(bool value) {
  PipeJson json;
  json.type_ = kBool;
  json.bool_ = value;
  return json;
}

PipeJson PipeJson::Number(double value) {
  PipeJson json;
  json.type_ = kNumber;
  json.number_ = value;
  return json;
}

PipeJson PipeJson::Integer(long long value) {
  PipeJson json;
  json.type_ = kNumber;
  json.is_int_ = true;
  json.int_ = value;
  json.number_ = static_cast<double>(value);
  return json;
}

PipeJson PipeJson::String(const std::string& value) {
  PipeJson json;
  json.type_ = kString;
  json.string_ = value;
  return json;
}

PipeJson PipeJson::Object() {
  PipeJson json;
  json.type_ = kObject;
  return json;
}

PipeJson PipeJson::Array() {
  PipeJson json;
  json.type_ = kArray;
  return json;
}

void PipeJson::set(const std::string& key, const PipeJson& value) {
  for (size_t i = 0; i < object_.size(); ++i) {
    if (object_[i].first == key) {
      object_[i].second = value;
      return;
    }
  }
  object_.push_back(std::make_pair(key, value));
}

void PipeJson::push(const PipeJson& value) { array_.push_back(value); }

const PipeJson* PipeJson::find(const std::string& key) const {
  if (type_ != kObject)
    return NULL;
  for (size_t i = 0; i < object_.size(); ++i) {
    if (object_[i].first == key)
      return &object_[i].second;
  }
  return NULL;
}

std::string PipeJson::stringify() const {
  switch (type_) {
    case kNull:
      return "null";
    case kBool:
      return bool_ ? "true" : "false";
    case kNumber: {
      if (is_int_)
        return std::to_string(int_);
      char buf[64];
      snprintf(buf, sizeof(buf), "%.17g", number_);
      return buf;
    }
    case kString:
      return Quote(string_);
    case kArray: {
      std::string out = "[";
      for (size_t i = 0; i < array_.size(); ++i) {
        if (i)
          out.push_back(',');
        out += array_[i].stringify();
      }
      out.push_back(']');
      return out;
    }
    case kObject: {
      std::string out = "{";
      for (size_t i = 0; i < object_.size(); ++i) {
        if (i)
          out.push_back(',');
        out += Quote(object_[i].first);
        out.push_back(':');
        out += object_[i].second.stringify();
      }
      out.push_back('}');
      return out;
    }
  }
  return "null";
}

bool parse_pipe_json(const std::string& text, PipeJson* out) {
  Parser parser(text);
  PipeJson value = parser.parse_value();
  if (!parser.consumed() || !out)
    return false;
  *out = value;
  return true;
}

}  // namespace content
