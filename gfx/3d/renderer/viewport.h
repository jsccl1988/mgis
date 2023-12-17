#ifndef _BL_BAS_STRUCT_H
#define _BL_BAS_STRUCT_H

#include "core/core.h"
#include "core/core_export.h"


namespace core {

struct CORE_EXPORT Viewport {
  float m_fVOX;
  float m_fVOY;
  float m_fVHeight;
  float m_fVWidth;

  Viewport() : m_fVOX(0), m_fVOY(0), m_fVHeight(0), m_fVWidth(0) {}
};

struct CORE_EXPORT Windowport {
  float m_fWOX;
  float m_fWOY;
  float m_fWHeight;
  float m_fWWidth;

  Windowport() : m_fWOX(0), m_fWOY(0), m_fWHeight(0), m_fWWidth(0) {}
};
}  // namespace core

#endif  //_BL_BAS_STRUCT_H