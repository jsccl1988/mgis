// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "examples/ui_designer/shell.h"

#include <cstdio>
#include <vector>

#include "base/basictypes.h"

#include "base/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop.h"
#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "examples/ui_designer/canvas.h"
#include "examples/ui_designer/document.h"
#include "examples/ui_designer/io.h"
#include "ui/gfx/rect.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/markup/loader/markup_loader.h"
#include "ui/views/markup/style/flex_style.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <commdlg.h>

namespace ui_designer {
namespace {

const char* kPaletteTags[] = {
    "View",       "FlexRow",    "FlexColumn", "Label",      "LabelButton",
    "Textfield",  "Checkbox",   "RadioButton", "Combobox",  "Slider",
    "ScrollView", "ImageView",  "Separator",  "Placeholder",
};

class StackView : public views::View {
 public:
  StackView() {}
  virtual void Layout() OVERRIDE {
    int y = 0;
    for (int i = 0; i < child_count(); ++i) {
      child_at(i)->SetBounds(0, y, width(), 24);
      y += 26;
    }
  }
  virtual gfx::Size GetPreferredSize() OVERRIDE {
    return gfx::Size(width() > 0 ? width() : 160, child_count() * 26);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(StackView);
};

class TagButton : public views::LabelButton {
 public:
  TagButton(views::ButtonListener* listener, const std::string& tag)
      : views::LabelButton(listener, UTF8ToUTF16(tag)), tag_(tag) {}
  const std::string& tag() const { return tag_; }

 private:
  std::string tag_;
  DISALLOW_COPY_AND_ASSIGN(TagButton);
};

class NodeButton : public views::LabelButton {
 public:
  NodeButton(views::ButtonListener* listener,
             const string16& label,
             pugi::xml_node node)
      : views::LabelButton(listener, label), node_(node) {}
  pugi::xml_node node() const { return node_; }

