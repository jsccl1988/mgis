// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "core/sdb/sdbd_json.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace core {
namespace sdb {
namespace {

class Parser {
 public:
  explicit Parser(const std::string& text) : text_(text) {}

  bool Parse(Json* out) {
    Skip();
    if (!ParseValue(out)) {
      return false;
    }
    Skip();
    return pos_ == text_.size();
  }

  const std::string& error() const { return error_; }

 private:
  const std::string& text_;
  std::size_t pos_ = 0;
  std::string error_;

  void Fail(const char* msg) {
    if (error_.empty()) {
      error_ = msg;
    }
  }

  void Skip() {
    while (pos_ < text_.size() &&
           std::isspace(static_cast<unsigned char>(text_[pos_]))) {
      ++pos_;
    }
  }

  bool Peek(char c) {
    Skip();
    return pos_ < text_.size() && text_[pos_] == c;
  }

  bool Eat(char c) {
    Skip();
    if (pos_ < text_.size() && text_[pos_] == c) {
      ++pos_;
      return true;
    }
    return false;
  }

  bool ParseValue(Json* out) {
    Skip();
    if (pos_ >= text_.size()) {
      Fail("unexpected end");
      return false;
    }
    const char c = text_[pos_];
    if (c == '{') {
      return ParseObject(out);
    }
    if (c == '[') {
      return ParseArray(out);
    }
    if (c == '"') {
      return ParseString(&out->s) && ((out->type = Json::kString), true);
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
    Fail("invalid value");
    return false;
  }

  bool ParseObject(Json* out) {
    if (!Eat('{')) {
      Fail("expected '{'");
      return false;
    }
    out->type = Json::kObject;
    Skip();
    if (Eat('}')) {
      return true;
    }
    for (;;) {
      std::string key;
      if (!ParseString(&key)) {
        return false;
      }
      if (!Eat(':')) {
        Fail("expected ':'");
        return false;
      }
      Json value;
      if (!ParseValue(&value)) {
        return false;
      }
      out->o.emplace(std::move(key), std::move(value));
      Skip();
      if (Eat('}')) {
        return true;
      }
      if (!Eat(',')) {
        Fail("expected ',' or '}'");
        return false;
      }
    }
  }

  bool ParseArray(Json* out) {
    if (!Eat('[')) {
      Fail("expected '['");
      return false;
    }
    out->type = Json::kArray;
    Skip();
    if (Eat(']')) {
      return true;
    }
    for (;;) {
      Json value;
      if (!ParseValue(&value)) {
        return false;
      }
      out->a.push_back(std::move(value));
      Skip();
      if (Eat(']')) {
        return true;
      }
      if (!Eat(',')) {
        Fail("expected ',' or ']'");
        return false;
      }
    }
  }

  bool ParseString(std::string* out) {
    Skip();
    if (!Eat('"')) {
      Fail("expected string");
      return false;
    }
    out->clear();
    while (pos_ < text_.size()) {
      char c = text_[pos_++];
      if (c == '"') {
        return true;
      }
      if (c == '\\') {
        if (pos_ >= text_.size()) {
          Fail("bad escape");
          return false;
        }
        const char e = text_[pos_++];
        switch (e) {
          case '"':
          case '\\':
          case '/':
            out->push_back(e);
            break;
          case 'b':
            out->push_back('\b');
            break;
          case 'f':
            out->push_back('\f');
            break;
          case 'n':
            out->push_back('\n');
            break;
          case 'r':
            out->push_back('\r');
            break;
          case 't':
            out->push_back('\t');
            break;
          case 'u':
            if (pos_ + 4 > text_.size()) {
              Fail("bad unicode escape");
              return false;
            }
            pos_ += 4;
            out->push_back('?');
            break;
          default:
            out->push_back(e);
            break;
        }
        continue;
      }
      out->push_back(c);
    }
    Fail("unterminated string");
    return false;
  }

  bool ParseNumber(Json* out) {
    Skip();
    const std::size_t start = pos_;
    if (pos_ < text_.size() && text_[pos_] == '-') {
      ++pos_;
    }
    while (pos_ < text_.size() &&
           std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
      ++pos_;
    }
    if (pos_ < text_.size() && text_[pos_] == '.') {
      ++pos_;
      while (pos_ < text_.size() &&
             std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
        ++pos_;
      }
    }
    if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
      ++pos_;
      if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) {
        ++pos_;
      }
      while (pos_ < text_.size() &&
             std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
        ++pos_;
      }
    }
    out->type = Json::kNumber;
    out->s = text_.substr(start, pos_ - start);
    out->n = std::strtod(out->s.c_str(), nullptr);
    return true;
  }

