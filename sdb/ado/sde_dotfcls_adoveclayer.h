#ifndef _SDE_DOTFCLS_ADOLAYER_H
#define _SDE_DOTFCLS_ADOLAYER_H

#include "sde_ado.h"

namespace _SDEAdo {
class DotFclsAdoLayer : public AdoVecLayer {
 public:
  DotFclsAdoLayer(DataSource *pOwnerDs);
  virtual ~DotFclsAdoLayer(void);

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

  long AppendGeom(const Geometry *pGeom);
  long GetGeom(Geometry *&pGeom);

  long GetLayerAttribute(void);

 private:
  long AppendPoint(const Point *pPoint);
  long AppendPoints(const MultiPoint *pPoints);
  long GetPoint(Geometry *&pPoint);
  long GetPoints(Geometry *&pPoints);
};
}  // namespace _SDEAdo

#endif  //_SDE_DOTFCLS_ADOLAYER_H