 private:
  pugi::xml_node node_;
  DISALLOW_COPY_AND_ASSIGN(NodeButton);
};

class UiDesignerWindow : public views::WidgetDelegateView,
                         public views::ButtonListener {
 public:
  UiDesignerWindow()
      : open_button_(NULL),
        save_button_(NULL),
        reload_button_(NULL),
        apply_button_(NULL),
        palette_scroll_(NULL),
        tree_scroll_(NULL),
        canvas_(NULL),
        status_(NULL),
        id_field_(NULL),
        text_field_(NULL),
        class_field_(NULL),
        width_field_(NULL),
        height_field_(NULL),
        grow_field_(NULL),
        gap_field_(NULL),
        palette_title_(NULL),
        tree_title_(NULL),
        props_title_(NULL),
        weak_factory_(this) {
    open_button_ = new views::LabelButton(this, L"Open");
    save_button_ = new views::LabelButton(this, L"Save");
    reload_button_ = new views::LabelButton(this, L"Reload");
    apply_button_ = new views::LabelButton(this, L"Apply");
    palette_scroll_ = new views::ScrollView();
    tree_scroll_ = new views::ScrollView();
    canvas_ = new MarkupCanvas();
    status_ = new views::Label();
    id_field_ = new views::Textfield();
    text_field_ = new views::Textfield();
    class_field_ = new views::Textfield();
    width_field_ = new views::Textfield();
    height_field_ = new views::Textfield();
    grow_field_ = new views::Textfield();
    gap_field_ = new views::Textfield();

    AddChildView(open_button_);
    AddChildView(save_button_);
    AddChildView(reload_button_);
    AddChildView(palette_scroll_);
    AddChildView(tree_scroll_);
    AddChildView(canvas_);
    AddChildView(status_);
    AddChildView(apply_button_);
    AddChildView(id_field_);
    AddChildView(text_field_);
    AddChildView(class_field_);
    AddChildView(width_field_);
    AddChildView(height_field_);
    AddChildView(grow_field_);
    AddChildView(gap_field_);
    AddFieldLabel(L"id");
    AddFieldLabel(L"text");
    AddFieldLabel(L"class");
    AddFieldLabel(L"width");
    AddFieldLabel(L"height");
    AddFieldLabel(L"flex-grow");
    AddFieldLabel(L"gap");
    palette_title_ = new views::Label(L"Palette");
    tree_title_ = new views::Label(L"Hierarchy");
    props_title_ = new views::Label(L"Properties");
    AddChildView(palette_title_);
    AddChildView(tree_title_);
    AddChildView(props_title_);

    BuildPalette();

    const std::string sample = views::ResolveMarkupPath("preview_sample.ui.xml");
    if (!sample.empty())
      LoadFile(sample);
    else
      SetStatus("preview_sample.ui.xml was not found");
    SchedulePoll();
  }

  virtual ~UiDesignerWindow() {}

  virtual bool CanResize() const OVERRIDE { return true; }
  virtual bool CanMaximize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE { return L"UiDesigner"; }
  virtual views::View* GetContentsView() OVERRIDE { return this; }

  virtual void WindowClosing() OVERRIDE {
    weak_factory_.InvalidateWeakPtrs();
    base::MessageLoopForUI::current()->Quit();
  }

  virtual void ButtonPressed(views::Button* sender,
                             const ui::Event& event) OVERRIDE {
    (void)event;
    if (sender == open_button_) {
      OpenDialog();
      return;
    }
    if (sender == save_button_) {
      Save();
      return;
    }
    if (sender == reload_button_) {
      Reload();
      return;
    }
    if (sender == apply_button_) {
      ApplyProperties();
      return;
    }
    if (TagButton* tag = dynamic_cast<TagButton*>(sender)) {
      InsertTag(tag->tag());
      return;
    }
    if (NodeButton* node = dynamic_cast<NodeButton*>(sender)) {
      selected_ = node->node();
      RefreshHierarchy();
      RefreshProperties();
    }
  }

  virtual void Layout() OVERRIDE {
    const int kBar = 32;
    const int kStatus = 22;
    const int kTitle = 18;
    open_button_->SetBounds(8, 4, 72, 24);
    save_button_->SetBounds(88, 4, 72, 24);
    reload_button_->SetBounds(168, 4, 72, 24);

    const int body_y = kBar + kTitle;
    const int body_h = std::max(0, height() - body_y - kStatus);
    int palette_w = 150;
    int tree_w = 220;
    int props_w = 220;
    int canvas_w = width() - palette_w - tree_w - props_w;
    if (canvas_w < 120) {
      palette_w = 120;
      tree_w = 160;
      props_w = 180;
      canvas_w = std::max(80, width() - palette_w - tree_w - props_w);
    }
    const int tree_x = palette_w;
    const int canvas_x = tree_x + tree_w;
    const int props_x = canvas_x + canvas_w;

    if (palette_title_)
      palette_title_->SetBounds(8, kBar, palette_w - 12, kTitle);
    if (tree_title_)
      tree_title_->SetBounds(tree_x + 8, kBar, tree_w - 12, kTitle);
    if (props_title_)
      props_title_->SetBounds(props_x + 8, kBar, props_w - 12, kTitle);

    palette_scroll_->SetBounds(0, body_y, palette_w, body_h);
    tree_scroll_->SetBounds(tree_x, body_y, tree_w, body_h);
    canvas_->SetBounds(canvas_x, body_y, canvas_w, body_h);

    LayoutProperties(props_x, body_y, props_w, body_h);
    status_->SetBounds(8, height() - kStatus, std::max(0, width() - 16),
                       kStatus);
  }

 private:
  void AddFieldLabel(const string16& text) {
    views::Label* label = new views::Label(text);
    field_labels_.push_back(label);
    AddChildView(label);
  }

  void LayoutProperties(int x, int y, int w, int h) {
    const int field_h = 22;
    const int row = 40;
    int cursor = y + 4;
    views::Textfield* fields[] = {
        id_field_,   text_field_, class_field_, width_field_,
        height_field_, grow_field_, gap_field_};
    const int count = static_cast<int>(field_labels_.size());
    for (int i = 0; i < count && i < 7; ++i) {
      field_labels_[i]->SetBounds(x + 8, cursor, w - 16, 16);
      fields[i]->SetBounds(x + 8, cursor + 16, w - 16, field_h);
      cursor += row;
    }
    apply_button_->SetBounds(x + 8, std::min(cursor, y + h - 28), 80, 24);
  }

  void SetStatus(const std::string& text) {
    status_->SetText(UTF8ToUTF16(text));
  }

  void RememberStamp() {
    last_write_.dwLowDateTime = 0;
    last_write_.dwHighDateTime = 0;
    if (!xml_path_.empty())
      ReadWriteTime(xml_path_, &last_write_);
  }

  void BuildPalette() {
    StackView* stack = new StackView();
    for (size_t i = 0; i < sizeof(kPaletteTags) / sizeof(kPaletteTags[0]);
         ++i) {
      stack->AddChildView(new TagButton(this, kPaletteTags[i]));
    }
    const int height = stack->child_count() * 26;
    stack->SetBounds(0, 0, 140, height);
    palette_scroll_->SetContents(stack);
  }

  void RefreshHierarchy() {
    std::vector<TreeEntry> rows;
    CollectElements(doc_.RawXml(), &rows);
    StackView* stack = new StackView();
    for (size_t i = 0; i < rows.size(); ++i) {
      std::string label = rows[i].label;
      if (selected_ && rows[i].node == selected_)
        label = ">" + label;
      stack->AddChildView(
          new NodeButton(this, UTF8ToUTF16(label), rows[i].node));
    }
    stack->SetBounds(0, 0, 200, std::max(26, stack->child_count() * 26));
    tree_scroll_->SetContents(stack);
  }

  static std::string FormatPx(float value) {
    char buf[32];
    sprintf_s(buf, "%g", value);
    return buf;
  }

  void RefreshProperties() {
    if (!selected_) {
      id_field_->SetText(string16());
      text_field_->SetText(string16());
      class_field_->SetText(string16());
      width_field_->SetText(string16());
      height_field_->SetText(string16());
      grow_field_->SetText(string16());
      gap_field_->SetText(string16());
      return;
    }
    id_field_->SetText(UTF8ToUTF16(AttributeOf(selected_, "id")));
    text_field_->SetText(UTF8ToUTF16(AttributeOf(selected_, "text")));
    class_field_->SetText(UTF8ToUTF16(AttributeOf(selected_, "class")));

    std::vector<std::string> classes;
    const std::string raw_class = AttributeOf(selected_, "class");
    std::string cur;
    for (size_t i = 0; i < raw_class.size(); ++i) {
      if (raw_class[i] == ' ') {
        if (!cur.empty())
          classes.push_back(cur);
        cur.clear();
      } else {
        cur.push_back(raw_class[i]);
      }
    }
    if (!cur.empty())
      classes.push_back(cur);
    const views::FlexStyle style = doc_.Stylesheet().Resolve(
        selected_.name(), AttributeOf(selected_, "id"), classes);
    width_field_->SetText(style.width ? UTF8ToUTF16(FormatPx(*style.width))
                                       : string16());
    height_field_->SetText(style.height ? UTF8ToUTF16(FormatPx(*style.height))
                                         : string16());
    grow_field_->SetText(style.flex_grow
                             ? UTF8ToUTF16(FormatPx(*style.flex_grow))
                             : string16());
    gap_field_->SetText(style.gap ? UTF8ToUTF16(FormatPx(*style.gap))
                                   : string16());
  }

  bool RebuildCanvas() {
    views::MarkupRoot built;
    if (!views::BuildMarkupTree(doc_, views::MarkupOptions(), &built) ||
        !built.ok()) {
      SetStatus(built.error.empty() ? "markup build failed" : built.error);
      return false;
    }
    canvas_->Install(built.root.release());
    return true;
  }

  bool LoadFile(const std::string& path) {
    const std::string xml = ReadFileUtf8(path);
    if (xml.empty()) {
      SetStatus("failed to read " + path);
      return false;
    }
    std::string error;
    if (!doc_.LoadXml(xml, DirnameOf(path), &error)) {
      SetStatus(error);
      return false;
    }
    xml_path_ = path;
    selected_ = pugi::xml_node();
    if (!RebuildCanvas())
      return false;
    RefreshHierarchy();
    RefreshProperties();
    RememberStamp();
    SetStatus(path);
    return true;
  }

  void Reload() {
    if (!xml_path_.empty())
      LoadFile(xml_path_);
  }

  void Save() {
    if (xml_path_.empty()) {
      SetStatus("nothing to save");
      return;
    }
    const std::string css = StylesheetPath(&doc_, xml_path_);
    if (!doc_.SaveFiles(xml_path_, css)) {
      SetStatus("save failed");
      return;
    }
    RememberStamp();
    SetStatus("saved " + xml_path_);
  }

  void OpenDialog() {
    wchar_t file[MAX_PATH] = {};
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetWidget() ? GetWidget()->GetNativeWindow() : NULL;
    ofn.lpstrFilter = L"UI markup\0*.ui.xml\0All files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn))
      return;
    LoadFile(WideToUtf8(file));
  }

  void InsertTag(const std::string& tag) {
    if (!doc_.RawXml()) {
      SetStatus("open a document first");
      return;
    }
    pugi::xml_node created =
        InsertElement(doc_.RawXml(), selected_, tag);
    if (!created) {
      SetStatus("insert failed");
      return;
    }
    selected_ = created;
    if (!RebuildCanvas())
      return;
    RefreshHierarchy();
    RefreshProperties();
    SetStatus("inserted " + tag);
  }

  static std::string FieldUtf8(const views::Textfield* field) {
    if (!field)
      return std::string();
    return UTF16ToUTF8(field->text());
  }

  static std::string WithPx(const std::string& value) {
    if (value.empty())
      return value;
    if (value.find("px") != std::string::npos)
      return value;
    return value + "px";
  }

  void ApplyProperties() {
    if (!selected_) {
      SetStatus("select a node");
      return;
    }
    SetAttribute(selected_, "id", FieldUtf8(id_field_));
    SetAttribute(selected_, "text", FieldUtf8(text_field_));
    SetAttribute(selected_, "class", FieldUtf8(class_field_));

    const std::string id = FieldUtf8(id_field_);
    if (!id.empty()) {
      std::vector<std::pair<std::string, std::string>> decls;
      const std::string width = FieldUtf8(width_field_);
      const std::string height = FieldUtf8(height_field_);
      const std::string grow = FieldUtf8(grow_field_);
      const std::string gap = FieldUtf8(gap_field_);
      if (!width.empty())
        decls.push_back(std::make_pair("width", WithPx(width)));
      if (!height.empty())
        decls.push_back(std::make_pair("height", WithPx(height)));
      if (!grow.empty())
        decls.push_back(std::make_pair("flex-grow", grow));
      if (!gap.empty())
        decls.push_back(std::make_pair("gap", WithPx(gap)));
      if (!decls.empty())
        doc_.Stylesheet().UpsertIdDeclarations(id, decls);
    }
    if (!RebuildCanvas())
      return;
    RefreshHierarchy();
    SetStatus(id.empty() ? "attributes updated" : "updated #" + id);
  }

  void PollFile() {
    if (!xml_path_.empty()) {
      FILETIME now;
      if (ReadWriteTime(xml_path_, &now) &&
          CompareFileTime(&now, &last_write_) != 0) {
        LoadFile(xml_path_);
      }
    }
    SchedulePoll();
  }

  void SchedulePoll() {
    base::MessageLoop::current()->PostDelayedTask(
        FROM_HERE,
        base::Bind(&UiDesignerWindow::PollFile, weak_factory_.GetWeakPtr()),
        base::TimeDelta::FromMilliseconds(500));
  }

  views::LabelButton* open_button_;
  views::LabelButton* save_button_;
  views::LabelButton* reload_button_;
  views::LabelButton* apply_button_;
  views::ScrollView* palette_scroll_;
  views::ScrollView* tree_scroll_;
  MarkupCanvas* canvas_;
  views::Label* status_;
  views::Label* palette_title_;
  views::Label* tree_title_;
  views::Label* props_title_;
  views::Textfield* id_field_;
  views::Textfield* text_field_;
  views::Textfield* class_field_;
  views::Textfield* width_field_;
  views::Textfield* height_field_;
  views::Textfield* grow_field_;
  views::Textfield* gap_field_;
  std::vector<views::Label*> field_labels_;
  views::MarkupDocument doc_;
  pugi::xml_node selected_;
  std::string xml_path_;
  FILETIME last_write_;
  base::WeakPtrFactory<UiDesignerWindow> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(UiDesignerWindow);
};

}  // namespace

void ShowUiDesigner() {
  views::Widget::CreateWindowWithBounds(new UiDesignerWindow(),
                                        gfx::Rect(40, 40, 1100, 720))
      ->Show();
}

}  // namespace ui_designer
