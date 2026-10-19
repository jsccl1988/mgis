// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "plugin/contribute.h"
#include "plugin/registry.h"

#include <stdio.h>

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "gtest/gtest.h"

namespace {

std::wstring ModulePath() {
  std::wstring buf(32768, L'\0');
  DWORD n = GetModuleFileNameW(NULL, &buf[0], static_cast<DWORD>(buf.size()));
  if (n == 0 || n >= buf.size())
    return std::wstring();
  buf.resize(n);
  return buf;
}

std::wstring ParentDir(const std::wstring& path) {
  const size_t slash = path.find_last_of(L"\\/");
  if (slash == std::wstring::npos || slash == 0)
    return std::wstring();
  return path.substr(0, slash);
}

bool FileExists(const std::wstring& path) {
  const DWORD attr = GetFileAttributesW(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES &&
         (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool IsRepoRoot(const std::wstring& dir) {
  return FileExists(dir + L"\\BUILD.gn") &&
         FileExists(dir + L"\\plugin\\sample\\plugin.json");
}

std::wstring FindRepoRootFrom(std::wstring dir) {
  for (int i = 0; i < 8 && !dir.empty(); ++i) {
    if (IsRepoRoot(dir))
      return dir;
    const std::wstring parent = ParentDir(dir);
    if (parent.empty() || parent == dir)
      break;
    dir = parent;
  }
  return std::wstring();
}

std::wstring RepoRoot() {
  const std::wstring from_exe = FindRepoRootFrom(ParentDir(ModulePath()));
  if (!from_exe.empty())
    return from_exe;
  wchar_t cwd[MAX_PATH];
  const DWORD n = GetCurrentDirectoryW(MAX_PATH, cwd);
  if (n == 0 || n >= MAX_PATH)
    return std::wstring();
  return FindRepoRootFrom(cwd);
}

std::string WideToUtf8(const std::wstring& wide) {
  if (wide.empty())
    return std::string();
  const int n = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
                                    static_cast<int>(wide.size()), NULL, 0,
                                    NULL, NULL);
  if (n <= 0)
    return std::string();
  std::string out(static_cast<size_t>(n), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
                      &out[0], n, NULL, NULL);
  return out;
}

bool EnsureDirectory(const std::wstring& path) {
  if (path.empty())
    return false;
  const DWORD attr = GetFileAttributesW(path.c_str());
  if (attr != INVALID_FILE_ATTRIBUTES)
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  const size_t slash = path.find_last_of(L"\\/");
  if (slash != std::wstring::npos && slash > 0) {
    if (!EnsureDirectory(path.substr(0, slash)))
      return false;
  }
  if (CreateDirectoryW(path.c_str(), NULL))
    return true;
  return GetLastError() == ERROR_ALREADY_EXISTS;
}

bool ReadFileBytes(const std::wstring& path, std::string* out) {
  HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  LARGE_INTEGER size;
  if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 ||
      size.QuadPart > 1024 * 1024) {
    CloseHandle(file);
    return false;
  }
  out->assign(static_cast<size_t>(size.QuadPart), '\0');
  DWORD read = 0;
  BOOL ok = TRUE;
  if (!out->empty()) {
    ok = ReadFile(file, &(*out)[0], static_cast<DWORD>(out->size()), &read,
                  NULL);
  }
  CloseHandle(file);
  return ok == TRUE && read == out->size();
}

bool WriteFileBytes(const std::wstring& path, const std::string& text) {
  if (!EnsureDirectory(ParentDir(path)))
    return false;
  HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  DWORD written = 0;
  BOOL ok = TRUE;
  if (!text.empty()) {
    ok = WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written,
                   NULL);
  }
  CloseHandle(file);
  return ok == TRUE && written == text.size();
}

int CountOf(const std::string& haystack, const std::string& needle) {
  int count = 0;
  size_t pos = 0;
  while ((pos = haystack.find(needle, pos)) != std::string::npos) {
    ++count;
    pos += needle.size();
  }
  return count;
}

bool IncludeLineForbidden(const std::string& line) {
  size_t i = 0;
  while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
    ++i;
  if (line.compare(i, 8, "#include") != 0)
    return false;
  const char* kForbidden[] = {
      "content/browser", "ui/views", "maplibre", "MapLibre",
      "Unreal",          "ue/",      "GisContentsClient",
  };
  for (size_t n = 0; n < sizeof(kForbidden) / sizeof(kForbidden[0]); ++n) {
    if (line.find(kForbidden[n]) != std::string::npos)
      return true;
  }
  return false;
}

bool PluginSourcesPassIncludeCheck(const std::wstring& plugin_dir,
                                   std::string* error) {
  const wchar_t* kFiles[] = {
      L"contribute.h", L"contribute.cc", L"registry.h", L"registry.cc",
  };
  for (size_t i = 0; i < sizeof(kFiles) / sizeof(kFiles[0]); ++i) {
    const std::wstring path = plugin_dir + L"\\" + kFiles[i];
    std::string text;
    if (!ReadFileBytes(path, &text)) {
      *error = "missing " + WideToUtf8(path);
      return false;
    }
    size_t start = 0;
    while (start < text.size()) {
      size_t end = text.find('\n', start);
      if (end == std::string::npos)
        end = text.size();
      std::string line = text.substr(start, end - start);
      if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
      if (IncludeLineForbidden(line)) {
        *error = WideToUtf8(path) + ": " + line;
        return false;
      }
      start = end + 1;
    }
  }
  return true;
}

// In-test hit table. Both views share one feature id and one llh.
// No UE scene and no MapLibre map.
class FeaturePickStub {
 public:
  FeaturePickStub(const std::string& feature_id, double lon, double lat,
                  double height_m)
      : feature_id_(feature_id),
        lon_(lon),
        lat_(lat),
        height_m_(height_m),
        map_hits_(0),
        view0_hits_(0),
        camera_lon_(0.0),
        camera_lat_(0.0),
        camera_h_(0.0),
        selected_(false) {}

