// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/pipe/pipe_client.h"

#include <windows.h>

#include <stdint.h>

namespace content {
namespace {

void* InvalidHandle() {
  return reinterpret_cast<void*>(static_cast<intptr_t>(-1));
}

void SetError(std::string* error, const std::string& message) {
  if (error)
    *error = message;
}

std::string WinError(const char* what) {
  return std::string(what) + " (" + std::to_string(GetLastError()) + ")";
}

}  // namespace

PipeClient::PipeClient() : handle_(InvalidHandle()) {}

PipeClient::~PipeClient() { close(); }

bool PipeClient::connected() const { return handle_ != InvalidHandle(); }

bool PipeClient::connect(std::string* error) {
  close();
  HANDLE handle = INVALID_HANDLE_VALUE;
  for (int attempt = 0; attempt < 50; ++attempt) {
    handle = CreateFileW(pipe_name_wide(), GENERIC_READ | GENERIC_WRITE, 0,
                         NULL, OPEN_EXISTING, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE)
      break;
    DWORD err = GetLastError();
    if (err != ERROR_FILE_NOT_FOUND && err != ERROR_PIPE_BUSY) {
      SetError(error, WinError("CreateFile pipe"));
      return false;
    }
    Sleep(20);
  }
  if (handle == INVALID_HANDLE_VALUE) {
    SetError(error, WinError("CreateFile pipe"));
    return false;
  }
  DWORD mode = PIPE_READMODE_MESSAGE;
  if (!SetNamedPipeHandleState(handle, &mode, NULL, NULL)) {
    std::string message = WinError("SetNamedPipeHandleState");
    CloseHandle(handle);
    SetError(error, message);
    return false;
  }
  handle_ = reinterpret_cast<void*>(handle);
  SetError(error, "");
  return true;
}

void PipeClient::close() {
  if (handle_ == InvalidHandle())
    return;
  CloseHandle(reinterpret_cast<HANDLE>(handle_));
  handle_ = InvalidHandle();
}

bool PipeClient::write_all(const std::string& frame, std::string* error) {
  if (!connected()) {
    SetError(error, "io");
    return false;
  }
  DWORD wrote = 0;
  BOOL ok = WriteFile(reinterpret_cast<HANDLE>(handle_), frame.data(),
                      static_cast<DWORD>(frame.size()), &wrote, NULL);
  if (!ok || wrote != frame.size()) {
    SetError(error, WinError("WriteFile"));
    return false;
  }
  return true;
}

bool PipeClient::read_message(std::string* json, std::string* error) {
  if (!connected() || !json) {
    SetError(error, "io");
    return false;
  }
  std::string buffer(kMaxPipeFrameBytes, '\0');
  DWORD got = 0;
  BOOL ok = ReadFile(reinterpret_cast<HANDLE>(handle_), &buffer[0],
                     static_cast<DWORD>(buffer.size()), &got, NULL);
  if (!ok) {
    SetError(error, WinError("ReadFile"));
    return false;
  }
  std::string code;
  if (!decode_pipe_frame(buffer.data(), got, json, &code)) {
    SetError(error, code.empty() ? "schema" : code);
    return false;
  }
  return true;
}

bool PipeClient::exchange_frame(const std::string& frame,
                                std::string* reply_json,
                                std::string* error) {
  if (frame.size() > kMaxPipeFrameBytes) {
    SetError(error, "too_large");
    return false;
  }
  if (!write_all(frame, error))
    return false;
  return read_message(reply_json, error);
}

bool PipeClient::transact(const PipeMessage& request,
                          PipeMessage* response,
                          std::string* error) {
  if (!response) {
    SetError(error, "io");
    return false;
  }
  std::string frame;
  std::string code;
  if (!encode_pipe_frame(serialize_pipe_message(request), &frame, &code)) {
    SetError(error, code.empty() ? "io" : code);
    return false;
  }
  std::string json;
  if (!exchange_frame(frame, &json, error))
    return false;
  if (!parse_pipe_message(json, response, &code)) {
    SetError(error, code.empty() ? "schema" : code);
    return false;
  }
  return true;
}

}  // namespace content
