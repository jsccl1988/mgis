// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#ifndef DEMO_WEB_RENDER_DELEGATE_WEB_RENDER_CLIENT_IMPL_H_
#define DEMO_WEB_RENDER_DELEGATE_WEB_RENDER_CLIENT_IMPL_H_

#include "delegate/content_client.h"

namespace gui {
class MapContentClientImpl :public content::MapContentClient {
 public:
  MapContentClientImpl() {}
  virtual ~MapContentClientImpl() {}

 public:
  // MapBoxDelegate
  void QueryMapBoxDelegate(
    content::MapBoxDelegate ** delegate) override;

  // MapViewDelegate
  void QueryMapViewDelegate(uint32 id
    , content::MapViewDelegate ** delegate) override;

  // WebDownloadDelegate
  void QueryWebDownloadDelegate(
    content::WebDownloadDelegate ** delegate) override;

  // WebURLFetcherDelegate
  void QueryWebURLFetcherDelegate(
    content::WebURLFetcherDelegate ** delegate) override;

  // WebCookieDelegate
  void QueryWebCookieDelegate(
    content::WebCookieDelegate ** delegate) override;

  // WebExtensionDelegate
  void QueryWebExtensionDelegate(
    content::WebExtensionDelegate ** delegate) override;

  // WebFormDelegate
  void QueryWebFormDelegate(
    content::WebFormDelegate ** delegate) override;

  // WebHTML5Delegate
  void QueryWebHTML5Delegate(
    content::WebHTML5Delegate ** delegate) override;

  // WebJSCmdDelegate
  void QueryWebJSCmdDelegate(
    content::WebJSCmdDelegate ** delegate) override;

  // WebNetworkDelegate
  void QueryWebNetworkDelegate(
    content::WebNetworkDelegate ** delegate) override;

  // WebPluginManagerDelegate
  void QueryWebPluginManagerDelegate(
    content::WebPluginManagerDelegate** delegate) override;

 private:
  base::ScopedRefPtr<content::MapBoxDelegate> map_box_delegate_;
  base::ScopedRefPtr<content::WebCookieDelegate> web_cookie_delegate_;
  base::ScopedRefPtr<content::WebDownloadDelegate>
    web_download_delegate_;
  base::ScopedRefPtr<content::WebURLFetcherDelegate>
    web_url_fetcher_delegate_;
  base::ScopedRefPtr<content::WebExtensionDelegate> web_extension_delegate_;
  base::ScopedRefPtr<content::WebFormDelegate> web_form_delegate_;
  base::ScopedRefPtr<content::WebHTML5Delegate> web_html5_delegate_;
  base::ScopedRefPtr<content::WebJSCmdDelegate> web_jscmd_delegate_;
  base::ScopedRefPtr<content::WebNetworkDelegate> web_network_delegate_;
  base::ScopedRefPtr<content::WebPluginManagerDelegate> web_plugin_manager_delegate_;
};
}  // namespace gui

#endif  // DEMO_WEB_RENDER_DELEGATE_WEB_RENDER_CLIENT_IMPL_H_
