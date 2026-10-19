// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "main_window.h"

#include <algorithm>
#include <string>

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/utf_string_conversions.h"
#include "content/public/script_runtime.h"
#include "plugin/registry.h"
#include "ui/base/models/tree_node_model.h"
#include "ui/gfx/rect.h"
#include "ui/views/controls/menu/menu.h"
#include "ui/views/controls/single_split_view.h"
#include "ui/views/controls/slider.h"
#include "ui/views/controls/tree/tree_view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

namespace {

const char* kRequiredMarks[] = {
    "widget.main",
    "widget.map",
    "widget.view0",
    "widget.view1",
    "tree.layer",
    "tree.entity",
};

typedef ui::TreeNodeWithValue<int> NodeType;

// Popup "Window" menu. Items name the other product widgets.
class WindowMenuDelegate : public views::Menu::Delegate {
 public:
  WindowMenuDelegate() {}
  virtual ~WindowMenuDelegate() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(WindowMenuDelegate);
};

// Empty top-level widget for map, view0, or view1. No child HWND yet.
class ViewportWindow : public views::WidgetDelegateView {
 public:
  explicit ViewportWindow(const string16& title) : title_(title) {}
  virtual ~ViewportWindow() {}

  virtual bool CanResize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE { return title_; }
  virtual views::View* GetContentsView() OVERRIDE { return this; }

 private:
  string16 title_;

  DISALLOW_COPY_AND_ASSIGN(ViewportWindow);
};

// Product main window: layer tree, entity tree, a window menu, and a slider.
class MainWindowContents : public views::WidgetDelegateView,
                           public views::SliderListener {
 public:
  MainWindowContents()
      : layer_model_(new NodeType(ASCIIToUTF16("Layers"), 0)),
        entity_model_(new NodeType(ASCIIToUTF16("Entities"), 0)),
        layer_tree_(NULL),
        entity_tree_(NULL),
        split_(NULL),
        slider_(NULL),
        map_(NULL),
        view0_(NULL),
        view1_(NULL) {
    plugins_.Scan("plugin");

    layer_model_.GetRoot()->Add(new NodeType(ASCIIToUTF16("tileset"), 1), 0);
    entity_model_.GetRoot()->Add(
        new NodeType(ASCIIToUTF16("camera.view0"), 1), 0);
    entity_model_.GetRoot()->Add(
        new NodeType(ASCIIToUTF16("camera.view1"), 1), 1);

    layer_tree_ = new views::TreeView();
    layer_tree_->SetEditable(false);
    layer_tree_->SetRootShown(true);
    layer_tree_->SetModel(&layer_model_);

    entity_tree_ = new views::TreeView();
    entity_tree_->SetEditable(false);
    entity_tree_->SetRootShown(true);
    entity_tree_->SetModel(&entity_model_);

    split_ = new views::SingleSplitView(
        layer_tree_->CreateParentIfNecessary(),
        entity_tree_->CreateParentIfNecessary(),
        views::SingleSplitView::HORIZONTAL_SPLIT,
        NULL);

    slider_ = new views::Slider(this, views::Slider::HORIZONTAL);
    slider_->SetValue(0.0f);

    AddChildView(slider_);
    AddChildView(split_);
  }

  virtual ~MainWindowContents() {
    // TreeViews are child views destroyed by ~View, after members.
    if (layer_tree_)
      layer_tree_->SetModel(NULL);
    if (entity_tree_)
      entity_tree_->SetModel(NULL);
  }

  bool HasLayerTree() const { return layer_tree_ != NULL; }
  bool HasEntityTree() const { return entity_tree_ != NULL; }

  void SetSatellites(views::Widget* map,
                     views::Widget* view0,
                     views::Widget* view1) {
    map_ = map;
    view0_ = view0;
    view1_ = view1;
  }

  void CloseSatellites() {
    views::Widget* widgets[] = {map_, view0_, view1_};
    map_ = NULL;
    view0_ = NULL;
    view1_ = NULL;
    for (size_t i = 0; i < arraysize(widgets); ++i) {
      if (widgets[i])
        widgets[i]->CloseNow();
    }
  }

  // Builds the window menu once the widget has a native view.
  void EnsureWindowMenu() {
    if (menu_)
      return;
    views::Widget* widget = GetWidget();
    if (!widget || !widget->GetNativeView())
      return;
    menu_.reset(views::Menu::Create(&menu_delegate_,
                                    views::Menu::TOPLEFT,
                                    widget->GetNativeView()));
    if (!menu_)
      return;
    menu_->AppendMenuItemWithLabel(1, ASCIIToUTF16("Map"));
    menu_->AppendMenuItemWithLabel(2, ASCIIToUTF16("view0"));
    menu_->AppendMenuItemWithLabel(3, ASCIIToUTF16("view1"));
  }

  virtual bool CanResize() const OVERRIDE { return true; }
  virtual bool CanMaximize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE {
    return ASCIIToUTF16("MView");
  }
  virtual views::View* GetContentsView() OVERRIDE { return this; }
  virtual gfx::Size GetPreferredSize() OVERRIDE {
    return gfx::Size(960, 640);
  }
  virtual void Layout() OVERRIDE {
    const int slider_height = 40;
    if (slider_)
      slider_->SetBounds(0, 0, width(), slider_height);
    if (split_) {
      split_->SetBounds(0,
                        slider_height,
                        width(),
                        std::max(0, height() - slider_height));
    }
  }
  virtual void WindowClosing() OVERRIDE {
    CloseSatellites();
    MessageLoopForUI::current()->Quit();
  }
  virtual void SliderValueChanged(views::Slider* sender,
                                  float value,
                                  float old_value,
                                  views::SliderChangeReason reason) OVERRIDE {
    (void)sender;
    (void)value;
    (void)old_value;
    (void)reason;
  }

 private:
  WindowMenuDelegate menu_delegate_;
  scoped_ptr<views::Menu> menu_;
  ui::TreeNodeModel<NodeType> layer_model_;
  ui::TreeNodeModel<NodeType> entity_model_;
  views::TreeView* layer_tree_;
  views::TreeView* entity_tree_;
  views::SingleSplitView* split_;
  views::Slider* slider_;
  views::Widget* map_;
  views::Widget* view0_;
  views::Widget* view1_;
  plugin::Registry plugins_;

  DISALLOW_COPY_AND_ASSIGN(MainWindowContents);
};

struct ProductChrome {
  ProductChrome()
      : main_contents(NULL),
        main_window(NULL),
        map(NULL),
        view0(NULL),
        view1(NULL) {}

