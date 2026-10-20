// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/pipe/ue_peer.h"

#include <math.h>
#include <windows.h>
#include <aclapi.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "content/browser/pipe/pipe_json.h"
#include "content/browser/pipe/pipe_message.h"

namespace content {
namespace {

const int kRecentIdLimit = 64;

struct View {
  View()
      : attached(false),
        has_pose(false),
        parent_hwnd(0),
        child_hwnd(0),
        width(0),
        height(0),
        dpi(0),
        lon(0),
        lat(0),
        height_m(0),
        heading(0),
        pitch(0),
        seq(-1) {}

  bool attached;
  bool has_pose;
  unsigned long long parent_hwnd;
  unsigned long long child_hwnd;
  int width;
  int height;
  int dpi;
  double lon;
  double lat;
  double height_m;
  double heading;
  double pitch;
  long long seq;
  std::string last_origin;
};

struct SecurityBundle {
  SecurityBundle() : acl(NULL), sd(NULL) { ZeroMemory(&sa, sizeof(sa)); }

  std::vector<unsigned char> token_user;
  PACL acl;
  PSECURITY_DESCRIPTOR sd;
  SECURITY_ATTRIBUTES sa;
};

std::string WinError(const char* what) {
  return std::string(what) + " (" + std::to_string(GetLastError()) + ")";
}

void FreeSecurity(SecurityBundle* bundle) {
  if (!bundle)
    return;
  if (bundle->sd) {
    LocalFree(bundle->sd);
    bundle->sd = NULL;
  }
  if (bundle->acl) {
    LocalFree(bundle->acl);
    bundle->acl = NULL;
  }
  bundle->token_user.clear();
  bundle->sa.lpSecurityDescriptor = NULL;
}

bool BuildUserSecurity(SecurityBundle* bundle, std::string* error) {
  HANDLE token = NULL;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
    *error = WinError("OpenProcessToken");
    return false;
  }
  DWORD need = 0;
  GetTokenInformation(token, TokenUser, NULL, 0, &need);
  if (need == 0) {
    CloseHandle(token);
    *error = WinError("GetTokenInformation");
    return false;
  }
  bundle->token_user.resize(need);
  if (!GetTokenInformation(token, TokenUser, &bundle->token_user[0], need,
                           &need)) {
    CloseHandle(token);
    *error = WinError("GetTokenInformation");
    return false;
  }
  CloseHandle(token);

  TOKEN_USER* user = reinterpret_cast<TOKEN_USER*>(&bundle->token_user[0]);
  EXPLICIT_ACCESSW access;
  ZeroMemory(&access, sizeof(access));
  access.grfAccessPermissions = GENERIC_ALL;
  access.grfAccessMode = SET_ACCESS;
  access.grfInheritance = NO_INHERITANCE;
  access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  access.Trustee.TrusteeType = TRUSTEE_IS_USER;
  access.Trustee.ptstrName = reinterpret_cast<LPWSTR>(user->User.Sid);

  DWORD acl_error = SetEntriesInAclW(1, &access, NULL, &bundle->acl);
  if (acl_error != ERROR_SUCCESS) {
    *error = "SetEntriesInAcl (" + std::to_string(acl_error) + ")";
    return false;
  }
  bundle->sd = static_cast<PSECURITY_DESCRIPTOR>(
      LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH));
  if (!bundle->sd ||
      !InitializeSecurityDescriptor(bundle->sd, SECURITY_DESCRIPTOR_REVISION) ||
      !SetSecurityDescriptorDacl(bundle->sd, TRUE, bundle->acl, FALSE)) {
    *error = WinError("security descriptor");
    return false;
  }
  // Owner is the creating user when this call is denied. The DACL still
  // names only that user's SID.
  SetSecurityDescriptorOwner(bundle->sd, user->User.Sid, FALSE);
  bundle->sa.nLength = sizeof(bundle->sa);
  bundle->sa.lpSecurityDescriptor = bundle->sd;
  bundle->sa.bInheritHandle = FALSE;
  return true;
}


