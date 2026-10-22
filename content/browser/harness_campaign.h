// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_HARNESS_CAMPAIGN_H_
#define CONTENT_BROWSER_HARNESS_CAMPAIGN_H_

#include "base/files/file_path.h"

namespace content {

// Runs browser.map, browser.pick, then browser.draw. A suite that misses a
// required mark does not get mark.json and the following suites do not start.
class HarnessCampaign {
 public:
  static bool Run(const base::FilePath& suite_root,
                  const base::FilePath& out_root);

 private:
  HarnessCampaign() = delete;
};

}  // namespace content

#endif  // CONTENT_BROWSER_HARNESS_CAMPAIGN_H_
