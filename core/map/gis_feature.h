#ifndef _GIS_FEATURE_H
#define _GIS_FEATURE_H

#include "bl_style.h"
#include "geo_geometry.h"
#include "gis_attribute.h"
#include "smt_bas_struct.h"
#include "smt_core.h"


using namespace Smt_Core;
using namespace Smt_Base;
using namespace Smt_Geo;

namespace Smt_GIS {
enum SmtFeatureType {
  //点状要素
  SmtFtDot,
  SmtFtAnno,
  SmtFtChildImage,
  //线要素
  SmtFtCurve,
  //面要素
  SmtFtSurface,
  //网格要素
  SmtFtGrid,
  //三角网要素
  SmtFtTin,
  //未知要素，纯属性
  SmtFtUnknown
};

class EXPORT_CLASS SmtFeature {
 public:
  SmtFeature(void);
  virtual ~SmtFeature(void);

  inline long GetID(void) const { return m_lID; }
  inline SmtFeatureType GetFeatureType(void) const { return m_SmtFeatureType; }
  inline SmtGeometry *GetGeometryRef(void) { return m_pGeom; }
  inline const SmtGeometry *GetGeometryRef(void) const { return m_pGeom; }

  inline SmtAttribute *GetAttributeRef(void) { return m_pAtt; }
  inline const SmtAttribute *GetAttributeRef(void) const { return m_pAtt; }

  //////////////////////////////////////////////////////////////////////////
  inline void SetID(long id) { m_lID = id; }
  inline void SetFeatureType(SmtFeatureType type);

  void SetGeometryDirectly(SmtGeometry *pGeom);
  void SetGeometry(const SmtGeometry *pGeom);

  void SetAttributeDirectly(SmtAttribute *pAtt);
  void SetAttribute(const SmtAttribute *pAtt);

  SmtFeature *Clone() const;

  virtual bool Equal(const SmtFeature *poFeature);

  //////////////////////////////////////////////////////////////////////////
  void AddField(SmtField &fld);
  void RemoveField(const char *szFldName);
  void SetName(int index, const char *name);
  void SetType(int index, varType uVt);

  int GetFieldIndexByName(const char *name);

  int SetFieldValue(int index, const SmtVariant &smtFld);
  int SetFieldValue(int index, int nValue);
  int SetFieldValue(int index, double dfValue);
  int SetFieldValue(int index, byte bValue);
  int SetFieldValue(int index, bool bValue);
  int SetFieldValue(int index, const char *pszValue);
  int SetFieldValue(int index, int nCount, int *panValues);
  int SetFieldValue(int index, int nCount, double *padfValues);
  int SetFieldValue(int index, char **papszValues);
  int SetFieldValue(int index, int nCount, byte *pabyBinary);
  int SetFieldValue(int index, int nYear, int nMonth, int nDay, int nHour = 0,
                    int nMinute = 0, int nSecond = 0, int nTZFlag = 0);

  //////////////////////////////////////////////////////////////////////////
  // style
  int SetStyle(const char *szStyleName);
  int SetStyleDirectly(SmtStyle *pStyle);
  int SetStyle(const SmtStyle *pStyle);

  SmtStyle *GetStyle(void) { return m_pStyle; }
  const SmtStyle *GetStyle(void) const { return m_pStyle; }

 protected:
  long m_lID;
  SmtFeatureType m_SmtFeatureType;
  SmtAttribute *m_pAtt;
  SmtGeometry *m_pGeom;
  SmtStyle *m_pStyle;
};
}  // namespace Smt_GIS
#endif  //_GIS_FEATURE_H