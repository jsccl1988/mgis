#ifndef GIS_CORE_STYLE_H
#define GIS_CORE_STYLE_H

#include "core/core.h"
#include "core/core_export.h"

#include "smt_bas_struct.h"
#include "smt_core.h"
#include "smt_env_struct.h"

namespace gfx2d {
enum StType {
  ST_PenDesc = 0x0001,
  ST_BrushDesc = 0x0002,
  ST_AnnoDesc = 0x0004,
  ST_SymbolDesc = 0x008
};

struct PenDesc {
  long lPenColor;
  long lPenStyle;
  float fPenWidth;

  PenDesc(void) {
    lPenColor = RGB(0, 255, 0);
    lPenStyle = PS_SOLID;
    fPenWidth = 0.002;
  }
};

struct BrushDesc {
  enum brushType { BT_Solid, BT_Hatch } brushTp;

  long lBrushColor;
  long lBrushStyle;

  BrushDesc(void) {
    brushTp = BT_Solid;
    lBrushColor = RGB(0, 255, 255);
    lBrushStyle = HS_FDIAGONAL;
  }
};

struct AnnotationDesc {
  float fHeight;
  float fWidth;
  long lEscapement;
  long lOrientation;
  long lWeight;
  byte lItalic;
  byte lUnderline;
  byte lStrikeOut;
  byte lCharSet;
  byte lOutPrecision;
  byte lClipPrecision;
  byte lQuality;
  byte lPitchAndFamily;
  char szFaceName[32];

  long lAnnoClr;
  float fAngle;
  float fSpace;

  AnnotationDesc(void) {
    fHeight = 1.6;
    fWidth = 1.6;
    lEscapement = 0;
    lOrientation = 0;
    lWeight = 50;
    lItalic = FALSE;
    lUnderline = FALSE;
    lStrikeOut = 0;
    lCharSet = DEFAULT_CHARSET;
    lOutPrecision = OUT_TT_PRECIS;
    lClipPrecision = CLIP_CHARACTER_PRECIS;
    lQuality = DEFAULT_QUALITY;
    lPitchAndFamily = FIXED_PITCH;
    strcpy(szFaceName, "Times New Roman");

    lAnnoClr = RGB(0, 0, 0);
    fAngle = 0;
    fSpace = 0.2;
  }
};

struct SymbolDesc {
  long lSymbolID;
  float fSymbolWidth;
  float fSymbolHeight;

  SymbolDesc(void) {
    lSymbolID = 0;
    fSymbolHeight = 0.4;
    fSymbolWidth = 0.4;
  }
};

class GIS_CORE_EXPORT Style {
 public:
  Style(void);
  Style(const char *szName, const PenDesc &penDesc,
           const BrushDesc &brushDesc, const AnnotationDesc &annoDesc,
           const SymbolDesc &symbolDesc);

  ~Style();

 public:
  const char *GetStyleName() const { return name_; }
  ulong GetStyleType(void) const { return m_stType; }

  void SetStyleType(ulong tp) { m_stType = tp; }
  void SetStyleName(const char *szName) { strcpy(name_, szName); }
  Style *Clone(const char *szNewName) const;

  //////////////////////////////////////////////////////////////////////////
  inline void SetPenDesc(const PenDesc &penDesc) { m_stPenDesc = penDesc; }
  inline void SetBrushDesc(const BrushDesc &brushDesc) {
    m_stBrushDesc = brushDesc;
  }
  inline void SetAnnoDesc(const AnnotationDesc &annoDesc) {
    m_stAnnoDesc = annoDesc;
  }
  inline void SetSymbolDesc(const SymbolDesc &symbolDesc) {
    m_stSymbolDesc = symbolDesc;
  }

  //////////////////////////////////////////////////////////////////////////
  inline PenDesc &GetPenDesc(void) { return m_stPenDesc; }
  inline BrushDesc &GetBrushDesc(void) { return m_stBrushDesc; }
  inline AnnotationDesc &GetAnnoDesc(void) { return m_stAnnoDesc; }
  inline SymbolDesc &GetSymbolDesc(void) { return m_stSymbolDesc; }

  inline const PenDesc &GetPenDesc(void) const { return m_stPenDesc; }
  inline const BrushDesc &GetBrushDesc(void) const { return m_stBrushDesc; }
  inline const AnnotationDesc &GetAnnoDesc(void) const {
    return m_stAnnoDesc;
  }
  inline const SymbolDesc &GetSymbolDesc(void) const {
    return m_stSymbolDesc;
  }

 protected:
  char name_[MAX_STYLENAME_LENGTH];
  ulong m_stType;

  PenDesc m_stPenDesc;
  BrushDesc m_stBrushDesc;
  AnnotationDesc m_stAnnoDesc;
  SymbolDesc m_stSymbolDesc;
};

typedef vector<Style *> StylePtrList;

class GIS_CORE_EXPORT StyleTable {
 public:
  StyleTable(void);
  ~StyleTable(void);

  inline int GetStyleCount(void) const { return m_nStyleCount; }

  StyleTable *Clone(void) const;

  int AddStyle(const char *stylename);
  void RemoveStyle(const char *stylename);

  Style *GetStyle(const char *stylename);
  const Style *GetStyle(const char *stylename) const;

  Style *GetStyle(int index);
  const Style *GetStyle(int index) const;

  const char *GetStyleName(int index);

 protected:
  int FindStyleNameIndex(const char *stylename) const;

 protected:
  char **m_pStyleNames;
  int m_nStyleCount;
};

class GIS_CORE_EXPORT StyleManager {
 private:
  StyleManager(void);

 public:
  virtual ~StyleManager(void);

  void SetDefaultStyle(const char *defName, PenDesc &penDesc,
                       BrushDesc &brushDesc, AnnotationDesc &annoDesc,
                       SymbolDesc &symbolDesc);
  Style *GetDefaultStyle(void);

 public:
  Style *CreateStyle(const char *defName, PenDesc &penDesc,
                        BrushDesc &brushDesc, AnnotationDesc &annoDesc,
                        SymbolDesc &symbolDesc);

  void DestroyStyle(const char *name);
  void DestroyStyle(Style *pStyle);
  void DestroyAllStyle();

  Style *GetStyle(const char *name);

 public:
  static StyleManager *GetSingletonPtr(void);

  static void DestoryInstance(void);

 private:
  StylePtrList m_StylePtrList;
  Style *m_pDefaultStyle;

  static StyleManager *singleton_;
};
}  // namespace core

#endif  //GIS_CORE_STYLE_H