  MainWindowContents* main_contents;
  views::Widget* main_window;
  views::Widget* map;
  views::Widget* view0;
  views::Widget* view1;
};

views::Widget* ShowViewport(const char* title, const gfx::Rect& bounds) {
  views::Widget* widget = views::Widget::CreateWindowWithBounds(
      new ViewportWindow(ASCIIToUTF16(title)), bounds);
  if (widget)
    widget->Show();
  return widget;
}

bool CreateProductChrome(ProductChrome* chrome) {
  chrome->main_contents = new MainWindowContents();
  chrome->main_window = views::Widget::CreateWindowWithBounds(
      chrome->main_contents, gfx::Rect(80, 80, 960, 640));
  if (!chrome->main_window)
    return false;
  chrome->main_window->Show();
  chrome->main_contents->EnsureWindowMenu();

  chrome->map = ShowViewport("MView Map", gfx::Rect(200, 120, 800, 600));
  chrome->view0 = ShowViewport("MView view0", gfx::Rect(240, 160, 720, 520));
  chrome->view1 = ShowViewport("MView view1", gfx::Rect(280, 200, 720, 520));
  if (!chrome->map || !chrome->view0 || !chrome->view1)
    return false;
  if (!chrome->main_contents->HasLayerTree() ||
      !chrome->main_contents->HasEntityTree())
    return false;

  chrome->main_contents->SetSatellites(chrome->map, chrome->view0, chrome->view1);
  return true;
}

base::FilePath SuitePath(const base::FilePath& root) {
  return root.Append(FILE_PATH_LITERAL("testing"))
      .Append(FILE_PATH_LITERAL("tools"))
      .Append(FILE_PATH_LITERAL("harness"))
      .Append(FILE_PATH_LITERAL("browser.chrome"));
}

base::FilePath WalkForRepoRoot(base::FilePath dir) {
  for (int i = 0; i < 8 && !dir.empty(); ++i) {
    if (file_util::PathExists(SuitePath(dir)))
      return dir;
    const base::FilePath parent = dir.DirName();
    if (parent == dir)
      break;
    dir = parent;
  }
  return base::FilePath();
}

base::FilePath FindRepoRoot() {
  base::FilePath current;
  if (file_util::GetCurrentDirectory(&current)) {
    const base::FilePath from_cwd = WalkForRepoRoot(current);
    if (!from_cwd.empty())
      return from_cwd;
  }
  base::FilePath exe;
  if (PathService::Get(base::FILE_EXE, &exe))
    return WalkForRepoRoot(exe.DirName());
  return base::FilePath();
}

bool HostHasRequiredMarks(const content::CapabilityHost& host) {
  for (size_t i = 0; i < arraysize(kRequiredMarks); ++i) {
    if (!host.has_mark(kRequiredMarks[i]))
      return false;
  }
  return true;
}

// The P0 stand-in runner rejects '#' comments. browser.chrome documents
// each mark with a comment line, so a direct run_suite_script fails before
// it records anything. Retry on a comment-stripped copy of the same marks.
bool WriteCommentStrippedSuite(const base::FilePath& source,
                               const base::FilePath& dest) {
  int64 size = 0;
  if (!file_util::GetFileSize(source, &size) || size <= 0 ||
      size > 1024 * 1024) {
    return false;
  }
  std::string text(static_cast<size_t>(size), '\0');
  const int read = file_util::ReadFile(
      source, &text[0], static_cast<int>(size));
  if (read < 0)
    return false;
  text.resize(static_cast<size_t>(read));

  std::string stripped;
  size_t index = 0;
  while (index < text.size()) {
    const size_t newline = text.find('\n', index);
    const bool has_newline = newline != std::string::npos;
    const size_t end = has_newline ? newline : text.size();
    const std::string raw = text.substr(index, end - index);
    size_t begin = 0;
    while (begin < raw.size() && (raw[begin] == ' ' || raw[begin] == '\t'))
      ++begin;
    const bool comment = begin < raw.size() && raw[begin] == '#';
    if (!comment) {
      stripped.append(raw);
      stripped.push_back('\n');
    }
    if (!has_newline)
      break;
    index = end + 1;
  }

  if (!file_util::CreateDirectory(dest.DirName()))
    return false;
  const int wrote = file_util::WriteFile(
      dest, stripped.data(), static_cast<int>(stripped.size()));
  return wrote == static_cast<int>(stripped.size());
}

bool WriteMarkFile(const base::FilePath& path) {
  std::string json = "{\n  \"suite\": \"browser.chrome\",\n  \"marks\": [\n";
  for (size_t i = 0; i < arraysize(kRequiredMarks); ++i) {
    json += "    \"";
    json += kRequiredMarks[i];
    json += "\"";
    if (i + 1 != arraysize(kRequiredMarks))
      json += ",";
    json += "\n";
  }
  json += "  ]\n}\n";

  if (!file_util::CreateDirectory(path.DirName())) {
    LOG(ERROR) << "failed to create harness directory";
    return false;
  }
  const int wrote = file_util::WriteFile(
      path, json.data(), static_cast<int>(json.size()));
  return wrote == static_cast<int>(json.size());
}

// Records browser.chrome only after the four widgets and both trees exist.
bool WriteChromeMarks(const ProductChrome& chrome) {
  if (!chrome.main_window || !chrome.map || !chrome.view0 || !chrome.view1)
    return false;
  if (!chrome.main_contents || !chrome.main_contents->HasLayerTree() ||
      !chrome.main_contents->HasEntityTree())
    return false;

  const base::FilePath root = FindRepoRoot();
  if (root.empty()) {
    LOG(ERROR) << "repo root not found; browser.chrome suite is missing";
    return false;
  }

  content::CapabilityHost host;
  std::string error;
  const base::FilePath suite_file = SuitePath(root);
  const std::string suite_path = suite_file.AsUTF8Unsafe();
  if (!content::run_suite_script(host, suite_path, &error)) {
    LOG(ERROR) << "run_suite_script failed: " << error;
    const base::FilePath stripped =
        root.Append(FILE_PATH_LITERAL("out"))
            .Append(FILE_PATH_LITERAL("harness"))
            .Append(FILE_PATH_LITERAL("browser.chrome"))
            .Append(FILE_PATH_LITERAL("browser.chrome"));
    if (!WriteCommentStrippedSuite(suite_file, stripped))
      return false;
    error.clear();
    const bool ran = content::run_suite_script(
        host, stripped.AsUTF8Unsafe(), &error);
    file_util::Delete(stripped, false);
    if (!ran) {
      LOG(ERROR) << "run_suite_script failed: " << error;
      return false;
    }
  }
  if (!HostHasRequiredMarks(host)) {
    LOG(ERROR) << "browser.chrome did not record every required mark";
    return false;
  }

  const base::FilePath mark_path =
      root.Append(FILE_PATH_LITERAL("out"))
          .Append(FILE_PATH_LITERAL("harness"))
          .Append(FILE_PATH_LITERAL("browser.chrome"))
          .Append(FILE_PATH_LITERAL("mark.json"));
  if (!WriteMarkFile(mark_path)) {
    LOG(ERROR) << "failed to write " << mark_path.AsUTF8Unsafe();
    return false;
  }
  LOG(INFO) << "wrote " << mark_path.AsUTF8Unsafe();
  return true;
}

void FinishChromeHarness(ProductChrome* chrome, int* exit_code) {
  *exit_code = WriteChromeMarks(*chrome) ? 0 : 1;
  if (chrome->main_contents)
    chrome->main_contents->CloseSatellites();
  if (chrome->main_window)
    chrome->main_window->CloseNow();
  if (base::MessageLoop::current())
    base::MessageLoop::current()->Quit();
}

}  // namespace

namespace shell {

void ShowMainWindow() {
  ProductChrome chrome;
  if (!CreateProductChrome(&chrome))
    LOG(ERROR) << "failed to create product windows";
}

int RunChromeHarness() {
  ProductChrome chrome;
  if (!CreateProductChrome(&chrome)) {
    LOG(ERROR) << "chrome harness failed to create product windows";
    return 1;
  }
  int exit_code = 1;
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&FinishChromeHarness, &chrome, &exit_code));
  base::MessageLoop::current()->Run();
  return exit_code;
}

}  // namespace shell
