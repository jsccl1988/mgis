// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_BROWSER_GIS_CONTENTS_CLIENT_H_
#define CONTENT_PUBLIC_BROWSER_GIS_CONTENTS_CLIENT_H_

namespace content {

class GisContents;

// Implemented by the shell. The only browser-process hook is browser_main.
// Plugins do not implement this type. Window creation stays in the shell.
class GisContentsClient {
 public:
  virtual ~GisContentsClient();

  // Shell entry for one session. |contents| is the viewport and document
  // host. The shell creates aura windows here and does not build pipe frames.
  // Returns a process exit code; 0 means the shell finished cleanly.
  virtual int browser_main(GisContents* contents) = 0;

 protected:
  GisContentsClient();

 private:
  GisContentsClient(const GisContentsClient&) = delete;
  GisContentsClient& operator=(const GisContentsClient&) = delete;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_GIS_CONTENTS_CLIENT_H_
