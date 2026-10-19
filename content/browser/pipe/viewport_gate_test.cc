// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/pipe/pipe_client.h"
#include "content/browser/pipe/pipe_message.h"
#include "content/browser/pipe/ue_peer.h"

#include <string>

#include "gtest/gtest.h"

#include <windows.h>

namespace {

class PipePeerFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    std::string error;
    ASSERT_TRUE(peer_.start(&error)) << error;
  }

  void TearDown() override { peer_.stop(); }

  content::UePeer peer_;
};

class ViewportGate : public PipePeerFixture {};

class WorldGate : public PipePeerFixture {};

content::PipeMessage MakeMessage(const std::string& id,
                                 const std::string& op,
                                 const content::PipeJson& body) {
  content::PipeMessage message;
  message.v = 1;
  message.id = id;
  message.op = op;
  message.body = body;
  return message;
}

content::PipeJson HelloBody() {
  content::PipeJson body = content::PipeJson::Object();
  body.set("protocol", content::PipeJson::Integer(1));
  body.set("build", content::PipeJson::String("viewport-gate"));
  return body;
}

content::PipeJson AttachBody(const char* view,
                             long long parent,
                             long long width,
                             long long height) {
  content::PipeJson body = content::PipeJson::Object();
  body.set("view", content::PipeJson::String(view));
  body.set("parentHwnd", content::PipeJson::Integer(parent));
  body.set("width", content::PipeJson::Integer(width));
  body.set("height", content::PipeJson::Integer(height));
  body.set("dpi", content::PipeJson::Integer(96));
  return body;
}

content::PipeJson CameraBody(const char* origin,
                             long long seq,
                             double longitude,
                             double latitude) {
  content::PipeJson body = content::PipeJson::Object();
  body.set("origin", content::PipeJson::String(origin));
  body.set("seq", content::PipeJson::Integer(seq));
  body.set("longitude", content::PipeJson::Number(longitude));
  body.set("latitude", content::PipeJson::Number(latitude));
  body.set("height", content::PipeJson::Number(50));
  body.set("heading", content::PipeJson::Number(0));
  body.set("pitch", content::PipeJson::Number(0));
  return body;
}

const content::PipeJson* ErrorCode(const content::PipeMessage& message) {
  const content::PipeJson* error = message.body.find("error");
  if (!error)
    return NULL;
  return error->find("code");
}

bool Transact(content::PipeClient* client,
              const content::PipeMessage& request,
              content::PipeMessage* response) {
  std::string error;
  if (!client->transact(request, response, &error)) {
    ADD_FAILURE() << error;
    return false;
  }
  return true;
}

std::wstring RepoRoot() {
  wchar_t path[MAX_PATH];
  DWORD length = GetModuleFileNameW(NULL, path, MAX_PATH);
  if (length == 0 || length >= MAX_PATH)
    return L".";
  std::wstring dir(path, length);
  for (int i = 0; i < 8; ++i) {
    std::wstring slash = dir.find_last_of(L"\\/") == std::wstring::npos
                             ? dir
                             : dir.substr(0, dir.find_last_of(L"\\/"));
    DWORD attr = GetFileAttributesW((slash + L"\\build.bat").c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
      return slash;
    if (slash == dir)
      break;
    dir = slash;
  }
  return L".";
}

bool EnsureDir(const std::wstring& dir) {
  DWORD attr = GetFileAttributesW(dir.c_str());
  if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
    return true;
  size_t split = dir.find_last_of(L"\\/");
  if (split != std::wstring::npos && split > 0) {
    if (!EnsureDir(dir.substr(0, split)))
      return false;
  }
  if (CreateDirectoryW(dir.c_str(), NULL))
    return true;
  return GetLastError() == ERROR_ALREADY_EXISTS;
}

bool WriteMarkFile(const wchar_t* suite_dir, const std::string& json) {
  std::wstring dir = RepoRoot() + L"\\out\\harness\\" + suite_dir;
  if (!EnsureDir(dir))
    return false;
  std::wstring path = dir + L"\\mark.json";
  HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  DWORD wrote = 0;
  BOOL ok = WriteFile(file, json.data(), static_cast<DWORD>(json.size()), &wrote,
                      NULL);
  CloseHandle(file);
  return ok && wrote == json.size();
}

}  // namespace

