// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "main_window.h"

#include <algorithm>
#include <cmath>
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
#include "native/mview_map/mview_map.h"
#include "tool/input_event.h"
#include "tool/workspace.h"
#include "ui/base/models/tree_node_model.h"
#include "ui/gfx/rect.h"
#include "ui/views/controls/menu/menu.h"
#include "ui/views/controls/single_split_view.h"
#include "ui/views/controls/slider.h"
#include "ui/views/controls/tree/tree_view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/win/hwnd_util.h"

#include "globe_view.h"

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

// Render-check destroys the widgets without running the message loop.
// Quit() outside Run() is a fatal check in this tree.
bool g_quit_when_main_closes = true;

// Popup "Window" menu. Items name the other product widgets.
class WindowMenuDelegate : public views::Menu::Delegate {
 public:
  WindowMenuDelegate() {}
  virtual ~WindowMenuDelegate() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(WindowMenuDelegate);
};

// Map or 3D viewport. Pointer and wheel go through tool::Workspace, then into
// the child HWND renderer (MapLibre/OpenGL mercator, or the textured globe).
class ViewportWindow : public views::WidgetDelegateView {
 public:
  enum Kind { kMap, kView0, kView1 };

  ViewportWindow(Kind kind, const string16& title)
      : kind_(kind),
        title_(title),
        workspace_(kind == kMap ? "map" : (kind == kView0 ? "view0" : "view1")),
        map_session_(NULL),
        pixel_ratio_(1.0),
        present_enabled_(false),
        frame_presented_(false),
        renderer_name_(""),
        dragging_(false),
        last_x_(0.0),
        last_y_(0.0) {
    if (kind_ == kMap)
      workspace_.execute("view.pan");
    else
      workspace_.execute("view3d.trackball");
  }
  virtual ~ViewportWindow() {
    if (map_session_) {
      mvm_destroy(map_session_);
      map_session_ = NULL;
    }
  }

  virtual bool CanResize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE { return title_; }
  virtual views::View* GetContentsView() OVERRIDE { return this; }

  void BindNative() {
    views::Widget* widget = GetWidget();
    if (!widget)
      return;
    HWND parent = views::HWNDForWidget(widget);
    if (!parent)
      return;
    RECT client;
    GetClientRect(parent, &client);
    int width = client.right - client.left;
    int height = client.bottom - client.top;
    if (width < 2)
      width = 640;
    if (height < 2)
      height = 480;
    if (kind_ == kMap) {
      map_session_ = mvm_create(parent, width, height, pixel_ratio_);
      if (!map_session_)
        return;
      mvm_set_camera(map_session_, 10.0, 20.0, 1.0, 0.0, 0.0);
      mvm_set_pointer_callback(map_session_, &ViewportWindow::PointerThunk, this);
    } else {
      const double yaw = kind_ == kView1 ? 1.3 : 0.35;
      if (!globe_.Attach(parent, width, height, yaw))
        return;
      globe_.SetPointerCallback(&ViewportWindow::PointerThunk, this);
    }
  }

  void EnablePresent() { present_enabled_ = true; }
  bool frame_presented() const { return frame_presented_; }
  const char* renderer_name() const { return renderer_name_; }
  int draw_calls() const {
    if (kind_ == kMap)
      return map_session_ ? mvm_last_draw_calls(map_session_) : 0;
    return globe_.draw_calls();
  }

  void PresentFrame() {
    if (kind_ == kMap) {
      if (!map_session_)
        return;
      const int code = mvm_present(map_session_);
      if (code > 0) {
        frame_presented_ = true;
        renderer_name_ = code == 2 ? "maplibre-native" : "opengl-mercator";
      }
      return;
    }
    if (globe_.Present()) {
      frame_presented_ = true;
      renderer_name_ = "opengl-textured-globe";
    }
  }

  // Presents, moves the camera, presents again. True only when both frames
  // drew real pixels and the second frame differs from the first.
  bool ConfirmBrowsedFrame() {
    EnablePresent();
    PresentFrame();
    const unsigned first = FrameToken();
    if (kind_ == kMap && map_session_) {
      double lon = 0.0;
      double lat = 0.0;
      double zoom = 0.0;
      double bearing = 0.0;
      double pitch = 0.0;
      mvm_get_camera(map_session_, &lon, &lat, &zoom, &bearing, &pitch);
      mvm_set_camera(map_session_, lon + 40.0, lat, zoom, bearing, pitch);
    } else {
      globe_.Orbit(80.0, 18.0);
    }
    PresentFrame();
    const unsigned second = FrameToken();
    frame_presented_ = first != 0 && second != 0 && first != second;
    return frame_presented_;
  }

  virtual void OnBoundsChanged(const gfx::Rect& previous_bounds) OVERRIDE {
    views::WidgetDelegateView::OnBoundsChanged(previous_bounds);
    if (width() < 2 || height() < 2)
      return;
    if (map_session_)
      mvm_resize(map_session_, width(), height(), pixel_ratio_);
    else if (globe_.attached())
      globe_.Resize(width(), height());
    if (present_enabled_)
      PresentFrame();
  }

