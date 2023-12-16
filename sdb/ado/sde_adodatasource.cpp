#include "sde_ado.h"
#include "sde_annofcls_adoveclayer.h"
#include "sde_cldimgfcls_adoveclayer.h"
#include "sde_curvefcls_adoveclayer.h"
#include "sde_dotfcls_adoveclayer.h"
#include "sde_gridfcls_adoveclayer.h"
#include "sde_surfacefcls_adoveclayer.h"
#include "sde_tinfcls_adoveclayer.h"
#include "smt_api.h"
#include "smt_logmanager.h"


namespace _SDEAdo {
AdoDataSource::AdoDataSource(void) {
  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetLog(C_STR_SDE_ADODEVICE_LOG);
  if (NULL == pLog) {
    pLogMgr->CreateLog(C_STR_SDE_ADODEVICE_LOG.c_str());
  }
}

AdoDataSource::~AdoDataSource(void) { Close(); }

bool AdoDataSource::Create(void) { return true; }

bool AdoDataSource::Open(void) {
  Close();

  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetLog(C_STR_SDE_ADODEVICE_LOG);

  pLog->LogMessage(
      "DS-Name:%s,DB Name:%s,DB Service:%s,User:%s,type:%d,provider:%d",
      m_dsInfo.szName, m_dsInfo.db.szDBName, m_dsInfo.db.szService,
      m_dsInfo.szUID, m_dsInfo.unType, m_dsInfo.unProvider);

  switch (m_dsInfo.unProvider) {
    case eDBProvider::PROVIDER_ACCESS: {
      string strAcessAllPath;
      strAcessAllPath += m_dsInfo.db.szService;
      strAcessAllPath += m_dsInfo.db.szDBName;
      strAcessAllPath += ".mdb";
      m_bOpen = m_Connection.ConnectAccess(strAcessAllPath.c_str(),
                                           m_dsInfo.szUID, m_dsInfo.szPWD);
    } break;
    case eDBProvider::PROVIDER_SQLSERVER: {
      m_bOpen = m_Connection.ConnectSqlServer(m_dsInfo.db.szService,
                                              m_dsInfo.db.szDBName,
                                              m_dsInfo.szUID, m_dsInfo.szPWD);
    } break;
  }

  if (m_bOpen) {
    m_Recordset.SetAdoConnection(&m_Connection);

    if (IsTableExist(DS_TABLE_NAME))
      GetLayerTableInfos();
    else
      CreateDsTable();
  }

  return m_bOpen;
}

DataSource *AdoDataSource::Clone(void) const {
  AdoDataSource *pDs = new AdoDataSource();
  pDs->SetInfo(m_dsInfo);

  return (DataSource *)pDs;
}

bool AdoDataSource::Close() {
  m_vLayerInfos.clear();
  m_Connection.Close();
  m_Recordset.Close();

  return true;
}

//////////////////////////////////////////////////////////////////////////

VectorLayer *AdoDataSource::CreateVectorLayer(const char *pszName,
                                              fRect &lyrRect,
                                              FeatureType ftType) {
  AdoVecLayer *pLayer = NULL;

  switch (ftType) {
    case FtDot: {
      pLayer = new DotFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(ftType);
      pLayer->SetLayerName(pszName);
      pLayer->SetLayerRect(lyrRect);

      if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);
    } break;
    case FtChildImage: {
      pLayer = new CldImgFclsDBLayer(this);
      pLayer->SetLayerFeatureType(ftType);
      pLayer->SetLayerName(pszName);
      pLayer->SetLayerRect(lyrRect);

      if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);
    } break;
    case FtAnno: {
      pLayer = new AnnoFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(ftType);
      pLayer->SetLayerName(pszName);
      pLayer->SetLayerRect(lyrRect);

      if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);
    } break;

    case FtCurve: {
      pLayer = new CurveFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(ftType);
      pLayer->SetLayerName(pszName);
      pLayer->SetLayerRect(lyrRect);

      if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);
    } break;

    case FtSurface: {
      pLayer = new SurfaceFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(ftType);
      pLayer->SetLayerName(pszName);
      pLayer->SetLayerRect(lyrRect);

      if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);
    } break;
    case FtGrid: {
      pLayer = new GridFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(ftType);
      pLayer->SetLayerName(pszName);
      pLayer->SetLayerRect(lyrRect);

      if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);
    } break;
    case FtTin: {
      pLayer = new TinFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(ftType);
      pLayer->SetLayerName(pszName);
      pLayer->SetLayerRect(lyrRect);

      if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);
    } break;
    default:
      break;
  }

  return (VectorLayer *)pLayer;
}

