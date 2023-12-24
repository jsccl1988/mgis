// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.



#include "gui/content/delegate/map_view_delegate_impl.h"

#include <vector>

#include "gui/base/thread/thread_manager.h"

#include "gui/browser.h"
#include "gui/browser_frame.h"

#include "gui/content/content_manager.h"
#include "third_party/bdlog/bdlog.h"

namespace gui {
static const string16 kCrashedRedictURL =
  L"hummingbird://error-pages/crash.html";

MapViewDelegateImpl::MapViewDelegateImpl(int32 id
  , const base::WeakPtr<content::MapView>& map_view)
  : id_(id)
  , map_view_(map_view)
  , full_screen_(false) {
}

MapViewDelegateImpl::~MapViewDelegateImpl() {
}
// notify display stuff
void MapViewDelegateImpl::OnTitleChange(const std::string &url
  , const std::string &title) {
  Browser::Instance()->TitleChange(id_, title);
}

void MapViewDelegateImpl::OnStatusTextChange(const std::string &status_text) {
  Browser::Instance()->StatusChange(id_, status_text);
}

void MapViewDelegateImpl::OnURLChange(const std::string &new_url
  , const std::string &old_url) {
  Browser::Instance()->URLChange(id_, new_url);
}

void MapViewDelegateImpl::OnZoomChange(int32 size) {
}

void MapViewDelegateImpl::OnGetPageZoomFactor(const std::string &url
  , int32 * zoom_size) {
  *zoom_size = 100;
}

void MapViewDelegateImpl::OnBackForwardStatusChange(bool enable_back
  , bool enable_forward) {
}

// audio
void MapViewDelegateImpl::OnAudioPlayStateChange(bool played) {
}

void MapViewDelegateImpl::OnBrowserChange(content::eMapBox type
  , bool unrecommended) {
  Browser::Instance()->OnBrowserChange(id_, type);
}

void MapViewDelegateImpl::OnSwitchComplete(content::eMapBox type
  , content::eCoreSwitchErrorCode error_code) {
}

// notify navigate stage
void MapViewDelegateImpl::OnBeforeNavigate(const std::string &url
  , bool mainframe, bool user_nav, const std::string &http_head
  , const std::string &post_data) {
}

void MapViewDelegateImpl::OnCanContinueNavigate(const std::string &url
  , bool mainframe, bool navigate, const std::string &http_head
  , const std::string &post_data, bool * cancel) {
  content::MapBox * map_box = NULL;
  MapContentManager::Instance()->GetMapBox(&map_box);
  if (!map_box) {
    assert(false);
    return;
  }

  content::WebSyncReply * web_sync_reply = NULL;
  map_box->QueryWebSyncReply(&web_sync_reply);
  if (!web_sync_reply) {
    assert(false);
    return;
  }

  uint32 reply_id = kUint32Max;
  web_sync_reply->PrepareDelayExecuteReply(&reply_id);
  ThreadManager::PostTask(ThreadManager::UI
    , std::bind(&MapViewDelegateImpl::DelayOnCanContinueNavigate
    , this, reply_id, url, mainframe, navigate, http_head, post_data));
}

void MapViewDelegateImpl::OnDocumentComplete(const std::string &url) {
}

void MapViewDelegateImpl::OnNavigateComplete(content::eMapBox type
  , const content::NavigateCompleteParams &params) {
}

void MapViewDelegateImpl::OnNavigateError(uint32 error_code
  , const std::string &url) {
}

void MapViewDelegateImpl::OnNavigateRedirect(const std::string &from
  , const std::string &to, const std::string &original_url
  , const std::string &original_reffer) {
}

void MapViewDelegateImpl::OnNavigationHistoryUpdate(content::eMapBox type
  , const content::NavigateHistoryParams &params) {
}

void MapViewDelegateImpl::OnCloudSript(content::eCloudScriptType script_type
  , const std::string& url, const std::string& refer
  , content::eMapBox browser_type, uint32 context_id) {
  BrowserFrame * browser_frame = Browser::Instance()->GetBrowserFrame(id_);
  if (browser_frame) {
    browser_frame->OnCloudSript(script_type, url, refer, browser_type,
      context_id);
  }
}

void MapViewDelegateImpl::OnCanBeJSClose(bool * can) {
  *can = true;
}

void MapViewDelegateImpl::FullScreen(bool enable) {
  full_screen_ = enable;
}
void MapViewDelegateImpl::OnGetFullScreenStatus(bool * isfull) {
  *isfull = full_screen_;
}
// notify network stage
void MapViewDelegateImpl::OnDownloadBegin(const std::string& url) {
}

void MapViewDelegateImpl::OnDownloadEnd(const std::string& url) {
}

void MapViewDelegateImpl::OnRequestComplete(const std::string& json_info) {
}

void MapViewDelegateImpl::OnResourcePrefetchFired(
  const content::ResourcePrefetchInfo& prefetch_info) {
}

void MapViewDelegateImpl::OnPageLoadStart() {
}

void MapViewDelegateImpl::OnPageLoadComplete(int64 load_time) {
}

// notify ssl stuff
void MapViewDelegateImpl::OnSSLStatus(content::eSSLStatus status) {
}

void MapViewDelegateImpl::OnMixedContentStatus(const std::string& secure_url
  , const std::string& resource_url, bool allowed) {
}

// notify find stuff
void MapViewDelegateImpl::OnFindAll(int32 current_index, int32 total_find) {
}

void MapViewDelegateImpl::OnFindNext(int32 current_index, int32 total_find
  , const content::FindParams& params) {
}

// notify window message
void MapViewDelegateImpl::OnWindowMessage(uint32 msg, uint64 wparam
  , uint64 lparam) {
}

void MapViewDelegateImpl::OnWindowClosing() {
  Browser::Instance()->CloseWindow(id_);
}

void MapViewDelegateImpl::OnFESetWindowSize(uint32 width, uint32 height) {
}

// window stuff delegate
void MapViewDelegateImpl::OnNewWindow(const content::NewWindowParams &params
  , uint32 * new_view_id
  , uint32 * window_id) {
  Browser::Instance()->
    NewWindow(params.sync ? id_ : kUint32Max, params.url, new_view_id);
}

void MapViewDelegateImpl::OnDrag(const std::string &content
  , content::eDragType flag, const base::vectorx<uint8>& keycodes) {
  switch (flag) {
  case content::DT_TEXT:
    break;
  case content::DT_IMG:
    break;
  case content::DT_HREF: {
      uint32 new_id = kUint32Max;
      Browser::Instance()->NewWindow(kUint32Max, content, &new_id);
    }
    break;
  default:
    break;
  }
}

void MapViewDelegateImpl::OnContextMenuShow(
  const content::ContextMenuParams &params
  , base::vectorx<content::MenuItem>* default_items, bool * override_ui) {
  bool redraw = false;
  if (redraw) {
    HMENU menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, 1, L"gui item 1");
    AppendMenu(menu, MF_STRING, 2, L"gui item 2");
    AppendMenu(menu, MF_STRING, 3, L"gui item 3");
    UINT flags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;
    TrackPopupMenu(menu, flags, params.xcoord, params.ycoord, 0,
      Browser::Instance()->GetBrowserFrame(id_)->m_hWnd,
      NULL);
  } else {
    if (default_items) {
      default_items->append(content::MenuItem(
        content::MENUITEMTYPE_COMMAND
        , 4433, L"测试专用"));

      default_items->append(content::MenuItem(
        content::MENUITEMTYPE_SEPARATOR
        , 4434, L"华丽的分割线"));

      default_items->append(content::MenuItem(
        content::MENUITEMTYPE_CHECK
        , 4435, L"Check_1"));

      content::MenuItem item(
        content::MENUITEMTYPE_SUBMENU
        , 5544, L"测试SubMenu");
      base::vectorx<content::MenuItem> sub_items;
      sub_items.append(content::MenuItem(
        content::MENUITEMTYPE_RADIO
        , 55441, L"radio", 2));
      sub_items.append(content::MenuItem(
        content::MENUITEMTYPE_RADIO
        , 55442, L"radio", 2));

      item.sub_items = sub_items;

      default_items->append(item);
      *override_ui = false;
    }
  }
}

