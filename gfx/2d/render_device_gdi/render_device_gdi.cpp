
#include "gfx/2d/render_device_gdi/render_device_gdi.h"

#include <math.h>

#include "base/logging.h"
#include "gfx/2d/render_device_gdi/utils.h"

#pragma warning(disable : 4244)

namespace gfx2d {
const float kDelay = 0.25;

int CreateRenderDevice(HINSTANCE instance, HRENDERDEVICE &render_device) {
  if (!render_device) {
    render_device = new RenderDeviceGDI(instance);
    return ERR_NONE;
  }

  return ERR_FAILURE;
}

int DestroyRenderDevice(HRENDERDEVICE &render_device) {
  if (!render_device) {
    return ERR_FAILURE;
  }

  SAFE_DELETE(render_device);

  return ERR_NONE;
}

RenderDeviceGDI::RenderDeviceGDI(HINSTANCE instance)
    : RenderDevice(instance),
      font_(NULL),
      pen_(NULL),
      brush_(NULL),
      icon_(NULL),
      old_font_(NULL),
      old_pen_(NULL),
      old_brush_(NULL),
      current_dc_(NULL),
      anno_angle_(0.),
      last_redraw_command_stamp_(0),
      last_redraw_stamp_(0),
      is_redraw_(false),
      use_current_style_(false),
      is_lock_style_(false) {
  rhi_api_ = RHI2D_GDI;
  current_dop_.x = 0;
  current_dop_.y = 0;
  anno_name_ = "Smart Gis";
}

RenderDeviceGDI::~RenderDeviceGDI(void) { Release(); }

int RenderDeviceGDI::Init(HWND hwnd) {
  if (hwnd == NULL) {
    LOG(ERROR) << __FUNCTION__ << "hwnd is nullptr!";
    return ERR_INVALID_PARAM;
  }

  hwnd_ = hwnd;
  render_buffer_.SetHWND(hwnd_);
  composit_render_buffer_.SetHWND(hwnd_);
  LOG(INFO) << __FUNCTION__ << "Init Gdi RenderDevice ok!";

  return ERR_NONE;
}

int RenderDeviceGDI::Destroy(void) {
  LOG(INFO) << __FUNCTION__ << "Destroy Gdi RenderDevice ok!";
  return ERR_NONE;
}

int RenderDeviceGDI::Release(void) {
  if (font_) {
    ::DeleteObject(font_);
    font_ = NULL;
  }

  if (pen_) {
    ::DeleteObject(pen_);
    pen_ = NULL;
  }

  if (brush_) {
    ::DeleteObject(brush_);
    brush_ = NULL;
  }

  if (icon_) {
    ::DeleteObject(icon_);
    icon_ = NULL;
  }

  LOG(INFO) << __FUNCTION__ << "Release Gdi RenderDevice ok!";

  return ERR_NONE;
}

int RenderDeviceGDI::Resize(DRect rect) {
  if (rect.width < 0 || rect.height < 0) {
    return ERR_FAILURE;
  }

  if (::gfx2d::equal(viewport_, rect)) {
    return ERR_FAILURE;
  }

  viewport_.x = rect.x;
  viewport_.y = rect.y;
  viewport_.height = rect.height;
  viewport_.width = rect.width;

  zoomin_viewport_ = viewport_;
  zoomout_viewport_ = viewport_;

  float xblc, yblc;
  xblc = viewport_.width / windowport_.width;
  yblc = viewport_.height / windowport_.height;

  blc_ = (xblc > yblc) ? yblc : xblc;

  if (ERR_NONE == render_buffer_.SetSize(viewport_.width, viewport_.height) &&
      ERR_NONE ==
          composit_render_buffer_.SetSize(viewport_.width, viewport_.height)) {
    if (ERR_NONE == render_buffer_.Swap(viewport_.x, viewport_.y,
                                        viewport_.width, viewport_.height,
                                        viewport_.x, viewport_.y) &&
        ERR_NONE == composit_render_buffer_.Swap(
                        viewport_.x, viewport_.y, viewport_.width,
                        viewport_.height, viewport_.x, viewport_.y)) {
      return ERR_NONE;
    }
  }

  return ERR_FAILURE;
}

int RenderDeviceGDI::LPToDP(float x, float y, long &X, long &Y) const {
  if (::gfx2d::is_empty(windowport_) && ::gfx2d::is_empty(viewport_)) {
    X = x;
    Y = y;
    return ERR_FAILURE;
  }

  X = long(viewport_.x + (x - windowport_.x) * blc_ + 0.5);
  Y = long(viewport_.y + (y - windowport_.y) * blc_ + 0.5);

  Y = viewport_.height - Y;

  return ERR_NONE;
}

int RenderDeviceGDI::DPToLP(long X, long Y, float &x, float &y) const {
  if (::gfx2d::is_empty(windowport_) && ::gfx2d::is_empty(viewport_)) {
    x = X;
    y = Y;
    return ERR_FAILURE;
  }

  Y = viewport_.height - Y;

  x = (X - viewport_.x) / blc_ + windowport_.x;
  y = (Y - viewport_.y) / blc_ + windowport_.y;

  return ERR_NONE;
}

int RenderDeviceGDI::LRectToDRect(const LRect &lrect, DRect &drect) const {
  long X = 0, Y = 0;
  LPToDP(lrect.x, lrect.y, drect.x, drect.y);
  LPToDP(lrect.x + lrect.width, lrect.y + lrect.height, X, Y);

  drect.width = X - drect.x;
  drect.height = Y - drect.y;

  return ERR_NONE;
}

int RenderDeviceGDI::DRectToLRect(const DRect &drect, LRect &lrect) const {
  float x = 0, y = 0;
  DPToLP(drect.x, drect.y, lrect.x, lrect.y);
  DPToLP(drect.x + drect.width, drect.y + drect.height, x, y);

  lrect.width = x - lrect.x;
  lrect.height = y - lrect.y;

  return ERR_NONE;
}

int RenderDeviceGDI::Refresh() {
  int invalidate_x1, invalidate_y1, invalidate_w1, invalidate_h1;
  int invalidate_x2, invalidate_y2, invalidate_w2, invalidate_h2;
  if (current_dop_.x >= 0) {
    if (current_dop_.y >= 0) {
      invalidate_x1 = invalidate_y1 = 0;
      invalidate_w1 = viewport_.width, invalidate_h1 = current_dop_.y;
      invalidate_x2 = 0, invalidate_y2 = current_dop_.y;
      invalidate_w2 = current_dop_.x,
      invalidate_h2 = viewport_.height - current_dop_.y;
    } else {
      invalidate_x1 = invalidate_y1 = 0;
      invalidate_w1 = current_dop_.x,
      invalidate_h1 = viewport_.height + current_dop_.y;
      invalidate_x2 = 0, invalidate_y2 = viewport_.height + current_dop_.y;
      invalidate_w2 = viewport_.width, invalidate_h2 = -current_dop_.y;
    }
  } else {
    if (current_dop_.y >= 0) {
      invalidate_x1 = invalidate_y1 = 0;
      invalidate_w1 = viewport_.width, invalidate_h1 = current_dop_.y;
      invalidate_x2 = viewport_.width + current_dop_.x,
      invalidate_y2 = current_dop_.y;
      invalidate_w2 = -current_dop_.x,
      invalidate_h2 = viewport_.height - current_dop_.y;
    } else {
      invalidate_x1 = viewport_.width + current_dop_.x, invalidate_y1 = 0;
      invalidate_w1 = -current_dop_.x,
      invalidate_h1 = viewport_.height + current_dop_.y;
      invalidate_x2 = 0, invalidate_y2 = viewport_.height + current_dop_.y;
      invalidate_w2 = viewport_.width, invalidate_h2 = -current_dop_.y;
    }
  }

  HDC dc = ::GetDC(hwnd_);
  ClearRect(dc, invalidate_x1, invalidate_y1, invalidate_w1, invalidate_h1);
  ClearRect(dc, invalidate_x2, invalidate_y2, invalidate_w2, invalidate_h2);

  render_buffer_.Clear(viewport_.x, viewport_.y, viewport_.width,
                       viewport_.width);

  composit_render_buffer_.Swap(
      render_buffer_, zoomin_viewport_.x, zoomin_viewport_.y,
      zoomin_viewport_.width, zoomin_viewport_.height, zoomout_viewport_.x,
      zoomout_viewport_.y, zoomout_viewport_.width, zoomout_viewport_.height,
      RenderBuffer::BLT_TRANSPARENT, SRCCOPY);

  render_buffer_.Swap(current_dop_.x, current_dop_.y, viewport_.width,
                      viewport_.height, viewport_.x, viewport_.y);

  ::ReleaseDC(hwnd_, dc);

  RECT rt;
  GetClientRect(hwnd_, &rt);
  InvalidateRect(hwnd_, &rt, true);

  return ERR_NONE;
}

int RenderDeviceGDI::Refresh(LRect lrect) {
  DRect drect;
  LRectToDRect(lrect, drect);
  RefreshDirectly(drect);

  return ERR_NONE;
}

int RenderDeviceGDI::RefreshDirectly(DRect rect, bool realtime) {
  return Redraw();
}

int RenderDeviceGDI::ZoomMove(LPoint offset, bool realtime) {
  zoomin_viewport_.x += offset.x * blc_;
  zoomin_viewport_.y -= offset.y * blc_;

  windowport_.x -= offset.x;
  windowport_.y -= offset.y;

  return Redraw();
}

int RenderDeviceGDI::ZoomScale(LPoint original_point, float scale,
                               bool realtime) {
  if (scale > 1.) {
    // for zoomin
    zoomin_viewport_.height /= scale;
    zoomin_viewport_.width /= scale;
    zoomin_viewport_.x =
        original_point.x - (original_point.x - zoomin_viewport_.x) / scale;
    zoomin_viewport_.y =
        original_point.y - (original_point.y - zoomin_viewport_.y) / scale;
  } else {
    // for zoomout
    zoomout_viewport_.height *= scale;
    zoomout_viewport_.width *= scale;
    zoomout_viewport_.x =
        original_point.x + (zoomout_viewport_.x - original_point.x) * scale;
    zoomout_viewport_.y =
        original_point.y + (zoomout_viewport_.y - original_point.y) * scale;
  }

  float x1, y1, x2, y2;
  DPToLP(original_point.x, original_point.y, x1, y1);

  windowport_.height *= scale;
  windowport_.width *= scale;

  blc_ /= scale;

  DPToLP(original_point.x, original_point.y, x2, y2);

  windowport_.x -= x2 - x1;
  windowport_.y -= y2 - y1;

  return Redraw();
}

int RenderDeviceGDI::ZoomToRect(LRect lrect, bool realtime) {
  DRect drect;
  LRectToDRect(lrect, drect);
  zoomout_viewport_.x = drect.x;
  zoomout_viewport_.y = drect.y + drect.height;
  zoomout_viewport_.height = drect.height;
  zoomout_viewport_.width = drect.width;

  windowport_.x = lrect.x;
  windowport_.y = lrect.y;
  windowport_.height = lrect.height;
  windowport_.width = lrect.width;

  float xblc = 0., yblc = 0.;
  xblc = viewport_.width / windowport_.width;
  yblc = viewport_.height / windowport_.height;

  blc_ = (xblc > yblc) ? yblc : xblc;

  if (xblc < yblc) {
    zoomout_viewport_.y += drect.height * (1 - yblc / xblc);
    windowport_.height = lrect.height * yblc / xblc;
    zoomout_viewport_.height = drect.height * yblc / xblc;
  } else {
    windowport_.width = lrect.width * xblc / yblc;
    zoomout_viewport_.width = drect.width * xblc / yblc;
  }

  return Redraw();
}

int RenderDeviceGDI::Timer() {
  if (is_redraw_) {
    double elapse = 0.;
    LONGLONG stamp = 0, per_count = 0;

    QueryPerformanceFrequency((LARGE_INTEGER *)&per_count);
    QueryPerformanceCounter((LARGE_INTEGER *)&stamp);

    elapse = (stamp - last_redraw_command_stamp_) / (double)per_count;

    if (elapse > kDelay) {
      is_redraw_ = false;
      composit_render_buffer_.Clear(viewport_.x, viewport_.y, viewport_.width,
                                    viewport_.height,
                                    (COLORREF)::GetSysColor(COLOR_WINDOW));

      HDC hPaintBufferDC = composit_render_buffer_.PrepareDC();
      BeginRender(RB_COMPOSIT);

      if (layer_ != NULL) {
        return RenderLayer(layer_, op_);
      }

      EndRender(RB_COMPOSIT);

      zoomin_viewport_ = viewport_;
      zoomout_viewport_ = viewport_;
    }
  }

  RECT rect;
  GetClientRect(hwnd_, &rect);
  InvalidateRect(hwnd_, &rect, true);

  return ERR_NONE;
}

int RenderDeviceGDI::BeginRender(eRenderBuffer render_buffer, bool clear,
                                 const Style *style, int op) {
  is_lock_style_ = (NULL != style);

  switch (render_buffer) {
    case RB_COMPOSIT: {
      if (clear)
        composit_render_buffer_.Clear(viewport_.x, viewport_.y, viewport_.width,
                                      viewport_.height);

      current_dc_ = composit_render_buffer_.PrepareDC();
    } break;
    case RB_DIRECT: {
      if (clear) {
        RECT rect;
        ::GetClientRect(hwnd_, &rect);
        ::InvalidateRect(hwnd_, &rect, true);
      }

      current_dc_ = ::GetDC(hwnd_);
    } break;
  }

  if (is_lock_style_) {
    PrepareForDrawing(style, op);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::EndRender(eRenderBuffer render_buffer) {
  if (is_lock_style_) EndDrawing();

  switch (render_buffer) {
    case RB_COMPOSIT: {
      composit_render_buffer_.EndDC();
    } break;
    case RB_DIRECT: {
      ::ReleaseDC(hwnd_, current_dc_);
    } break;
  }

  current_dc_ = NULL;
  is_lock_style_ = false;

  return ERR_NONE;
}

int RenderDeviceGDI::PrepareForDrawing(const Style *style, int draw_mode) {
  ::SetROP2(current_dc_, draw_mode);

  use_current_style_ = (NULL != style);

  if (use_current_style_) {
    uint64_t format = style->GetStyleType();
    if (format & ST_PenDesc) {
      PenDesc pen = style->GetPenDesc();

      if (pen_) {
        ::DeleteObject(pen_);
        pen_ = NULL;
      }

      pen_ = ::CreatePen(pen.style, pen.width, pen.color);
      old_pen_ = (HPEN)::SelectObject(current_dc_, pen_);
    }

    if (format & ST_BrushDesc) {
      BrushDesc brush = style->GetBrushDesc();

      if (brush_) {
        ::DeleteObject(brush_);
        brush_ = NULL;
      }

      if (brush.brush_type == BrushDesc::BT_Hatch) {
        brush_ = ::CreateHatchBrush(brush.style, brush.color);
      } else
        brush_ = ::CreateSolidBrush(brush.color);

      old_brush_ = (HBRUSH)::SelectObject(current_dc_, brush_);
    }

    if (format & ST_SymbolDesc) {
      SymbolDesc symbol = style->GetSymbolDesc();

      if (icon_) {
        ::DeleteObject(icon_);
        icon_ = NULL;
      }

      // icon_ =
      //     ::LoadIcon(instance_handle_, MAKEINTRESOURCE(symbol.id +
      //     IDI_ICON_A));
    }

    if (format & ST_AnnoDesc) {
      AnnotationDesc anno = style->GetAnnoDesc();

      if (font_) {
        ::DeleteObject(font_);
        font_ = NULL;
      }

      // font_ =
      //     ::CreateFont(anno.height * blc_, anno.width * blc_,
      //     anno.escapement,
      //                  anno.orientation, anno.weight, anno.italic,
      //                  anno.underline, anno.strikeout, anno.char_set,
      //                  anno.out_precision, anno.clip_precision, anno.quality,
      //                  anno.pitch_and_family, anno.face_name);

      // old_font_ = (HFONT)::SelectObject(current_dc_, font_);
      ::SetBkMode(current_dc_, TRANSPARENT);
      ::SetTextColor(current_dc_, anno.color);
    }
  }

  return ERR_NONE;
}

int RenderDeviceGDI::EndDrawing() {
  if (use_current_style_) {
    ::SelectObject(current_dc_, old_brush_);
    ::SelectObject(current_dc_, old_pen_);
    ::SelectObject(current_dc_, old_font_);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::Redraw() {
  if (!is_redraw_) {
    is_redraw_ = true;
    QueryPerformanceCounter((LARGE_INTEGER *)&last_redraw_stamp_);
  }

  QueryPerformanceCounter((LARGE_INTEGER *)&last_redraw_command_stamp_);

  return ERR_NONE;
}

int RenderDeviceGDI::Render(void) {
  render_buffer_.Clear(viewport_.x, viewport_.y, viewport_.width,
                       viewport_.height);

  composit_render_buffer_.Swap(
      render_buffer_, zoomin_viewport_.x, zoomin_viewport_.y,
      zoomin_viewport_.width, zoomin_viewport_.height, zoomout_viewport_.x,
      zoomout_viewport_.y, zoomout_viewport_.width, zoomout_viewport_.height,
      RenderBuffer::BLT_TRANSPARENT, SRCCOPY);

  render_buffer_.Swap(current_dop_.x, current_dop_.y, viewport_.width,
                      viewport_.height, viewport_.x, viewport_.y);

  return ERR_NONE;
}

int RenderDeviceGDI::RenderLayer(const OGRLayer *const_layer, int op) {
  OGRLayer *layer = const_cast<OGRLayer *>(const_layer);
  if (NULL == layer) {
    return ERR_INVALID_PARAM;
  }

  layer_ = layer;
  const auto *layer_defn = layer->GetLayerDefn();
  auto geomtry_count = layer_defn->GetGeomFieldCount();

  OGRFeature *feature = nullptr;
  layer->ResetReading();
  while ((feature = layer->GetNextFeature()) != nullptr) {
    for (int i = 0; i < geomtry_count; i++) {
      auto *geometry = feature->GetGeomFieldRef(i);
      RenderGeometry(geometry, op);
    }
  }

  return ERR_NONE;
}

int RenderDeviceGDI::RenderFeature(const OGRFeature *feature, int op) {
  return ERR_NONE;
}

int RenderDeviceGDI::RenderGeometry(const OGRGeometry *geometry, int op) {
  if (!geometry) {
    return ERR_INVALID_PARAM;
  }

  OGREnvelope envelope;
  geometry->getEnvelope(&envelope);

  ::SaveDC(current_dc_);
  // if (!is_lock_style_) {
  //   PrepareForDrawing(style);
  // }

  if (options_.show_mbr) {
    long X = 0, Y = 0;

    LPToDP(envelope.MinX, envelope.MinY, X, Y);
    ::MoveToEx(current_dc_, X, Y, NULL);

    LPToDP(envelope.MaxX, envelope.MinY, X, Y);
    ::LineTo(current_dc_, X, Y);

    LPToDP(envelope.MaxX, envelope.MaxY, X, Y);
    ::LineTo(current_dc_, X, Y);

    LPToDP(envelope.MinX, envelope.MaxY, X, Y);
    ::LineTo(current_dc_, X, Y);

    LPToDP(envelope.MinX, envelope.MinY, X, Y);
    ::LineTo(current_dc_, X, Y);
  }

  auto type = geometry->getGeometryType();
  switch (type) {
    case wkbPoint:
      DrawPoint((OGRPoint *)geometry);
      break;

    case wkbLineString:
      DrawLineString((OGRLineString *)geometry);
      break;

    case wkbPolygon:
      DrawPolygon((OGRPolygon *)geometry);
      break;

    case wkbMultiPoint:
      DrawMultiPoint((OGRMultiPoint *)geometry);
      break;

    case wkbMultiLineString:
      DrawMultiLineString((OGRMultiLineString *)geometry);
      break;

    case wkbMultiPolygon:
      DrawMultiPolygon((OGRMultiPolygon *)geometry);
      break;

    case wkbLinearRing:
      DrawLinearRing((OGRLinearRing *)geometry);
      break;

    case wkbNone:
    case wkbUnknown:
    default:
      break;
  }

  // if (!is_lock_style_) EndDrawing();

  ::RestoreDC(current_dc_, -1);

  return ERR_NONE;
}

int RenderDeviceGDI::DrawMultiLineString(
    const OGRMultiLineString *multi_linestring) {
  for (const auto i : *multi_linestring) {
    DrawLineString((OGRLineString *)(i));
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawMultiPoint(const OGRMultiPoint *multi_point) {
  for (const auto i : *multi_point) {
    DrawPoint((OGRPoint *)(i));
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawMultiPolygon(const OGRMultiPolygon *multi_polygon) {
  for (const auto i : *multi_polygon) {
    DrawPolygon((OGRPolygon *)(i));
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawPoint(const OGRPoint *point) {
  int r = options_.point_radius;
  long X = 0, Y = 0;
  LPToDP(point->getX(), point->getY(), X, Y);
  Ellipse(current_dc_, X - r, Y - r, X + r, Y + r);

  if (options_.show_point) {
    DrawCross(current_dc_, X, Y, r);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawAnno(const OGRPoint *point, const char *anno,
                              float angle, float height, float width,
                              float space) {
  if (anno == NULL) {
    return ERR_INVALID_PARAM;
  }

  height *= blc_;
  width *= blc_;
  space *= blc_;

  long X = 0, Y = 0;
  char bz[4];
  const char *c = anno;

  LPToDP(point->getX(), point->getY(), X, Y);

  X -= 2 * height * sin(angle);
  Y -= 2 * height * cos(angle);

  int char_size = (int)strlen(c);
  while (char_size > 0) {
    if (*c > 127 && *(c + 1) > 127) {
      // 如果下一个字符是汉字
      strncpy(bz, c, 2);
      bz[2] = 0;
      c = c + 2;
      ::TextOut(current_dc_, X, Y, (LPCWSTR)bz, 2);
      char_size -= 2;
      X += (width * 2 + space) * cos(angle);
      Y += (width * 2 + space) * sin(angle);
    } else {
      strncpy(bz, c, 1);
      bz[1] = 0;
      c++;
      ::TextOut(current_dc_, X, Y, (LPCWSTR)bz, 1);
      char_size -= 1;

      X += (width + space / 2.) * cos(angle);
      Y += (width + space / 2.) * sin(angle);
    }
  }

  if (options_.show_point) {
    long X = 0, Y = 0;
    LPToDP(point->getX(), point->getY(), X, Y);
    DrawCross(current_dc_, X, Y, options_.point_radius);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawSymbol(const OGRPoint *point, HICON icon, long height,
                                long width) {
  height *= blc_;
  width *= blc_;

  long X = 0, Y = 0;
  LPToDP(point->getX(), point->getY(), X, Y);
  ::DrawIconEx(current_dc_, X - width / 2, Y + height / 2, icon, width, height,
               0, NULL, DI_NORMAL);

  if (options_.show_point) {
    int r = options_.point_radius;
    long X = 0, Y = 0;
    LPToDP(point->getX(), point->getY(), X, Y);
    DrawCross(current_dc_, X, Y, r);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawLineString(const OGRLineString *line_string) {
  int point_size = line_string->getNumPoints();
  if (point_size < 2) {
    return ERR_INVALID_PARAM;
  }

  int i = 0;
  POINT *points = new POINT[point_size];
  for (int i = 0; i < point_size; ++i) {
  }

  if (options_.show_point) {
    OGRPoint point;
    for (int i = 0; i < point_size; i++) {
      line_string->getPoint(i, &point);
      LPToDP(point.getX(), point.getY(), points[i].x, points[i].y);
      DrawCross(current_dc_, points[i].x, points[i].y, options_.point_radius);
    }
  } else {
    OGRPoint point;
    for (int i = 0; i < point_size; i++) {
      line_string->getPoint(i, &point);
      LPToDP(point.getX(), point.getY(), points[i].x, points[i].y);
    }
  }

  MoveToEx(current_dc_, points[0].x, points[0].y, NULL);
  PolylineTo(current_dc_, points, point_size);

  SAFE_DELETE_A(points);

  return ERR_NONE;
}

int RenderDeviceGDI::DrawLinearRing(const OGRLinearRing *linear_ring) {
  return DrawLineString(linear_ring->toUpperClass());
}

int RenderDeviceGDI::DrawPolygon(const OGRPolygon *polygon) {
  BOOL ret = FALSE;
  int all_point_size = 0;

  const auto *exterior_ring = polygon->getExteriorRing();
  int exterior_point_size = exterior_ring->getNumPoints();
  if (exterior_point_size < 2) {
    return ERR_INVALID_PARAM;
  }
  all_point_size += exterior_point_size;

  int interior_ring_size = polygon->getNumInteriorRings();
  int *ring_point_sizes = new int[interior_ring_size + 1];
  ring_point_sizes[0] = exterior_point_size;
  for (int i = 0; i < interior_ring_size; i++) {
    const auto *interior_ring = polygon->getInteriorRing(i);
    ring_point_sizes[i + 1] = interior_ring->getNumPoints();
    all_point_size += ring_point_sizes[i + 1];
  }

  POINT *points = NULL;
  points = new POINT[all_point_size];

  if (options_.show_point) {
    int idx = 0;
    OGRPoint point;
    for (int i = 0; i < exterior_point_size; i++, idx++) {
      exterior_ring->getPoint(i, &point);
      LPToDP(point.getX(), point.getY(), points[idx].x, points[idx].y);
      DrawCross(current_dc_, points[idx].x, points[idx].y,
                options_.point_radius);
    }

    for (int i = 0; i < interior_ring_size; i++) {
      const auto *interior_ring = polygon->getInteriorRing(i);
      int nInteriorPts = interior_ring->getNumPoints();
      for (int j = 0; j < nInteriorPts; ++j, idx++) {
        interior_ring->getPoint(i, &point);
        LPToDP(point.getX(), point.getY(), points[idx].x, points[idx].y);
        DrawCross(current_dc_, points[idx].x, points[idx].y,
                  options_.point_radius);
      }
    }

    ret = ::PolyPolygon(current_dc_, points, ring_point_sizes,
                        interior_ring_size + 1);
  } else {
    int idx = 0;
    OGRPoint point;
    for (int i = 0; i < exterior_point_size; i++, idx++) {
      exterior_ring->getPoint(i, &point);
      LPToDP(point.getX(), point.getY(), points[idx].x, points[idx].y);
    }

    for (int i = 0; i < interior_ring_size; i++) {
      const auto *interior_ring = polygon->getInteriorRing(i);
      int nInteriorPts = interior_ring->getNumPoints();
      OGRPoint point;
      for (int j = 0; j < nInteriorPts; ++j, idx++) {
        interior_ring->getPoint(i, &point);
        LPToDP(point.getX(), point.getY(), points[idx].x, points[idx].y);
      }
    }

    ret = ::PolyPolygon(current_dc_, points, ring_point_sizes,
                        interior_ring_size + 1);
  }

  SAFE_DELETE_A(points);
  SAFE_DELETE_A(ring_point_sizes);

  return ERR_NONE;
}

int RenderDeviceGDI::DrawImage(const char *image_buffer, int image_buffer_size,
                               const LRect &lrect, long code_type_,
                               eRenderBuffer render_buffer) {
  long ret = ERR_FAILURE;

  DRect drect;
  LRectToDRect(lrect, drect);

  switch (render_buffer) {
    case RB_COMPOSIT: {
      ret = composit_render_buffer_.DrawImage(
          image_buffer, image_buffer_size, code_type_, drect.x,
          drect.y + drect.height, drect.width, drect.height);
    } break;
  }

  return ret;
}

int RenderDeviceGDI::StrethImage(const char *image_buffer,
                                 int image_buffer_size, const LRect &rect,
                                 long code_type_, eRenderBuffer render_buffer) {
  long ret = ERR_FAILURE;

  DRect drect;
  LRectToDRect(rect, drect);

  switch (render_buffer) {
    case RB_COMPOSIT: {
      ret = composit_render_buffer_.StrethImage(
          image_buffer, image_buffer_size, code_type_, drect.x,
          drect.y + drect.height, drect.width, drect.height);
    } break;
  }

  return ret;
}

int RenderDeviceGDI::SaveImage(const char *file_path,
                               eRenderBuffer render_buffer,
                               bool backgroud_transparent) {
  long ret = ERR_FAILURE;

  switch (render_buffer) {
    case RB_COMPOSIT: {
      ret =
          composit_render_buffer_.Save2Image(file_path, backgroud_transparent);
    } break;
  }

  return ret;
}

int RenderDeviceGDI::Save2ImageBuffer(char *&image_buffer,
                                      long &image_buffer_size, long code_type_,
                                      eRenderBuffer render_buffer,
                                      bool backgroud_transparent) {
  long ret = ERR_FAILURE;

  switch (render_buffer) {
    case RB_COMPOSIT: {
      ret = composit_render_buffer_.Save2ImageBuffer(
          image_buffer, image_buffer_size, code_type_, backgroud_transparent);
    } break;
  }

  return ret;
}

int RenderDeviceGDI::FreeImageBuffer(char *&image_buffer) {
  return RenderBuffer::FreeImageBuffer(image_buffer);
}
}  // namespace gfx2d