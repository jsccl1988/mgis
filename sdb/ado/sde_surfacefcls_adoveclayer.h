#ifndef _SDE_SURFACEFCLS_ADOLAYER_H
#define _SDE_SURFACEFCLS_ADOLAYER_H

#include "sde_ado.h"

namespace _SDEAdo {
class SurfaceFclsAdoLayer : public AdoVecLayer {
 public:
  SurfaceFclsAdoLayer(DataSource *pOwnerDs);
  virtual ~SurfaceFclsAdoLayer(void);

 public:
  bool Create(void);

  // feature
  long AppendFeature(const Feature *pFeature, bool bclone = false);
  long AppendFeatureBatch(const Feature *pFeature, bool bClone = false);

  long UpdateFeature(const Feature *pFeature);
  //////////////////////////////////////////////////////////////////////////
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
  long AppendPolygon(const Polygon *pPolygon);
  long PutCollectLinearingNums(const Polygon *pPolygon);
  long PutCollectLinearings(const Polygon *pPolygon);
  long PutCollectLinearing(const LinearRing *pLinearRing, SAFEARRAY *pSa,
                           long &index);

  long AppendFan(const Fan *pFan);

  long GetPolygon(Geometry *&pPolygon);
  long GetCollectLinearingNums(Polygon *pPloygon);
  long GetCollectLinearings(Polygon *pPloygon);
  long GetCollectLinearing(LinearRing *pLinearRing, RawPoint *&pRawPointBuf);

  long GetFan(Geometry *&pFan);
};
}  // namespace _SDEAdo

#endif  //_SDE_SURFACEFCLS_ADOLAYER_H