#include "sde_tinfcls_adoveclayer.h"

#include "smt_api.h"


using namespace core;
using namespace core;
using namespace _GIS;

namespace _SDEAdo {
TinFclsAdoLayer::TinFclsAdoLayer(DataSource *pOwnerDs)
    : AdoVecLayer(pOwnerDs) {}

TinFclsAdoLayer::~TinFclsAdoLayer(void) {}

//////////////////////////////////////////////////////////////////////////
bool TinFclsAdoLayer::Create(void) {
  bool bRet = true;

  SetDefaultAttFields();
  SetDefaultGeomFields();
  SetTableFields();

  //////////////////////////////////////////////////////////////////////////
  sprintf(m_szLayerTableName, "%s_TinFcls", m_szLayerName);
  LayerInfo info;
  sprintf(info.szArchiveName, m_szLayerTableName);
  sprintf(info.szName, m_szLayerName);
  info.unFeatureType = FtTin;

  return ((AdoDataSource *)m_pOwnerDs)
      ->CreateLayerTable(info, m_pTableFieldCollect);
}

//////////////////////////////////////////////////////////////////////////
void TinFclsAdoLayer::SetDefaultAttFields(void) {
  SMT_SAFE_DELETE(m_pAtt);
  m_pAtt = new Attribute();

  Field smtFld;

  smtFld.SetName("ID");
  smtFld.SetType(VarType::Integer);
  m_pAtt->AddField(smtFld);

  smtFld.SetName("point_num");
  smtFld.SetType(VarType::Integer);
  m_pAtt->AddField(smtFld);

  smtFld.SetName("triangle_num");
  smtFld.SetType(VarType::Integer);
  m_pAtt->AddField(smtFld);
}

void TinFclsAdoLayer::SetDefaultGeomFields(void) {
  SMT_SAFE_DELETE(m_pGeomFieldCollect);
  m_pGeomFieldCollect = new FieldCollect();

  Field smtFld;

  // geom
  smtFld.SetName("geom_type");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

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

  smtFld.SetName("grid_id");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_pointnum");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_points");
  smtFld.SetType(VarType::Binary);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_trianglenum");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_triangles");
  smtFld.SetType(VarType::Binary);
  m_pGeomFieldCollect->AddField(smtFld);
}

void TinFclsAdoLayer::SetTableFields(void) {
  SMT_SAFE_DELETE(m_pTableFieldCollect);
  m_pTableFieldCollect = new FieldCollect();

  Field smtFld;

  smtFld.SetName("ID");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  //////////////////////////////////////////////////////////////////////////
  // geom
  smtFld.SetName("geom_type");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

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

  smtFld.SetName("geom_pointnum");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_points");
  smtFld.SetType(VarType::Binary);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_trianglenum");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_triangles");
  smtFld.SetType(VarType::Binary);
  m_pTableFieldCollect->AddField(smtFld);

  //////////////////////////////////////////////////////////////////////////

  smtFld.SetName("style");
  smtFld.SetType(VarType::Binary);
  m_pTableFieldCollect->AddField(smtFld);
}

void TinFclsAdoLayer::GetFeature(Feature *pFeature) {
  if (!pFeature) return;

  int nID = 0;
  int nGridID = 0;
  Style *pStyle = NULL;

  Envelope env;
  m_Recordset.GetCollect("ID", nID);
  m_Recordset.GetCollect("mbr_xmin", env.MinX);
  m_Recordset.GetCollect("mbr_ymin", env.MinY);
  m_Recordset.GetCollect("mbr_xmax", env.MaxX);
  m_Recordset.GetCollect("mbr_ymax", env.MaxY);
  m_Recordset.GetCollect("grid_id", nGridID);
  GetStyle(pStyle);

  //////////////////////////////////////////////////////////////////////////
  Geometry *pGeom = NULL;
  GetGeom(pGeom);
  //////////////////////////////////////////////////////////////////////////
  pFeature->SetID(nID);
  pFeature->SetFeatureType(FeatureType::FtTin);
  pFeature->SetStyleDirectly(pStyle);
  pFeature->SetGeometryDirectly(pGeom);
}