  bool ParseBool(Json* out) {
    Skip();
    if (text_.compare(pos_, 4, "true") == 0) {
      pos_ += 4;
      out->type = Json::kBool;
      out->b = true;
      return true;
    }
    if (text_.compare(pos_, 5, "false") == 0) {
      pos_ += 5;
      out->type = Json::kBool;
      out->b = false;
      return true;
    }
    Fail("expected bool");
    return false;
  }

  bool ParseNull(Json* out) {
    Skip();
    if (text_.compare(pos_, 4, "null") == 0) {
      pos_ += 4;
      out->type = Json::kNull;
      return true;
    }
    Fail("expected null");
    return false;
  }
};

bool LooksLikeJsonKey(const std::string& body, const char* key) {
  const std::string needle = std::string("\"") + key + "\"";
  std::size_t pos = 0;
  while ((pos = body.find(needle, pos)) != std::string::npos) {
    std::size_t colon = pos + needle.size();
    while (colon < body.size() &&
           std::isspace(static_cast<unsigned char>(body[colon]))) {
      ++colon;
    }
    if (colon < body.size() && body[colon] == ':') {
      return true;
    }
    pos += needle.size();
  }
  return false;
}

std::string IdFromJson(const Json& value) {
  if (value.type == Json::kString) {
    return value.s;
  }
  if (value.type == Json::kNumber) {
    if (std::fabs(value.n - std::llround(value.n)) < 1e-9) {
      return std::to_string(static_cast<long long>(std::llround(value.n)));
    }
    return value.s.empty() ? std::to_string(value.n) : value.s;
  }
  return {};
}

BBox ParseBBox(const Json& json) {
  BBox box;
  box.min_x = json.Number("min_x");
  box.min_y = json.Number("min_y");
  box.max_x = json.Number("max_x");
  box.max_y = json.Number("max_y");
  return box;
}

}  // namespace

bool Json::Has(const char* key) const {
  return o.find(key) != o.end();
}

const Json* Json::Get(const char* key) const {
  auto it = o.find(key);
  if (it == o.end()) {
    return nullptr;
  }
  return &it->second;
}

std::string Json::String(const char* key, const std::string& def) const {
  const Json* v = Get(key);
  if (!v) {
    return def;
  }
  if (v->type == Json::kString) {
    return v->s;
  }
  if (v->type == Json::kNumber) {
    return IdFromJson(*v);
  }
  return def;
}

double Json::Number(const char* key, double def) const {
  const Json* v = Get(key);
  if (!v || v->type != Json::kNumber) {
    return def;
  }
  return v->n;
}

bool Json::Bool(const char* key, bool def) const {
  const Json* v = Get(key);
  if (!v) {
    return def;
  }
  if (v->type == Json::kBool) {
    return v->b;
  }
  return def;
}

std::string Json::AsString() const {
  if (type == Json::kString) {
    return s;
  }
  if (type == Json::kNumber) {
    return IdFromJson(*this);
  }
  if (type == Json::kBool) {
    return b ? "true" : "false";
  }
  return {};
}

bool ParseJson(const std::string& text, Json* out, std::string* err) {
  if (!out) {
    return false;
  }
  *out = Json{};
  Parser parser(text);
  if (!parser.Parse(out)) {
    if (err) {
      *err = parser.error().empty() ? "bad_request" : parser.error();
    }
    return false;
  }
  return true;
}

std::string JsonEscape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (unsigned char c : s) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
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
        out.push_back(static_cast<char>(c));
        break;
    }
  }
  return out;
}

std::string OpenRequestToJson(const OpenRequest& req) {
  std::ostringstream o;
  o << "{\"layer\":\"" << JsonEscape(req.layer) << "\",\"columns\":[";
  for (std::size_t i = 0; i < req.columns.size(); ++i) {
    if (i) {
      o << ',';
    }
    o << '"' << JsonEscape(req.columns[i]) << '"';
  }
  o << "],\"bbox\":{\"min_x\":" << req.bbox.min_x << ",\"min_y\":"
    << req.bbox.min_y << ",\"max_x\":" << req.bbox.max_x << ",\"max_y\":"
    << req.bbox.max_y << "},\"predicate\":\"" << JsonEscape(req.predicate)
    << '"';
  if (!req.crs.empty()) {
    o << ",\"crs\":\"" << JsonEscape(req.crs) << '"';
  }
  o << ",\"limit\":" << req.limit << '}';
  return o.str();
}

