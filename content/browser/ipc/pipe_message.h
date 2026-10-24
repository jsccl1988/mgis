// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_IPC_PIPE_MESSAGE_H_
#define CONTENT_BROWSER_IPC_PIPE_MESSAGE_H_

#include <string>

#include "content/browser/ipc/pipe_json.h"

namespace content {

// Whole frame, little-endian uint32 length plus UTF-8 JSON, must not exceed
// this. The length counts the JSON bytes only.
static const size_t kMaxPipeFrameBytes = 65536;

// UE is the single server. The browser connects to this name.
const wchar_t* pipe_name_wide();

// One envelope: { "v", "id", "op", "body" }. |body| is a RapidJSON object
// (same Document usage as smartgis sdbd_json / args_json).
struct PipeMessage {
  PipeMessage();

  int v;
  std::string id;
  std::string op;
  rapidjson::Document body;
};

// Failure body: { "ok": false, "error": { "code", "message" } }.
rapidjson::Document pipe_error_body(const std::string& code,
                                    const std::string& message);

bool encode_pipe_frame(const std::string& json,
                       std::string* frame,
                       std::string* error_code);
bool decode_pipe_frame(const char* data,
                       size_t size,
                       std::string* json,
                       std::string* error_code);

bool parse_pipe_message(const std::string& json,
                        PipeMessage* message,
                        std::string* error_code);
std::string serialize_pipe_message(const PipeMessage& message);

}  // namespace content

#endif  // CONTENT_BROWSER_IPC_PIPE_MESSAGE_H_
