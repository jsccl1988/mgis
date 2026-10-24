// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_IPC_PIPE_CLIENT_H_
#define CONTENT_BROWSER_IPC_PIPE_CLIENT_H_

#include <string>

#include "content/browser/ipc/pipe_message.h"

namespace content {

// Browser end of \\.\pipe\mview. Sends one JSON envelope and reads one reply.
// The process that owns the pipe server is the UE stand-in, not this class.
class PipeClient {
 public:
  PipeClient();
  ~PipeClient();

  bool connect(std::string* error);
  void close();
  bool connected() const;

  // Serializes |request|, rejects a frame over kMaxPipeFrameBytes, and parses
  // the reply. Returns false on a transport failure. An application error
  // body is still a successful exchange.
  bool transact(const PipeMessage& request,
                PipeMessage* response,
                std::string* error);

  // Writes one raw frame and reads one reply payload. A frame larger than
  // kMaxPipeFrameBytes is refused with error code too_large and is not sent.
  bool exchange_frame(const std::string& frame,
                      std::string* reply_json,
                      std::string* error);

 private:
  PipeClient(const PipeClient&) = delete;
  PipeClient& operator=(const PipeClient&) = delete;

  bool write_all(const std::string& frame, std::string* error);
  bool read_message(std::string* json, std::string* error);

  void* handle_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_IPC_PIPE_CLIENT_H_