bool ParseLayerInfo(const Json& json, LayerInfo* out) {
  if (!out || json.type != Json::kObject) {
    return false;
  }
  out->name = json.String("name");
  out->crs = json.String("crs");
  out->geom_type = json.String("geom_type");
  out->geom_column = json.String("geom_column");
  if (const Json* extent = json.Get("extent")) {
    out->extent = ParseBBox(*extent);
  }
  out->fields.clear();
  if (const Json* fields = json.Get("fields")) {
    if (fields->type == Json::kArray) {
      for (const auto& f : fields->a) {
        FieldInfo info;
        info.name = f.String("name");
        info.type = f.String("type");
        if (!info.name.empty()) {
          out->fields.push_back(std::move(info));
        }
      }
    }
  }
  return !out->name.empty();
}

bool ParseLayerInfo(const std::string& text, LayerInfo* out, std::string* err) {
  Json json;
  if (!ParseJson(text, &json, err)) {
    return false;
  }
  return ParseLayerInfo(json, out);
}

bool ParseCatalog(const std::string& text, std::vector<LayerInfo>* out,
                  std::string* err) {
  if (!out) {
    return false;
  }
  Json json;
  if (!ParseJson(text, &json, err)) {
    return false;
  }
  if (json.type != Json::kArray) {
    if (err) {
      *err = "catalog must be a JSON array";
    }
    return false;
  }
  out->clear();
  for (const auto& item : json.a) {
    LayerInfo info;
    if (!ParseLayerInfo(item, &info)) {
      if (err) {
        *err = "invalid LayerInfo";
      }
      return false;
    }
    out->push_back(std::move(info));
  }
  return true;
}

bool ParseFeatureSet(const std::string& text, FeatureSet* out,
                     std::string* err) {
  if (!out) {
    return false;
  }
  Json json;
  if (!ParseJson(text, &json, err)) {
    return false;
  }
  if (json.type != Json::kObject) {
    if (err) {
      *err = "FeatureSet must be an object";
    }
    return false;
  }
  out->layer = json.String("layer");
  out->crs = json.String("crs");
  out->count = static_cast<int>(json.Number("count"));
  out->offset = static_cast<int>(json.Number("offset"));
  out->features.clear();
  const Json* features = json.Get("features");
  if (features && features->type == Json::kArray) {
    for (const auto& item : features->a) {
      Feature feature;
      if (const Json* id = item.Get("id")) {
        feature.id = IdFromJson(*id);
      }
      feature.geom_wkt = item.String("geom_wkt");
      if (const Json* attrs = item.Get("attrs")) {
        if (attrs->type == Json::kObject) {
          for (const auto& kv : attrs->o) {
            feature.attrs[kv.first] = kv.second.AsString();
          }
        }
      }
      out->features.push_back(std::move(feature));
    }
  }
  if (out->count == 0) {
    out->count = static_cast<int>(out->features.size());
  }
  out->MoveFirst();
  return true;
}

bool ParseErrorBody(const std::string& text, SdbdResult* out) {
  if (!out) {
    return false;
  }
  Json json;
  if (!ParseJson(text, &json, nullptr) || json.type != Json::kObject) {
    return false;
  }
  out->ok = json.Bool("ok", false);
  out->error = json.String("error");
  out->message = json.String("message");
  return !out->error.empty() || json.Has("ok");
}

bool BodyHasForbiddenSql(const std::string& body) {
  Json json;
  if (ParseJson(body, &json, nullptr) && json.type == Json::kObject) {
    return json.Has("sql") || json.Has("raw_sql") || json.Has("raw_query");
  }
  return LooksLikeJsonKey(body, "sql") || LooksLikeJsonKey(body, "raw_sql") ||
         LooksLikeJsonKey(body, "raw_query");
}

void FeatureSet::MoveFirst() { index_ = 0; }

bool FeatureSet::MoveNext() {
  if (index_ < features.size()) {
    ++index_;
  }
  return !IsEnd();
}

bool FeatureSet::IsEnd() const { return index_ >= features.size(); }

const Feature* FeatureSet::GetFeature() const {
  if (IsEnd()) {
    return nullptr;
  }
  return &features[index_];
}

}  // namespace sdb
}  // namespace core