double SeparationMeters(double lon1, double lat1, double lon2, double lat2) {
  const double kPi = 3.14159265358979323846;
  const double kEarth = 6371000.0;
  double lat1_rad = lat1 * kPi / 180.0;
  double lat2_rad = lat2 * kPi / 180.0;
  double dlat = (lat2 - lat1) * kPi / 180.0;
  double dlon = (lon2 - lon1) * kPi / 180.0;
  double sin_dlat = sin(dlat / 2.0);
  double sin_dlon = sin(dlon / 2.0);
  double a = sin_dlat * sin_dlat +
             cos(lat1_rad) * cos(lat2_rad) * sin_dlon * sin_dlon;
  if (a < 0.0)
    a = 0.0;
  if (a > 1.0)
    a = 1.0;
  return kEarth * (2.0 * atan2(sqrt(a), sqrt(1.0 - a)));
}

PipeMessage ErrorReply(const PipeMessage& request,
                       const char* code,
                       const char* message) {
  PipeMessage reply;
  reply.v = 1;
  reply.id = request.id.empty() ? "0" : request.id;
  reply.op = request.op.empty() ? "health.ping" : request.op;
  reply.body = pipe_error_body(code, message);
  return reply;
}

PipeMessage Echo(const PipeMessage& request) {
  PipeMessage reply;
  reply.v = 1;
  reply.id = request.id;
  reply.op = request.op;
  pipe_json_copy(&reply.body, request.body);
  return reply;
}

View* FindView(View* view0, View* view1, const std::string& name) {
  if (name == "view0")
    return view0;
  if (name == "view1")
    return view1;
  return NULL;
}

int CountCameras(const View& view0, const View& view1) {
  return (view0.attached ? 1 : 0) + (view1.attached ? 1 : 0);
}

enum WaitKind { kWaitOk, kWaitStop, kWaitFail };

WaitKind WaitIo(HANDLE stop_event, HANDLE pipe, OVERLAPPED* overlapped, DWORD* bytes) {
  HANDLE waits[2];
  waits[0] = stop_event;
  waits[1] = overlapped->hEvent;
  DWORD result = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
  if (result == WAIT_OBJECT_0) {
    // Stop won the race. Cancel and drain before the stack OVERLAPPED is
    // destroyed; otherwise a late completion corrupts the stack.
    CancelIoEx(pipe, overlapped);
    DWORD ignored = 0;
    GetOverlappedResult(pipe, overlapped, &ignored, TRUE);
    return kWaitStop;
  }
  if (result != WAIT_OBJECT_0 + 1)
    return kWaitFail;
  if (!GetOverlappedResult(pipe, overlapped, bytes, FALSE))
    return kWaitFail;
  return kWaitOk;
}

}  // namespace

const char kStandInTilesetLayerId[] = "3d-tiles";

struct UePeerState {
  UePeerState()
      : stop(false),
        frames(0),
        stop_event(NULL),
        ready_event(NULL),
        pipe(INVALID_HANDLE_VALUE),
        child_serial(0),
        origin_lon(0),
        origin_lat(0),
        origin_set(false),
        tileset_visible(true),
        tileset_hidden(false),
        tileset_opacity(1) {}

  std::atomic<bool> stop;
  std::atomic<unsigned long long> frames;
  HANDLE stop_event;
  HANDLE ready_event;
  HANDLE pipe;
  std::thread io;
  std::thread game;
  std::string start_error;
  SecurityBundle security;
  mutable std::mutex mu;
  View view0;
  View view1;
  unsigned long long child_serial;
  double origin_lon;
  double origin_lat;
  bool origin_set;
  bool tileset_visible;
  bool tileset_hidden;
  double tileset_opacity;
  std::vector<std::pair<std::string, std::string> > recent;
};

