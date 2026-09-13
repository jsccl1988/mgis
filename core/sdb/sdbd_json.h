// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#ifndef SDB_SDBD_JSON_H
#define SDB_SDBD_JSON_H

#include "core/sdb/sdbd_types.h"

#include <map>
#include <string>
#include <vector>

namespace core {
namespace sdb {

struct Json {
  enum Type { kNull, kBool, kNumber, kString, kArray, kObject };
  Type type = kNull;
  bool b = false;
  double n = 0;
  std::string s;
  std::vector<Json> a;
  std::map<std::string, Json> o;

  bool Has(const char* key) const;
  const Json* Get(const char* key) const;
  std::string String(const char* key, const std::string& def = {}) const;
  double Number(const char* key, double def = 0) const;
  bool Bool(const char* key, bool def = false) const;
  std::string AsString() const;
};

bool ParseJson(const std::string& text, Json* out, std::string* err);
std::string JsonEscape(const std::string& s);

std::string OpenRequestToJson(const OpenRequest& req);
bool ParseLayerInfo(const Json& json, LayerInfo* out);
bool ParseLayerInfo(const std::string& text, LayerInfo* out, std::string* err);
bool ParseCatalog(const std::string& text, std::vector<LayerInfo>* out,
                  std::string* err);
bool ParseFeatureSet(const std::string& text, FeatureSet* out, std::string* err);
bool ParseErrorBody(const std::string& text, SdbdResult* out);
bool BodyHasForbiddenSql(const std::string& body);

}  // namespace sdb
}  // namespace core

#endif  // SDB_SDBD_JSON_H
