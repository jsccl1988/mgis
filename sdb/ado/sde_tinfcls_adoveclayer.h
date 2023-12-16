#ifndef _SDE_TINFCLS_ADOLAYER_H
#define _SDE_TINFCLS_ADOLAYER_H

#include "sde_ado.h"

namespace _SDEAdo {
class TinFclsAdoLayer : public AdoVecLayer {
 public:
  TinFclsAdoLayer(DataSource *pOwnerDs);
  virtual ~TinFclsAdoLayer(void);

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
  long AppendTin(const Tin *pTin);
  long AppendTinTriangles(const Tin *pTin);
  long AppendTinNodes(const Tin *pTin);

  long GetTin(Geometry *&pTin);
  long GetTinTriangles(Tin *&pTin);
  long GetTinNodes(Tin *&pTin);
};
}  // namespace _SDEAdo

#endif  //_SDE_TINFCLS_ADOLAYER_H