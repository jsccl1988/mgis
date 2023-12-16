#ifndef _SDE_CURVEFCLS_ADOLAYER_H
#define _SDE_CURVEFCLS_ADOLAYER_H

#include "sde_ado.h"

namespace _SDEAdo {
class CurveFclsAdoLayer : public AdoVecLayer {
 public:
  CurveFclsAdoLayer(DataSource *pOwnerDs);
  virtual ~CurveFclsAdoLayer(void);

 public:
  bool Create(void);

  // feature
  long AppendFeature(const Feature *pFeature, bool bclone = false);
  long AppendFeatureBatch(const Feature *pFeature, bool bClone = false);

  long UpdateFeature(const Feature *pFeature);

 protected:
  void SetDefaultAttFields(void);
  void SetDefaultGeomFields(void);
  void SetTableFields(void);

  void GetFeature(Feature *pFeature);

 private:
  long AppendStyle(const Style *pStyle);
  long GetStyle(Style *&pStyle);

 private:
  long AppendGeom(const Geometry *pGeom);
  long GetGeom(Geometry *&pGeom);

 private:
  long AppendLineString(const LineString *pLineString);
  long AppendSpline(const Spline *pSpline);
  long AppendLinearRing(const LinearRing *pLinearRing);
  long AppendArc(const Arc *pArc);

  long GetLineString(Geometry *&pLineString);
  long GetSpline(Geometry *&pSpline);
  long GetLinearRing(Geometry *&pLinearRing);
  long GetArc(Geometry *&pArc);
};
}  // namespace _SDEAdo

#endif  //_SDE_CURVEFCLS_ADOLAYER_H