//////////////////////////////////////////////////////////////////////////
long TinFclsAdoLayer::AppendFeature(const Feature *pFeature, bool bclone) {
  long lRet = SMT_ERR_NONE;
  //////////////////////////////////////////////////////////////////////////
  // db
  if (!IsOpen())
    if (!Open(m_szLayerTableName)) return SMT_ERR_DB_OPER;

  int nGridID = 0;
  Envelope env;
  const Geometry *pGeom = pFeature->GetGeometryRef();
  const Attribute *pAtt = pFeature->GetAttributeRef();
  const Style *pStyle = pFeature->GetStyle();

  pGeom->GetEnvelope(&env);

  m_Recordset.AddNew();
  m_Recordset.PutCollect("ID", _variant_t(pFeature->GetID()));
  m_Recordset.PutCollect("mbr_xmin", _variant_t(env.MinX));
  m_Recordset.PutCollect("mbr_ymin", _variant_t(env.MinY));
  m_Recordset.PutCollect("mbr_xmax", _variant_t(env.MaxX));
  m_Recordset.PutCollect("mbr_ymax", _variant_t(env.MaxY));
  m_Recordset.PutCollect("grid_id", _variant_t(nGridID));

  //////////////////////////////////////////////////////////////////////////
  lRet = AppendStyle(pStyle);

  //////////////////////////////////////////////////////////////////////////
  lRet = AppendGeom(pGeom);

  //////////////////////////////////////////////////////////////////////////
  if (lRet != SMT_ERR_DB_OPER && !m_Recordset.Update()) {
    SMT_SAFE_DELETE(pFeature);
    return SMT_ERR_DB_OPER;
  }

  // mem
  m_pMemLayer->AppendFeature(pFeature, bclone);

  CalEnvelope();

  return lRet;
}

long TinFclsAdoLayer::AppendFeatureBatch(const Feature *pFeature, bool bclone) {
  long lRet = SMT_ERR_NONE;
  //////////////////////////////////////////////////////////////////////////
  // db
  if (!IsOpen() && !Open(m_szLayerTableName)) return SMT_ERR_DB_OPER;

  int nGridID = 0;
  Envelope env;

  const Geometry *pGeom = pFeature->GetGeometryRef();
  const Attribute *pAtt = pFeature->GetAttributeRef();
  const Style *pStyle = pFeature->GetStyle();

  pGeom->GetEnvelope(&env);

  m_Recordset.AddNew();
  m_Recordset.PutCollect("ID", _variant_t(pFeature->GetID()));
  m_Recordset.PutCollect("mbr_xmin", _variant_t(env.MinX));
  m_Recordset.PutCollect("mbr_ymin", _variant_t(env.MinY));
  m_Recordset.PutCollect("mbr_xmax", _variant_t(env.MaxX));
  m_Recordset.PutCollect("mbr_ymax", _variant_t(env.MaxY));
  m_Recordset.PutCollect("grid_id", _variant_t(nGridID));

  //////////////////////////////////////////////////////////////////////////
  lRet = AppendStyle(pStyle);

  //////////////////////////////////////////////////////////////////////////
  lRet = AppendGeom(pGeom);

  // mem
  m_pMemLayer->AppendFeature(pFeature, bclone);
  CalEnvelope();

  return lRet;
}