namespace {

void ResetWorld(UePeerState* state) {
  state->frames.store(0);
  state->view0 = View();
  state->view1 = View();
  state->child_serial = 0;
  state->origin_lon = 0;
  state->origin_lat = 0;
  state->origin_set = false;
  state->tileset_visible = true;
  state->tileset_hidden = false;
  state->tileset_opacity = 1;
  state->recent.clear();
  state->start_error.clear();
}

bool FindRecent(UePeerState* state, const std::string& id, std::string* json) {
  for (size_t i = 0; i < state->recent.size(); ++i) {
    if (state->recent[i].first == id) {
      *json = state->recent[i].second;
      return true;
    }
  }
  return false;
}

void Remember(UePeerState* state, const std::string& id, const std::string& json) {
  state->recent.push_back(std::make_pair(id, json));
  if (static_cast<int>(state->recent.size()) > kRecentIdLimit)
    state->recent.erase(state->recent.begin());
}

PipeMessage MakeWelcome(UePeerState* state, const PipeMessage& request) {
  PipeMessage reply;
  reply.v = 1;
  reply.id = request.id;
  reply.op = "session.welcome";
  rapidjson::Document::AllocatorType& alloc = reply.body.GetAllocator();

  rapidjson::Value origin(rapidjson::kObjectType);
  origin.AddMember("longitude", state->origin_lon, alloc);
  origin.AddMember("latitude", state->origin_lat, alloc);

  rapidjson::Value snapshot(rapidjson::kObjectType);
  snapshot.AddMember("cameras", CountCameras(state->view0, state->view1),
                     alloc);
  snapshot.AddMember(
      "frames",
      rapidjson::Value().SetInt64(
          static_cast<int64_t>(state->frames.load())),
      alloc);
  snapshot.AddMember("origin", origin, alloc);

  rapidjson::Value layer(rapidjson::kObjectType);
  layer.AddMember("id", rapidjson::Value(kStandInTilesetLayerId, alloc), alloc);
  layer.AddMember("kind", rapidjson::Value("tileset", alloc), alloc);
  layer.AddMember("visible", state->tileset_visible, alloc);

  rapidjson::Value layers(rapidjson::kArrayType);
  layers.PushBack(layer, alloc);

  reply.body.AddMember("snapshot", snapshot, alloc);
  reply.body.AddMember("layers", layers, alloc);
  reply.body.AddMember("entities", rapidjson::Value(rapidjson::kArrayType),
                       alloc);
  return reply;
}

bool ReadViewName(const rapidjson::Value& body, std::string* view) {
  return pipe_json_string(body, "view", view) &&
         (*view == "view0" || *view == "view1");
}

PipeMessage HandleHello(UePeerState* state, const PipeMessage& request) {
  static const char* kKeys[] = {"protocol", "build"};
  long long protocol = 0;
  std::string build;
  if (!pipe_json_exact_keys(request.body, kKeys, 2) ||
      !pipe_json_int64(request.body, "protocol", &protocol) || protocol != 1 ||
      !pipe_json_string(request.body, "build", &build) || build.empty()) {
    return ErrorReply(request, "schema", "session.hello body");
  }
  return MakeWelcome(state, request);
}

PipeMessage HandleAttach(UePeerState* state, const PipeMessage& request) {
  static const char* kKeys[] = {"view", "parentHwnd", "width", "height", "dpi"};
  std::string view_name;
  long long parent = 0;
  long long width = 0;
  long long height = 0;
  long long dpi = 0;
  if (!pipe_json_exact_keys(request.body, kKeys, 5) ||
      !ReadViewName(request.body, &view_name) ||
      !pipe_json_int64(request.body, "parentHwnd", &parent) || parent < 0 ||
      !pipe_json_int64(request.body, "width", &width) || width < 0 ||
      width > 2147483647LL ||
      !pipe_json_int64(request.body, "height", &height) || height < 0 ||
      height > 2147483647LL ||
      !pipe_json_int64(request.body, "dpi", &dpi) || dpi < 1 ||
      dpi > 2147483647LL) {
    return ErrorReply(request, "schema", "viewport.attach body");
  }
  View* view = FindView(&state->view0, &state->view1, view_name);
  view->attached = true;
  view->parent_hwnd = static_cast<unsigned long long>(parent);
  view->width = static_cast<int>(width);
  view->height = static_cast<int>(height);
  view->dpi = static_cast<int>(dpi);
  state->child_serial += 1;
  view->child_hwnd = 0x10000ull + state->child_serial;
  return Echo(request);
}

PipeMessage HandleResize(UePeerState* state, const PipeMessage& request) {
  static const char* kKeys[] = {"view", "width", "height"};
  std::string view_name;
  long long width = 0;
  long long height = 0;
  if (!pipe_json_exact_keys(request.body, kKeys, 3) ||
      !ReadViewName(request.body, &view_name) ||
      !pipe_json_int64(request.body, "width", &width) || width < 0 ||
      width > 2147483647LL ||
      !pipe_json_int64(request.body, "height", &height) || height < 0 ||
      height > 2147483647LL) {
    return ErrorReply(request, "schema", "viewport.resize body");
  }
  View* view = FindView(&state->view0, &state->view1, view_name);
  if (!view->attached)
    return ErrorReply(request, "not_found", "viewport is not attached");
  view->width = static_cast<int>(width);
  view->height = static_cast<int>(height);
  return Echo(request);
}

PipeMessage HandleClose(UePeerState* state, const PipeMessage& request) {
  static const char* kKeys[] = {"view"};
  std::string view_name;
  if (!pipe_json_exact_keys(request.body, kKeys, 1) ||
      !ReadViewName(request.body, &view_name))
    return ErrorReply(request, "schema", "viewport.close body");
  View* view = FindView(&state->view0, &state->view1, view_name);
  if (!view->attached)
    return ErrorReply(request, "not_found", "viewport is not attached");
  view->attached = false;
  view->child_hwnd = 0;
  view->width = 0;
  view->height = 0;
  return Echo(request);
}

PipeMessage HandlePing(const PipeMessage& request) {
  if (!request.body.IsObject() || request.body.MemberCount() != 0)
    return ErrorReply(request, "schema", "health.ping body");
  return Echo(request);
}

// Shared georeference origin. A camera.set that would place two attached
// cameras farther than the fixed limit is refused and the origin stays put.
PipeMessage HandleCamera(UePeerState* state, const PipeMessage& request) {
  static const char* kKeys[] = {"origin", "seq",     "longitude", "latitude",
                                "height", "heading", "pitch"};
  std::string origin;
  long long seq = 0;
  double longitude = 0;
  double latitude = 0;
  double height = 0;
  double heading = 0;
  double pitch = 0;
  if (!pipe_json_exact_keys(request.body, kKeys, 7) ||
      !pipe_json_string(request.body, "origin", &origin) ||
      (origin != "ui" && origin != "view0" && origin != "view1") ||
      !pipe_json_int64(request.body, "seq", &seq) || seq < 0 ||
      !pipe_json_double(request.body, "longitude", &longitude) ||
      !pipe_json_double(request.body, "latitude", &latitude) ||
      !pipe_json_double(request.body, "height", &height) ||
      !pipe_json_double(request.body, "heading", &heading) ||
      !pipe_json_double(request.body, "pitch", &pitch)) {
    return ErrorReply(request, "schema", "camera.set body");
  }
  View* target = origin == "view1" ? &state->view1 : &state->view0;
  View* other = target == &state->view0 ? &state->view1 : &state->view0;
  if (!target->attached)
    return ErrorReply(request, "not_found", "viewport is not attached");
  if (target->has_pose && target->seq == seq && target->last_origin == origin)
    return Echo(request);

  View saved = *target;
  target->has_pose = true;
  target->lon = longitude;
  target->lat = latitude;
  target->height_m = height;
  target->heading = heading;
  target->pitch = pitch;
  target->seq = seq;
  target->last_origin = origin;

  if (other->attached && other->has_pose) {
    double separation =
        SeparationMeters(target->lon, target->lat, other->lon, other->lat);
    if (separation > origin_separation_limit_meters()) {
      *target = saved;
      return ErrorReply(request, "origin_conflict",
                        "cameras exceed the shared-origin separation limit");
    }
    state->origin_lon = (target->lon + other->lon) / 2.0;
    state->origin_lat = (target->lat + other->lat) / 2.0;
  } else {
    state->origin_lon = target->lon;
    state->origin_lat = target->lat;
  }
  state->origin_set = true;
  return Echo(request);
}

PipeMessage HandleLayer(UePeerState* state, const PipeMessage& request) {
  static const char* kKeys[] = {"layerId", "viewport", "visible", "opacity"};
  std::string layer_id;
  std::string viewport;
  bool visible = false;
  double opacity = 0;
  if (!pipe_json_exact_keys(request.body, kKeys, 4) ||
      !pipe_json_string(request.body, "layerId", &layer_id) ||
      layer_id.empty() ||
      !pipe_json_string(request.body, "viewport", &viewport) ||
      (viewport != "view0" && viewport != "view1" && viewport != "both") ||
      !pipe_json_bool(request.body, "visible", &visible) ||
      !pipe_json_double(request.body, "opacity", &opacity) || opacity < 0 ||
      opacity > 1) {
    return ErrorReply(request, "schema", "layer.set body");
  }
  if (layer_id != kStandInTilesetLayerId)
    return ErrorReply(request, "not_found", "layer not found");
  state->tileset_visible = visible;
  state->tileset_opacity = opacity;
  if (!visible)
    state->tileset_hidden = true;
  return Echo(request);
}

PipeMessage HandleLocked(UePeerState* state, const PipeMessage& request) {
  if (request.v != 1)
    return ErrorReply(request, "schema", "envelope v is not 1");
  if (request.op == "session.hello")
    return HandleHello(state, request);
  if (request.op == "viewport.attach")
    return HandleAttach(state, request);
  if (request.op == "viewport.resize")
    return HandleResize(state, request);
  if (request.op == "viewport.close")
    return HandleClose(state, request);
  if (request.op == "health.ping")
    return HandlePing(request);
  if (request.op == "camera.set")
    return HandleCamera(state, request);
  if (request.op == "layer.set")
    return HandleLayer(state, request);
  return ErrorReply(request, "schema", "unsupported op");
}

std::string ReplyForFrame(UePeerState* state, const char* data, DWORD bytes) {
  std::string json;
  std::string code;
  if (!decode_pipe_frame(data, static_cast<size_t>(bytes), &json, &code)) {
    PipeMessage reply;
    reply.v = 1;
    reply.id = "0";
    reply.op = "health.ping";
    reply.body = pipe_error_body(code.empty() ? "schema" : code, "frame rejected");
    return serialize_pipe_message(reply);
  }
  PipeMessage request;
  if (!parse_pipe_message(json, &request, &code)) {
    PipeMessage reply;
    reply.v = 1;
    reply.id = "0";
    reply.op = "health.ping";
    reply.body = pipe_error_body("schema", "envelope rejected");
    return serialize_pipe_message(reply);
  }
  std::lock_guard<std::mutex> guard(state->mu);
  std::string cached;
  if (FindRecent(state, request.id, &cached))
    return cached;
  PipeMessage reply = HandleLocked(state, request);
  std::string serialized = serialize_pipe_message(reply);
  Remember(state, request.id, serialized);
  return serialized;
}

bool WriteFrame(HANDLE pipe, HANDLE stop_event, HANDLE io_event, const std::string& json) {
  std::string frame;
  std::string code;
  if (!encode_pipe_frame(json, &frame, &code))
    return false;
  OVERLAPPED overlapped;
  ZeroMemory(&overlapped, sizeof(overlapped));
  overlapped.hEvent = io_event;
  ResetEvent(io_event);
  DWORD wrote = 0;
  BOOL ok = WriteFile(pipe, frame.data(), static_cast<DWORD>(frame.size()), &wrote,
                      &overlapped);
  if (!ok) {
    if (GetLastError() != ERROR_IO_PENDING)
      return false;
    DWORD bytes = 0;
    if (WaitIo(stop_event, pipe, &overlapped, &bytes) != kWaitOk)
      return false;
    wrote = bytes;
  }
  return wrote == frame.size();
}

// Returns true when the I/O thread should stop. A dropped client returns false
// so the server can accept the next shell.
bool ServeClient(UePeerState* state, HANDLE io_event) {
  std::vector<char> buffer(kMaxPipeFrameBytes);
  while (!state->stop.load()) {
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.hEvent = io_event;
    ResetEvent(io_event);
    DWORD bytes = 0;
    BOOL ok = ReadFile(state->pipe, &buffer[0], static_cast<DWORD>(buffer.size()),
                       &bytes, &overlapped);
    if (!ok) {
      DWORD err = GetLastError();
      if (err == ERROR_IO_PENDING) {
        WaitKind kind = WaitIo(state->stop_event, state->pipe, &overlapped, &bytes);
        if (kind == kWaitStop)
          return true;
        if (kind != kWaitOk)
          return false;
      } else if (err == ERROR_MORE_DATA) {
        std::string reply = ReplyForFrame(state, NULL, static_cast<DWORD>(kMaxPipeFrameBytes + 1));
        WriteFrame(state->pipe, state->stop_event, io_event, reply);
        return false;
      } else {
        return false;
      }
    }
    std::string reply = ReplyForFrame(state, &buffer[0], bytes);
    if (!WriteFrame(state->pipe, state->stop_event, io_event, reply))
      return state->stop.load();
  }
  return true;
}

bool AcceptClient(UePeerState* state, HANDLE io_event) {
  OVERLAPPED overlapped;
  ZeroMemory(&overlapped, sizeof(overlapped));
  overlapped.hEvent = io_event;
  ResetEvent(io_event);
  BOOL ok = ConnectNamedPipe(state->pipe, &overlapped);
  if (ok)
    return true;
  DWORD err = GetLastError();
  if (err == ERROR_PIPE_CONNECTED)
    return true;
  if (err != ERROR_IO_PENDING)
    return false;
  DWORD bytes = 0;
  WaitKind kind = WaitIo(state->stop_event, state->pipe, &overlapped, &bytes);
  return kind == kWaitOk;
}

}  // namespace