void MapViewDelegateImpl::OnContextMenuCommand(
  const content::ContextMenuParams &params, int32 command_id) {
}

void MapViewDelegateImpl::OnContextMenuDismissed() {
}

void MapViewDelegateImpl::OnPermissionBar(int32 type
  , const std::string &url, const std::string &tip
  , content::WebPermissionCallback* callback) {
  int32 choice = ::MessageBox(0, tip.c_str(), url.c_str(), MB_YESNO);
  if (choice == IDYES) {
    callback->OnPermission(content::PC_ACCEPT);
  } else {
    callback->OnPermission(content::PC_DENY);
  }
}

void MapViewDelegateImpl::OnJSPopupWindow(content::eJSPopWindowType pop_type
  , const std::string &message, const std::string &text
  , content::JSPopupWindowResult * result) {
  switch (pop_type) {
  case content::JSPWT_ALERT: {
    ::MessageBoxW(0, message.c_str(), L"ALERT", MB_OK);
    return;
  }
  case content::JSPWT_CONFIRM: {
    result->arg0 =
      ::MessageBoxW(0, message.c_str(), L"CONFIRM", MB_YESNO) == IDYES;
    return;
  }
  case content::JSPWT_PROMOTE: {
    result->arg0 =
      ::MessageBoxW(0, message.c_str(), L"PROMOTE", MB_YESNO) == IDYES;
    result->arg1 = L"done";
    return;
  }
  default:
    break;
  }
}

