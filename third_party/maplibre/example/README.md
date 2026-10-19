<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# MapLibre Native 示例

样式 `style_background.json` 只有一层纯色背景，用来跑上游 `mbgl-render`，不进 aura 的 `ui::Layer`。

源码 pin 与 smartgis 相同（`ios-v6.30.0`）。已有检出时先做联接，见 `third_party/README.md`。

```bat
py -3 third_party\maplibre\build_native.py third_party\.src\maplibre-native third_party\.build\maplibre-example Release
third_party\.build\maplibre-example\bin\maplibre_headless_example.exe -s third_party\maplibre\example\style_background.json -o third_party\.build\maplibre-example\still.png -w 256 -h 256
```
