#include "sde_ado.h"
#include "sde_mem.h"

using namespace core;
using namespace core;
using namespace _SDEMem;
using namespace _GIS;

namespace _SDEAdo {
AdoVecLayer::AdoVecLayer(DataSource *pOwnerDs)
    : VectorLayer(pOwnerDs),
      m_pGeomFieldCollect(NULL),
      m_pTableFieldCollect(NULL) {
  m_pOwnerDs = pOwnerDs->Clone();
  m_pOwnerDs->Open();

  m_pFilterGeom = NULL;
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

    m_pMemLayer =
        pDS->CreateVectorLayer("MemVecLayer", lyrRect, FeatureType::FtUnknown);
  }

  SMT_SAFE_DELETE(pDS);
}

AdoVecLayer::~AdoVecLayer(void) {
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
bool AdoVecLayer::Open(const char *szLayerTableName) {
  sprintf(m_szLayerTableName, szLayerTableName);

  if (m_Recordset.IsOpen()) m_Recordset.Close();

  m_bOpen = ((AdoDataSource *)m_pOwnerDs)
                ->OpenTable(m_szLayerTableName, &m_Recordset) &&
            m_pMemLayer->Open(m_szLayerTableName);

  return m_bOpen;
}

bool AdoVecLayer::Close(void) {
  m_bOpen = !(((AdoDataSource *)m_pOwnerDs)
                  ->CloseTable(m_szLayerTableName, &m_Recordset) &&
              m_pMemLayer->Close());
  return true;
}

bool AdoVecLayer::Fetch(eFetchType type) {
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
    Feature *pFeature = new Feature();
    GetFeature(pFeature);
    m_pMemLayer->AppendFeature(pFeature);
    m_Recordset.MoveNext();
  }

  Open(m_szLayerTableName);

  CalEnvelope();

  return true;
}

long AdoVecLayer::UpdateFeatureBatch(void) {
  if (!IsOpen())
    return SMT_ERR_DB_OPER;
  else {
    m_Recordset.UpdateBatch();
    CalEnvelope();
  }

  return SMT_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int AdoVecLayer::GetFeatureCount(void) const {
  return m_pMemLayer->GetFeatureCount();
}

void AdoVecLayer::MoveFirst(void) const { m_pMemLayer->MoveFirst(); }

void AdoVecLayer::MoveNext(void) const { m_pMemLayer->MoveNext(); }

void AdoVecLayer::MoveLast(void) const { m_pMemLayer->MoveLast(); }

void AdoVecLayer::Delete(void) {
  Feature *pFea = m_pMemLayer->GetFeature();
  if (NULL != pFea) {
    AdoRecordSet smtRecordset;
    smtRecordset.SetAdoConnection(
        &(((AdoDataSource *)m_pOwnerDs)->GetConnection()));

    char szSQL[TEMP_BUFFER_SIZE];
    sprintf_s(szSQL, TEMP_BUFFER_SIZE, "select * from %s where ID = %d;",
              m_szLayerTableName, pFea->GetID());
    if (smtRecordset.Open(szSQL)) {
      smtRecordset.MoveFirst();
      if (smtRecordset.IsOpen() && !smtRecordset.IsEOF() &&
          smtRecordset.GetRecordCount() == 1) {
        if (smtRecordset.Delete()) {
          // mem
          m_pMemLayer->Delete();
          CalEnvelope();
        }
      }

      Open(m_szLayerTableName);
    }
  }
}

bool AdoVecLayer::IsEnd(void) const { return m_pMemLayer->IsEnd(); }

void AdoVecLayer::DeleteAll(void) {
  if (((AdoDataSource *)m_pOwnerDs)->ClearTable(m_szLayerTableName)) {
    // mem
    m_pMemLayer->DeleteAll();
    CalEnvelope();
  }
}

//////////////////////////////////////////////////////////////////////////
Feature *AdoVecLayer::GetFeature() const { return m_pMemLayer->GetFeature(); }

Feature *AdoVecLayer::GetFeature(int index) const {
  return m_pMemLayer->GetFeature(index);
}

Feature *AdoVecLayer::GetFeatureByID(uint unID) const {
  return m_pMemLayer->GetFeatureByID(unID);
}

long AdoVecLayer::DeleteFeature(const Feature *pFeature) {
  if (NULL == pFeature) return SMT_ERR_INVALID_PARAM;

  char szSQL[TEMP_BUFFER_SIZE];
  sprintf_s(szSQL, TEMP_BUFFER_SIZE, "select * from %s where ID = %d;",
            m_szLayerTableName, pFeature->GetID());
  if (m_Recordset.Open(szSQL)) {
    m_Recordset.MoveFirst();
    if (m_Recordset.IsOpen() && !m_Recordset.IsEOF() &&
        m_Recordset.GetRecordCount() == 1) {
      if (m_Recordset.Delete()) {
        // mem
        m_pMemLayer->DeleteFeature(pFeature);
        CalEnvelope();
      }
    }
  }

  return SMT_ERR_NONE;
}

long AdoVecLayer::Query(const GQueryDesc *pGQueryDesc,
                        const PQueryDesc *pPQueryDesc,
                        VectorLayer *pQueryResult) {
  return m_pMemLayer->Query(pGQueryDesc, pPQueryDesc, pQueryResult);
}

void AdoVecLayer::CalEnvelope(void) {
  Envelope env;
  m_pMemLayer->CalEnvelope();
  m_pMemLayer->GetEnvelope(env);
  memcpy(&m_lyrEnv, &env, sizeof(Envelope));
}

void AdoVecLayer::SetLayerRect(const fRect &lyrRect) {
  m_pMemLayer->SetLayerRect(lyrRect);
  memcpy(&m_lyrEnv, &lyrRect, sizeof(fRect));
}
}  // namespace _SDEAdo