  bool SelectionPoint(const std::string& view, std::string* feature_id) {
    if (view == "map")
      ++map_hits_;
    else if (view == "view0")
      ++view0_hits_;
    else
      return false;
    *feature_id = feature_id_;
    return true;
  }

  bool EntitySelect(const std::string& feature_id) {
    if (feature_id != feature_id_)
      return false;
    camera_lon_ = lon_;
    camera_lat_ = lat_;
    camera_h_ = height_m_;
    selected_ = true;
    return true;
  }

  int map_hits() const { return map_hits_; }
  int view0_hits() const { return view0_hits_; }
  double camera_lon() const { return camera_lon_; }
  double camera_lat() const { return camera_lat_; }
  double camera_h() const { return camera_h_; }
  bool selected() const { return selected_; }
  const std::string& feature_id() const { return feature_id_; }

 private:
  std::string feature_id_;
  double lon_;
  double lat_;
  double height_m_;
  int map_hits_;
  int view0_hits_;
  double camera_lon_;
  double camera_lat_;
  double camera_h_;
  bool selected_;
};

bool ReadJsonNumber(const std::string& text, const char* key, double* out) {
  const std::string pattern = std::string("\"") + key + "\": ";
  const size_t pos = text.find(pattern);
  if (pos == std::string::npos)
    return false;
  return sscanf(text.c_str() + pos + pattern.size(), "%lf", out) == 1;
}

}  // namespace

