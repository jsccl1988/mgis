// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/ipc/pipe_message.h"

namespace content {
namespace {

void SetCode(std::string* error_code, const char* code) {
  if (error_code)
    *error_code = code;
}

bool ExactRoot(const rapidjson::Value& root) {
  static const char* kKeys[] = {"v", "id", "op", "body"};
  return pipe_json_exact_keys(root, kKeys, 4);
}

}  // namespace

const wchar_t* pipe_name_wide() {
  return L"\\\\.\\pipe\\mview";
}

PipeMessage::PipeMessage() : v(1) {
  body.SetObject();
}

rapidjson::Document pipe_error_body(const std::string& code,
                                    const std::string& message) {
  rapidjson::Document body;
  body.SetObject();
  rapidjson::Document::AllocatorType& alloc = body.GetAllocator();

  rapidjson::Value error(rapidjson::kObjectType);
  error.AddMember("code",
                  rapidjson::Value(code.c_str(),
                                   static_cast<rapidjson::SizeType>(code.size()),
                                   alloc),
                  alloc);
  error.AddMember(
      "message",
      rapidjson::Value(message.c_str(),
                       static_cast<rapidjson::SizeType>(message.size()), alloc),
      alloc);

  body.AddMember("ok", false, alloc);
  body.AddMember("error", error, alloc);
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

  rapidjson::Document root;
  if (!parse_pipe_json(json, &root) || !ExactRoot(root)) {
    SetCode(error_code, "schema");
    return false;
  }

  long long version_i = 0;
  std::string id_text;
  std::string op_text;
  const rapidjson::Value* body = pipe_json_member(root, "body");
  if (!pipe_json_int64(root, "v", &version_i) ||
      !pipe_json_string(root, "id", &id_text) || id_text.empty() ||
      !pipe_json_string(root, "op", &op_text) || op_text.empty() || !body ||
      !body->IsObject()) {
    SetCode(error_code, "schema");
    return false;
  }

  message->v = static_cast<int>(version_i);
  message->id = id_text;
  message->op = op_text;
  pipe_json_copy(&message->body, *body);
  SetCode(error_code, "");
  return true;
}

std::string serialize_pipe_message(const PipeMessage& message) {
  rapidjson::Document root;
  root.SetObject();
  rapidjson::Document::AllocatorType& alloc = root.GetAllocator();

  root.AddMember("v", message.v, alloc);
  root.AddMember(
      "id",
      rapidjson::Value(message.id.c_str(),
                       static_cast<rapidjson::SizeType>(message.id.size()),
                       alloc),
      alloc);
  root.AddMember(
      "op",
      rapidjson::Value(message.op.c_str(),
                       static_cast<rapidjson::SizeType>(message.op.size()),
                       alloc),
      alloc);

  rapidjson::Value body(rapidjson::kObjectType);
  body.CopyFrom(message.body, alloc);
  root.AddMember("body", body, alloc);

  return pipe_json_stringify(root);
}

}  // namespace content