void MapViewDelegateImpl::OnRenderProcessCrashed(
  content::eMapBox browser_type, const std::string &url) {
  map_view_->Recover(content::RT_SWITCH);
}

void MapViewDelegateImpl::OnRecoverFailed(content::eRecoverType type) {
  content::NavigateParams navigate_params(kCrashedRedictURL);
  map_view_->Navigate(navigate_params);
}

void MapViewDelegateImpl::OnAuthenticationRequired(const std::string &host
  , const std::string &default_user
  , bool * cancel, std::string * user, std::string * password) {
  *cancel = false;
  // *user = L"XXXX";
  // *password = L"XXXX";
}

// flash popup stuff
void MapViewDelegateImpl::OnAddMediaInfo(uint32 player_id,
  const std::string& url, int64 size, const std::string& filename,
  const std::string& type) {
  BrowserFrame * browser_frame = Browser::Instance()->GetBrowserFrame(id_);
  if (browser_frame) {
    browser_frame->OnAddMediaInfo(player_id, url, size, filename, type);
  }
}

void MapViewDelegateImpl::OnClearMediaInfo(void) {
  BrowserFrame * browser_frame = Browser::Instance()->GetBrowserFrame(id_);
  if (browser_frame) {
    browser_frame->OnClearMediaInfo();
  }
}

void MapViewDelegateImpl::OnHideMediaTipBar(uint32 player_id,
  bool immediately) {
  BrowserFrame * browser_frame = Browser::Instance()->GetBrowserFrame(id_);
  if (browser_frame) {
    browser_frame->OnHideMediaTipBar(player_id, immediately);
  }
}

void MapViewDelegateImpl::OnShowMediaTipBarInfo(uint32 player_id, int32 left,
  int32 top, int32 right, int32 bottom) {
  BrowserFrame * browser_frame = Browser::Instance()->GetBrowserFrame(id_);
  if (browser_frame) {
    browser_frame->OnShowMediaTipBarInfo(player_id, left, top, right, bottom);
  }
}

void MapViewDelegateImpl::OnFlashSetFullScreen(bool enable) {
  BrowserFrame * browser_frame = Browser::Instance()->GetBrowserFrame(id_);
  if (browser_frame) {
    browser_frame->OnFlashSetFullScreen(enable);
  }
}

void MapViewDelegateImpl::OnFlashUDPDataRecevied(const base::Rect& rect
  , int32 instance_id
  , uint32 byte_count) {
}

void MapViewDelegateImpl::OnCheckPopupWindowBlock(const std::string& url
  , bool * block) {
  *block = false;
}

void MapViewDelegateImpl::OnPopupWindowBlockListChanged(
  const base::vectorx<std::string> &urls) {
}

