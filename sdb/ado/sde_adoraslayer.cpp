#include "bl_api.h"
#include "sde_ado.h"
#include "sde_mem.h"


using namespace core;
using namespace core;
using namespace _SDEMem;
using namespace _GIS;

namespace _SDEAdo {
AdoRasLayer::AdoRasLayer(DataSource *pOwnerDs)
    : RasterLayer(pOwnerDs),
      m_pGeomFieldCollect(NULL),
      m_pTableFieldCollect(NULL) {
  m_pOwnerDs = pOwnerDs->Clone();
  m_pOwnerDs->Open();

  m_bIsVisible = true;

  m_Recordset.SetAdoConnection(
      &(((AdoDataSource *)m_pOwnerDs)->GetConnection()));

  DataSource *pDS = new MemDataSource();
  if (pDS) {
    fRect lyrRect;
    lyrRect.lb.x = 0;
    lyrRect.lb.y = 0;
    lyrRect.rt.x = 500;
    lyrRect.rt.y = 500;

    m_pMemLayer = pDS->CreateRasterLayer("MemRasLayer", lyrRect, -1);
  }

  SMT_SAFE_DELETE(pDS);
}

AdoRasLayer::~AdoRasLayer(void) {
  //////////////////////////////////////////////////////////////////////////
  SMT_SAFE_DELETE(m_pGeomFieldCollect);
  SMT_SAFE_DELETE(m_pTableFieldCollect);

  //////////////////////////////////////////////////////////////////////////
  // db
  if (IsOpen()) {
    Close();
  }

  //////////////////////////////////////////////////////////////////////////
  // mem
  SMT_SAFE_DELETE(m_pMemLayer);

  //////////////////////////////////////////////////////////////////////////

  if (m_Recordset.IsOpen()) m_Recordset.Close();

  if (m_pOwnerDs->IsOpen()) {
    m_pOwnerDs->Close();
    SMT_SAFE_DELETE(m_pOwnerDs);
  }
}
//////////////////////////////////////////////////////////////////////////
bool AdoRasLayer::Open(const char *szLayerTableName) {
  sprintf(m_szLayerTableName, szLayerTableName);

  if (m_Recordset.IsOpen()) m_Recordset.Close();

  m_bOpen = ((AdoDataSource *)m_pOwnerDs)
                ->OpenTable(m_szLayerTableName, &m_Recordset) &&
            m_pMemLayer->Open(m_szLayerTableName);

  return m_bOpen;
}

bool AdoRasLayer::Close(void) {
  m_bOpen = !(((AdoDataSource *)m_pOwnerDs)
                  ->CloseTable(m_szLayerTableName, &m_Recordset) &&
              m_pMemLayer->Close());
  return true;
}

bool AdoRasLayer::Fetch(eFetchType type) {
  if (m_Recordset.IsOpen()) m_Recordset.Close();

  m_bOpen =
      ((AdoDataSource *)m_pOwnerDs)
          ->OpenTable(m_szLayerTableName, &m_Recordset, adOpenForwardOnly);

  if (!IsOpen()) return false;

  if (m_pAtt == NULL) SetDefaultAttFields();

  if (m_pGeomFieldCollect == NULL) SetDefaultGeomFields();

  if (m_pTableFieldCollect == NULL) SetTableFields();

  fRect lyrRect;
  SetLayerRect(lyrRect);

  m_Recordset.MoveFirst();
  while (!m_Recordset.IsEOF()) {
    SyncRead();
    m_Recordset.MoveNext();
  }

  Open(m_szLayerTableName);

  CalEnvelope();

  return true;
}

bool AdoRasLayer::Create(void) {
  bool bRet = true;

  //
  SetDefaultAttFields();
  SetDefaultGeomFields();
  SetTableFields();

  sprintf(m_szLayerTableName, "%s_RasFcls", m_szLayerName);
  LayerInfo info;
  sprintf(info.szArchiveName, m_szLayerTableName);
  sprintf(info.szName, m_szLayerName);
  info.unFeatureType = Layer_Ras;
  EnvelopeToRect(info.lyrRect, m_lyrEnv);

  return ((AdoDataSource *)m_pOwnerDs)
      ->CreateLayerTable(info, m_pTableFieldCollect);
}

//////////////////////////////////////////////////////////////////////////
void AdoRasLayer::SetDefaultAttFields(void) {
  SMT_SAFE_DELETE(m_pAtt);
  m_pAtt = new Attribute();

  /*Field smtFld;

  smtFld.SetName("ID");
  smtFld.SetType(VarType::Integer);
  m_pAtt->AddField(smtFld);*/
}

void AdoRasLayer::SetDefaultGeomFields(void) {
  SMT_SAFE_DELETE(m_pGeomFieldCollect);
  m_pGeomFieldCollect = new FieldCollect();

  Field smtFld;

  // raster
  smtFld.SetName("mbr_xmin");
  smtFld.SetType(VarType::Real);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("mbr_ymin");
  smtFld.SetType(VarType::Real);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("mbr_xmax");
  smtFld.SetType(VarType::Real);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("mbr_ymax");
  smtFld.SetType(VarType::Real);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("image_code");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("image_size");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("image_bin");
  smtFld.SetType(VarType::Binary);
  m_pGeomFieldCollect->AddField(smtFld);
}

void AdoRasLayer::SetTableFields(void) {
  SMT_SAFE_DELETE(m_pTableFieldCollect);
  m_pTableFieldCollect = new FieldCollect();

  Field smtFld;
  //////////////////////////////////////////////////////////////////////////
  // raster
  smtFld.SetName("mbr_xmin");
  smtFld.SetType(VarType::Real);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("mbr_ymin");
  smtFld.SetType(VarType::Real);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("mbr_xmax");
  smtFld.SetType(VarType::Real);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("mbr_ymax");
  smtFld.SetType(VarType::Real);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("image_code");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("image_size");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("image_bin");
  smtFld.SetType(VarType::Binary);
  m_pTableFieldCollect->AddField(smtFld);
  //////////////////////////////////////////////////////////////////////////
}

long AdoRasLayer::SyncWrite(void) {
  long lImageCode = -1;
  char *pRasterBuf = NULL;
  long lRasterBufSize = 0;
  fRect fRasterRect;

  m_pMemLayer->GetRasterNoClone(pRasterBuf, lRasterBufSize, fRasterRect,
                                lImageCode);

  m_Recordset.AddNew();
  m_Recordset.PutCollect("mbr_xmin", _variant_t(fRasterRect.lb.x));
  m_Recordset.PutCollect("mbr_ymin", _variant_t(fRasterRect.lb.y));
  m_Recordset.PutCollect("mbr_xmax", _variant_t(fRasterRect.rt.x));
  m_Recordset.PutCollect("mbr_ymax", _variant_t(fRasterRect.rt.y));

  m_Recordset.PutCollect("image_code", _variant_t(lImageCode));
  m_Recordset.PutCollect("image_size", _variant_t(lRasterBufSize));

  char *pBuf = NULL;
  VARIANT varBLOB;
  SAFEARRAY *pSa = NULL;
  SAFEARRAYBOUND rgSaBound[1];
  if (pRasterBuf) {
    rgSaBound[0].lLbound = 0;
    rgSaBound[0].cElements = lRasterBufSize;

    pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
    if (SafeArrayAccessData(pSa, (void **)&pBuf) == NOERROR) {
      memcpy(pBuf, pRasterBuf, lRasterBufSize);
    }
    SafeArrayUnaccessData(pSa);

    varBLOB.vt = VT_ARRAY | VT_UI1;
    varBLOB.parray = pSa;

    m_Recordset.GetField("image_bin")->AppendChunk(varBLOB);

    SafeArrayDestroy(pSa);
  }

  if (!m_Recordset.Update()) return SMT_ERR_DB_OPER;

  return SMT_ERR_NONE;
}

long AdoRasLayer::SyncRead(void) {
  long lImageCode = -1;
  char *pRasterBuf = NULL;
  long lRasterBufSize = 0;
  fRect fRasterRect;

  m_Recordset.GetCollect("mbr_xmin", fRasterRect.lb.x);
  m_Recordset.GetCollect("mbr_ymin", fRasterRect.lb.y);
  m_Recordset.GetCollect("mbr_xmax", fRasterRect.rt.x);
  m_Recordset.GetCollect("mbr_ymax", fRasterRect.rt.y);

  m_Recordset.GetCollect("image_code", lImageCode);
  m_Recordset.GetCollect("image_size", lRasterBufSize);

  long lDataSize = m_Recordset.GetField("image_bin")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("image_bin")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      SafeArrayAccessData(varBLOB.parray, (void **)&pRasterBuf);

      long lRtn = m_pMemLayer->CreaterRaster(pRasterBuf, lRasterBufSize,
                                             fRasterRect, lImageCode);

      SafeArrayUnaccessData(varBLOB.parray);

      CalEnvelope();
    }
  }

  return SMT_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////////