UePeer::UePeer() : state_(new UePeerState()) {
  state_->stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
  state_->ready_event = CreateEventW(NULL, TRUE, FALSE, NULL);
}

UePeer::~UePeer() {
  stop();
  if (state_->stop_event)
    CloseHandle(state_->stop_event);
  if (state_->ready_event)
    CloseHandle(state_->ready_event);
  delete state_;
  state_ = NULL;
}

void UePeer::game_main() {
  // Stand-in game thread. Frame advance only; this thread must not call ReadFile.
  for (;;) {
    state_->frames.fetch_add(1);
    if (WaitForSingleObject(state_->stop_event, 5) != WAIT_TIMEOUT)
      break;
  }
}

void UePeer::io_main() {
  std::string error;
  if (!BuildUserSecurity(&state_->security, &error)) {
    state_->start_error = error;
    SetEvent(state_->ready_event);
    return;
  }
  HANDLE pipe = INVALID_HANDLE_VALUE;
  for (int attempt = 0; attempt < 50 && !state_->stop.load(); ++attempt) {
    pipe = CreateNamedPipeW(
        pipe_name_wide(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1,
        static_cast<DWORD>(kMaxPipeFrameBytes),
        static_cast<DWORD>(kMaxPipeFrameBytes), 0, &state_->security.sa);
    if (pipe != INVALID_HANDLE_VALUE)
      break;
    Sleep(20);
  }
  state_->pipe = pipe;
  if (pipe == INVALID_HANDLE_VALUE) {
    state_->start_error = WinError("CreateNamedPipe");
    SetEvent(state_->ready_event);
    return;
  }
  HANDLE io_event = CreateEventW(NULL, TRUE, FALSE, NULL);
  SetEvent(state_->ready_event);
  while (!state_->stop.load()) {
    if (!AcceptClient(state_, io_event))
      break;
    if (state_->stop.load())
      break;
    bool stopping = ServeClient(state_, io_event);
    DisconnectNamedPipe(state_->pipe);
    if (stopping || state_->stop.load())
      break;
  }
  if (io_event)
    CloseHandle(io_event);
}

bool UePeer::start(std::string* error) {
  if (state_->io.joinable() || state_->game.joinable()) {
    if (error)
      *error = "peer already started";
    return false;
  }
  if (!state_->stop_event || !state_->ready_event) {
    if (error)
      *error = "peer events were not created";
    return false;
  }
  ResetWorld(state_);
  state_->stop.store(false);
  ResetEvent(state_->stop_event);
  ResetEvent(state_->ready_event);
  state_->io = std::thread(&UePeer::io_main, this);
  state_->game = std::thread(&UePeer::game_main, this);
  WaitForSingleObject(state_->ready_event, 5000);
  if (!state_->start_error.empty() || state_->pipe == INVALID_HANDLE_VALUE) {
    std::string message = state_->start_error.empty() ? "pipe server did not start"
                                                      : state_->start_error;
    stop();
    if (error)
      *error = message;
    return false;
  }
  if (error)
    error->clear();
  return true;
}

void UePeer::stop() {
  if (!state_)
    return;
  state_->stop.store(true);
  if (state_->stop_event)
    SetEvent(state_->stop_event);
  if (state_->pipe != INVALID_HANDLE_VALUE)
    CancelIoEx(state_->pipe, NULL);
  if (state_->io.joinable())
    state_->io.join();
  if (state_->game.joinable())
    state_->game.join();
  if (state_->pipe != INVALID_HANDLE_VALUE) {
    CloseHandle(state_->pipe);
    state_->pipe = INVALID_HANDLE_VALUE;
  }
  FreeSecurity(&state_->security);
}

unsigned long long UePeer::frame_count() const { return state_->frames.load(); }

int UePeer::additional_cameras() const {
  std::lock_guard<std::mutex> guard(state_->mu);
  return CountCameras(state_->view0, state_->view1);
}

bool UePeer::view_attached(const std::string& view) const {
  std::lock_guard<std::mutex> guard(state_->mu);
  const View* found = FindView(const_cast<View*>(&state_->view0),
                               const_cast<View*>(&state_->view1), view);
  return found && found->attached;
}

unsigned long long UePeer::child_hwnd(const std::string& view) const {
  std::lock_guard<std::mutex> guard(state_->mu);
  const View* found = FindView(const_cast<View*>(&state_->view0),
                               const_cast<View*>(&state_->view1), view);
  return found ? found->child_hwnd : 0;
}

bool UePeer::view_size(const std::string& view, int* width, int* height) const {
  std::lock_guard<std::mutex> guard(state_->mu);
  const View* found = FindView(const_cast<View*>(&state_->view0),
                               const_cast<View*>(&state_->view1), view);
  if (!found || !width || !height)
    return false;
  *width = found->width;
  *height = found->height;
  return true;
}

bool UePeer::tileset_hidden() const {
  std::lock_guard<std::mutex> guard(state_->mu);
  return state_->tileset_hidden;
}

void UePeer::origin(double* longitude, double* latitude) const {
  std::lock_guard<std::mutex> guard(state_->mu);
  if (longitude)
    *longitude = state_->origin_lon;
  if (latitude)
    *latitude = state_->origin_lat;
}

}  // namespace content
