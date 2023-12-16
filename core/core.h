#ifndef GIS_CORE_CORE_H
#define GIS_CORE_CORE_H

#define SmartGIS 2.0

#include <assert.h>
#include <math.h>
#include <windef.h>
#include <windows.h>

#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "base/base_config.h"
#include "base/base_types.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/port.h"

namespace core {
enum eMouseStatusType {
  typeNoStatus,
  typeDLButtonDown,
  typeLButtonDown,
  typeLButtonUp,
  typeMouseMove,
  typeDRButtonDown,
  typeRButtonDown,
  typeRButtonUp,
  typeMouseWeel
};

enum eOperType {
  typeNoOper,
  typeAppendFeature,
  typeModifyFeature,
  typeAnalysis,
  typeViewMap
};
}  // namespace core

#endif  // GIS_CORE_CORE_H