  void OnPointer(int kind,
                 int action,
                 double x,
                 double y,
                 double wheel_dx,
                 double wheel_dy) {
    if (!present_enabled_)
      return;
    tool::InputEvent event;
    event.kind = kind == static_cast<int>(tool::kWheel) ? tool::kWheel
                                                        : tool::kMouse;
    if (action == static_cast<int>(tool::kMousePress))
      event.action = tool::kMousePress;
    else if (action == static_cast<int>(tool::kMouseRelease))
      event.action = tool::kMouseRelease;
    else
      event.action = tool::kMouseMove;
    event.view_id = workspace_.view_id();
    event.x_px = x;
    event.y_px = y;
    event.wheel_dx = wheel_dx;
    event.wheel_dy = wheel_dy;
    if (!workspace_.dispatch(event))
      return;
    if (event.kind == tool::kWheel) {
      ApplyZoom(wheel_dy);
    } else if (event.action == tool::kMousePress) {
      dragging_ = true;
      last_x_ = x;
      last_y_ = y;
    } else if (event.action == tool::kMouseMove && dragging_) {
      ApplyPan(x - last_x_, y - last_y_);
      last_x_ = x;
      last_y_ = y;
    } else if (event.action == tool::kMouseRelease) {
      dragging_ = false;
    }
    PresentFrame();
  }

 private:
  static void PointerThunk(void* user,
                           int kind,
                           int action,
                           double x,
                           double y,
                           double wheel_dx,
                           double wheel_dy) {
    if (!user)
      return;
    static_cast<ViewportWindow*>(user)->OnPointer(kind, action, x, y, wheel_dx,
                                                  wheel_dy);
  }

  unsigned FrameToken() const {
    if (kind_ == kMap)
      return map_session_ ? mvm_frame_token(map_session_) : 0;
    return globe_.frame_token();
  }

  void ApplyPan(double dx, double dy) {
    if (kind_ == kMap) {
      if (!map_session_)
        return;
      double lon = 0.0;
      double lat = 0.0;
      double zoom = 0.0;
      double bearing = 0.0;
      double pitch = 0.0;
      mvm_get_camera(map_session_, &lon, &lat, &zoom, &bearing, &pitch);
      const double degrees = 360.0 / (256.0 * std::pow(2.0, zoom));
      lon -= dx * degrees;
      lat += dy * degrees;
      while (lon > 180.0)
        lon -= 360.0;
      while (lon < -180.0)
        lon += 360.0;
      if (lat > 85.0)
        lat = 85.0;
      if (lat < -85.0)
        lat = -85.0;
      mvm_set_camera(map_session_, lon, lat, zoom, bearing, pitch);
      return;
    }
    globe_.Orbit(dx, dy);
  }

  void ApplyZoom(double wheel_dy) {
    if (kind_ == kMap) {
      if (!map_session_)
        return;
      double lon = 0.0;
      double lat = 0.0;
      double zoom = 0.0;
      double bearing = 0.0;
      double pitch = 0.0;
      mvm_get_camera(map_session_, &lon, &lat, &zoom, &bearing, &pitch);
      zoom += wheel_dy;
      if (zoom < 0.0)
        zoom = 0.0;
      if (zoom > 20.0)
        zoom = 20.0;
      mvm_set_camera(map_session_, lon, lat, zoom, bearing, pitch);
      return;
    }
    globe_.Zoom(wheel_dy);
  }

  Kind kind_;
  string16 title_;
  tool::Workspace workspace_;
  MvmMap* map_session_;
  GlobeView globe_;
  double pixel_ratio_;
  bool present_enabled_;
  bool frame_presented_;
  const char* renderer_name_;
  bool dragging_;
  double last_x_;
  double last_y_;

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
    if (g_quit_when_main_closes)
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
  ViewportWindow* map;
  ViewportWindow* view0;
  ViewportWindow* view1;
};

ViewportWindow* ShowViewport(ViewportWindow::Kind kind,
                             const char* title,
                             const gfx::Rect& bounds) {
  ViewportWindow* view = new ViewportWindow(kind, ASCIIToUTF16(title));
  views::Widget* widget = views::Widget::CreateWindowWithBounds(view, bounds);
  if (!widget) {
    delete view;
    return NULL;
  }
  widget->Show();
  view->BindNative();
  return view;
}

