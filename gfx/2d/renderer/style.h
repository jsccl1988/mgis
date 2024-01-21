// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_2D_RENDERER_STYLE_H
#define GFX_2D_RENDERER_STYLE_H

#include <map>
#include <vector>

#include "base/base_config.h"
#include "base/basictypes.h"
#include "base/error.h"
#include "base/macros.h"
#include "gfx/2d/renderer/render_export.h"

namespace gfx2d {
enum eStyleType {
  ST_PenDesc = 0x0001,
  ST_BrushDesc = 0x0002,
  ST_AnnoDesc = 0x0004,
  ST_SymbolDesc = 0x008
};

struct PenDesc {
  long color;
  long style;
  float width;

  PenDesc(void) {
    color = RGB(0, 255, 0);
    style = PS_SOLID;
    width = 0.002f;
  }
};

struct BrushDesc {
  enum BrushType { BT_Solid, BT_Hatch } brush_type;

  long color;
  long style;

  BrushDesc(void) {
    brush_type = BT_Solid;
    color = RGB(0, 255, 255);
    style = HS_FDIAGONAL;
  }
};

struct AnnotationDesc {
  float height;
  float width;
  long escapement;
  long orientation;
  long weight;
  bool italic;
  bool underline;
  uint8_t strikeout;
  uint8_t char_set;
  uint8_t out_precision;
  uint8_t clip_precision;
  uint8_t quality;
  uint8_t pitch_and_family;
  std::wstring face_name;

  long color;
  float angle;
  float space;

  AnnotationDesc(void) {
    height = 1.6f;
    width = 1.6f;
    escapement = 0;
    orientation = 0;
    weight = 50;
    italic = FALSE;
    underline = FALSE;
    strikeout = 0;
    char_set = DEFAULT_CHARSET;
    out_precision = OUT_TT_PRECIS;
    clip_precision = CLIP_CHARACTER_PRECIS;
    quality = DEFAULT_QUALITY;
    pitch_and_family = FIXED_PITCH;
    face_name = L"Times New Roman";

    color = RGB(0, 0, 0);
    angle = 0.f;
    space = 0.2f;
  }
};

struct SymbolDesc {
  uint32_t id;
  float width;
  float height;

  SymbolDesc(void) {
    id = 0;
    height = 0.4f;
    width = 0.4f;
  }
};

class Style {
 public:
  Style(void);
  Style(const char *name, const PenDesc &pen_desc, const BrushDesc &brush_desc,
        const AnnotationDesc &anno_desc, const SymbolDesc &symbol_desc);

  ~Style();

 public:
  const char *GetStyleName() const { return name_.c_str(); }
  uint32_t GetStyleType(void) const { return style_type_; }

  void SetStyleType(uint32_t tp) { style_type_ = tp; }
  void SetStyleName(const char *name) { name_ = name; }
  Style *Clone(const char *new_name) const;

  inline void SetPenDesc(const PenDesc &pen_desc) { pen_desc_ = pen_desc; }
  inline void SetBrushDesc(const BrushDesc &brush_desc) {
    brush_desc_ = brush_desc;
  }
  inline void SetAnnoDesc(const AnnotationDesc &anno_desc) {
    anno_desc_ = anno_desc;
  }
  inline void SetSymbolDesc(const SymbolDesc &symbol_desc) {
    symbol_desc_ = symbol_desc;
  }

  inline PenDesc &GetPenDesc(void) { return pen_desc_; }
  inline BrushDesc &GetBrushDesc(void) { return brush_desc_; }
  inline AnnotationDesc &GetAnnoDesc(void) { return anno_desc_; }
  inline SymbolDesc &GetSymbolDesc(void) { return symbol_desc_; }

  inline const PenDesc &GetPenDesc(void) const { return pen_desc_; }
  inline const BrushDesc &GetBrushDesc(void) const { return brush_desc_; }
  inline const AnnotationDesc &GetAnnoDesc(void) const { return anno_desc_; }
  inline const SymbolDesc &GetSymbolDesc(void) const { return symbol_desc_; }

 protected:
  std::string name_;
  uint32_t style_type_;

  PenDesc pen_desc_;
  BrushDesc brush_desc_;
  AnnotationDesc anno_desc_;
  SymbolDesc symbol_desc_;
};

using StyleList = std::vector<Style *>;
using StyleMap = std::map<std::string, Style *>;

class StyleManager {
 private:
  StyleManager(void);

 public:
  virtual ~StyleManager(void);

  void SetDefaultStyle(const char *defName, PenDesc &pen_desc,
                       BrushDesc &brush_desc, AnnotationDesc &anno_desc,
                       SymbolDesc &symbol_desc);
  Style *GetDefaultStyle(void);

 public:
  Style *CreateStyle(const char *defName, PenDesc &pen_desc,
                     BrushDesc &brush_desc, AnnotationDesc &anno_desc,
                     SymbolDesc &symbol_desc);

  void DestroyStyle(const char *name);
  void DestroyStyle(Style *style);
  void DestroyAllStyle();

  Style *GetStyle(const char *name);

 public:
  static StyleManager *GetSingletonPtr(void);

  static void DestoryInstance(void);

 private:
  StyleList style_list_;
  Style *default_style_;

  static StyleManager *singleton_;
};
}  // namespace gfx2d

#endif  // GFX_2D_RENDERER_STYLE_H