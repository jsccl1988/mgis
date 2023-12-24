// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#include "gui/content/delegate/map_box_delegate_impl.h"

#include "gui/browser.h"

namespace gui {
const string16 kChromeCore = L"chrome";
const string16 kTridentCore = L"trident";

static string16 GetBrowserName(
  content::eBrowserID browser_id) {
  switch (browser_id) {
  case content::C_CRHOME_PLUGIN:
    return kChromeCore;
  case content::C_TRIDENT:
  case content::C_TRIDENT_PLUGIN_IE8:
  case content::C_TRIDENT_PLUGIN_IE10:
    return kTridentCore;
  case content::C_MAX:
    return L"";
    break;
  default:
    break;
  }

  return L"";
}

void MapBoxDelegateImpl::PreMainMessageLoopRun() {
  gui::Browser::Instance()->InitScheme();
  gui::Browser::Instance()->InitNetworkInspector();
}

void MapBoxDelegateImpl::GetErrorPage(const std::string& url
  , int32 errorcode, std::string * page_url, std::string * page_content) {
  char16 temp[256];
  swprintf(temp, L"%s%d", L"Error Page,Error Code:"
    , errorcode);
  std::string html_content(temp);
  *page_content = html_content;
}

void MapBoxDelegateImpl::OnBrowserStatus(
  const content::BrowserStatusParams& params) {
  switch (params.status) {
    case content::CS_FAILED: {
      Browser::Instance()->
        StatusChange(params.browser_id, L"browser failed");
    }
    break;
    case content::CS_SUCCESS: {
      CString status;
      status.Format(L"%s browser available"
        , GetBrowserName(params.browser_id).c_str());
      LPCTSTR p_status = (LPCTSTR)status;
      string16 temp_status;
      if (p_status) {
        temp_status = p_status;
      }
      Browser::Instance()->
        StatusChange(params.browser_id, temp_status);

      if (params.browser_id == content::C_CRHOME_PLUGIN
        || params.browser_id == content::C_TRIDENT
        || params.browser_id == content::C_TRIDENT_PLUGIN_IE8
        || params.browser_id == content::C_TRIDENT_PLUGIN_IE10) {
        static bool init = (gui::Browser::Instance()->FirstFrame()
          , true);
      }
    }
    default:
      break;
  }
}

void MapBoxDelegateImpl::OnVersionChange(const std::string& version) {
}
}  // namespace gui
