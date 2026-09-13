// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "content/control/window/map_window.h"

#include "base/logging.h"
#include "base/path/base_paths.h"
#include "base/util/string_util.h"
#include "content/mapd_link.h"
#include "content/sdbd_link.h"
#include "content/spatial_link.h"
#include "gfx/2d/renderer/style.h"

#include <cstdio>
#include <string>
#include <vector>

#define ToDPoint(point) gfx2d::DPoint(point.x, point.y)

namespace content {
namespace {
static const UINT kRefreshTimer = 69;
static const UINT kNotifyTimer = 70;

const wchar_t* kWorldLayerFiles[] = {L"countries.geojson", L"graticule.geojson",
                                     L"capitals.geojson"};
const char* kWorldLayerNames[] = {"countries", "graticule", "capitals"};

void LogBoundLayers(const char* where, const std::vector<OGRLayer*>& layers) {
  LOG(INFO) << where << ": bind layers=" << layers.size();
  std::string text =
      std::string(where) + ": bind layers=" + std::to_string(layers.size()) +
      "\n";
  for (OGRLayer* layer : layers) {
    if (!layer) {
      continue;
    }
    const char* name = layer->GetName();
    const char* geom = OGRGeometryTypeToName(layer->GetGeomType());
    const GIntBig n = layer->GetFeatureCount();
    LOG(INFO) << where << ": layer=" << (name ? name : "?")
              << " geom=" << (geom ? geom : "?") << " features=" << n;
    text += "layer=";
    text += name ? name : "?";
    text += " geom=";
    text += geom ? geom : "?";
    text += " features=";
    text += std::to_string(static_cast<long long>(n));
    text += "\n";
  }
  FILE* f = nullptr;
  if (fopen_s(&f, "world_bind.log", "w") == 0 && f) {
    fwrite(text.data(), 1, text.size(), f);
    fclose(f);
  }
}

bool OpenWorldDemo(const base::PathString& dir, GDALDataset** out) {
  if (!out) {
    return false;
  }
  GDALDriver* mem = GetGDALDriverManager()->GetDriverByName("Memory");
  if (!mem) {
    LOG(ERROR) << "InitMap: Memory driver missing";
    return false;
  }
  GDALDataset* ds = mem->Create("world_demo", 0, 0, 0, GDT_Unknown, nullptr);
  if (!ds) {
    return false;
  }
  int copied = 0;
  for (int i = 0; i < 3; ++i) {
    const base::PathString path = dir + L"\\" + kWorldLayerFiles[i];
    const std::string utf8 = base::UTF16ToUTF8(path);
    GDALDataset* src = static_cast<GDALDataset*>(GDALOpenEx(
        utf8.c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY, nullptr, nullptr,
        nullptr));
    if (!src) {
      LOG(WARNING) << "InitMap: missing world layer " << utf8
                   << " gdal=" << CPLGetLastErrorMsg();
      continue;
    }
    OGRLayer* layer = src->GetLayer(0);
    if (layer && ds->CopyLayer(layer, kWorldLayerNames[i])) {
      ++copied;
    }
    GDALClose(src);
  }
  if (copied != 3) {
    GDALClose(ds);
    return false;
  }
  *out = ds;
  return true;
}

bool OpenShanghaiFallback(const base::PathString& module_dir,
                          GDALDataset** out) {
  const base::PathString candidates[] = {
      module_dir + L"\\data\\sh\\POLYGON.shp",
      module_dir + L"\\..\\core\\data\\sh\\POLYGON.shp",
  };
  std::string file_path;
  for (const auto& shp_path : candidates) {
    file_path = base::UTF16ToUTF8(shp_path);
    GDALDataset* ds = static_cast<GDALDataset*>(GDALOpenEx(
        file_path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (ds) {
      *out = ds;
      LOG(INFO) << "InitMap: opened fallback " << file_path
                << " layers=" << ds->GetLayerCount();
      return true;
    }
  }
  LOG(ERROR) << "InitMap: failed to open " << file_path
             << " gdal=" << CPLGetLastErrorMsg();
  return false;
}
}  // namespace

LRESULT MapWindow::OnCreate(LPCREATESTRUCT lpcs) {
  CMessageLoop* loop = _Module.GetMessageLoop();
  if (loop) {
    loop->AddMessageFilter(this);
    loop->AddIdleHandler(this);
  }

  if (!InitStyle()) {
    return -1;
  }

  // Demo shapefile is optional. A missing/unreadable file must not skip
  // renderer setup — OnPaint calls render_device_->Swap().
  InitMap();

  if (!InitRenderer()) {
    return -1;
  }

  if (!InitTool()) {
    return -1;
  }

  if (!InitMenu()) {
    return -1;
  }

  if (!InitTimer()) {
    return -1;
  }

  return 0;
}

void MapWindow::OnDestroy() {
  ::PostQuitMessage(0);
  ::KillTimer(m_hWnd, kRefreshTimer);
  ::KillTimer(m_hWnd, kNotifyTimer);

  content::ToolFactory::DestoryTool(navigate_tool_);
  content::ToolFactory::DestoryTool(select_tool_);
  content::ToolFactory::DestoryTool(flash_tool_);
  content::ToolFactory::DestoryTool(edit_tool_);

  ::DestroyMenu(m_hMainMenu);
  ::DestroyMenu(m_hContexMenu);

  if (render_device_) {
    render_device_->Unbind();
    render_device_ = nullptr;
  }
  if (renderer_) {
    renderer_->ReleaseDevice();
    delete renderer_;
    renderer_ = nullptr;
  }

  if (dataset_) {
    if (content::SpatialLinkKind() != "sdbd") {
      GDALClose(dataset_);
    }
    dataset_ = nullptr;
  }
}

void MapWindow::OnTimer(UINT_PTR event) {
  switch (event) {
    case kRefreshTimer: {
      auto& tool_manager = content::ToolManager::GetInstance();
      auto* tool =
          dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
      if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
        tool->Timer();
      }
    } break;
    case kNotifyTimer: {
      if (render_device_) {
        auto& environment = content::Environment::GetInstance();
        auto system_options = environment.get()->GetSystemOptions();
        gfx2d::RenderOptions options;
        options.show_mbr = system_options.show_mbr;
        options.show_point = system_options.show_point;
        options.point_radius = system_options.point_radius;

        render_device_->SetRenderOptions(options);
      }
    } break;
    default:
      break;
  }
}

void MapWindow::OnSize(UINT nType, CSize size) {
  auto& environment = content::Environment::GetInstance();
  if (render_device_ && nType != SIZE_MINIMIZED && size.cx > 0 &&
      size.cy > 0) {
    auto system_options = environment.get()->GetSystemOptions();
    gfx2d::RenderOptions options;
    options.show_mbr = system_options.show_mbr;
    options.show_point = system_options.show_point;
    options.point_radius = system_options.point_radius;

    gfx2d::DRect rect;
    rect.x = rect.y = 0;
    rect.width = size.cx;
    rect.height = size.cy;

    render_device_->SetRenderOptions(options);
    render_device_->Resize(rect);
    render_device_->Refresh(true);
  }
}
void MapWindow::OnPaint(HDC /*hDC*/) {
  auto& environment = content::Environment::GetInstance();
  auto& tool_manager = content::ToolManager::GetInstance();
  content::Tool* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());

  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->AuxDraw();
  }

