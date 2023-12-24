// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#ifndef DEMO_WEB_RENDER_DELEGATE_MAP_VIEW_DELEGATE_IMPL_H_
#define DEMO_WEB_RENDER_DELEGATE_MAP_VIEW_DELEGATE_IMPL_H_

#include <vector>

#include "content/public/frame/map_view.h"
#include "delegate/map_view_delegate.h"
namespace gui {
class MapViewDelegateImpl:public content::MapViewDelegate {
 public:
  MapViewDelegateImpl(int32 id
    , const base::WeakPtr<content::MapView>& map_view);
  virtual ~MapViewDelegateImpl();

 public:
  // notify display stuff
  void OnTitleChange(const std::string &url
    , const std::string &title) override;
  void OnStatusTextChange(const std::string &status_text) override;
  void OnURLChange(const std::string &new_url
    , const std::string &old_url) override;
  void OnZoomChange(int32 size) override;
  void OnGetPageZoomFactor(const std::string &url
    , int32 * zoom_size) override;
  void OnBrowserChange(content::eMapBox type
    , bool unrecommended) override;
  void OnSwitchComplete(content::eMapBox type
    , content::eCoreSwitchErrorCode error_code) override;
  void OnBackForwardStatusChange(bool enable_back
    , bool enable_forward) override;
  // audio
  void OnAudioPlayStateChange(bool played) override;

  // notify navigate stage
  void OnBeforeNavigate(const std::string &url, bool mainframe
    , bool user_nav, const std::string &http_head
    , const std::string &post_data) override;
  void OnCanContinueNavigate(const std::string &url, bool mainframe
    , bool navigate, const std::string &http_head
    , const std::string &post_data, bool * cancel) override;
  void OnDocumentComplete(const std::string &url) override;
  void OnNavigateComplete(content::eMapBox type
    , const content::NavigateCompleteParams
      &params) override;
  void OnNavigateError(uint32 error_code, const std::string &url) override;
  void OnNavigateRedirect(const std::string &from, const std::string &to
    , const std::string &original_url
    , const std::string &original_reffer) override;

  void OnNavigationHistoryUpdate(content::eMapBox type
    , const content::NavigateHistoryParams &params) override;

  void OnCloudSript(content::eCloudScriptType script_type
    , const std::string& url, const std::string& refer
    , content::eMapBox browser_type, uint32 context_id) override;

  void OnCanBeJSClose(bool * can) override;

  void FullScreen(bool enable) override;
  void OnGetFullScreenStatus(bool * isfull) override;

  // notify network stage
  void OnDownloadBegin(const std::string& url) override;
  void OnDownloadEnd(const std::string& url) override;
  void OnRequestComplete(const std::string& json_info) override;
  void OnResourcePrefetchFired(
    const content::ResourcePrefetchInfo& prefetch_info) override;
  void OnPageLoadStart() override;
  void OnPageLoadComplete(int64 load_time) override;

  // notify ssl stuff
  void OnSSLStatus(content::eSSLStatus status) override;
  void OnMixedContentStatus(const std::string& secure_url
    , const std::string& resource_url, bool allowed) override;

  // notify find stuff
  void OnFindAll(int32 current_index, int32 total_find) override;
  void OnFindNext(int32 current_index, int32 total_find
    , const content::FindParams& params) override;

  // notify window message
  void OnWindowMessage(uint32 msg, uint64 wparam, uint64 lparam) override;
  void OnWindowClosing() override;
  void OnFESetWindowSize(uint32 width, uint32 height) override;

  // window stuff delegate
  void OnNewWindow(const content::NewWindowParams &params
    , uint32 * new_view_id
    , uint32 * window_id) override;
  void OnDrag(const std::string &keyword, content::eDragType flag
    , const base::vectorx<uint8>& keycodes) override;
  void OnContextMenuShow(const content::ContextMenuParams &params
    , base::vectorx<content::MenuItem>* default_items
    , bool * override_ui) override;
  void OnContextMenuCommand(const content::ContextMenuParams &params
    , int32 command_id) override;
  void OnContextMenuDismissed() override;
  void OnPermissionBar(int32 type, const std::string &url
    , const std::string &tip
    , content::WebPermissionCallback* callback) override;
  void OnJSPopupWindow(content::eJSPopWindowType pop_type
    , const std::string &message, const std::string &text
    , content::JSPopupWindowResult * result) override;

  void OnRenderProcessCrashed(content::eMapBox browser_type
    , const std::string &url) override;
  void OnRecoverFailed(content::eRecoverType type) override;

  // authentication stuff
  void OnAuthenticationRequired(const std::string &host
    , const std::string &default_user
    , bool * cancel, std::string * user, std::string * password) override;

  // flash popup stuff
  void OnAddMediaInfo(uint32 player_id, const std::string& url, int64 size,
    const std::string& filename, const std::string& type) override;
  void OnClearMediaInfo(void) override;
  void OnHideMediaTipBar(uint32 player_id, bool immediately) override;
  void OnShowMediaTipBarInfo(uint32 player_id, int32 left, int32 top,
    int32 right, int32 bottom) override;
  void OnFlashSetFullScreen(bool enable) override;
  void OnFlashUDPDataRecevied(const base::Rect& rect, int32 instance_id
    , uint32 byte_count) override;

  void OnCheckPopupWindowBlock(const std::string& url, bool * block) override;
  void OnPopupWindowBlockListChanged(
    const base::vectorx<std::string> &urls) override;
  void OnPopupWindowBlockCountChanged(
    const std::string &url, uint32 count) override;

  void OnCheckURLSafe(const std::string& url, bool* safe) override;
  void OnNavigateUnSafeURL(const std::string &url) override;

  // auto fill form
  void OnGetSuggestInfo(const std::string& url,
    const std::string& key) override;
  void OnDeleteSuggestInfo(const std::string& url,
    const std::string& name, const std::string& value) override;
  void OnFetchFormData(const std::string& url) override;
  void OnSubmitFormData(const content::FormDataContent& form_data) override;

  void OnNotifyCharset(const std::string& encoding) override;
  void OnNotifyPrerenderSwitched(int64 time
    , bool baidu_search_prerender) override;
  void OnNotifyRendererUnresponsive(const std::string& url) override;
  void OnNotifySensitiveWordCheckResult(bool has_word) override;

  // notify to request favoicon
  void OnRequestFavoIcon(const std::string& page_url
    , const base::vectorx<content::FavoIconItemInfo>& icons) override;

  // notify to launch external protocal url
  void OnLaunchExternalProtocol(const std::string& external_url
    , const std::string& recommend_exe_path
    , content::MapViewCommonCallback* callback) override;

  void OnCheckCompatView(const std::string& url
    , bool navigate_complete) override;
  void OnNotifyCompatView(bool compatible) override;

  void OnNotifyClickElement(int32 x, int32 y
    , const base::vectorx<std::string> & tag_names) override;

 private:
  void DelayOnCanContinueNavigate(uint32 reply_id, const std::string &url
    , bool mainframe, bool navigate, const std::string &http_head
    , const std::string &post_data);

 private:
  int32 id_;
  bool full_screen_;
  base::WeakPtr<content::MapView> map_view_;
};
}  // namespace gui
#endif  // DEMO_WEB_RENDER_DELEGATE_MAP_VIEW_DELEGATE_IMPL_H_
