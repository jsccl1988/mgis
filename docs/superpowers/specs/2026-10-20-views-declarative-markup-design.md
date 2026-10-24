<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# Design: Views declarative markup (mview dialect) + UiDesigner

**Status:** active (product workbench chrome opts in)  
**Date:** 2026-10-23  
**Diagram:** [`../diagrams/views-declarative-markup.html`](../diagrams/views-declarative-markup.html) · [`../diagrams/mgis-workbench-chrome.html`](../diagrams/mgis-workbench-chrome.html)

**Living files considered:** [`2026-10-19-ue-mview-maplibre-design.md`](2026-10-19-ue-mview-maplibre-design.md) (UE dual viewport + MapLibre + pipe).  
**Why a new dated file:** declarative XML/CSS/Yoga + independent UiDesigner is a **new toolkit subsystem**. Folding into the UE/MapLibre living spec would bury a views IR/editor contract inside a GIS host narrative.

**Reference (do not copy APIs verbatim):** smartgis `src/ui/views/markup/` + `src/app/ui_designer/` (snake_case / smartgis View). This tree uses Chromium `views` + `PascalCase`.

---

## Locked decisions

| ID | Decision |
| --- | --- |
| **Scope** | Runtime load **and** independent `UiDesigner.exe` in the same milestone. |
| **Host** | Separate process under `app/ui_designer/` (output `UiDesigner`). Not embedded in product `shell`. |
| **Controls** | Tag creators bind to Chromium `ui/views/controls` (`Label`, `LabelButton`, `Textfield`, …). Do **not** vendor smartgis `primitives/`. |
| **Dialect** | **mview** XML/CSS dialect (Chromium-flavored tag names). Intentionally **not** compatible with smartgis `.ui.xml`. |
| **Libs** | `//third_party:pugixml` + `//third_party:yoga` (already pinned). Product shell does not force-link; dependents opt in. |
| **NO-QT** | No Qt / WebView2 / CEF for chrome or editor. |
| **GIT** | Work on `master`. |

---

## Goal

1. Load `.ui.xml` + linked `.ui.css` into a `views::View` tree with Yoga flex layout and `NamedViewMap` id binding.  
2. Ship `UiDesigner.exe`: live canvas on the same pipeline + open/save/hot-reload; palette / tree / properties / drag insert-reorder aligned with smartgis UiDesigner responsibilities.  
3. gtest coverage for CSS subset, Yoga layout, and `LoadMarkup` factory path.

---

## Architecture

| Layer | Responsibility | Land |
| --- | --- | --- |
| Assets | `.ui.xml` + `.ui.css` | `ui/views/markup/testdata/`, later product resources |
| Parse | pugixml + Flex CSS subset | `ui/views/markup/document|style` |
| Layout | Yoga → `View::SetBoundsRect` | `YogaLayoutManager` : `LayoutManager` |
| Factory | Tag → Chromium control | `ControlFactory` + register TUs |
| Load | Path/bytes → `MarkupRoot` | `LoadMarkup` / `LoadMarkupInto` |
| Designer | Independent exe | `app/ui_designer/` |

GN sketch:

- `:views_control_factory` — registry only (no pugixml/yoga)  
- `:views_markup` — document/style/layout/loader + layout tag regs; deps yoga + pugixml  
- Chromium control tag registration in markup factory TU (depends on controls already in `:views`)  
- `app/ui_designer:ui_designer` → deps `:views` + pugixml  

Default product `shell` entry stays thin; **`app/mgis` workbench** opts into `//ui/views` markup for chrome zones (`workbench.ui.xml` / embedded bytes). Other shell surfaces still do not force-link markup.

---

## Resource format (mview v1)

### XML (illustrative)

```xml
<View name="preview_sample">
  <Style src="preview_sample.ui.css"/>
  <FlexColumn id="root" class="form">
    <Label id="title" text="UiDesigner sample"/>
    <FlexRow id="row" class="row">
      <LabelButton id="ok" text="OK"/>
      <LabelButton id="cancel" text="Cancel"/>
    </FlexRow>
    <Textfield id="note" text="edit me"/>
  </FlexColumn>
</View>
```

### CSS subset

Selectors: `tag` / `#id` / `.class` (no combinators, no pseudo-classes).  
Properties: flex display/direction/justify/align, flex/grow/shrink, gap, padding/margin, width/height/min/max, color, background-color, font-size.  
Sugar: `FlexRow` / `FlexColumn` imply `display:flex` + direction when CSS omits it.

### Public C++ API (Chromium style)

```cpp
views::MarkupRoot root;
views::LoadMarkupInto("preview_sample.ui.xml", {}, &root);
if (root.ok()) {
  views::Label* title = root.ids.FindAs<views::Label>("title");
}
```

---

## Acceptance gates

1. `markup_unittests` green (CSS, Yoga, LoadMarkup tags/ids).  
2. `UiDesigner.exe` opens sample, hot-reloads on save, open/save round-trip.  
3. Drag insert/reorder + property edits persist to XML/CSS (editor-owned `#id` rules).

---

## Product workbench chrome (opt-in)

| Item | Convention |
| --- | --- |
| Assets | `app/mgis/workbench/workbench.ui.xml` + `.ui.css` (DIP lengths) |
| Embed | `WorkbenchMarkupXml()` fallback when file resolve fails |
| Inflate | `LoadMarkupInto` / `LoadMarkupBytes` → slot Views + `FillLayout` mount of horizon/viewport widgets |
| Layout | `YogaLayoutManager` owns zone bounds; `WorkbenchContents::Layout` only sizes markup root + scene HWND |
| DPI | CSS DIP × `ui::win::GetDeviceScaleFactor()` inside Yoga (`StyleScale`) |

## Out of scope (this milestone)

- smartgis dialect compatibility / converters  
- Registering GIS panel types as ControlFactory tags (slots + FillLayout instead)  
- Text2UI / LLM backends inside UiDesigner  
- Forcing markup into every product dialog by default