  if (render_device_) {
    render_device_->Swap();
  }
}

BOOL MapWindow::OnEraseBkgnd(CDCHandle dc) { return TRUE; }
void MapWindow::OnMouseMove(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->MouseMove(nFlags, ToDPoint(point));
  }
}
BOOL MapWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->MouseWheel(nFlags, zDelta, ToDPoint(point));
  }
  return TRUE;
}
void MapWindow::OnLButtonDown(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonDown(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnLButtonUp(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonUp(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonDClick(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnRButtonDown(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonDown(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnRButtonUp(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonUp(nFlags, ToDPoint(point));

    if (tool->IsEnableContexMenu()) {
      CMenu contex_menu;
      contex_menu.Attach(m_hContexMenu);
      contex_menu.TrackPopupMenu(
          TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
          m_hWnd);
      contex_menu.Detach();
    }
  }
}
void MapWindow::OnRButtonDblClk(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonDClick(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->KeyDown(nChar, nRepCnt, nFlags);
  }
}
void MapWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->KeyUp(nChar, nRepCnt, nFlags);
  }
}
void MapWindow::OnContextMenu(CWindow wnd, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    if (tool->IsEnableContexMenu()) {
      CMenu contex_menu;
      contex_menu.Attach(m_hContexMenu);
      contex_menu.TrackPopupMenu(
          TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
          m_hWnd);
      contex_menu.Detach();
    }
  }
}
void MapWindow::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl) {
  content::MessageListener::Message message;
  message.id = nID;
  message.source_window = m_hWnd;

  if (nID == MESSAGE_CMD_LINK_MAPD) {
    OnLinkMapd();
    return;
  }
  if (nID == MESSAGE_CMD_LINK_SDBD) {
    OnLinkSdbd();
    return;
  }

  if (nID >= MESSAGE_CMD_BEGIN && nID <= MESSAGE_CMD_END) {
    auto& tool_manager = content::ToolManager::GetInstance();
    tool_manager.get()->Notify(TOOL_BROADCAST, message);
  }
}

void MapWindow::OnEnterSizeMove() {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnExitSizeMove() {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
#if (_WIN32_WINNT >= 0x0400)
void MapWindow::OnMouseHover(WPARAM wParam, CPoint ptPos) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnMouseLeave() {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
#endif /* _WIN32_WINNT >= 0x0400 */

bool MapWindow::InitStyle() {
  auto& environment = content::Environment::GetInstance();
  auto& style_manager = gfx2d::StyleManager::GetInstance();
  auto& style_options = environment.get()->GetSystemOptions().style_options;
  auto& flash_options = environment.get()->GetSystemOptions().flash_options;

  gfx2d::PenDesc pen_desc;
  gfx2d::BrushDesc brush_desc;
  gfx2d::AnnotationDesc anno_desc;
  gfx2d::SymbolDesc symbol_desc;

  style_manager.get()->SetDefaultStyle("Default", pen_desc, brush_desc,
                                       anno_desc, symbol_desc);

  auto* style1 = style_manager.get()->CreateStyle(
      style_options.point_style.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);
  auto* style2 = style_manager.get()->CreateStyle(
      style_options.curve_style.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);
  auto* style3 = style_manager.get()->CreateStyle(
      style_options.surface_style.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);
  auto* style4 = style_manager.get()->CreateStyle(
      style_options.aux_style.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);

  auto* style5 = style_manager.get()->CreateStyle(
      style_options.point_flash_style1.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);
  auto* style6 = style_manager.get()->CreateStyle(
      style_options.point_flash_style2.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);

  auto* style7 = style_manager.get()->CreateStyle(
      style_options.curve_flash_style1.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);
  auto* style8 = style_manager.get()->CreateStyle(
      style_options.curve_flash_style2.c_str(), pen_desc, brush_desc, anno_desc,
      symbol_desc);

  auto* style9 = style_manager.get()->CreateStyle(
      style_options.surface_flash_style1.c_str(), pen_desc, brush_desc,
      anno_desc, symbol_desc);
  auto* style10 = style_manager.get()->CreateStyle(
      style_options.surface_flash_style2.c_str(), pen_desc, brush_desc,
      anno_desc, symbol_desc);

  //////////////////////////////////////////////////////////////////////////
  style1->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc |
                       gfx2d::ST_AnnoDesc | gfx2d::ST_SymbolDesc);
  style2->SetStyleType(gfx2d::ST_PenDesc);
  style3->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc);
  style4->SetStyleType(gfx2d::ST_PenDesc);

  style5->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc |
                       gfx2d::ST_AnnoDesc | gfx2d::ST_SymbolDesc);
  style6->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc |
                       gfx2d::ST_AnnoDesc | gfx2d::ST_SymbolDesc);

  style7->SetStyleType(gfx2d::ST_PenDesc);
  style8->SetStyleType(gfx2d::ST_PenDesc);

  style9->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc);
  style10->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc);

  //////////////////////////////////////////////////////////////////////////
  // 1
  pen_desc.color = RGB(255, 0, 0);
  style1->SetPenDesc(pen_desc);

  brush_desc.color = RGB(0, 255, 255);
  style1->SetBrushDesc(brush_desc);

  symbol_desc.id = 0;
  symbol_desc.width = symbol_desc.height = 1.6f;
  style1->SetSymbolDesc(symbol_desc);

  //////////////////////////////////////////////////////////////////////////
  // 2
  pen_desc.color = RGB(0, 0, 255);
  style2->SetPenDesc(pen_desc);

  //////////////////////////////////////////////////////////////////////////
  // 3
  pen_desc.width = 0.001f;
  pen_desc.color = RGB(255, 0, 0);
  style3->SetPenDesc(pen_desc);

  brush_desc.color = RGB(77, 255, 0);
  style3->SetBrushDesc(brush_desc);

  //////////////////////////////////////////////////////////////////////////
  // 4
  pen_desc.color = RGB(255, 0, 0);
  style4->SetPenDesc(pen_desc);

  //////////////////////////////////////////////////////////////////////////
  // 5
  pen_desc.color = flash_options.color1;
  pen_desc.width = 0.002f;
  style5->SetPenDesc(pen_desc);

  brush_desc.color = flash_options.color2;
  style5->SetBrushDesc(brush_desc);

  anno_desc.color = flash_options.color1;
  style5->SetAnnoDesc(anno_desc);

  //////////////////////////////////////////////////////////////////////////
  // 6
  pen_desc.color = flash_options.color2;
  pen_desc.width = 0.002;
  style6->SetPenDesc(pen_desc);

  brush_desc.color = flash_options.color1;
  style6->SetBrushDesc(brush_desc);

  anno_desc.color = flash_options.color2;
  style6->SetAnnoDesc(anno_desc);

  //////////////////////////////////////////////////////////////////////////
  // 7
  pen_desc.color = flash_options.color1;
  pen_desc.width = 0.002;
  style7->SetPenDesc(pen_desc);

  //////////////////////////////////////////////////////////////////////////
  // 8
  pen_desc.color = flash_options.color2;
  pen_desc.width = 0.002;
  style8->SetPenDesc(pen_desc);

  //////////////////////////////////////////////////////////////////////////
  // 9
  pen_desc.color = flash_options.color1;
  pen_desc.width = 0.002;
  style9->SetPenDesc(pen_desc);

  brush_desc.color = flash_options.color2;
  style9->SetBrushDesc(brush_desc);

  //////////////////////////////////////////////////////////////////////////
  // 10
  pen_desc.color = flash_options.color2;
  pen_desc.width = 0.002;
  style10->SetPenDesc(pen_desc);

  brush_desc.color = flash_options.color1;
  style10->SetBrushDesc(brush_desc);

  return true;
}
bool MapWindow::InitMap() {
  CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
  CPLSetConfigOption("SHAPE_ENCODING", "");
  GDALAllRegister();

  std::string link_url;
  if (content::SpatialLinkRequested(&link_url)) {
    if (!content::TryOpenSpatialLink(link_url)) {
      LOG(ERROR) << "InitMap: " << content::SpatialLinkKind()
                 << " 未就绪 not_ready url=" << link_url
                 << " err=" << content::SpatialLinkError();
      return false;
    }
    dataset_ = content::TakeSpatialLinkDataset();
    LOG(INFO) << "InitMap: spatial link kind=" << content::SpatialLinkKind()
              << " url=" << link_url
              << " layers=" << (dataset_ ? dataset_->GetLayerCount() : 0);
    return true;
  }

  base::PathString module_dir;
  base::PathProvider(base::FILE_MODULE_DIR, &module_dir);
  const base::PathString world_dirs[] = {
      module_dir + L"\\data\\world",
      module_dir + L"\\..\\core\\data\\world",
  };
  for (const auto& dir : world_dirs) {
    if (OpenWorldDemo(dir, &dataset_)) {
      LOG(INFO) << "InitMap: opened world demo "
                << base::UTF16ToUTF8(dir)
                << " layers=" << dataset_->GetLayerCount();
      return true;
    }
  }

  if (OpenShanghaiFallback(module_dir, &dataset_)) {
    return true;
  }
  return false;
}
bool MapWindow::InitRenderer() {
  auto& environment = content::Environment::GetInstance();
  renderer_ = new gfx2d::Renderer(::GetModuleHandle(NULL));
  if (ERR_NONE != renderer_->CreateDevice(L"render_device_gdi")) {
    return false;
  }

  render_device_ = renderer_->GetDevice();
  if (!render_device_) {
    return false;
  }

  render_device_->Init(m_hWnd);
  render_device_->SetMapMode(MM_TEXT);

  std::vector<OGRLayer*> layers;
  gfx2d::DRect drect;
  drect.x = drect.y = 0;
  drect.width = 1;
  drect.height = 1;

  RECT client{};
  ::GetClientRect(m_hWnd, &client);
  if (client.right > 0) {
    drect.width = client.right;
  }
  if (client.bottom > 0) {
    drect.height = client.bottom;
  }

  gfx2d::LRect lrect;
  lrect.x = 0;
  lrect.y = 0;
  lrect.width = 1;
  lrect.height = 1;

  if (dataset_) {
    OGREnvelope envelop;
    auto layer_count = dataset_->GetLayerCount();
    for (size_t i = 0; i < layer_count; i++) {
      auto* layer = dataset_->GetLayer(i);
      layers.push_back(layer);
      OGREnvelope layer_envelop;
      layer->GetExtent(&layer_envelop);
      envelop.Merge(layer_envelop);
    }

    if (envelop.MaxX > envelop.MinX && envelop.MaxY > envelop.MinY) {
      lrect.x = envelop.MinX;
      lrect.y = envelop.MinY;
      lrect.width = envelop.MaxX - envelop.MinX;
      lrect.height = envelop.MaxY - envelop.MinY;
    }
  }

  map_extent_ = lrect;
  auto system_options = environment.get()->GetSystemOptions();
  gfx2d::RenderOptions options;
  options.show_mbr = system_options.show_mbr;
  options.show_point = true;
  options.point_radius = system_options.point_radius;
  render_device_->SetRenderOptions(options);
  LogBoundLayers("InitRenderer", layers);
  render_device_->Bind(layers);
  render_device_->Resize(drect);
  render_device_->ZoomToRect(lrect);
  return true;
}
bool MapWindow::InitTool() {
  auto& environment = content::Environment::GetInstance();
  auto& style_manager = gfx2d::StyleManager::GetInstance();
  auto& style_options = environment.get()->GetSystemOptions().style_options;
  content::ToolFactory::CreateTool(navigate_tool_,
                                   content::ToolFactory::Navigate);
  content::ToolFactory::CreateTool(select_tool_, content::ToolFactory::Select);
  content::ToolFactory::CreateTool(flash_tool_, content::ToolFactory::Flash);
  content::ToolFactory::CreateTool(edit_tool_, content::ToolFactory::Edit);

  navigate_tool_->SetToolStyleName(style_options.aux_style.c_str());
  if (ERR_NONE != navigate_tool_->Init(m_hWnd, render_device_)) {
    return false;
  }

  select_tool_->SetToolStyleName(style_options.aux_style.c_str());
  if (ERR_NONE != select_tool_->Init(m_hWnd, render_device_)) {
    return false;
  }

  flash_tool_->SetToolStyleName(style_options.aux_style.c_str());
  if (ERR_NONE != flash_tool_->Init(m_hWnd, render_device_)) {
    return false;
  }

  edit_tool_->SetToolStyleName(style_options.aux_style.c_str());
  if (ERR_NONE != edit_tool_->Init(m_hWnd, render_device_)) {
    return false;
  }

  navigate_tool_->SetActive();

  return true;
}
bool MapWindow::InitMenu() {
  auto& environment = content::Environment::GetInstance();
  m_hMainMenu = ::CreatePopupMenu();
  m_hContexMenu = ::CreatePopupMenu();

  HMENU hMenu =
      content::CreateListenerMenu(navigate_tool_, content::FIG_2DMFMENU);
  if (GetMenuItemCount(hMenu) > 0)
    AppendMenu(m_hMainMenu, MF_POPUP, (UINT)hMenu, navigate_tool_->GetName());

  content::AppendListenerMenu(m_hContexMenu, navigate_tool_,
                              content::FIG_2DVIEW, false);
  content::AppendListenerMenu(m_hContexMenu, select_tool_, content::FIG_2DVIEW,
                              true);
  content::AppendListenerMenu(m_hContexMenu, flash_tool_, content::FIG_2DVIEW,
                              true);
  content::AppendListenerMenu(m_hContexMenu, edit_tool_, content::FIG_2DVIEW,
                              true);

  AppendMenu(m_hMainMenu, MF_STRING, MESSAGE_CMD_LINK_MAPD, TEXT("链接 mapd"));
  AppendMenu(m_hMainMenu, MF_STRING, MESSAGE_CMD_LINK_SDBD, TEXT("链接 sdbd"));
  AppendMenu(m_hContexMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenu(m_hContexMenu, MF_STRING, MESSAGE_CMD_LINK_MAPD, TEXT("链接 mapd"));
  AppendMenu(m_hContexMenu, MF_STRING, MESSAGE_CMD_LINK_SDBD, TEXT("链接 sdbd"));

  return true;
}

void MapWindow::BindOpenedDataset() {
  if (!render_device_) {
    return;
  }
  render_device_->Unbind();

  std::vector<OGRLayer*> layers;
  gfx2d::LRect lrect;
  lrect.x = 0;
  lrect.y = 0;
  lrect.width = 1;
  lrect.height = 1;

  if (dataset_) {
    OGREnvelope envelop;
    const int layer_count = dataset_->GetLayerCount();
    for (int i = 0; i < layer_count; ++i) {
      auto* layer = dataset_->GetLayer(i);
      layers.push_back(layer);
      OGREnvelope layer_envelop;
      layer->GetExtent(&layer_envelop);
      envelop.Merge(layer_envelop);
    }
    if (envelop.MaxX > envelop.MinX && envelop.MaxY > envelop.MinY) {
      lrect.x = envelop.MinX;
      lrect.y = envelop.MinY;
      lrect.width = envelop.MaxX - envelop.MinX;
      lrect.height = envelop.MaxY - envelop.MinY;
    }
  }

  map_extent_ = lrect;
  LogBoundLayers("BindOpenedDataset", layers);
  render_device_->Bind(layers);
  render_device_->ZoomToRect(lrect);
  render_device_->Refresh(true);
}

void MapWindow::OnLinkMapd() {
  const std::string url = content::DefaultMapdBaseUrl();
  if (render_device_) {
    render_device_->Unbind();
  }
  if (dataset_) {
    if (content::SpatialLinkKind() != "sdbd") {
      GDALClose(dataset_);
    }
    dataset_ = nullptr;
  }
  if (!content::TryOpenSpatialLink(url)) {
    LOG(ERROR) << "链接 mapd: 未就绪 not_ready url=" << url
               << " err=" << content::SpatialLinkError();
    BindOpenedDataset();
    return;
  }
  dataset_ = content::TakeSpatialLinkDataset();
  LOG(INFO) << "链接 mapd: opened " << url
            << " layers=" << content::SpatialLinkLayerCount();
  BindOpenedDataset();
}

void MapWindow::OnLinkSdbd() {
  const std::string url = content::DefaultSdbdBaseUrl();
  if (render_device_) {
    render_device_->Unbind();
  }
  if (dataset_) {
    if (content::SpatialLinkKind() != "sdbd") {
      GDALClose(dataset_);
    }
    dataset_ = nullptr;
  }
  if (!content::TryOpenSpatialLink(url)) {
    LOG(ERROR) << "链接 sdbd: 未就绪 not_ready url=" << url
               << " err=" << content::SpatialLinkError();
    BindOpenedDataset();
    return;
  }
  dataset_ = content::TakeSpatialLinkDataset();
  LOG(INFO) << "链接 sdbd: opened " << url
            << " kind=" << content::SpatialLinkKind()
            << " layers=" << content::SpatialLinkLayerCount();
  BindOpenedDataset();
}

bool MapWindow::InitTimer() {
  auto& environment = content::Environment::GetInstance();
  auto system_options = environment.get()->GetSystemOptions();
  m_uiRefreshTimer = ::SetTimer(m_hWnd, kRefreshTimer,
                                system_options.view2d_refresh_elapse, 0);
  m_uiNotifyTimer =
      ::SetTimer(m_hWnd, kNotifyTimer, system_options.view2d_notify_elapse, 0);
  return true;
}
}  // namespace content