TEST_F(ViewportGate, AttachCloseAndReattach) {
  std::string frame;
  std::string frame_error;
  EXPECT_TRUE(content::encode_pipe_frame(std::string(65532, 'a'), &frame,
                                         &frame_error));
  EXPECT_EQ(content::kMaxPipeFrameBytes, frame.size());
  EXPECT_FALSE(content::encode_pipe_frame(std::string(65533, 'a'), &frame,
                                          &frame_error));
  EXPECT_EQ(std::string("too_large"), frame_error);
  std::string decoded;
  EXPECT_FALSE(content::decode_pipe_frame(NULL, 65537, &decoded, &frame_error));
  EXPECT_EQ(std::string("too_large"), frame_error);

  content::PipeClient client;
  std::string error;
  ASSERT_TRUE(client.connect(&error)) << error;

  content::PipeMessage bad_version;
  bad_version.v = 2;
  bad_version.id = "bad-v";
  bad_version.op = "health.ping";
  bad_version.body = content::PipeJson::Object();
  content::PipeMessage response;
  ASSERT_TRUE(Transact(&client, bad_version, &response));
  const content::PipeJson* bad_code = ErrorCode(response);
  ASSERT_TRUE(bad_code);
  EXPECT_EQ(std::string("schema"), bad_code->string_value());

  std::string oversized(8, '\0');
  oversized[0] = static_cast<char>(0x70);
  oversized[1] = static_cast<char>(0x11);
  oversized[2] = static_cast<char>(0x01);
  oversized[3] = '\0';
  std::string reply_json;
  ASSERT_TRUE(client.exchange_frame(oversized, &reply_json, &error)) << error;
  content::PipeMessage oversized_reply;
  ASSERT_TRUE(content::parse_pipe_message(reply_json, &oversized_reply, &error))
      << error;
  const content::PipeJson* large_code = ErrorCode(oversized_reply);
  ASSERT_TRUE(large_code);
  EXPECT_EQ(std::string("too_large"), large_code->string_value());

  ASSERT_TRUE(Transact(&client, MakeMessage("hello-1", "session.hello", HelloBody()),
                       &response));
  EXPECT_EQ(std::string("session.welcome"), response.op);
  ASSERT_TRUE(response.body.find("snapshot"));
  ASSERT_TRUE(response.body.find("layers"));
  ASSERT_TRUE(response.body.find("entities"));

  ASSERT_TRUE(Transact(
      &client, MakeMessage("att-0", "viewport.attach", AttachBody("view0", 4096, 640, 480)),
      &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);
  ASSERT_TRUE(Transact(
      &client, MakeMessage("att-1", "viewport.attach", AttachBody("view1", 8192, 320, 240)),
      &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);
  EXPECT_EQ(2, peer_.additional_cameras());
  unsigned long long child0 = peer_.child_hwnd("view0");
  unsigned long long child1 = peer_.child_hwnd("view1");
  EXPECT_NE(0ull, child0);
  EXPECT_NE(0ull, child1);
  EXPECT_NE(child0, child1);

  content::PipeJson resize = content::PipeJson::Object();
  resize.set("view", content::PipeJson::String("view0"));
  resize.set("width", content::PipeJson::Integer(800));
  resize.set("height", content::PipeJson::Integer(600));
  ASSERT_TRUE(Transact(&client, MakeMessage("rsz-0", "viewport.resize", resize),
                       &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);
  int width = 0;
  int height = 0;
  ASSERT_TRUE(peer_.view_size("view0", &width, &height));
  EXPECT_EQ(800, width);
  EXPECT_EQ(600, height);

  ASSERT_TRUE(Transact(&client,
                       MakeMessage("ping-1", "health.ping", content::PipeJson::Object()),
                       &response));
  EXPECT_EQ(std::string("health.ping"), response.op);
  EXPECT_EQ(std::string("ping-1"), response.id);
  EXPECT_TRUE(ErrorCode(response) == NULL);

  content::PipeJson close_body = content::PipeJson::Object();
  close_body.set("view", content::PipeJson::String("view1"));
  ASSERT_TRUE(Transact(&client, MakeMessage("cls-1", "viewport.close", close_body),
                       &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);
  EXPECT_FALSE(peer_.view_attached("view1"));
  EXPECT_EQ(1, peer_.additional_cameras());

  ASSERT_TRUE(Transact(&client, MakeMessage("hello-2", "session.hello", HelloBody()),
                       &response));
  const content::PipeJson* snapshot = response.body.find("snapshot");
  ASSERT_TRUE(snapshot);
  const content::PipeJson* cameras = snapshot->find("cameras");
  ASSERT_TRUE(cameras);
  ASSERT_TRUE(cameras->is_int());
  EXPECT_EQ(1, static_cast<int>(cameras->int_value()));

  unsigned long long frames_before = peer_.frame_count();
  client.close();
  Sleep(100);
  unsigned long long frames_after = peer_.frame_count();
  EXPECT_GT(frames_after, frames_before);
  EXPECT_EQ(1, peer_.additional_cameras());

  content::PipeClient again;
  ASSERT_TRUE(again.connect(&error)) << error;
  ASSERT_TRUE(Transact(&again, MakeMessage("hello-3", "session.hello", HelloBody()),
                       &response));
  EXPECT_EQ(std::string("session.welcome"), response.op);
  ASSERT_TRUE(Transact(
      &again, MakeMessage("att-2", "viewport.attach", AttachBody("view0", 12288, 800, 600)),
      &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);
  unsigned long long reattached = peer_.child_hwnd("view0");
  EXPECT_NE(0ull, reattached);
  EXPECT_NE(child0, reattached);
  EXPECT_EQ(1, peer_.additional_cameras());
  EXPECT_FALSE(peer_.view_attached("view1"));

  int mark_cameras = peer_.additional_cameras();
  ASSERT_EQ(1, mark_cameras);
  std::string mark =
      "{\n"
      "  \"suite\": \"browser.viewport\",\n"
      "  \"cameras\": " +
      std::to_string(mark_cameras) +
      ",\n"
      "  \"viewport.reattach\": true,\n"
      "  \"marks\": [\"viewport.reattach\"]\n"
      "}\n";
  ASSERT_TRUE(WriteMarkFile(L"browser.viewport", mark));
}

TEST_F(WorldGate, OriginConflictAndTilesetHidden) {
  // view3d.trackball is not this layer. This fixture only counts
  // AdditionalCameras and applies the shared-origin limit.
  content::PipeClient client;
  std::string error;
  ASSERT_TRUE(client.connect(&error)) << error;

  content::PipeMessage response;
  ASSERT_TRUE(Transact(&client, MakeMessage("w-hello", "session.hello", HelloBody()),
                       &response));
  ASSERT_TRUE(Transact(
      &client, MakeMessage("w-att-0", "viewport.attach", AttachBody("view0", 4096, 640, 480)),
      &response));
  ASSERT_TRUE(Transact(
      &client, MakeMessage("w-att-1", "viewport.attach", AttachBody("view1", 8192, 640, 480)),
      &response));
  ASSERT_EQ(2, peer_.additional_cameras());

  ASSERT_TRUE(Transact(&client,
                       MakeMessage("cam-0", "camera.set",
                                   CameraBody("view0", 1, 116.4074, 39.9042)),
                       &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);
  ASSERT_TRUE(Transact(&client,
                       MakeMessage("cam-1", "camera.set",
                                   CameraBody("view1", 1, 116.50, 39.95)),
                       &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);

  double origin_lon = 0;
  double origin_lat = 0;
  peer_.origin(&origin_lon, &origin_lat);

  ASSERT_TRUE(Transact(&client,
                       MakeMessage("cam-far", "camera.set",
                                   CameraBody("view1", 2, 121.4737, 31.2304)),
                       &response));
  const content::PipeJson* code = ErrorCode(response);
  ASSERT_TRUE(code);
  EXPECT_EQ(std::string("origin_conflict"), code->string_value());
  const content::PipeJson* ok = response.body.find("ok");
  ASSERT_TRUE(ok);
  EXPECT_FALSE(ok->bool_value());

  double after_lon = 0;
  double after_lat = 0;
  peer_.origin(&after_lon, &after_lat);
  bool origin_moved = after_lon != origin_lon || after_lat != origin_lat;
  EXPECT_FALSE(origin_moved);
  EXPECT_DOUBLE_EQ(origin_lon, after_lon);
  EXPECT_DOUBLE_EQ(origin_lat, after_lat);

  content::PipeJson layer = content::PipeJson::Object();
  layer.set("layerId", content::PipeJson::String(content::kStandInTilesetLayerId));
  layer.set("viewport", content::PipeJson::String("both"));
  layer.set("visible", content::PipeJson::Bool(false));
  layer.set("opacity", content::PipeJson::Number(1));
  ASSERT_TRUE(Transact(&client, MakeMessage("layer-hide", "layer.set", layer),
                       &response));
  EXPECT_TRUE(ErrorCode(response) == NULL);
  EXPECT_TRUE(peer_.tileset_hidden());
  EXPECT_EQ(2, peer_.additional_cameras());

  ASSERT_FALSE(origin_moved);
  ASSERT_TRUE(peer_.tileset_hidden());
  std::string mark =
      "{\n"
      "  \"suite\": \"browser.world\",\n"
      "  \"additional_cameras\": " +
      std::to_string(peer_.additional_cameras()) +
      ",\n"
      "  \"origin_conflict\": true,\n"
      "  \"origin_moved\": false,\n"
      "  \"tileset.hidden\": true,\n"
      "  \"marks\": [\"tileset.hidden\"]\n"
      "}\n";
  ASSERT_TRUE(WriteMarkFile(L"browser.world", mark));
}
