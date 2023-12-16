#include "sde_curvefcls_adoveclayer.h"

#include "geo_api.h"
#include "smt_api.h"


using namespace core;
using namespace core;
using namespace _GIS;

namespace _SDEAdo {
CurveFclsAdoLayer::CurveFclsAdoLayer(DataSource *pOwnerDs)
    : AdoVecLayer(pOwnerDs) {
  ;
}

CurveFclsAdoLayer::~CurveFclsAdoLayer(void) {}

//////////////////////////////////////////////////////////////////////////
bool CurveFclsAdoLayer::Create(void) {
  bool bRet = true;

  SetDefaultAttFields();
  SetDefaultGeomFields();
  SetTableFields();

  sprintf(m_szLayerTableName, "%s_CurveFcls", m_szLayerName);
  LayerInfo info;
  sprintf(info.szArchiveName, m_szLayerTableName);
  sprintf(info.szName, m_szLayerName);
  info.unFeatureType = FtCurve;

  return ((AdoDataSource *)m_pOwnerDs)
      ->CreateLayerTable(info, m_pTableFieldCollect);
}

//////////////////////////////////////////////////////////////////////////
void CurveFclsAdoLayer::SetDefaultAttFields(void) {
  SMT_SAFE_DELETE(m_pAtt);
  m_pAtt = new Attribute();

  Field smtFld;

  smtFld.SetName("ID");
  smtFld.SetType(VarType::Integer);
  m_pAtt->AddField(smtFld);

  smtFld.SetName("length");
  smtFld.SetType(VarType::Real);
  m_pAtt->AddField(smtFld);
}

void CurveFclsAdoLayer::SetDefaultGeomFields(void) {
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

  smtFld.SetName("line_anlytype");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_pointnum");
  smtFld.SetType(VarType::Integer);
  m_pGeomFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_points");
  smtFld.SetType(VarType::Binary);
  m_pGeomFieldCollect->AddField(smtFld);
}

void CurveFclsAdoLayer::SetTableFields(void) {
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

  smtFld.SetName("grid_id");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("line_anlytype");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_pointnum");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("geom_points");
  smtFld.SetType(VarType::Binary);
  m_pTableFieldCollect->AddField(smtFld);
  //////////////////////////////////////////////////////////////////////////
  smtFld.SetName("style");
  smtFld.SetType(VarType::Binary);
  m_pTableFieldCollect->AddField(smtFld);
  //////////////////////////////////////////////////////////////////////////
  smtFld.SetName("length");
  smtFld.SetType(VarType::Real);
  m_pTableFieldCollect->AddField(smtFld);
}

//////////////////////////////////////////////////////////////////////////
void CurveFclsAdoLayer::GetFeature(Feature *pFeature) {
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
  pFeature->SetFeatureType(FeatureType::FtCurve);
  pFeature->SetStyleDirectly(pStyle);
  pFeature->SetGeometryDirectly(pGeom);
  pFeature->SetFieldValue(pFeature->GetFieldIndexByName("length"),
                          ((Curve *)pGeom)->GetLength());
}

//////////////////////////////////////////////////////////////////////////
long CurveFclsAdoLayer::AppendFeature(const Feature *pFeature, bool bclone) {
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
  m_Recordset.PutCollect(
      "length", _variant_t(((Curve *)pFeature->GetGeometryRef())->GetLength()));

  if (lRet != SMT_ERR_DB_OPER && !m_Recordset.Update()) {
    SMT_SAFE_DELETE(pFeature);
    return SMT_ERR_DB_OPER;
  }

  // mem
  m_pMemLayer->AppendFeature(pFeature, bclone);
  CalEnvelope();

  return lRet;
}

long CurveFclsAdoLayer::AppendFeatureBatch(const Feature *pFeature,
                                           bool bclone) {
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
  m_Recordset.PutCollect(
      "length", _variant_t(((Curve *)pFeature->GetGeometryRef())->GetLength()));

  if (lRet != SMT_ERR_DB_OPER && !m_Recordset.Update()) {
    SMT_SAFE_DELETE(pFeature);
    return SMT_ERR_DB_OPER;
  }

  // mem
  m_pMemLayer->AppendFeature(pFeature, bclone);
  CalEnvelope();

  return lRet;
}

long CurveFclsAdoLayer::UpdateFeature(const Feature *pFeature) {
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

        //////////////////////////////////////////////////////////////////////////
        m_Recordset.PutCollect(
            "length",
            _variant_t(((Curve *)pFeature->GetGeometryRef())->GetLength()));

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
long CurveFclsAdoLayer::AppendStyle(const Style *pStyle) {
  char *pBuf = (char *)pStyle;
  int nLen = sizeof(Style);

  VARIANT varBLOB;
  SAFEARRAY *pSa;
  SAFEARRAYBOUND rgSaBound[1];
  if (pBuf) {
    rgSaBound[0].lLbound = 0;
    rgSaBound[0].cElements = nLen;
    pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
    for (long i = 0; i < (long)nLen; i++) SafeArrayPutElement(pSa, &i, pBuf++);

    varBLOB.vt = VT_ARRAY | VT_UI1;
    varBLOB.parray = pSa;
    m_Recordset.GetField("style")->AppendChunk(varBLOB);
  }

  SafeArrayDestroy(pSa);
  return SMT_ERR_NONE;
}

long CurveFclsAdoLayer::AppendGeom(const Geometry *pGeom) {
  long lRet = SMT_ERR_NONE;
  GeometryType type = pGeom->GetGeometryType();

  switch (type) {
    case GTLineString:
      lRet = AppendLineString((LineString *)pGeom);
      break;

    case GTLinearRing:
      lRet = AppendLinearRing((LinearRing *)pGeom);
      break;

    case GTSpline:
      lRet = AppendSpline((Spline *)pGeom);
      break;

    case GTArc:
      lRet = AppendArc((Arc *)pGeom);
      break;

    default:
      break;
  }

  return lRet;
}

//////////////////////////////////////////////////////////////////////////
long CurveFclsAdoLayer::GetStyle(Style *&pStyle) {
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

long CurveFclsAdoLayer::GetGeom(Geometry *&pGeom) {
  long lRet = SMT_ERR_NONE;
  int nGeomType = 0;
  m_Recordset.GetCollect("geom_type", nGeomType);

  switch (nGeomType) {
    case GTLineString:
      lRet = GetLineString(pGeom);
      break;

    case GTLinearRing:
      lRet = GetLinearRing(pGeom);
      break;

    case GTSpline:
      lRet = GetSpline(pGeom);
      break;

    case GTArc:
      lRet = GetArc(pGeom);
      break;

    default:
      break;
  }

  return lRet;
}

//////////////////////////////////////////////////////////////////////////
long CurveFclsAdoLayer::AppendLineString(const LineString *pLineString) {
  int nPoints = pLineString->GetNumPoints();

  if (nPoints < 0) return SMT_ERR_DB_OPER;

  RawPoint *pRawPoints = new RawPoint[nPoints];
  pLineString->GetPoints(pRawPoints);
  //////////////////////////////////////////////////////////////////////////
  m_Recordset.PutCollect("geom_type", _variant_t(GTLineString));
  m_Recordset.PutCollect("geom_pointnum", _variant_t(nPoints));
  m_Recordset.PutCollect("line_anlytype", _variant_t(-1));

  //////////////////////////////////////////////////////////////////////////

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

    /* pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
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

long CurveFclsAdoLayer::AppendSpline(const Spline *pSpline) {
  int nPoints = pSpline->GetNumPoints();

  if (nPoints < 0) return SMT_ERR_DB_OPER;

  RawPoint *pRawPoints = new RawPoint[nPoints];
  pSpline->GetPoints(pRawPoints);
  //////////////////////////////////////////////////////////////////////////
  m_Recordset.PutCollect("geom_type", _variant_t(GTSpline));
  m_Recordset.PutCollect("geom_pointnum", _variant_t(nPoints));
  m_Recordset.PutCollect("line_anlytype",
                         _variant_t(pSpline->GetAnalyticType()));

  //////////////////////////////////////////////////////////////////////////

  char *pBuf = (char *)pRawPoints;
  int nLen = sizeof(RawPoint) * nPoints;

  VARIANT varBLOB;
  SAFEARRAY *pSa;
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
    m_Recordset.GetField("geom_points")->AppendChunk(varBLOB);
  }

  SafeArrayDestroy(pSa);
  SMT_SAFE_DELETE_A(pRawPoints);

  return SMT_ERR_NONE;
}

long CurveFclsAdoLayer::AppendLinearRing(const LinearRing *pLinearRing) {
  int nPoints = pLinearRing->GetNumPoints();

  if (nPoints < 0) return SMT_ERR_DB_OPER;

  //////////////////////////////////////////////////////////////////////////
  m_Recordset.PutCollect("geom_type", _variant_t(GTLinearRing));
  m_Recordset.PutCollect("geom_pointnum", _variant_t(nPoints));
  m_Recordset.PutCollect("line_anlytype",
                         _variant_t(pLinearRing->GetAnalyticType()));

  //////////////////////////////////////////////////////////////////////////
  RawPoint *pRawPoints = new RawPoint[nPoints];
  pLinearRing->GetPoints(pRawPoints);

  char *pBuf = (char *)pRawPoints;
  int nLen = sizeof(RawPoint) * nPoints;

  VARIANT varBLOB;
  SAFEARRAY *pSa;
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

long CurveFclsAdoLayer::AppendArc(const Arc *pArc) {
  //////////////////////////////////////////////////////////////////////////
  m_Recordset.PutCollect("geom_type", _variant_t(GTArc));
  m_Recordset.PutCollect("geom_pointnum", _variant_t(3));
  m_Recordset.PutCollect("line_anlytype", _variant_t(-1));

  //////////////////////////////////////////////////////////////////////////

  Point oPoint[3];
  RawPoint *pRawPoints = new RawPoint[3];
  pArc->GetCenterPoint(&oPoint[0]);
  pArc->StartPoint(&oPoint[1]);
  pArc->EndPoint(&oPoint[2]);

  pRawPoints[0].x = oPoint[0].GetX();
  pRawPoints[0].y = oPoint[0].GetY();

  pRawPoints[1].x = oPoint[1].GetX();
  pRawPoints[1].y = oPoint[1].GetY();

  pRawPoints[2].x = oPoint[2].GetX();
  pRawPoints[2].y = oPoint[2].GetY();

  char *pBuf = (char *)pRawPoints;
  int nLen = sizeof(RawPoint) * 3;

  VARIANT varBLOB;
  SAFEARRAY *pSa;
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

    SafeArrayDestroy(pSa);
  }

  SMT_SAFE_DELETE_A(pRawPoints);

  return SMT_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
long CurveFclsAdoLayer::GetLineString(Geometry *&pGeom) {
  LineString *pLineString = new LineString();

  int nPoints = 0;
  m_Recordset.GetCollect("geom_pointnum", nPoints);

  pLineString->SetNumPoints(nPoints);

  long lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      RawPoint *pRawPointBuf = NULL;
      SafeArrayAccessData(varBLOB.parray, (void **)&pRawPointBuf);
      pLineString->SetPoints(nPoints, pRawPointBuf);
      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  pGeom = pLineString;

  return SMT_ERR_NONE;
}

long CurveFclsAdoLayer::GetSpline(Geometry *&pGeom) {
  Spline *pSpline = new Spline();

  int nAnlyType = 0;
  m_Recordset.GetCollect("line_anlytype", nAnlyType);

  int nPoints = 0;
  m_Recordset.GetCollect("geom_pointnum", nPoints);

  pSpline->SetNumPoints(nPoints);
  pSpline->SetAnalyticType(nAnlyType);

  long lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      RawPoint *pRawPointBuf = NULL;
      SafeArrayAccessData(varBLOB.parray, (void **)&pRawPointBuf);
      pSpline->SetPoints(nPoints, pRawPointBuf);
      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  pSpline->CalcAnalyticPoints();

  pGeom = pSpline;

  return SMT_ERR_NONE;
}

long CurveFclsAdoLayer::GetLinearRing(Geometry *&pGeom) {
  LinearRing *pLinearRing = new LinearRing();

  int nPoints = 0;
  m_Recordset.GetCollect("geom_pointnum", nPoints);

  pLinearRing->SetNumPoints(nPoints);

  long lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      RawPoint *pRawPointBuf = NULL;
      SafeArrayAccessData(varBLOB.parray, (void **)&pRawPointBuf);
      pLinearRing->SetPoints(nPoints, pRawPointBuf);
      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  pLinearRing->CloseRings();

  pGeom = pLinearRing;

  return SMT_ERR_NONE;
}

long CurveFclsAdoLayer::GetArc(Geometry *&pGeom) {
  Arc *pArc = new Arc();
  RawPoint *pRawPoints = new RawPoint[3];

  int nPoints = 0;
  m_Recordset.GetCollect("geom_pointnum", nPoints);

  if (nPoints != 3) return SMT_ERR_DB_OPER;

  long lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      char *pBuf = NULL;
      RawPoint *pRawPoints1 = pRawPoints;
      SafeArrayAccessData(varBLOB.parray, (void **)&pBuf);
      for (int index = 0; index < nPoints; index++) {
        memcpy(pRawPoints1, pBuf, sizeof(RawPoint));
        pRawPoints1++;
        pBuf += sizeof(RawPoint);
      }

      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  pArc->SetCenterPoint(pRawPoints[0]);
  pArc->SetStartPoint(pRawPoints[1]);
  pArc->SetEndPoint(pRawPoints[2]);

  float r = Distance(pRawPoints[0].x, pRawPoints[0].y, pRawPoints[1].x,
                     pRawPoints[1].y);
  pArc->SetRadius(r);

  SMT_SAFE_DELETE_A(pRawPoints);
  pGeom = pArc;

  return SMT_ERR_NONE;
}
}  // namespace _SDEAdo