void MapViewDelegateImpl::OnPopupWindowBlockCountChanged(
  const std::string &from_url, uint32 count) {
}

void MapViewDelegateImpl::OnCheckURLSafe(const std::string& url, bool* safe) {
  *safe = true;
}

void MapViewDelegateImpl::OnNavigateUnSafeURL(const std::string &url) {
}

void MapViewDelegateImpl::OnGetSuggestInfo(const std::string& url,
  const std::string& key) {
}

void MapViewDelegateImpl::OnDeleteSuggestInfo(const std::string& url,
  const std::string& name, const std::string& value) {
}

void MapViewDelegateImpl::OnFetchFormData(const std::string& url) {
}

void MapViewDelegateImpl::OnSubmitFormData(
  const content::FormDataContent& form_data) {
}

void MapViewDelegateImpl::OnNotifyCharset(const std::string& encoding) {
}

void MapViewDelegateImpl::OnNotifyPrerenderSwitched(int64 time
  , bool baidu_search_prerender) {
}

void MapViewDelegateImpl::OnNotifyRendererUnresponsive(
  const std::string& url) {
}

void MapViewDelegateImpl::OnNotifySensitiveWordCheckResult(bool has_word) {
}

void MapViewDelegateImpl::OnRequestFavoIcon(const std::string& page_url
  , const base::vectorx<content::FavoIconItemInfo>& icons) {
#ifdef _DEBUG
  string16 str_page_url = page_url.ToString16();
  uint32 size = icons.size();
  for (uint32 i = 0; i < size; ++i) {
    content::FavoIconItemInfo item_info;
    icons.get(i, &item_info);
    string16 str_rel = item_info.rel.ToString16();
    string16 str_type = item_info.type.ToString16();
    string16 str_sizes = item_info.sizes.ToString16();
    string16 str_icon_url = item_info.icon_url.ToString16();
    Log(LL_DEBUG, TAG(L"Demo")
      , _T("FUNC:%s, icon_url:%s, page_url:%s")
      , __FUNCTIONW__, str_icon_url.c_str(), str_page_url.c_str());
  }
#endif
}

// notify to launch external protocal url
void MapViewDelegateImpl::OnLaunchExternalProtocol(
  const std::string& external_url
  , const std::string& recommend_exe_path
  , content::MapViewCommonCallback* callback) {
  Log(LL_DEBUG, TAG(L"Demo"), _T("FUNC:%s, external_url:%s"), __FUNCTIONW__
    , external_url.c_str());
  int32 choice = ::MessageBox(0, L"Are you allow Launch External Application?"
    , L"Warn", MB_YESNO);
  bool allow = (choice == IDYES);
#define USE_EXTERNAL_APP 0
#if USE_EXTERNAL_APP
  callback->LaunchExternalProtocolPolicy(allow
    , L"C:\\Users\\tyg\\AppData\\Local\\Baidu\\BaiduClient\\2.5.19.2443\\Baidu.exe"
    , L"--debug=service"
    , external_url);
#else
  callback->LaunchExternalProtocolPolicy(allow, recommend_exe_path, L""
    , external_url);
#endif
#undef USE_EXTERNAL_APP
}

void MapViewDelegateImpl::OnCheckCompatView(const std::string& url
  , bool navigate_complete) {
}

void MapViewDelegateImpl::OnNotifyCompatView(bool compatible) {
  if (compatible) {
    // do something
  }
}

void MapViewDelegateImpl::OnNotifyClickElement(int32 x, int32 y
  , const base::vectorx<std::string> & tag_names) {
}

void MapViewDelegateImpl::DelayOnCanContinueNavigate(uint32 reply_id
  , const std::string &url, bool mainframe, bool navigate
  , const std::string &http_head, const std::string &post_data) {
  content::MapBox * map_box = NULL;
  MapContentManager::Instance()->GetMapBox(&map_box);
  if (!map_box) {
    assert(false);
    return;
  }

  content::WebSyncReply * web_sync_reply = NULL;
  map_box->QueryWebSyncReply(&web_sync_reply);
  if (!web_sync_reply) {
    assert(false);
    return;
  }

  web_sync_reply->ReplyCanContinueNavigate(reply_id, false);
}
}  // namespace gui
