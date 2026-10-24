// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_IPC_UE_PEER_H_
#define CONTENT_BROWSER_IPC_UE_PEER_H_

#include <string>

namespace content {

// Fixed product limit for the stand-in. Two AdditionalCameras share one
// georeference origin, so a separation above this is origin_conflict.
inline double origin_separation_limit_meters() { return 100000.0; }

// Layer id seeded in the stand-in catalog. Its kind is tileset.
extern const char kStandInTilesetLayerId[];

// Opaque pipe-server state. Defined next to the stand-in implementation.
struct UePeerState;

// In-process stand-in for the UE pipe server. There is one UWorld: view0 and
// view1 are AdditionalCameras while attached. The game thread only advances a
// frame counter and never calls ReadFile. ReadFile runs on the I/O thread.
// Unreal is not required for the viewport and origin gates.
class UePeer {
 public:
  UePeer();
  ~UePeer();

  bool start(std::string* error);
  void stop();

  unsigned long long frame_count() const;
  int additional_cameras() const;
  bool view_attached(const std::string& view) const;
  unsigned long long child_hwnd(const std::string& view) const;
  bool view_size(const std::string& view, int* width, int* height) const;
  bool tileset_hidden() const;
  void origin(double* longitude, double* latitude) const;

 private:
  UePeer(const UePeer&) = delete;
  UePeer& operator=(const UePeer&) = delete;

  void game_main();
  void io_main();

  UePeerState* state_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_IPC_UE_PEER_H_
