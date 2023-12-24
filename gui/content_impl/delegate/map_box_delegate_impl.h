// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef DEMO_WEB_RENDER_DELEGATE_MAP_BOX_DELEGATE_IMPL_H_
#define DEMO_WEB_RENDER_DELEGATE_MAP_BOX_DELEGATE_IMPL_H_

#include "delegate/map_box_delegate.h"

namespace gui {
class MapBoxDelegateImpl :public content::MapBoxDelegate {
 public:
  MapBoxDelegateImpl() {}
  virtual ~MapBoxDelegateImpl() {}

 public:
  void PreMainMessageLoopRun() override;
  void GetErrorPage(const std::string& url
    , int32 errorcode, std::string * page_url
    , std::string * page_content) override;
  void OnBrowserStatus(
    const content::BrowserStatusParams& status) override;
  void OnVersionChange(const std::string& version) override;
};
}  // namespace gui
#endif  // DEMO_WEB_RENDER_DELEGATE_MAP_BOX_DELEGATE_IMPL_H_
