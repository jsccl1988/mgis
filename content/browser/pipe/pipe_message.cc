// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/pipe/pipe_message.h"

namespace content {
namespace {

void SetCode(std::string* error_code, const char* code) {
  if (error_code)
    *error_code = code;
}

bool AsString(const PipeJson* value, std::string* out) {
  if (!value || value->type() != PipeJson::kString)
    return false;
  *out = value->string_value();
  return true;
}

bool ExactRoot(const PipeJson& root) {
  static const char* kKeys[] = {"v", "id", "op", "body"};
  if (root.type() != PipeJson::kObject || root.object_size() != 4)
    return false;
  for (size_t i = 0; i < 4; ++i) {
    if (!root.find(kKeys[i]))
      return false;
  }
  return true;
}

}  // namespace

const wchar_t* pipe_name_wide() { return L"\\\\.\\pipe\\mview"; }

PipeMessage::PipeMessage() : v(1), body(PipeJson::Object()) {}

PipeJson pipe_error_body(const std::string& code, const std::string& message) {
  PipeJson error = PipeJson::Object();
  error.set("code", PipeJson::String(code));
  error.set("message", PipeJson::String(message));
  PipeJson body = PipeJson::Object();
  body.set("ok", PipeJson::Bool(false));
  body.set("error", error);
  return body;
}

bool encode_pipe_frame(const std::string& json,
                       std::string* frame,
                       std::string* error_code) {
  if (!frame) {
    SetCode(error_code, "io");
    return false;
  }
  if (json.size() + 4 > kMaxPipeFrameBytes) {
    SetCode(error_code, "too_large");
    return false;
  }
  unsigned long length = static_cast<unsigned long>(json.size());
  frame->clear();
  frame->push_back(static_cast<char>(length & 0xff));
  frame->push_back(static_cast<char>((length >> 8) & 0xff));
  frame->push_back(static_cast<char>((length >> 16) & 0xff));
  frame->push_back(static_cast<char>((length >> 24) & 0xff));
  frame->append(json);
  SetCode(error_code, "");
  return true;
}

bool decode_pipe_frame(const char* data,
                       size_t size,
                       std::string* json,
                       std::string* error_code) {
  if (!json) {
    SetCode(error_code, "io");
    return false;
  }
  if (size > kMaxPipeFrameBytes) {
    SetCode(error_code, "too_large");
    return false;
  }
  if (size < 4 || !data) {
    SetCode(error_code, "schema");
    return false;
  }
  const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
  unsigned long length = static_cast<unsigned long>(bytes[0]) |
                         (static_cast<unsigned long>(bytes[1]) << 8) |
                         (static_cast<unsigned long>(bytes[2]) << 16) |
                         (static_cast<unsigned long>(bytes[3]) << 24);
  if (static_cast<unsigned long long>(length) + 4 > kMaxPipeFrameBytes) {
    SetCode(error_code, "too_large");
    return false;
  }
  if (length + 4 != size) {
    SetCode(error_code, "schema");
    return false;
  }
  json->assign(data + 4, length);
  SetCode(error_code, "");
  return true;
}

bool parse_pipe_message(const std::string& json,
                        PipeMessage* message,
                        std::string* error_code) {
  if (!message) {
    SetCode(error_code, "io");
    return false;
  }
  PipeJson root;
  if (!parse_pipe_json(json, &root) || !ExactRoot(root)) {
    SetCode(error_code, "schema");
    return false;
  }
  const PipeJson* version = root.find("v");
  const PipeJson* id = root.find("id");
  const PipeJson* op = root.find("op");
  const PipeJson* body = root.find("body");
  if (!version || version->type() != PipeJson::kNumber || !version->is_int() ||
      !body || body->type() != PipeJson::kObject) {
    SetCode(error_code, "schema");
    return false;
  }
  std::string id_text;
  std::string op_text;
  if (!AsString(id, &id_text) || id_text.empty() || !AsString(op, &op_text) ||
      op_text.empty()) {
    SetCode(error_code, "schema");
    return false;
  }
  message->v = static_cast<int>(version->int_value());
  message->id = id_text;
  message->op = op_text;
  message->body = *body;
  SetCode(error_code, "");
  return true;
}

std::string serialize_pipe_message(const PipeMessage& message) {
  std::string json = "{\"v\":";
  json += std::to_string(message.v);
  json += ",\"id\":";
  json += PipeJson::String(message.id).stringify();
  json += ",\"op\":";
  json += PipeJson::String(message.op).stringify();
  json += ",\"body\":";
  json += message.body.stringify();
  json.push_back('}');
  return json;
}

}  // namespace content
