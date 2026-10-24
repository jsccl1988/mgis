// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "app/mgis/workbench/workbench_markup.h"

namespace mgis {

std::string_view WorkbenchMarkupXml() {
  // Keep in sync with workbench.ui.xml + workbench.ui.css (inline Style so
  // LoadMarkupBytes works without a base_dir CSS file).
  static constexpr char kXml[] =
      R"(<?xml version="1.0" encoding="UTF-8"?>
<View name="workbench" class="chrome_host">
  <Style>
.chrome_host {
  display: flex;
  flex-direction: column;
  background-color: theme(shell_bg);
}
.chrome {
  display: flex;
  flex-direction: column;
  flex: 1;
  background-color: theme(shell_bg);
}
.toolbar {
  height: 32px;
  flex-shrink: 0;
  background-color: theme(panel_header);
}
.mid {
  display: flex;
  flex-direction: row;
  flex: 1;
  min-height: 240px;
}
.catalog {
  width: 220px;
  min-width: 160px;
  flex-shrink: 1;
  background-color: theme(panel_bg);
}
.viewport {
  flex: 1;
  min-width: 360px;
  background-color: theme(map_placeholder);
}
.tools {
  width: 280px;
  min-width: 260px;
  flex-shrink: 0;
  background-color: theme(panel_bg);
}
.bottom {
  height: 140px;
  min-height: 72px;
  flex-shrink: 1;
  background-color: theme(panel_bg);
}
.status {
  height: 22px;
  flex-shrink: 0;
  background-color: theme(caption_bg);
  color: theme(text);
}
  </Style>
  <FlexColumn id="root" class="chrome">
    <View id="toolbar_slot" class="toolbar"/>
    <FlexRow id="mid" class="mid">
      <View id="catalog_slot" class="catalog"/>
      <View id="viewport_slot" class="viewport"/>
      <View id="tools_slot" class="tools"/>
    </FlexRow>
    <View id="bottom_slot" class="bottom"/>
    <View id="status_slot" class="status"/>
  </FlexColumn>
</View>
)";
  return kXml;
}

}  // namespace mgis
