#ifndef _SDE_GRIDFCLS_ADOLAYER_H
#define _SDE_GRIDFCLS_ADOLAYER_H

#include "sde_ado.h"

namespace _SDEAdo {
class GridFclsAdoLayer : public AdoVecLayer {
 public:
  GridFclsAdoLayer(DataSource *pOwnerDs);
  virtual ~GridFclsAdoLayer(void);

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
  long AppendGrid(const Grid *pGrid);
  long AppendGridSize(const Grid *pGrid);
  long AppendGridNodes(const Grid *pGrid);

  long GetGrid(Geometry *&pGrid);
  long GetGridSize(Grid *&pGrid);
  long GetGridNodes(Grid *&pGrid);
};
}  // namespace _SDEAdo

#endif  //_SDE_GRIDFCLS_ADOLAYER_H