long TinFclsAdoLayer::UpdateFeature(const Feature *pFeature) {
  //////////////////////////////////////////////////////////////////////////
  // db
  Feature *pFea = m_pMemLayer->GetFeature();
  if (NULL != pFea) {
    char szSQL[TEMP_BUFFER_SIZE];
    sprintf_s(szSQL, TEMP_BUFFER_SIZE, "select * from %s where ID = %d;",
              m_szLayerTableName, pFea->GetID());
    if (m_Recordset.Open(szSQL)) {
      m_Recordset.MoveFirst();
      if (m_Recordset.IsOpen() && !m_Recordset.IsEOF() &&
          m_Recordset.GetRecordCount() == 1) {
        int nGridID = 0;
        Envelope env;
        const Geometry *pGeom = pFeature->GetGeometryRef();
        const Attribute *pAtt = pFeature->GetAttributeRef();
        const Style *pStyle = pFeature->GetStyle();

        pGeom->GetEnvelope(&env);

        m_Recordset.AddNew();
        m_Recordset.PutCollect("ID", _variant_t(pFeature->GetID()));
        m_Recordset.PutCollect("mbr_xmin", _variant_t(env.MinX));
        m_Recordset.PutCollect("mbr_ymin", _variant_t(env.MinY));
        m_Recordset.PutCollect("mbr_xmax", _variant_t(env.MaxX));
        m_Recordset.PutCollect("mbr_ymax", _variant_t(env.MaxY));
        m_Recordset.PutCollect("grid_id", _variant_t(nGridID));

        //////////////////////////////////////////////////////////////////////////
        AppendStyle(pStyle);

        //////////////////////////////////////////////////////////////////////////
        AppendGeom(pGeom);

        if (!m_Recordset.Update()) return SMT_ERR_DB_OPER;

        // mem
        m_pMemLayer->UpdateFeature(pFeature);
        CalEnvelope();
      }
    }
  }

  return SMT_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
long TinFclsAdoLayer::AppendStyle(const Style *pStyle) {
  char *pBuf = (char *)pStyle;
  int nLen = sizeof(Style);

  VARIANT varBLOB;
  SAFEARRAY *pSa = NULL;
  SAFEARRAYBOUND rgSaBound[1];
  if (pBuf) {
    rgSaBound[0].lLbound = 0;
    rgSaBound[0].cElements = nLen;

    char *pSABuf = NULL;
    pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
    if (SafeArrayAccessData(pSa, (void **)&pSABuf) == NOERROR) {
      memcpy(pSABuf, pBuf, nLen);
    }
    SafeArrayUnaccessData(pSa);

    /*pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
    for (long i = 0; i < (long)nLen; i++)
            SafeArrayPutElement (pSa, &i, pBuf++); */

    varBLOB.vt = VT_ARRAY | VT_UI1;
    varBLOB.parray = pSa;
    m_Recordset.GetField("style")->AppendChunk(varBLOB);
  }

  SafeArrayDestroy(pSa);

  return SMT_ERR_NONE;
}

long TinFclsAdoLayer::AppendGeom(const Geometry *pGeom) {
  long lRet = SMT_ERR_NONE;
  GeometryType type = pGeom->GetGeometryType();

  switch (type) {
    case GTTin: {
      lRet = AppendTin((Tin *)pGeom);
    } break;
    default:
      break;
  }

  return lRet;
}

//////////////////////////////////////////////////////////////////////////
long TinFclsAdoLayer::GetStyle(Style *&pStyle) {
  pStyle = new Style();
  long lDataSize = m_Recordset.GetField("style")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("style")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      Style *pBuf = NULL;
      SafeArrayAccessData(varBLOB.parray, (void **)&pBuf);
      memcpy(pStyle, pBuf, sizeof(Style));
      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  return SMT_ERR_NONE;
}

long TinFclsAdoLayer::GetGeom(Geometry *&pGeom) {
  long lRet = SMT_ERR_NONE;
  int nMeshType = 0;
  m_Recordset.GetCollect("geom_type", nMeshType);

  switch (nMeshType) {
    case GTTin: {
      lRet = GetTin(pGeom);
    }
    default:
      break;
  }

  return lRet;
}

//////////////////////////////////////////////////////////////////////////
long TinFclsAdoLayer::AppendTin(const Tin *pTin) {
  m_Recordset.PutCollect("geom_type", _variant_t(GTTin));

  AppendTinTriangles(pTin);

  AppendTinNodes(pTin);

  return SMT_ERR_NONE;
}

long TinFclsAdoLayer::AppendTinTriangles(const Tin *pTin) {
  m_Recordset.PutCollect("geom_trianglenum",
                         _variant_t(pTin->GetTriangleCount()));

  int nTrianges = pTin->GetTriangleCount();

  if (nTrianges < 1) return SMT_ERR_DB_OPER;

  Triangle *pTriBuf = new Triangle[nTrianges];

  for (int i = 0; i < pTin->GetTriangleCount(); i++)
    pTriBuf[i] = pTin->GetTriangle(i);

  char *pBuf = (char *)pTriBuf;
  int nLen = sizeof(Triangle) * nTrianges;

  VARIANT varBLOB;
  SAFEARRAY *pSa = NULL;
  SAFEARRAYBOUND rgSaBound[1];
  if (pBuf) {
    rgSaBound[0].lLbound = 0;
    rgSaBound[0].cElements = nLen;

    char *pSABuf = NULL;
    pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
    if (SafeArrayAccessData(pSa, (void **)&pSABuf) == NOERROR) {
      memcpy(pSABuf, pBuf, nLen);
    }
    SafeArrayUnaccessData(pSa);

    /*pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
    for (long i = 0; i < (long)nLen; i++)
            SafeArrayPutElement (pSa, &i, pBuf++);*/

    varBLOB.vt = VT_ARRAY | VT_UI1;
    varBLOB.parray = pSa;
    m_Recordset.GetField("geom_triangles")->AppendChunk(varBLOB);
  }

  SafeArrayDestroy(pSa);

  SMT_SAFE_DELETE_A(pTriBuf);

  return SMT_ERR_NONE;
}

long TinFclsAdoLayer::AppendTinNodes(const Tin *pTin) {
  m_Recordset.PutCollect("geom_pointnum", _variant_t(pTin->GetPointCount()));

  int nPoints = pTin->GetPointCount();

  if (nPoints < 1) return SMT_ERR_DB_OPER;

  RawPoint *pRawPoints = new RawPoint[nPoints];

  for (int i = 0; i < pTin->GetPointCount(); i++) {
    Point oPoint = pTin->GetPoint(i);
    pRawPoints[i].x = oPoint.GetX();
    pRawPoints[i].y = oPoint.GetY();
  }

  char *pBuf = (char *)pRawPoints;
  int nLen = sizeof(RawPoint) * nPoints;

  VARIANT varBLOB;
  SAFEARRAY *pSa = NULL;
  SAFEARRAYBOUND rgSaBound[1];
  if (pBuf) {
    rgSaBound[0].lLbound = 0;
    rgSaBound[0].cElements = nLen;

    char *pSABuf = NULL;
    pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
    if (SafeArrayAccessData(pSa, (void **)&pSABuf) == NOERROR) {
      memcpy(pSABuf, pBuf, nLen);
    }
    SafeArrayUnaccessData(pSa);

    /*	pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
            for (long i = 0; i < (long)nLen; i++)
                    SafeArrayPutElement (pSa, &i, pBuf++); */

    varBLOB.vt = VT_ARRAY | VT_UI1;
    varBLOB.parray = pSa;
    m_Recordset.GetField("geom_points")->AppendChunk(varBLOB);
  }

  SafeArrayDestroy(pSa);

  SMT_SAFE_DELETE_A(pRawPoints);

  return SMT_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
long TinFclsAdoLayer::GetTin(Geometry *&pGeom) {
  Tin *pTin = new Tin();

  GetTinTriangles(pTin);
  GetTinNodes(pTin);

  pGeom = pTin;

  return SMT_ERR_NONE;
}

long TinFclsAdoLayer::GetTinTriangles(Tin *&pTin) {
  int nTriangles = 0;
  m_Recordset.GetCollect("geom_trianglenum", nTriangles);

  long lDataSize = m_Recordset.GetField("geom_triangles")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("geom_triangles")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      Triangle *pTriBuf = NULL;
      SafeArrayAccessData(varBLOB.parray, (void **)&pTriBuf);

      pTin->AddTriangleCollection(pTriBuf, nTriangles);

      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  return SMT_ERR_NONE;
}

long TinFclsAdoLayer::GetTinNodes(Tin *&pTin) {
  int nPoints = 0;
  m_Recordset.GetCollect("geom_pointnum", nPoints);

  long lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      RawPoint *pRawPointBuf = NULL;
      Point *pPointBuf = new Point[nPoints];
      SafeArrayAccessData(varBLOB.parray, (void **)&pRawPointBuf);

      for (int i = 0; i < nPoints; i++) {
        pPointBuf[i].SetX(pRawPointBuf[i].x);
        pPointBuf[i].SetY(pRawPointBuf[i].y);
      }

      SafeArrayUnaccessData(varBLOB.parray);

      //
      pTin->AddPointCollection(pPointBuf, nPoints);

      SMT_SAFE_DELETE_A(pPointBuf);
    }
  }

  return SMT_ERR_NONE;
}
}  // namespace _SDEAdo