// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/document/named_view_map.h"
#include "ui/views/markup/factory/control_factory.h"
#include "ui/views/markup/factory/placeholder_view.h"
#include "ui/views/markup/layout/yoga_layout_manager.h"
#include "ui/views/markup/loader/markup_loader.h"
#include "ui/views/markup/style/css_parser.h"
#include "ui/views/markup/style/flex_style.h"
#include "ui/views/view.h"

#include <cmath>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "gtest/gtest.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/win/dpi.h"

namespace views {
namespace {

int DipPx(int dip) {
  const float scale = ui::win::GetDeviceScaleFactor();
  const float s = scale > 0.f ? scale : 1.f;
  return static_cast<int>(std::lround(static_cast<float>(dip) * s));
}

TEST(CssParserTest, CascadeAndColor) {
  CssParser parser;
  std::string error;
  ASSERT_TRUE(parser.Parse(
      "label { flex-direction: column; }"
      ".form { gap: 8px; display: flex; }"
      "#title { height: 24px; color: #abc; }",
      &error));
  const FlexStyle style = parser.Resolve("Label", "title", {"form"});
  ASSERT_TRUE(style.gap.has_value());
  EXPECT_FLOAT_EQ(8.f, *style.gap);
  ASSERT_TRUE(style.height.has_value());
  EXPECT_FLOAT_EQ(24.f, *style.height);
  ASSERT_TRUE(style.flex_direction.has_value());
  EXPECT_EQ(FlexStyle::FlexDirection::kColumn, *style.flex_direction);
  ASSERT_TRUE(style.display.has_value());
  EXPECT_EQ(FlexStyle::Display::kFlex, *style.display);
  ASSERT_TRUE(style.color.has_value());
  EXPECT_EQ(SkColorSetRGB(0xAA, 0xBB, 0xCC), *style.color);
}

TEST(CssParserTest, RejectsPercentAndMissingBrace) {
  CssParser parser;
  ASSERT_TRUE(parser.Parse("#a { width: 100%; height: 12px; }"));
  const FlexStyle style = parser.Resolve("view", "a", {});
  EXPECT_FALSE(style.width.has_value());
  ASSERT_TRUE(style.height.has_value());

  std::string error;
  EXPECT_FALSE(parser.Parse("#a { height: 1px;", &error));
  EXPECT_FALSE(error.empty());
}

TEST(CssParserTest, UpsertIdAndSerialize) {
  CssParser parser;
  ASSERT_TRUE(parser.Parse("#title { height: 10px; }"));
  parser.UpsertIdDeclarations("title", {{"height", "24px"}, {"gap", "4px"}});
  const FlexStyle style = parser.Resolve("label", "title", {});
  ASSERT_TRUE(style.height.has_value());
  EXPECT_FLOAT_EQ(24.f, *style.height);
  ASSERT_TRUE(style.gap.has_value());
  EXPECT_FLOAT_EQ(4.f, *style.gap);
  const std::string css = parser.Serialize();
  EXPECT_NE(std::string::npos, css.find("#title"));
  EXPECT_NE(std::string::npos, css.find("height: 24px"));
}

TEST(FlexStyleTest, MergeFromOverwritesSetFields) {
  FlexStyle base;
  base.width = 10.f;
  base.height = 4.f;
  FlexStyle over;
  over.width = 20.f;
  base.MergeFrom(over);
  EXPECT_FLOAT_EQ(20.f, *base.width);
  EXPECT_FLOAT_EQ(4.f, *base.height);
}

TEST(YogaLayoutTest, RowPlacesChildren) {
  View host;
  host.SetBounds(0, 0, 200, 100);
  View* a = new View();
  View* b = new View();
  host.AddChildView(a);
  host.AddChildView(b);

  YogaLayoutManager* yoga = new YogaLayoutManager();
  FlexStyle host_style;
  host_style.display = FlexStyle::Display::kFlex;
  host_style.flex_direction = FlexStyle::FlexDirection::kRow;
  host_style.gap = 10.f;
  yoga->SetHostStyle(host_style);

  FlexStyle child_a;
  child_a.width = 40.f;
  child_a.height = 20.f;
  yoga->SetChildStyle(a, child_a);
  FlexStyle child_b;
  child_b.width = 50.f;
  child_b.height = 20.f;
  yoga->SetChildStyle(b, child_b);
  host.SetLayoutManager(yoga);
  host.Layout();

  EXPECT_EQ(0, a->x());
  EXPECT_EQ(DipPx(40), a->width());
  EXPECT_EQ(DipPx(20), a->height());
  EXPECT_EQ(DipPx(50), b->x());
  EXPECT_EQ(DipPx(50), b->width());
}

TEST(ControlFactoryTest, RegistersChromiumTags) {
  const ControlFactory& factory = ControlFactory::GetDefault();
  EXPECT_TRUE(factory.HasTag("View"));
  EXPECT_TRUE(factory.HasTag("flexrow"));
  EXPECT_TRUE(factory.HasTag("Label"));
  EXPECT_TRUE(factory.HasTag("LabelButton"));
  EXPECT_TRUE(factory.HasTag("Textfield"));
  EXPECT_TRUE(factory.HasTag("Checkbox"));
  EXPECT_TRUE(factory.HasTag("RadioButton"));
  EXPECT_TRUE(factory.HasTag("Combobox"));
  EXPECT_TRUE(factory.HasTag("Slider"));
  EXPECT_TRUE(factory.HasTag("ScrollView"));
  EXPECT_TRUE(factory.HasTag("ImageView"));
  EXPECT_TRUE(factory.HasTag("Separator"));
  EXPECT_TRUE(factory.HasTag("Placeholder"));
  EXPECT_FALSE(factory.RegisteredTags().empty());
}

TEST(LoadMarkupTest, BytesBuildIdsAndYogaSize) {
  const char kXml[] =
      "<View name=\"t\">"
      "  <Style>"
      "    #slot { width: 80px; height: 30px; }"
      "    .row { height: 40px; }"
      "  </Style>"
      "  <FlexColumn id=\"root\" class=\"form\">"
      "    <Placeholder id=\"slot\" text=\"box\"/>"
      "    <Separator id=\"rule\"/>"
      "    <FlexRow id=\"row\" class=\"row\">"
      "      <Placeholder id=\"a\" text=\"a\"/>"
      "      <ImageView id=\"icon\"/>"
      "    </FlexRow>"
      "    <Weird id=\"w\"/>"
      "  </FlexColumn>"
      "</View>";

  MarkupRoot loaded;
  ASSERT_TRUE(LoadMarkupBytes(kXml, "", MarkupOptions(), &loaded));
  EXPECT_TRUE(loaded.ok());
  EXPECT_EQ("t", loaded.name);
  EXPECT_TRUE(loaded.ids.Contains("root"));
  EXPECT_TRUE(loaded.ids.Contains("slot"));
  EXPECT_TRUE(loaded.ids.Contains("row"));
  EXPECT_TRUE(loaded.ids.Contains("icon"));
  PlaceholderView* slot = loaded.ids.FindAs<PlaceholderView>("slot");
  ASSERT_TRUE(slot != NULL);
  EXPECT_EQ("box", slot->caption());
  PlaceholderView* weird = loaded.ids.FindAs<PlaceholderView>("w");
  ASSERT_TRUE(weird != NULL);

  loaded.root->SetBounds(0, 0, DipPx(200), DipPx(120));
  loaded.root->Layout();
  EXPECT_EQ(DipPx(80), slot->width());
  EXPECT_EQ(DipPx(30), slot->height());
  View* row = loaded.ids.Find("row");
  ASSERT_TRUE(row != NULL);
  EXPECT_EQ(2, row->child_count());
}

}  // namespace
}  // namespace views

namespace {

void MarkupAssertHandler(const std::string& str) {
  // Fail the process without a modal dialog (View ctor uses CommandLine).
  fprintf(stderr, "FATAL DCHECK: %s\n", str.c_str());
  fflush(stderr);
  abort();
}

}  // namespace

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  CommandLine::Init(argc, argv);
  logging::SetLogAssertHandler(&MarkupAssertHandler);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