void AdoRasLayer::CalEnvelope(void) {
  Envelope env;
  m_pMemLayer->CalEnvelope();
  m_pMemLayer->GetEnvelope(env);
  memcpy(&m_lyrEnv, &env, sizeof(Envelope));
}

void AdoRasLayer::SetLayerRect(const fRect &lyrRect) {
  m_pMemLayer->SetLayerRect(lyrRect);
  memcpy(&m_lyrEnv, &lyrRect, sizeof(fRect));
}

//////////////////////////////////////////////////////////////////////////
// raster oper
long AdoRasLayer::SetRasterRect(const fRect &fLocRect) {
  SetLayerRect(fLocRect);

  return m_pMemLayer->SetRasterRect(fLocRect);
}

long AdoRasLayer::CreaterRaster(const char *pRasterBuf, long lRasterBufSize,
                                const fRect &fRasterRect, long lImageCode) {
  if (SMT_ERR_NONE == m_pMemLayer->CreaterRaster(pRasterBuf, lRasterBufSize,
                                                 fRasterRect, lImageCode)) {
    CalEnvelope();

    SyncWrite();

    return SMT_ERR_NONE;
  }

  return SMT_ERR_FAILURE;
}

long AdoRasLayer::GetRaster(char *&pRasterBuf, long &lRasterBufSize,
                            fRect &fRasterRect, long &lImageCode) const {
  return m_pMemLayer->GetRaster(pRasterBuf, lRasterBufSize, fRasterRect,
                                lImageCode);
}

long AdoRasLayer::GetRasterNoClone(char *&pRasterBuf, long &lRasterBufSize,
                                   fRect &fRasterRect, long &lImageCode) const {
  return m_pMemLayer->GetRasterNoClone(pRasterBuf, lRasterBufSize, fRasterRect,
                                       lImageCode);
}

long AdoRasLayer::GetRasterRect(fRect &fLocRect) const {
  return m_pMemLayer->GetRasterRect(fLocRect);
}
}  // namespace _SDEAdo