bool CreateProductChrome(ProductChrome* chrome) {
  chrome->main_contents = new MainWindowContents();
  chrome->main_window = views::Widget::CreateWindowWithBounds(
      chrome->main_contents, gfx::Rect(80, 80, 960, 640));
  if (!chrome->main_window)
    return false;
  chrome->main_window->Show();
  chrome->main_contents->EnsureWindowMenu();

  chrome->map = ShowViewport(ViewportWindow::kMap, "MView Map",
                             gfx::Rect(200, 120, 800, 600));
  chrome->view0 = ShowViewport(ViewportWindow::kView0, "MView view0",
                               gfx::Rect(240, 160, 720, 520));
  chrome->view1 = ShowViewport(ViewportWindow::kView1, "MView view1",
                               gfx::Rect(280, 200, 720, 520));
  if (!chrome->map || !chrome->view0 || !chrome->view1)
    return false;
  if (!chrome->map->GetWidget() || !chrome->view0->GetWidget() ||
      !chrome->view1->GetWidget())
    return false;
  if (!chrome->main_contents->HasLayerTree() ||
      !chrome->main_contents->HasEntityTree())
    return false;

  chrome->main_contents->SetSatellites(chrome->map->GetWidget(),
                                       chrome->view0->GetWidget(),
                                       chrome->view1->GetWidget());
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
  if (!chrome.map->GetWidget() || !chrome.view0->GetWidget() ||
      !chrome.view1->GetWidget())
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
  if (!CreateProductChrome(&chrome)) {
    LOG(ERROR) << "failed to create product windows";
    return;
  }
  if (chrome.map)
    chrome.map->EnablePresent();
  if (chrome.view0)
    chrome.view0->EnablePresent();
  if (chrome.view1)
    chrome.view1->EnablePresent();
  if (chrome.map)
    chrome.map->PresentFrame();
  if (chrome.view0)
    chrome.view0->PresentFrame();
  if (chrome.view1)
    chrome.view1->PresentFrame();
  LOG(INFO) << "map renderer " << (chrome.map ? chrome.map->renderer_name() : "");
  LOG(INFO) << "view0 renderer "
            << (chrome.view0 ? chrome.view0->renderer_name() : "");
  LOG(INFO) << "view1 renderer "
            << (chrome.view1 ? chrome.view1->renderer_name() : "");
}

bool WriteRenderMark(const ProductChrome& chrome) {
  const bool map_frame = chrome.map && chrome.map->frame_presented();
  const bool view0_frame = chrome.view0 && chrome.view0->frame_presented();
  const char* map_renderer =
      chrome.map ? chrome.map->renderer_name() : "";
  const char* view0_renderer =
      chrome.view0 ? chrome.view0->renderer_name() : "";
  const int map_draws = chrome.map ? chrome.map->draw_calls() : 0;
  const int view0_draws = chrome.view0 ? chrome.view0->draw_calls() : 0;

  std::string json = "{\n  \"map.frame\": ";
  json += map_frame ? "true" : "false";
  json += ",\n  \"view0.frame\": ";
  json += view0_frame ? "true" : "false";
  json += ",\n  \"map.renderer\": \"";
  json += map_renderer ? map_renderer : "";
  json += "\",\n  \"view0.renderer\": \"";
  json += view0_renderer ? view0_renderer : "";
  json += "\",\n  \"map.draw_calls\": ";
  json += std::to_string(static_cast<long long>(map_draws));
  json += ",\n  \"view0.draw_calls\": ";
  json += std::to_string(static_cast<long long>(view0_draws));
  json += "\n}\n";

  const base::FilePath root = FindRepoRoot();
  if (root.empty()) {
    LOG(ERROR) << "repo root not found; render-check mark was not written";
    return false;
  }
  const base::FilePath mark_path =
      root.Append(FILE_PATH_LITERAL("out"))
          .Append(FILE_PATH_LITERAL("harness"))
          .Append(FILE_PATH_LITERAL("render-check"))
          .Append(FILE_PATH_LITERAL("mark.json"));
  if (!file_util::CreateDirectory(mark_path.DirName()))
    return false;
  const int wrote = file_util::WriteFile(
      mark_path, json.data(), static_cast<int>(json.size()));
  if (wrote != static_cast<int>(json.size()))
    return false;
  LOG(INFO) << "wrote " << mark_path.AsUTF8Unsafe();
  return map_frame && view0_frame;
}

int RunRenderCheck() {
  ProductChrome chrome;
  if (!CreateProductChrome(&chrome)) {
    LOG(ERROR) << "render-check failed to create product windows";
    return 1;
  }
  const bool map_ok = chrome.map && chrome.map->ConfirmBrowsedFrame();
  const bool view0_ok = chrome.view0 && chrome.view0->ConfirmBrowsedFrame();
  if (chrome.view1) {
    chrome.view1->EnablePresent();
    chrome.view1->PresentFrame();
  }
  LOG(INFO) << "render-check map=" << (map_ok ? "ok" : "fail")
            << " renderer=" << (chrome.map ? chrome.map->renderer_name() : "");
  LOG(INFO) << "render-check view0=" << (view0_ok ? "ok" : "fail")
            << " renderer="
            << (chrome.view0 ? chrome.view0->renderer_name() : "");
  const bool wrote = WriteRenderMark(chrome);
  g_quit_when_main_closes = false;
  if (chrome.main_contents)
    chrome.main_contents->CloseSatellites();
  if (chrome.main_window)
    chrome.main_window->CloseNow();
  return wrote && map_ok && view0_ok ? 0 : 1;
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
