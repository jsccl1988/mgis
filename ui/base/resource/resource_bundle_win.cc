// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/resource/resource_bundle_win.h"

#include "base/logging.h"
#include "base/path_service.h"
#include "base/utf_string_conversions.h"
#include "ui/base/layout.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_data_dll_win.h"
#include "ui/base/win/dpi.h"

namespace ui {

namespace {

HINSTANCE resources_data_dll;

HINSTANCE GetCurrentResourceDLL() {
  if (resources_data_dll)
    return resources_data_dll;
  return GetModuleHandle(NULL);
}

base::FilePath GetResourcesPakFilePath(const std::string& pak_name) {
  base::FilePath path;
  if (PathService::Get(base::DIR_MODULE, &path))
    return path.AppendASCII(pak_name.c_str());

  // Return just the name of the pack file.
  return base::FilePath(ASCIIToUTF16(pak_name));
}

}  // namespace

void ResourceBundle::LoadCommonResources() {
  // As a convenience, add the current resource module as a data packs.
  data_packs_.push_back(new ResourceDataDLL(GetCurrentResourceDLL()));
  // Optional high-DPI packs when present. Never require touch_* packs: this
  // tree ships ui_resources_100_percent.pak only, and forcing touch_100 after
  // a missing 140/180 pack left the process without a baseline image pack.

  float scale = ui::win::GetDeviceScaleFactor();
  switch (ui::GetScaleFactorFromScale(scale)) {
    case ui::SCALE_FACTOR_200P:
      AddOptionalDataPackFromPath(
          GetResourcesPakFilePath("ui_resources_200_percent.pak"),
          SCALE_FACTOR_200P);
      break;
    case ui::SCALE_FACTOR_180P:
      AddOptionalDataPackFromPath(
          GetResourcesPakFilePath("ui_resources__touch_180_percent.pak"),
          SCALE_FACTOR_180P);
      break;
    case ui::SCALE_FACTOR_140P:
      AddOptionalDataPackFromPath(
          GetResourcesPakFilePath("ui_resources__touch_140_percent.pak"),
          SCALE_FACTOR_140P);
      break;
    default:
      break;
  }
  if (ui::GetDisplayLayout() == ui::LAYOUT_TOUCH) {
    AddOptionalDataPackFromPath(
        GetResourcesPakFilePath("ui_resources__touch_100_percent.pak"),
        SCALE_FACTOR_100P);
  }
  AddDataPackFromPath(
      GetResourcesPakFilePath("ui_resources_100_percent.pak"),
      SCALE_FACTOR_100P);
}

gfx::Image& ResourceBundle::GetNativeImageNamed(int resource_id, ImageRTL rtl) {
  // Flipped image is not used on Windows.
  DCHECK_EQ(rtl, RTL_DISABLED);

  // Windows only uses SkBitmap for gfx::Image, so this is the same as
  // GetImageNamed.
  return GetImageNamed(resource_id);
}

void SetResourcesDataDLL(HINSTANCE handle) {
  resources_data_dll = handle;
}

HICON LoadThemeIconFromResourcesDataDLL(int icon_id) {
  return ::LoadIcon(GetCurrentResourceDLL(), MAKEINTRESOURCE(icon_id));
}

}  // namespace ui;