VectorLayer *AdoDataSource::OpenVectorLayer(const char *szName) {
  LayerInfo info;

  if (!GetLayerTableInfoByLayerName(info, szName)) return NULL;

  AdoVecLayer *pLayer = NULL;

  switch (info.unFeatureType) {
    case FtDot: {
      pLayer = new DotFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(FtDot);
      pLayer->SetLayerName(info.szName);

      if (!pLayer->Open(info.szArchiveName)) {
        SMT_SAFE_DELETE(pLayer);
      } else
        pLayer->Fetch();
    } break;
    case FtChildImage: {
      pLayer = new CldImgFclsDBLayer(this);
      pLayer->SetLayerFeatureType(FtChildImage);
      pLayer->SetLayerName(info.szName);

      if (!pLayer->Open(info.szArchiveName)) {
        SMT_SAFE_DELETE(pLayer);
      } else
        pLayer->Fetch();
    } break;
    case FtAnno: {
      pLayer = new AnnoFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(FtAnno);
      pLayer->SetLayerName(info.szName);

      if (!pLayer->Open(info.szArchiveName)) {
        SMT_SAFE_DELETE(pLayer);
      } else
        pLayer->Fetch();
    } break;

    case FtCurve: {
      pLayer = new CurveFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(FtCurve);
      pLayer->SetLayerName(info.szName);

      if (!pLayer->Open(info.szArchiveName)) {
        SMT_SAFE_DELETE(pLayer);
      } else
        pLayer->Fetch();
    } break;
    case FtGrid: {
      pLayer = new GridFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(FtGrid);
      pLayer->SetLayerName(info.szName);

      if (!pLayer->Open(info.szArchiveName)) {
        SMT_SAFE_DELETE(pLayer);
      } else
        pLayer->Fetch();
    } break;
    case FtTin: {
      pLayer = new TinFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(FtTin);
      pLayer->SetLayerName(info.szName);

      if (!pLayer->Open(info.szArchiveName)) {
        SMT_SAFE_DELETE(pLayer);
      } else
        pLayer->Fetch();
    } break;
    case FtSurface: {
      pLayer = new SurfaceFclsAdoLayer(this);
      pLayer->SetLayerFeatureType(FtSurface);
      pLayer->SetLayerName(info.szName);

      if (!pLayer->Open(info.szArchiveName)) {
        SMT_SAFE_DELETE(pLayer);
      } else
        pLayer->Fetch();
    } break;

    default:
      break;
  }

  return (VectorLayer *)pLayer;
}

bool AdoDataSource::DeleteVectorLayer(const char *szName) {
  char szSqlBuf[SQL_STRING_BUF_LENGTH];
  sprintf(szSqlBuf, "select * from %s where %s = '%s';", DS_TABLE_NAME,
          DS_FLD_LYR_NAME, szName);

  if (!m_Recordset.Open(szSqlBuf)) return false;

  LayerInfo info;
  GetLayerTableInfo(info);

  if (!m_Recordset.Delete()) return false;

  DropTable(info.szArchiveName);

  return true;
}

