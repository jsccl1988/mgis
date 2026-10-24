// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include <string>

#include <windows.h>

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "content/browser/harness_campaign.h"
#include "gtest/gtest.h"

namespace {

bool WriteText(const base::FilePath& path, const std::string& text) {
  if (!file_util::CreateDirectory(path.DirName()))
    return false;
  return file_util::WriteFile(path, text.data(),
                              static_cast<int>(text.size())) ==
         static_cast<int>(text.size());
}

base::FilePath MarkPath(const base::FilePath& out_root, const char* suite) {
  return out_root.AppendASCII("harness")
      .AppendASCII(suite)
      .AppendASCII("mark.json");
}

base::FilePath RepoHarnessRoot() {
  wchar_t module[MAX_PATH];
  const DWORD n = GetModuleFileNameW(NULL, module, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return base::FilePath();
  base::FilePath dir(module);
  dir = dir.DirName();
  for (int i = 0; i < 8; ++i) {
    const base::FilePath suite =
        dir.AppendASCII("testing")
            .AppendASCII("tools")
            .AppendASCII("harness")
            .AppendASCII("browser.map")
            .AppendASCII("suite.txt");
    if (file_util::PathExists(suite)) {
      return dir.AppendASCII("testing")
          .AppendASCII("tools")
          .AppendASCII("harness");
    }
    const base::FilePath parent = dir.DirName();
    if (parent == dir)
      break;
    dir = parent;
  }
  return base::FilePath();
}

}  // namespace

TEST(HarnessCampaign, StopsWhenMarkMissing) {
  base::ScopedTempDir temp;
  ASSERT_TRUE(temp.CreateUniqueTempDir());
  const base::FilePath suite_root = temp.path().AppendASCII("suites");
  const base::FilePath out_root = temp.path().AppendASCII("out");

  ASSERT_TRUE(WriteText(
      suite_root.AppendASCII("browser.map").AppendASCII("suite.txt"),
      "mark session.ready\n"));
  ASSERT_TRUE(WriteText(
      suite_root.AppendASCII("browser.map").AppendASCII("required_marks.txt"),
      "pixel_ratio\n"));
  ASSERT_TRUE(WriteText(
      suite_root.AppendASCII("browser.pick").AppendASCII("suite.txt"),
      "mark should_not_run\n"));
  ASSERT_TRUE(WriteText(
      suite_root.AppendASCII("browser.pick").AppendASCII("required_marks.txt"),
      "should_not_run\n"));

  EXPECT_FALSE(content::HarnessCampaign::Run(suite_root, out_root));
  EXPECT_FALSE(file_util::PathExists(MarkPath(out_root, "browser.map")));
  EXPECT_FALSE(file_util::PathExists(MarkPath(out_root, "browser.pick")));
  EXPECT_FALSE(file_util::PathExists(MarkPath(out_root, "browser.draw")));
}

TEST(HarnessCampaign, WritesMarksInOrder) {
  const base::FilePath suite_root = RepoHarnessRoot();
  ASSERT_FALSE(suite_root.empty());

  base::ScopedTempDir temp;
  ASSERT_TRUE(temp.CreateUniqueTempDir());
  const base::FilePath out_root = temp.path().AppendASCII("out");

  EXPECT_TRUE(content::HarnessCampaign::Run(suite_root, out_root));

  std::string map_json;
  std::string pick_json;
  std::string draw_json;
  ASSERT_TRUE(file_util::ReadFileToString(MarkPath(out_root, "browser.map"),
                                          &map_json));
  ASSERT_TRUE(file_util::ReadFileToString(MarkPath(out_root, "browser.pick"),
                                          &pick_json));
  ASSERT_TRUE(file_util::ReadFileToString(MarkPath(out_root, "browser.draw"),
                                          &draw_json));
  EXPECT_NE(std::string::npos, map_json.find("\"pixel_ratio\""));
  const size_t pick_2d = pick_json.find("\"pick.2d\"");
  const size_t pick_3d = pick_json.find("\"pick.3d\"");
  EXPECT_NE(std::string::npos, pick_2d);
  EXPECT_NE(std::string::npos, pick_3d);
  EXPECT_LT(pick_2d, pick_3d);
  const size_t draft_map = draw_json.find("\"draft.basemap\"");
  const size_t draft_ground = draw_json.find("\"draft.groundwater\"");
  EXPECT_NE(std::string::npos, draft_map);
  EXPECT_NE(std::string::npos, draft_ground);
  EXPECT_LT(draft_map, draft_ground);
}
