// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef APP_MGIS_HORIZON_STATUS_BAR_H_
#define APP_MGIS_HORIZON_STATUS_BAR_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/view.h"

namespace views {
class Label;
}

namespace mgis {

// Bottom strip: message | scale | CRS | XY (SmartGIS StatusBar field set).
class StatusBar : public views::View {
 public:
  StatusBar();
  virtual ~StatusBar();

  void SetMessage(const std::string& utf8);
  void SetScaleText(const std::string& utf8);
  void SetCrsText(const std::string& utf8);
  void SetCoordText(const std::string& utf8);

  virtual void Layout() OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;

 private:
  views::Label* message_;
  views::Label* scale_;
  views::Label* crs_;
  views::Label* coord_;

  DISALLOW_COPY_AND_ASSIGN(StatusBar);
};

}  // namespace mgis

#endif  // APP_MGIS_HORIZON_STATUS_BAR_H_
