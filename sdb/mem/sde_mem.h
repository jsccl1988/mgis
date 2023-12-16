#ifndef _SDE_MEM_H
#define _SDE_MEM_H

#include "gis_sde.h"

using namespace _GIS;

namespace _SDEMem {
class GIS_CORE_EXPORT MemVecLayer : public VectorLayer {
 public:
  MemVecLayer(DataSource *pOwnerDs);
  virtual ~MemVecLayer(void);

 public:
  bool Create(void);
  bool Open(const char *szName);
  bool Close(void);
  bool Fetch(eFetchType type = FETCH_ALL);

  //////////////////////////////////////////////////////////////////////////
  int GetFeatureCount(void) const { return m_vFeatures.size(); }

  // iter record
  void MoveFirst(void) const;
  void MoveNext(void) const;
  void MoveLast(void) const;
  void Delete(void);
  bool IsEnd(void) const;

  void DeleteAll(void);

  //////////////////////////////////////////////////////////////////////////
  // spatial index
  long CreateSpatialIndex(const char *szName, uint type) {
    return SMT_ERR_NONE;
  }

  // query
  long Query(const GQueryDesc *pGQueryDesc, const PQueryDesc *pPQueryDesc,
             VectorLayer *pQueryResult);

  // feature
  long AppendFeature(const Feature *pFeature, bool bclone = false);

  long AppendFeatureBatch(const Feature *pFeature, bool bClone = false);
  long UpdateFeatureBatch(void) { return SMT_ERR_NONE; }

  long UpdateFeature(const Feature *pFeature);
  long DeleteFeature(const Feature *pFeature);

  Feature *GetFeature() const;
  Feature *GetFeature(int index) const;
  Feature *GetFeatureByID(uint unID) const;

  //////////////////////////////////////////////////////////////////////////
  void CalEnvelope(void);

 protected:
  vector<Feature *> m_vFeatures;
  mutable int m_nIteratorIndex;
};

class MemRasLayer : public RasterLayer {
 public:
  MemRasLayer(DataSource *pOwnerDs);
  virtual ~MemRasLayer(void);

 public:
  //////////////////////////////////////////////////////////////////////////
  // create
  bool Create(void);
  bool Open(const char *szLayerArchiveName);
  bool Close(void);
  bool Fetch(eFetchType type = FETCH_ALL);

  void CalEnvelope(void);
  //////////////////////////////////////////////////////////////////////////
  // raster oper
  long CreaterRaster(const char *pRasterBuf, long lRasterBufSize,
                     const fRect &fRasterRect, long lImageCode);
  long SetRasterRect(const fRect &fLocRect);

  long GetRaster(char *&pRasterBuf, long &lRasterBufSize, fRect &fRasterRect,
                 long &lImageCode) const;
  long GetRasterNoClone(char *&pRasterBuf, long &lRasterBufSize,
                        fRect &fLocRect, long &lImageCode) const;

  long GetRasterRect(fRect &fLocRect) const;

 protected:
  fRect m_fRasterRect;

  char *m_pRasterBuf;
  long m_lRasterBufSize;
  long m_lCodeType;
};

class MemTileLayer : public TileLayer {
 public:
  MemTileLayer(DataSource *pOwnerDs);
  virtual ~MemTileLayer(void);

 public:
  //////////////////////////////////////////////////////////////////////////
  // create
  bool Create(void);
  bool Open(const char *szLayerArchiveName);
  bool Close(void);
  bool Fetch(eFetchType type = FETCH_ALL);

  void CalEnvelope(void);

 public:
  //////////////////////////////////////////////////////////////////////////
  // title set oper
  int GetTileCount(void) const { return m_vTilePtrs.size(); }

  void MoveFirst(void) const;
  void MoveNext(void) const;
  void MoveLast(void) const;
  void Delete(void);
  bool IsEnd(void) const;

  void DeleteAll(void);

 public:
  long AppendTile(const Tile *pTile, bool bClone = false);
  long UpdateTile(const Tile *pTile);
  long DeleteTile(const Tile *pTile);

  Tile *GetTile() const;
  Tile *GetTile(int index) const;
  Tile *GetTileByID(uint unID) const;

 protected:
  vector<Tile *> m_vTilePtrs;
  mutable int m_nIteratorIndex;
};

//////////////////////////////////////////////////////////////////////////
class GIS_CORE_EXPORT MemDataSource : public DataSource {
 public:
  MemDataSource(void);
  virtual ~MemDataSource(void);

 public:
  bool Create(void);
  bool Open();
  bool Close();

  DataSource *Clone(void) const;

 public:
  VectorLayer *CreateVectorLayer(const char *pszName, fRect &lyrRect,
                                 FeatureType ftType = FeatureType::FtDot);
  VectorLayer *OpenVectorLayer(const char *pszLayerFileName);
  bool DeleteVectorLayer(const char *pszName);

  RasterLayer *CreateRasterLayer(const char *szName, fRect &lyrRect,
                                 long lImageCode);
  RasterLayer *OpenRasterLayer(const char *szName);
  bool DeleteRasterLayer(const char *szName);

  TileLayer *CreateTileLayer(const char *szName, fRect &lyrRect,
                             long lImageCode);
  TileLayer *OpenTileLayer(const char *szName);
  bool DeleteTileLayer(const char *szName);
};
}  // namespace _SDEMem

#endif  //_SDE_MEM_H