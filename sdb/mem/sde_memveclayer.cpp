#include "geo_geometry.h"
#include "gis_feature.h"
#include "sde_mem.h"
#include "smt_api.h"


using namespace core;
using namespace core;
using namespace _GIS;

namespace _SDEMem {
MemVecLayer::MemVecLayer(DataSource *pOwnerDs) : VectorLayer(pOwnerDs) {
  m_pOwnerDs = NULL;
  m_pFilterGeom = NULL;
  sprintf(m_szLayerName, "MemLayer");
  m_bIsVisible = true;
  m_nIteratorIndex = 0;
}

MemVecLayer::~MemVecLayer(void) {
  if (IsOpen()) Close();

  SMT_SAFE_DELETE(m_pFilterGeom);

  DeleteAll();
}
//////////////////////////////////////////////////////////////////////////
bool MemVecLayer::Open(const char *szName) {
  m_bOpen = true;
  strcpy(m_szLayerName, szName);

  return m_bOpen;
}

bool MemVecLayer::Create(void) { return true; }

bool MemVecLayer::Close(void) {
  m_bOpen = false;
  return true;
}

bool MemVecLayer::Fetch(eFetchType type) {
  if (!IsOpen()) return false;

  return true;
}

void MemVecLayer::MoveFirst(void) const {
  if (!IsOpen()) return;

  m_nIteratorIndex = 0;
}

void MemVecLayer::MoveNext(void) const {
  if (!IsOpen()) return;

  if (m_nIteratorIndex < m_vFeatures.size()) m_nIteratorIndex++;
}

void MemVecLayer::MoveLast(void) const {
  if (!IsOpen()) return;

  m_nIteratorIndex = m_vFeatures.size() - 1;
}

void MemVecLayer::Delete(void) {
  if (!IsOpen()) return;

  int i = 0;
  vector<Feature *>::iterator iter = m_vFeatures.begin();
  while (iter != m_vFeatures.end() && i < m_nIteratorIndex) {
    i++;
    iter++;
  }

  if (iter != m_vFeatures.end()) {
    SMT_SAFE_DELETE(*iter);
    m_vFeatures.erase(iter);
  }

  CalEnvelope();
}

bool MemVecLayer::IsEnd(void) const {
  return (m_nIteratorIndex == m_vFeatures.size());
}

void MemVecLayer::DeleteAll(void) {
  if (!IsOpen()) return;

  vector<Feature *>::iterator i = m_vFeatures.begin();

  while (i != m_vFeatures.end()) {
    SMT_SAFE_DELETE(*i);
    ++i;
  }
  m_vFeatures.clear();
  m_nIteratorIndex = 0;
}

long MemVecLayer::Query(const GQueryDesc *pGQueryDesc,
                        const PQueryDesc *pPQueryDesc,
                        VectorLayer *pQueryResult) {
  if (!IsOpen()) return SMT_ERR_DB_OPER;

  if (pQueryResult) {
    vector<Feature *>::iterator iter = m_vFeatures.begin();
    while (iter != m_vFeatures.end()) {
      Geometry *pGeom = (*iter)->GetGeometryRef();
      if (pGeom) {
        long lRs =
            pGQueryDesc->pQueryGeom->Relationship(pGeom, pGQueryDesc->select_margin);

        if (pGQueryDesc->sSRs & lRs) pQueryResult->AppendFeature((*iter), true);
      }
      iter++;
    }
  }
  return SMT_ERR_NONE;
}

long MemVecLayer::AppendFeature(const Feature *pFeature, bool bclone) {
  /*
  vector<Feature*>::iterator iter = m_vFeatures.begin() ;
  while (iter != m_vFeatures.end())
  {
  if (pFeature->GetID() == (*iter)->GetID())
  return SMT_ERR_FAILURE;
  iter++;
  }
  */

  if (!IsOpen()) return SMT_ERR_DB_OPER;

  m_vFeatures.push_back((bclone) ? pFeature->Clone()
                                 : const_cast<Feature *>(pFeature));

  Envelope oGeomEnv;
  const Geometry *pGeom = pFeature->GetGeometryRef();
  if (pGeom) {
    pGeom->GetEnvelope(&oGeomEnv);
    m_lyrEnv.Merge(oGeomEnv);
  }

  return SMT_ERR_NONE;
}

long MemVecLayer::AppendFeatureBatch(const Feature *pFeature, bool bClone) {
  return AppendFeature(pFeature, bClone);
}

long MemVecLayer::UpdateFeature(const Feature *pFeature) {
  if (!IsOpen()) return SMT_ERR_DB_OPER;

  vector<Feature *>::iterator iter = m_vFeatures.begin();
  while (iter != m_vFeatures.end()) {
    if (pFeature == (*iter)) return SMT_ERR_NONE;

    if (pFeature->GetID() == (*iter)->GetID()) {
      SMT_SAFE_DELETE(*iter);
      (*iter) = pFeature->Clone();

      return SMT_ERR_NONE;
    }
    iter++;
  }
  return SMT_ERR_FAILURE;
}

long MemVecLayer::DeleteFeature(const Feature *pFeature) {
  if (!IsOpen()) return SMT_ERR_DB_OPER;

  vector<Feature *>::iterator iter = m_vFeatures.begin();
  while (iter != m_vFeatures.end()) {
    if (pFeature->GetID() == (*iter)->GetID()) {
      SMT_SAFE_DELETE(*iter);
      m_vFeatures.erase(iter);
      return SMT_ERR_NONE;
    }
    iter++;
  }
  return SMT_ERR_FAILURE;
}

Feature *MemVecLayer::GetFeature() const {
  if (!IsOpen()) return NULL;

  return GetFeature(m_nIteratorIndex);
}

Feature *MemVecLayer::GetFeature(int index) const {
  if (!IsOpen()) return NULL;

  if (m_vFeatures.size() < 1 || index < 0 || index > (m_vFeatures.size() - 1))
    return NULL;

  return m_vFeatures[index];
}

Feature *MemVecLayer::GetFeatureByID(uint unID) const {
  if (!IsOpen()) return NULL;

  /*	vector<Feature*>::iterator iter = m_vFeatures.begin() ;
  while (iter != m_vFeatures.end())
  {
          if (unID == (*iter)->GetID())
          {
                  return (*iter);
          }
          iter++;
  }*/
  for (int i = 0; i < m_vFeatures.size(); i++) {
    if (NULL != m_vFeatures[i] && unID == (m_vFeatures[i])->GetID()) {
      return (m_vFeatures[i]);
    }
  }

  return NULL;
}

void MemVecLayer::CalEnvelope(void) {
  if (!IsOpen()) return;

  m_lyrEnv = Envelope();

  Envelope oGeomEnv;
  for (int iFeature = 0; iFeature < m_vFeatures.size(); iFeature++) {
    m_vFeatures[iFeature]->GetGeometryRef()->GetEnvelope(&oGeomEnv);
    m_lyrEnv.Merge(oGeomEnv);
  }
}
}  // namespace _SDEMem