TEST(PluginSample, LoadsManifestAndRunsCommand) {
  const std::wstring root = RepoRoot();
  ASSERT_FALSE(root.empty());

  std::string include_error;
  ASSERT_TRUE(PluginSourcesPassIncludeCheck(root + L"\\plugin", &include_error))
      << include_error;

  plugin::Registry registry;
  const std::string sample_dir = WideToUtf8(root + L"\\plugin\\sample");
  ASSERT_FALSE(sample_dir.empty());
  ASSERT_TRUE(registry.Scan(sample_dir)) << sample_dir;

  const plugin::Manifest* manifest = registry.Find("plugin.sample");
  ASSERT_TRUE(manifest != NULL);
  EXPECT_EQ(std::string("Sample"), manifest->name);
  ASSERT_EQ(static_cast<size_t>(1), manifest->commands.size());
  EXPECT_EQ(std::string("plugin.sample.run"), manifest->commands[0]);

  EXPECT_FALSE(plugin::Contribute(NULL));
  EXPECT_FALSE(plugin::Contribute(""));
  EXPECT_FALSE(plugin::Contribute("plugin.sample.missing"));
  ASSERT_TRUE(plugin::Contribute(manifest->commands[0].c_str()));

  const std::wstring mark_path =
      root + L"\\out\\harness\\plugin.sample\\mark.json";
  const std::string mark =
      "{\n"
      "  \"command\": \"plugin.sample.run\",\n"
      "  \"marks\": [\"plugin.sample.ran\"]\n"
      "}\n";
  ASSERT_TRUE(WriteFileBytes(mark_path, mark));

  std::string written;
  ASSERT_TRUE(ReadFileBytes(mark_path, &written));
  EXPECT_NE(std::string::npos, written.find("plugin.sample.ran"));
}

TEST(BrowserPick, SameFeatureIdOnMapAndView0) {
  const std::wstring root = RepoRoot();
  ASSERT_FALSE(root.empty());

  const double kLon = 116.25;
  const double kLat = 39.5;
  const double kHeightM = 20.0;
  const std::string kFeatureId = "fid-7c2e";

  FeaturePickStub stub(kFeatureId, kLon, kLat, kHeightM);
  std::string map_id;
  std::string view0_id;
  ASSERT_TRUE(stub.SelectionPoint("map", &map_id));
  ASSERT_TRUE(stub.SelectionPoint("view0", &view0_id));
  EXPECT_EQ(1, stub.map_hits());
  EXPECT_EQ(1, stub.view0_hits());
  EXPECT_EQ(map_id, view0_id);
  EXPECT_EQ(kFeatureId, map_id);
  EXPECT_FALSE(stub.SelectionPoint("view1", &map_id));

  ASSERT_TRUE(stub.EntitySelect(view0_id));
  EXPECT_TRUE(stub.selected());
  EXPECT_NEAR(kLon, stub.camera_lon(), 1e-6);
  EXPECT_NEAR(kLat, stub.camera_lat(), 1e-6);
  EXPECT_NEAR(kHeightM, stub.camera_h(), 1.0);

  char camera[160];
  snprintf(camera, sizeof(camera),
           "  \"camera_lon\": %.10f,\n"
           "  \"camera_lat\": %.10f,\n"
           "  \"camera_h\": %.10f\n",
           stub.camera_lon(), stub.camera_lat(), stub.camera_h());

  const std::string mark =
      std::string("{\n") +
      "  \"tools\": [\"selection.point\", \"entity.select\"],\n"
      "  \"featureId\": \"" + kFeatureId + "\",\n"
      "  \"views\": [\"map\", \"view0\"],\n" +
      camera + "}\n";

  const std::wstring mark_path =
      root + L"\\out\\harness\\browser.pick\\mark.json";
  ASSERT_TRUE(WriteFileBytes(mark_path, mark));

  std::string written;
  ASSERT_TRUE(ReadFileBytes(mark_path, &written));
  EXPECT_EQ(1, CountOf(written, kFeatureId));
  EXPECT_NE(std::string::npos, written.find("\"map\""));
  EXPECT_NE(std::string::npos, written.find("\"view0\""));

  double lon = 0.0;
  double lat = 0.0;
  double height_m = 0.0;
  ASSERT_TRUE(ReadJsonNumber(written, "camera_lon", &lon));
  ASSERT_TRUE(ReadJsonNumber(written, "camera_lat", &lat));
  ASSERT_TRUE(ReadJsonNumber(written, "camera_h", &height_m));
  EXPECT_NEAR(kLon, lon, 1e-6);
  EXPECT_NEAR(kLat, lat, 1e-6);
  EXPECT_NEAR(kHeightM, height_m, 1.0);
}
