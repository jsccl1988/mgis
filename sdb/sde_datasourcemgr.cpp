#include "sde_datasourcemgr.h"

#include "sde_ado.h"
#include "sde_mem.h"
#include "sde_smf.h"
#include "sde_ws.h"
#include "smt_api.h"

//#include "smt_xml.h"

using namespace _SDESmf;
using namespace _SDEAdo;
using namespace _SDEMem;
using namespace _SDEWS;
using namespace core;

namespace _SDEDevMgr {
DataSourceMgr *DataSourceMgr::singleton_ = NULL;

DataSourceMgr *DataSourceMgr::GetSingletonPtr(void) {
  if (singleton_ == NULL) {
    singleton_ = new DataSourceMgr();
  }

  return singleton_;
}

void DataSourceMgr::DestoryInstance(void) { SMT_SAFE_DELETE(singleton_); }

//////////////////////////////////////////////////////////////////////////
Layer *DataSourceMgr::CreateMemLayer(LayerType eLyrType) {
  if (eLyrType == LYR_VECTOR)
    return CreateMemVecLayer();
  else if (eLyrType == LYR_RASTER)
    return CreateMemRasLayer();

  return NULL;
}

void DataSourceMgr::DestoryMemLayer(Layer *&pLayer) { SMT_SAFE_DELETE(pLayer); }

//////////////////////////////////////////////////////////////////////////
VectorLayer *DataSourceMgr::CreateMemVecLayer(void) {
  VectorLayer *pLayer = NULL;
  DataSource *pDS = new MemDataSource();

  if (pDS) {
    fRect lyrRect;
    lyrRect.lb.x = 0;
    lyrRect.lb.y = 0;
    lyrRect.rt.x = 500;
    lyrRect.rt.y = 500;

    pLayer =
        pDS->CreateVectorLayer("MemVecLayer", lyrRect, FeatureType::FtUnknown);
  }

  SMT_SAFE_DELETE(pDS);
  return pLayer;
}

void DataSourceMgr::DestoryMemVecLayer(VectorLayer *&pLayer) {
  SMT_SAFE_DELETE(pLayer);
}

//////////////////////////////////////////////////////////////////////////
RasterLayer *DataSourceMgr::CreateMemRasLayer(void) {
  RasterLayer *pLayer = NULL;
  DataSource *pDS = new MemDataSource();
  if (pDS) {
    fRect lyrRect;
    lyrRect.lb.x = 0;
    lyrRect.lb.y = 0;
    lyrRect.rt.x = 500;
    lyrRect.rt.y = 500;

    pLayer =
        pDS->CreateRasterLayer("MemRasLayer", lyrRect, FeatureType::FtUnknown);
  }

  SMT_SAFE_DELETE(pDS);
  return pLayer;
}

void DataSourceMgr::DestoryMemRasLayer(RasterLayer *&pLayer) {
  SMT_SAFE_DELETE(pLayer);
}

//////////////////////////////////////////////////////////////////////////
TileLayer *DataSourceMgr::CreateMemTileLayer(void) {
  TileLayer *pLayer = NULL;
  DataSource *pDS = new MemDataSource();
  if (pDS) {
    fRect lyrRect;
    lyrRect.lb.x = 0;
    lyrRect.lb.y = 0;
    lyrRect.rt.x = 500;
    lyrRect.rt.y = 500;

    pLayer =
        pDS->CreateTileLayer("MemTileLayer", lyrRect, FeatureType::FtUnknown);
  }

  SMT_SAFE_DELETE(pDS);

  return pLayer;
}

void DataSourceMgr::DestoryMemTileLayer(TileLayer *&pLayer) {
  SMT_SAFE_DELETE(pLayer);
}

//////////////////////////////////////////////////////////////////////////
DataSourceMgr::DataSourceMgr(void) {
  m_pActiveDS = NULL;
  m_nIteratorIndex = 0;
  m_strDSMFilePath = "";
}

DataSourceMgr::~DataSourceMgr(void) {
  Save();

  vector<DataSource *>::iterator iter = m_vDataSources.begin();

  while (iter != m_vDataSources.end()) {
    SMT_SAFE_DELETE(*iter);
    ++iter;
  }
  m_vDataSources.clear();

  m_pActiveDS = NULL;
}

DataSource *DataSourceMgr::GetDataSource(const char *szName) {
  DataSource *pDS = NULL;
  vector<DataSource *>::iterator iter = m_vDataSources.begin();

  while (iter != m_vDataSources.end()) {
    if (strcmp((*iter)->GetName(), szName) == 0) {
      pDS = (*iter);
      break;
    }
    ++iter;
  }

  return pDS;
}

//////////////////////////////////////////////////////////////////////////
DataSource *DataSourceMgr::CreateTmpDataSource(eDSType type) {
  DataSource *pDS = NULL;
  switch (type) {
    case DS_DB_ADO: {
      pDS = new AdoDataSource();
    } break;
    case DS_FILE_SMF: {
      pDS = new SmfDataSource();
    } break;
    case DS_MEM: {
      pDS = new MemDataSource();
    } break;
    case DS_WS: {
      pDS = new WSDataSource();
    } break;
    default:
      break;
  }

  return pDS;
}

void DataSourceMgr::DestoryTmpDataSource(DataSource *&pTmpFileDS) {
  SMT_SAFE_DELETE(pTmpFileDS);
}

//////////////////////////////////////////////////////////////////////////

DataSource *DataSourceMgr::CreateDataSource(DataSourceInfo &info) {
  if (strlen(info.szName) == 0) return NULL;

  if (GetDataSource(info.szName) != NULL) {
    return NULL;
  }

  DataSource *pDS = NULL;
  switch (info.unType) {
    case DS_DB_ADO: {
      pDS = new AdoDataSource();
      sprintf_s(info.szUrl, MAX_URL_LENGTH, "sdb:%s\\%s,%s,%s,%s,%d,%d",
                info.szName, info.db.szService, info.db.szDBName, info.szUID,
                info.szPWD, info.unType, info.unProvider);
    } break;
    case DS_FILE_SMF: {
      pDS = new SmfDataSource();
      sprintf_s(info.szUrl, MAX_URL_LENGTH, "sfile:%s\\%s,%s,%s,%s,%d,%d",
                info.szName, info.file.szPath, info.file.szFileName, info.szUID,
                info.szPWD, info.unType, info.unProvider);
    } break;
    case DS_MEM: {
      pDS = new MemDataSource();
      sprintf_s(info.szUrl, MAX_URL_LENGTH, "smem:%s\\%s,%s,%d,%d", info.szName,
                info.szUID, info.szPWD, info.unType, info.unProvider);
    } break;
    case DS_WS: {
      pDS = new WSDataSource();
      sprintf_s(info.szUrl, MAX_URL_LENGTH, "sws:%s\\%s,%s,%d,%d", info.szName,
                info.szUID, info.szPWD, info.unType, info.unProvider);
    } break;
    default:
      break;
  }

  if (NULL != pDS) {
    pDS->SetInfo(info);

    if (!pDS->Open()) {
      SMT_SAFE_DELETE(pDS);
    } else {
      pDS->Close();
      m_vDataSources.push_back(pDS);
    }
  }

  return pDS;
}

bool DataSourceMgr::DeleteDataSource(const char *szName) {
  bool bRet = true;
  vector<DataSource *>::iterator iter = m_vDataSources.begin();
  while (iter != m_vDataSources.end()) {
    if (strcmp((*iter)->GetName(), szName) == 0) {
      if ((*iter) == m_pActiveDS) m_pActiveDS = NULL;

      SMT_SAFE_DELETE(*iter);
      break;
    }
    iter++;
  }

  if (iter != m_vDataSources.end()) {
    m_vDataSources.erase(iter);

    bRet = true;
  }

  return bRet;
}

void DataSourceMgr::SetActiveDataSource(const char *szActiveDSName) {
  m_pActiveDS = GetDataSource(szActiveDSName);
}

//////////////////////////////////////////////////////////////////////////
void DataSourceMgr::MoveFirst(void) { m_nIteratorIndex = 0; }

void DataSourceMgr::MoveNext(void) {
  if (m_nIteratorIndex < m_vDataSources.size()) m_nIteratorIndex++;
}

void DataSourceMgr::MoveLast(void) {
  m_nIteratorIndex = m_vDataSources.size() - 1;
}

void DataSourceMgr::Delete(void) {
  DataSource *pDS = m_vDataSources[m_nIteratorIndex];

  SMT_SAFE_DELETE(pDS);

  m_vDataSources.erase(m_vDataSources.begin() + m_nIteratorIndex);
}

bool DataSourceMgr::IsEnd(void) {
  return (m_nIteratorIndex == m_vDataSources.size());
}

DataSource *DataSourceMgr::GetDataSource() {
  return GetDataSource(m_nIteratorIndex);
}

DataSource *DataSourceMgr::GetDataSource(int index) {
  if (index < 0 || index > (m_vDataSources.size() - 1)) return NULL;

  return m_vDataSources[index];
}

//////////////////////////////////////////////////////////////////////////
bool DataSourceMgr::Open(const char *szDSMFile) {
  if (strlen(szDSMFile) == 0) return false;

  m_strDSMFilePath = szDSMFile;

  ifstream infile;

  locale loc = locale::global(locale(".936"));
  infile.open(m_strDSMFilePath.c_str(), ios::out | ios::binary);
  locale::global(std::locale(loc));

  if (!infile.is_open()) {
    return SMT_ERR_FAILURE;
  }

  //////////////////////////////////////////////////////////////////////////
  // head
  char szHead[4] = "DSM";
  infile.read((char *)(szHead), sizeof(char) * 4);

  // content
  int nDSs = 0;
  infile.read((char *)(&nDSs), sizeof(int));

  DataSourceInfo info;
  for (int i = 0; i < nDSs; i++) {
    infile.read((char *)(&info), sizeof(DataSourceInfo));
    CreateDataSource(info);
  }
  //////////////////////////////////////////////////////////////////////////

  infile.close();

  return true;
}

bool DataSourceMgr::Save(void) {
  if (m_strDSMFilePath == "") {
    m_strDSMFilePath = GetAppPath() + "sys\\smartgis.dsm";
  }

  return SaveAs(m_strDSMFilePath.c_str());
}

bool DataSourceMgr::SaveAs(const char *szDSMFile) {
  if (strlen(szDSMFile) == 0) return false;

  ofstream outfile;

  locale loc = locale::global(locale(".936"));
  outfile.open(szDSMFile, ios::out | ios::binary);
  locale::global(std::locale(loc));

  if (!outfile.is_open()) {
    return false;
  }

  //////////////////////////////////////////////////////////////////////////
  // head
  char szHead[4] = "DSM";
  outfile.write((char *)(szHead), sizeof(char) * 4);

  // content
  int nDSs = m_vDataSources.size();
  outfile.write((char *)(&nDSs), sizeof(int));

  DataSourceInfo info;
  vector<DataSource *>::iterator iter = m_vDataSources.begin();

  while (iter != m_vDataSources.end()) {
    (*iter)->GetInfo(info);
    outfile.write((char *)(&info), sizeof(DataSourceInfo));
    ++iter;
  }
  //////////////////////////////////////////////////////////////////////////

  outfile.close();

  return true;
}

}  // namespace _SDEDevMgr