RasterLayer *AdoDataSource::CreateRasterLayer(const char *szName,
                                              fRect &lyrRect, long lImageCode) {
  AdoRasLayer *pLayer = NULL;
  pLayer = new AdoRasLayer(this);
  pLayer->SetLayerName(szName);
  pLayer->SetLayerRect(lyrRect);

  if (!pLayer->Create()) SMT_SAFE_DELETE(pLayer);

  return (RasterLayer *)pLayer;
}

RasterLayer *AdoDataSource::OpenRasterLayer(const char *szName) {
  LayerInfo info;
  if (!GetLayerTableInfoByLayerName(info, szName)) return NULL;

  AdoRasLayer *pLayer = NULL;

  pLayer = new AdoRasLayer(this);
  pLayer->SetLayerName(info.szName);

  if (!pLayer->Open(info.szArchiveName)) {
    SMT_SAFE_DELETE(pLayer);
  } else
    pLayer->Fetch();

  return (RasterLayer *)pLayer;
}

bool AdoDataSource::DeleteRasterLayer(const char *szName) {
  char szSqlBuf[SQL_STRING_BUF_LENGTH];
  sprintf(szSqlBuf, "select * from %s where %s = '%s';", DS_TABLE_NAME,
          DS_FLD_LYR_NAME, szName);

  if (!m_Recordset.Open(szSqlBuf)) return false;

  LayerInfo info;
  GetLayerTableInfo(info);

  if (!m_Recordset.Delete()) return false;

  DropTable(info.szArchiveName);

  return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdoDataSource::CreateDsTable(void) {
  bool bRet = TRUE;
  Attribute smtAtt;
  Field smtFld;

  smtFld.SetName(DS_FLD_LYR_NAME);
  smtFld.SetType(VarType::String);
  smtAtt.AddField(smtFld);

  smtFld.SetName(DS_FLD_LYR_TABLENAME);
  smtFld.SetType(VarType::String);
  smtAtt.AddField(smtFld);

  smtFld.SetName(DS_FLD_LYR_TYPE);
  smtFld.SetType(VarType::Integer);
  smtAtt.AddField(smtFld);

  smtFld.SetName(DS_FLD_LYR_SPIDXTYPE);
  smtFld.SetType(VarType::Integer);
  smtAtt.AddField(smtFld);

  smtFld.SetName(DS_FLD_LYR_XMIN);
  smtFld.SetType(VarType::Real);
  smtAtt.AddField(smtFld);

  smtFld.SetName(DS_FLD_LYR_YMIN);
  smtFld.SetType(VarType::Real);
  smtAtt.AddField(smtFld);

  smtFld.SetName(DS_FLD_LYR_XMAX);
  smtFld.SetType(VarType::Real);
  smtAtt.AddField(smtFld);

  smtFld.SetName(DS_FLD_LYR_YMAX);
  smtFld.SetType(VarType::Real);
  smtAtt.AddField(smtFld);

  bRet = CreateTable(DS_TABLE_NAME, &smtAtt);

  return bRet;
}

bool AdoDataSource::GetLayerTableInfos(void) {
  if (!OpenTable(DS_TABLE_NAME, &m_Recordset)) return false;

  m_vLayerInfos.clear();

  m_Recordset.MoveFirst();
  while (!m_Recordset.IsEOF()) {
    LayerInfo info;
    GetLayerTableInfo(info);
    m_vLayerInfos.push_back(info);
    m_Recordset.MoveNext();
  }

  return true;
}

bool AdoDataSource::GetLayerTableInfoByLayerName(LayerInfo &info,
                                                 const char *szName) {
  char szSqlBuf[SQL_STRING_BUF_LENGTH];
  sprintf_s(szSqlBuf, SQL_STRING_BUF_LENGTH,
            "select * from %s where %s = '%s';", DS_TABLE_NAME, DS_FLD_LYR_NAME,
            szName);

  if (!m_Recordset.Open(szSqlBuf) || m_Recordset.GetRecordCount() < 1)
    return false;

  m_Recordset.MoveFirst();
  if (!m_Recordset.IsEOF()) GetLayerTableInfo(info);

  return true;
}

void AdoDataSource::GetLayerTableInfo(LayerInfo &info) {
  char szStrBuf[TEMP_BUFFER_SIZE];
  int ntype;
  float fTemp;
  m_Recordset.GetCollect(DS_FLD_LYR_NAME, szStrBuf, TEMP_BUFFER_SIZE);
  strcpy(info.szName, szStrBuf);

  m_Recordset.GetCollect(DS_FLD_LYR_TABLENAME, szStrBuf, TEMP_BUFFER_SIZE);
  strcpy(info.szArchiveName, szStrBuf);

  m_Recordset.GetCollect(DS_FLD_LYR_TYPE, ntype);
  info.unFeatureType = ntype;

  m_Recordset.GetCollect(DS_FLD_LYR_SPIDXTYPE, ntype);
  info.unSIType = ntype;

  m_Recordset.GetCollect(DS_FLD_LYR_XMIN, fTemp);
  info.lyrRect.lb.x = fTemp;

  m_Recordset.GetCollect(DS_FLD_LYR_YMIN, fTemp);
  info.lyrRect.lb.y = fTemp;

  m_Recordset.GetCollect(DS_FLD_LYR_XMAX, fTemp);
  info.lyrRect.rt.x = fTemp;

  m_Recordset.GetCollect(DS_FLD_LYR_YMAX, fTemp);
  info.lyrRect.rt.y = fTemp;
}

bool AdoDataSource::AppendLayerTableInfo(LayerInfo &info) {
  if (!OpenTable(DS_TABLE_NAME, &m_Recordset)) return false;

  m_Recordset.AddNew();

  m_Recordset.PutCollect(DS_FLD_LYR_NAME, _variant_t(info.szName));
  m_Recordset.PutCollect(DS_FLD_LYR_TABLENAME, _variant_t(info.szArchiveName));
  m_Recordset.PutCollect(DS_FLD_LYR_TYPE, _variant_t((long)info.unFeatureType));
  m_Recordset.PutCollect(DS_FLD_LYR_SPIDXTYPE, _variant_t((long)info.unSIType));
  m_Recordset.PutCollect(DS_FLD_LYR_XMIN, _variant_t(info.lyrRect.lb.x));
  m_Recordset.PutCollect(DS_FLD_LYR_YMIN, _variant_t(info.lyrRect.lb.y));
  m_Recordset.PutCollect(DS_FLD_LYR_XMAX, _variant_t(info.lyrRect.rt.x));
  m_Recordset.PutCollect(DS_FLD_LYR_YMAX, _variant_t(info.lyrRect.rt.x));

  if (!m_Recordset.Update()) {
    return false;
  }

  return true;
}

bool AdoDataSource::DeleteLayerTableInfo(LayerInfo &info) {
  if (!OpenTable(DS_TABLE_NAME, &m_Recordset)) return false;
  return true;
}

bool AdoDataSource::UpdateLayerTableInfo(LayerInfo &info) {
  if (!OpenTable(DS_TABLE_NAME, &m_Recordset)) return false;

  return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdoDataSource::CreateLayerTable(LayerInfo &info, Attribute *pAtt) {
  bool bRet = false;
  if (CreateTable(info.szArchiveName, pAtt) && AppendLayerTableInfo(info))
    bRet = true;

  return bRet;
}

//////////////////////////////////////////////////////////////////////////
bool AdoDataSource::CreateTable(const char *szTableName, Attribute *pAtt) {
  bool bExist = IsTableExist(szTableName);
  if (bExist) return true;

  string sql = "create table ";
  sql += szTableName;
  sql += "( ";

  int nCount = pAtt->GetFieldCount();

  if (nCount < 1) return false;

  string str;
  Field *pFld = pAtt->GetFieldPtr(0);
  CetTableFldCreatingString(str, *pFld);

  str += " primary key ";

  for (int i = 1; i < nCount; i++) {
    str += ",";
    Field *pFld = pAtt->GetFieldPtr(i);
    CetTableFldCreatingString(str, *pFld);
  }

  sql += str;
  sql += ");";
  return m_Connection.Execute(sql.c_str());
}

void AdoDataSource::CetTableFldCreatingString(string &str, Field &smtFld) {
  varType type = smtFld.GetType();
  switch (type) {
    case Integer: {
      str += smtFld.GetName();
      str += " int ";
    } break;

    case Real: {
      str += smtFld.GetName();
      str += " float ";
    } break;

    case Bool: {
      str += smtFld.GetName();
      str += " char(1) ";
    } break;

    case Byte: {
      str += smtFld.GetName();
      str += " char(1) ";
    } break;

    case String: {
      str += smtFld.GetName();
      str += " varchar(200) ";
    } break;

    case IntegerList: {
      str += smtFld.GetName();
      str += " image ";
    } break;

    case RealList: {
      str += smtFld.GetName();
      str += " image ";
    } break;

    case StringList: {
      str += smtFld.GetName();
      str += " image ";
    } break;

    case Binary: {
      str += smtFld.GetName();
      str += " image ";
    } break;

    case Date: {
      str += smtFld.GetName();
      str += " image ";
    } break;
    case Time: {
      str += smtFld.GetName();
      str += " image ";
    } break;

    case DateTime: {
      str += smtFld.GetName();
      str += " image ";
    } break;

    case Unknown:  //未知类型
      break;

    default:
      break;
  }
}

bool AdoDataSource::OpenTable(const char *szTableName, AdoRecordSet *pRecordset,
                              CursorTypeEnum CursorType,
                              LockTypeEnum LockType) {
  BOOL bRet = TRUE;
  if (pRecordset) {
    bRet = pRecordset->Open(szTableName, adCmdTable, CursorType, LockType);
  }
  return ((bRet == TRUE) ? true : false);
}

bool AdoDataSource::CloseTable(const char *szTableName,
                               AdoRecordSet *pRecordset) {
  if (pRecordset) {
    pRecordset->Close();
  }
  return true;
}

bool AdoDataSource::DropTable(const char *szTableName) {
  BOOL bExist = IsTableExist(szTableName);
  if (!bExist)
    return true;
  else {
    char sql[TEMP_BUFFER_SIZE];
    sprintf_s(sql, TEMP_BUFFER_SIZE, "drop table %s;", szTableName);

    return m_Connection.Execute(sql);
  }
}

bool AdoDataSource::DropTrigger(const char *szTriggleName) {
  char sql[TEMP_BUFFER_SIZE];
  sprintf_s(sql, TEMP_BUFFER_SIZE, "drop triggle %s;", szTriggleName);

  return m_Connection.Execute(sql);
}

bool AdoDataSource::ClearTable(const char *szTableName) {
  BOOL bExist = IsTableExist(szTableName);
  if (!bExist)
    return true;
  else {
    char sql[TEMP_BUFFER_SIZE];
    sprintf_s(sql, TEMP_BUFFER_SIZE, "delete from %s;", szTableName);

    return m_Connection.Execute(sql);
  }
}

bool AdoDataSource::IsTableExist(const char *szTableName) {
  if (m_Recordset.IsOpen()) m_Recordset.Close();

  m_Recordset = m_Connection.OpenSchema(adSchemaTables);  //枚举表的名称处理

  m_Recordset.MoveFirst();
  while (!(m_Recordset.IsEOF())) {
    char strTableName[TEMP_BUFFER_SIZE];

    _bstr_t table_name =
        m_Recordset.GetFields()->GetItem("TABLE_NAME")->Value;  //获取表的名称

    sprintf_s(strTableName, TEMP_BUFFER_SIZE, "%s", (LPCSTR)table_name);

    if (!strcmp(strTableName, szTableName)) {
      return TRUE;
    }

    m_Recordset.MoveNext();
  }

  return FALSE;
}
}  // namespace _SDEAdo