// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/ipc/pipe_json.h"

#include <climits>
#include <cstdint>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace content {

bool parse_pipe_json(const std::string& text, rapidjson::Document* out) {
  if (!out)
    return false;
  if (text.empty()) {
    out->SetObject();
    return true;
  }

  rapidjson::StringStream stream(text.c_str());
  out->ParseStream<rapidjson::kParseStopWhenDoneFlag>(stream);
  if (out->HasParseError()) {
    out->SetNull();
    return false;
  }
  while (stream.Peek() != '\0') {
    const char c = stream.Peek();
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      out->SetNull();
      return false;
    }
    stream.Take();
  }
  return true;
}

const rapidjson::Value* pipe_json_member(const rapidjson::Value& obj,
                                         const char* key) {
  if (!key || !obj.IsObject())
    return NULL;
  rapidjson::Value::ConstMemberIterator it = obj.FindMember(key);
  if (it == obj.MemberEnd())
    return NULL;
  return &it->value;
}

bool pipe_json_string(const rapidjson::Value& obj,
                      const char* key,
                      std::string* out) {
  if (!out)
    return false;
  const rapidjson::Value* value = pipe_json_member(obj, key);
  if (!value || !value->IsString())
    return false;
  *out = std::string(value->GetString(), value->GetStringLength());
  return true;
}

bool pipe_json_double(const rapidjson::Value& obj,
                      const char* key,
                      double* out) {
  if (!out)
    return false;
  const rapidjson::Value* value = pipe_json_member(obj, key);
  if (!value || !value->IsNumber())
    return false;
  *out = value->GetDouble();
  return true;
}

bool pipe_json_bool(const rapidjson::Value& obj, const char* key, bool* out) {
  if (!out)
    return false;
  const rapidjson::Value* value = pipe_json_member(obj, key);
  if (!value || !value->IsBool())
    return false;
  *out = value->GetBool();
  return true;
}

bool pipe_json_int64(const rapidjson::Value& obj,
                     const char* key,
                     long long* out) {
  if (!out)
    return false;
  const rapidjson::Value* value = pipe_json_member(obj, key);
  if (!value)
    return false;
  if (value->IsInt64()) {
    *out = value->GetInt64();
    return true;
  }
  if (value->IsUint64()) {
    const uint64_t u = value->GetUint64();
    if (u > static_cast<uint64_t>(LLONG_MAX))
      return false;
    *out = static_cast<long long>(u);
    return true;
  }
  if (value->IsInt()) {
    *out = value->GetInt();
    return true;
  }
  if (value->IsUint()) {
    *out = value->GetUint();
    return true;
  }
  return false;
}

bool pipe_json_exact_keys(const rapidjson::Value& obj,
                          const char* const* keys,
                          size_t count) {
  if (!obj.IsObject() || obj.MemberCount() != count || !keys)
    return false;
  for (size_t i = 0; i < count; ++i) {
    if (!obj.HasMember(keys[i]))
      return false;
  }
  return true;
}

void pipe_json_copy(rapidjson::Document* dest, const rapidjson::Value& src) {
  if (!dest)
    return;
  dest->SetObject();
  dest->CopyFrom(src, dest->GetAllocator());
}

std::string pipe_json_stringify(const rapidjson::Value& value) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  value.Accept(writer);
  return std::string(buffer.GetString(), buffer.GetSize());
}

}  // namespace content
