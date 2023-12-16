#ifndef _SDE_ADO_H
#define _SDE_ADO_H

#include "gis_sde.h"
using namespace _GIS;

#include "ado_adoconnection.h"
#include "ado_adorecordset.h"


using namespace _Ado;

const string C_STR_SDE_ADODEVICE_LOG = "SDEAdoDevice";

#define MAX_LAYER_TB_NAME MAX_LAYER_ARCHIVE_NAME
#define DS_TABLE_NAME ("SMT_DS_TB")
#define DS_FLD_LYR_NAME ("layer_name")
#define DS_FLD_LYR_TABLENAME ("layer_tablename")
#define DS_FLD_LYR_TYPE ("layer_type")
#define DS_FLD_LYR_SPIDXTYPE ("layer_spidx_type")

#define DS_FLD_LYR_XMIN ("layer_xmin")
#define DS_FLD_LYR_YMIN ("layer_xmin")
#define DS_FLD_LYR_XMAX ("layer_xmin")
#define DS_FLD_LYR_YMAX ("layer_xmin")

#define SQL_STRING_BUF_LENGTH TEMP_BUFFER_SIZE

namespace _SDEAdo {
typedef Attribute FieldCollect;

class GIS_CORE_EXPORT AdoVecLayer : public VectorLayer {
 public:
  AdoVecLayer(DataSource *pOwnerDs);
  virtual ~AdoVecLayer(void);

 public:
  bool Open(const char *szLayerTableName);
  bool Close(void);
  bool Fetch(eFetchType type = FETCH_ALL);

  //////////////////////////////////////////////////////////////////////////
  int GetFeatureCount(void) const;

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
  virtual long AppendFeature(const Feature *pFeature, bool bclone = false) = 0;

  long UpdateFeatureBatch(void);

  virtual long UpdateFeature(const Feature *pFeature) = 0;
  virtual long DeleteFeature(const Feature *pFeature);

  Feature *GetFeature() const;
  Feature *GetFeature(int index) const;
  Feature *GetFeatureByID(uint unID) const;

  //////////////////////////////////////////////////////////////////////////
  void CalEnvelope(void);
  void SetLayerRect(const fRect &lyrRect);

 public:
  virtual void SetDefaultAttFields(void) = 0;
  virtual void SetDefaultGeomFields(void) = 0;
  virtual void SetTableFields(void) = 0;

  virtual void GetFeature(Feature *pFeature) = 0;

 protected:
  VectorLayer *m_pMemLayer;
  FieldCollect *m_pGeomFieldCollect;
  FieldCollect *m_pTableFieldCollect;

  AdoRecordSet m_Recordset;
  char m_szLayerTableName[MAX_LAYER_TB_NAME];
};

class AdoRasLayer : public RasterLayer {
 public:
  AdoRasLayer(DataSource *pOwnerDs);
  virtual ~AdoRasLayer(void);

 public:
  //////////////////////////////////////////////////////////////////////////
  // create
  bool Create(void);
  bool Open(const char *szLayerArchiveName);
  bool Close(void);
  bool Fetch(eFetchType type = FETCH_ALL);

  void CalEnvelope(void);
  void SetLayerRect(const fRect &lyrRect);
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

  //////////////////////////////////////////////////////////////////////////
 public:
  virtual void SetDefaultAttFields(void);
  virtual void SetDefaultGeomFields(void);
  virtual void SetTableFields(void);

 private:
  long SyncWrite(void);
  long SyncRead(void);

 protected:
  RasterLayer *m_pMemLayer;
  FieldCollect *m_pGeomFieldCollect;
  FieldCollect *m_pTableFieldCollect;

  AdoRecordSet m_Recordset;
  char m_szLayerTableName[MAX_LAYER_TB_NAME];
};

//////////////////////////////////////////////////////////////////////////
class GIS_CORE_EXPORT AdoDataSource : public DataSource {
 public:
  AdoDataSource(void);
  virtual ~AdoDataSource(void);

 public:
  bool Create(void);
  bool Open();
  bool Close();

  DataSource *Clone(void) const;

 public:
  AdoConnection &GetConnection(void) { return m_Connection; }

 public:
  VectorLayer *CreateVectorLayer(const char *szName, fRect &lyrRect,
                                 FeatureType ftType = FeatureType::FtDot);
  VectorLayer *OpenVectorLayer(const char *szLayerTableName);
  bool DeleteVectorLayer(const char *szName);

  RasterLayer *CreateRasterLayer(const char *szName, fRect &lyrRect,
                                 long lImageCode);
  RasterLayer *OpenRasterLayer(const char *szName);
  bool DeleteRasterLayer(const char *szName);

  // unsupport
  TileLayer *CreateTileLayer(const char *szName, fRect &lyrRect,
                             long lImageCode) {
    return NULL;
  }
  TileLayer *OpenTileLayer(const char *szName) { return NULL; }
  bool DeleteTileLayer(const char *szName) { return false; }

 public:
  bool CreateLayerTable(LayerInfo &info, Attribute *pAtt);

  bool IsTableExist(const char *szTableName);
  bool CreateTable(const char *szTableName, Attribute *pAtt);
  inline bool OpenTable(const char *szTableName, AdoRecordSet *pRecordset,
                        CursorTypeEnum CursorType = adOpenStatic,
                        LockTypeEnum LockType = adLockOptimistic);
  inline bool CloseTable(const char *szTableName, AdoRecordSet *pRecordset);
  inline bool DropTable(const char *szTableName);
  inline bool DropTrigger(const char *szTableName);
  inline bool ClearTable(const char *szTableName);

  inline void CetTableFldCreatingString(string &str, Field &smtFld);

 protected:
  bool CreateDsTable(void);

  bool GetLayerTableInfos(void);

  void GetLayerTableInfo(LayerInfo &info);
  bool GetLayerTableInfoByLayerName(LayerInfo &info, const char *szName);

  bool AppendLayerTableInfo(LayerInfo &info);
  bool DeleteLayerTableInfo(LayerInfo &info);
  bool UpdateLayerTableInfo(LayerInfo &info);

 protected:
  AdoConnection m_Connection;
  AdoRecordSet m_Recordset;
};
}  // namespace _SDEAdo
#endif  //_SDE_ADO_H