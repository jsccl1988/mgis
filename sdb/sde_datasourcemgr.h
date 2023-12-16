#ifndef _SMT_DSMGR_H
#define _SMT_DSMGR_H

#include "gis_sde.h"

using namespace _GIS;

namespace _SDEDevMgr {
class GIS_CORE_EXPORT DataSourceMgr {
 private:
  DataSourceMgr(void);

 public:
  virtual ~DataSourceMgr(void);

 public:
  static Layer *CreateMemLayer(LayerType eLyrType);
  static void DestoryMemLayer(Layer *&pLayer);

  static VectorLayer *CreateMemVecLayer(void);
  static void DestoryMemVecLayer(VectorLayer *&pLayer);

  static RasterLayer *CreateMemRasLayer(void);
  static void DestoryMemRasLayer(RasterLayer *&pLayer);

  static TileLayer *CreateMemTileLayer(void);
  static void DestoryMemTileLayer(TileLayer *&pLayer);

 public:
  static DataSourceMgr *GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  bool Open(const char *szDSMFile);
  bool Save(void);
  bool SaveAs(const char *szDSMFile);

 public:
  DataSource *CreateTmpDataSource(eDSType type);
  void DestoryTmpDataSource(DataSource *&pTmpFileDS);

  DataSource *CreateDataSource(DataSourceInfo &info);
  bool DeleteDataSource(const char *szName);

 public:
  //////////////////////////////////////////////////////////////////////////
  int GetDataSourceCount(void) { return m_vDataSources.size(); }

  void MoveFirst(void);
  void MoveNext(void);
  void MoveLast(void);
  void Delete(void);
  bool IsEnd(void);

  DataSource *GetDataSource();

 public:
  DataSource *GetDataSource(int index);
  DataSource *GetDataSource(const char *szName);

  inline DataSource *GetActiveDataSource(void) { return m_pActiveDS; }
  void SetActiveDataSource(const char *szActiveDSName);
  inline void SetActiveDataSource(DataSource *pActiveDS) {
    if (pActiveDS) {
      m_pActiveDS = pActiveDS;
    }
  }

 private:
  DataSource *m_pActiveDS;
  vector<DataSource *> m_vDataSources;
  int m_nIteratorIndex;
  string m_strDSMFilePath;

 private:
  static DataSourceMgr *singleton_;
};
}  // namespace _SDEDevMgr

#if !defined(Export_SDEDeviceMgr)
#if defined(_DEBUG)
#pragma comment(lib, "SDEDeviceMgrD.lib")
#else
#pragma comment(lib, "SDEDeviceMgr.lib")
#endif
#endif

#endif  //_SMT_DSMGR_H