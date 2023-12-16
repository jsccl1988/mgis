#include "sde_annofcls_adoveclayer.h"

using namespace core;
using namespace core;
using namespace _GIS;

namespace _SDEAdo {
AnnoFclsAdoLayer::AnnoFclsAdoLayer(DataSource *pOwnerDs)
    : AdoVecLayer(pOwnerDs) {}

AnnoFclsAdoLayer::~AnnoFclsAdoLayer(void) {}

//////////////////////////////////////////////////////////////////////////
bool AnnoFclsAdoLayer::Create(void) {
  bool bRet = true;

  //
  SetDefaultAttFields();
  SetDefaultGeomFields();
  SetTableFields();

  sprintf(m_szLayerTableName, "%s_AnnoFcls", m_szLayerName);
  LayerInfo info;
  sprintf(info.szArchiveName, m_szLayerTableName);
  sprintf(info.szName, m_szLayerName);
  info.unFeatureType = FtAnno;

  return ((AdoDataSource *)m_pOwnerDs)
      ->CreateLayerTable(info, m_pTableFieldCollect);
}

//////////////////////////////////////////////////////////////////////////
void AnnoFclsAdoLayer::SetDefaultAttFields(void) {
  SMT_SAFE_DELETE(m_pAtt);
  m_pAtt = new Attribute();

  Field smtFld;

  smtFld.SetName("ID");
  smtFld.SetType(VarType::Integer);
  m_pAtt->AddField(smtFld);

  smtFld.SetName("anno");
  smtFld.SetType(VarType::String);
  m_pAtt->AddField(smtFld);

  smtFld.SetName("color");
  smtFld.SetType(VarType::Integer);
  m_pAtt->AddField(smtFld);
}

void AnnoFclsAdoLayer::SetDefaultGeomFields(void) {
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
}

void AnnoFclsAdoLayer::SetTableFields(void) {
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
  smtFld.SetName("anno");
  smtFld.SetType(VarType::String);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("color");
  smtFld.SetType(VarType::Integer);
  m_pTableFieldCollect->AddField(smtFld);

  smtFld.SetName("angle");
  smtFld.SetType(VarType::Real);
  m_pTableFieldCollect->AddField(smtFld);
}

//////////////////////////////////////////////////////////////////////////
void AnnoFclsAdoLayer::GetFeature(Feature *pFeature) {
  if (!pFeature) return;

  int nID = 0;
  int nGridID = 0;
  int nClr = 0;
  double angle;
  char szAnnoBuf[TEMP_BUFFER_SIZE];
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
  // geom
  Geometry *pGeom = NULL;
  GetGeom(pGeom);
  //////////////////////////////////////////////////////////////////////////
  m_Recordset.GetCollect("anno", szAnnoBuf, TEMP_BUFFER_SIZE);
  m_Recordset.GetCollect("color", nClr);
  m_Recordset.GetCollect("angle", angle);
  //////////////////////////////////////////////////////////////////////////
  pFeature->SetID(nID);
  pFeature->SetFeatureType(FeatureType::FtAnno);
  pFeature->SetStyleDirectly(pStyle);
  pFeature->SetGeometryDirectly(pGeom);
  pFeature->SetFieldValue(pFeature->GetFieldIndexByName("anno"), szAnnoBuf);
  pFeature->SetFieldValue(pFeature->GetFieldIndexByName("color"), nClr);
  pFeature->SetFieldValue(pFeature->GetFieldIndexByName("angle"), angle);
}

//////////////////////////////////////////////////////////////////////////
long AnnoFclsAdoLayer::AppendFeature(const Feature *pFeature, bool bclone) {
  long lRet = SMT_ERR_NONE;
  //////////////////////////////////////////////////////////////////////////
  // db
  if (!IsOpen())
    if (!Open(m_szLayerTableName)) return SMT_ERR_DB_OPER;

  int nGridID = 0;
  char szAnnoBuf[TEMP_BUFFER_SIZE];
  int nClr = 0;
  double fAngle = 0.;

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
  const Field *pFld = NULL;

  pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("anno"));
  sprintf(szAnnoBuf, pFld->GetValueAsString());
  m_Recordset.PutCollect("anno", _variant_t(szAnnoBuf));

  pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("color"));
  nClr = pFld->GetValueAsInteger();
  m_Recordset.PutCollect("color", _variant_t(nClr));

  pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("angle"));
  fAngle = pFld->GetValueAsDouble();
  m_Recordset.PutCollect("angle", _variant_t(fAngle));

  if (!m_Recordset.Update()) {
    SMT_SAFE_DELETE(pFeature);
    return SMT_ERR_DB_OPER;
  }

  // mem
  m_pMemLayer->AppendFeature(pFeature, bclone);
  CalEnvelope();

  return lRet;
}

long AnnoFclsAdoLayer::AppendFeatureBatch(const Feature *pFeature,
                                          bool bclone) {
  long lRet = SMT_ERR_NONE;
  //////////////////////////////////////////////////////////////////////////
  // db
  if (!IsOpen())
    if (!Open(m_szLayerTableName)) return SMT_ERR_DB_OPER;

  Point *oPoint = (Point *)(pFeature->GetGeometryRef());
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
  char szAnnoBuf[TEMP_BUFFER_SIZE];
  int nClr = 0;
  double fAngle = 0.;
  const Field *pFld = NULL;
  pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("anno"));
  sprintf(szAnnoBuf, pFld->GetValueAsString());
  m_Recordset.PutCollect("anno", _variant_t(szAnnoBuf));

  pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("color"));
  nClr = pFld->GetValueAsInteger();
  m_Recordset.PutCollect("color", _variant_t(nClr));

  pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("angle"));
  fAngle = pFld->GetValueAsDouble();
  m_Recordset.PutCollect("angle", _variant_t(fAngle));

  //////////////////////////////////////////////////////////////////////////
  // mem
  m_pMemLayer->AppendFeature(pFeature, bclone);
  CalEnvelope();

  return lRet;
}

long AnnoFclsAdoLayer::UpdateFeature(const Feature *pFeature) {
  long lRet = SMT_ERR_NONE;
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
        char szAnnoBuf[TEMP_BUFFER_SIZE];
        int nClr = 0;
        double fAngle = 0.;

        Envelope env;
        const Geometry *pGeom = pFeature->GetGeometryRef();
        const Attribute *pAtt = pFeature->GetAttributeRef();
        const Style *pStyle = pFeature->GetStyle();

        pGeom->GetEnvelope(&env);

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
        const Field *pFld = NULL;

        pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("anno"));
        sprintf(szAnnoBuf, pFld->GetValueAsString());
        m_Recordset.PutCollect("anno", _variant_t(szAnnoBuf));

        pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("color"));
        nClr = pFld->GetValueAsInteger();
        m_Recordset.PutCollect("color", _variant_t(nClr));

        pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("angle"));
        fAngle = pFld->GetValueAsDouble();
        m_Recordset.PutCollect("angle", _variant_t(fAngle));

        if (!m_Recordset.Update()) {
          SMT_SAFE_DELETE(pFeature);
          return SMT_ERR_DB_OPER;
        }

        // mem
        m_pMemLayer->UpdateFeature(pFeature);
        CalEnvelope();
      }
    }
  }

  return lRet;
}

long AnnoFclsAdoLayer::AppendStyle(const Style *pStyle) {
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

long AnnoFclsAdoLayer::GetStyle(Style *&pStyle) {
  pStyle = new Style();
  long lDataSize = m_Recordset.GetField("style")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("style")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      Style *pBuf = NULL;
      SafeArrayAccessData(varBLOB.parray, (void **)&pBuf);
      int nSize = sizeof(Style);
      memcpy(pStyle, pBuf, sizeof(Style));
      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  return SMT_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
long AnnoFclsAdoLayer::AppendGeom(const Geometry *pGeom) {
  long lRet = SMT_ERR_NONE;
  GeometryType type = pGeom->GetGeometryType();

  switch (type) {
    case GTPoint:
      lRet = AppendPoint((Point *)pGeom);
      break;
    default:
      break;
  }

  return lRet;
}

long AnnoFclsAdoLayer::GetGeom(Geometry *&pGeom) {
  long lRet = SMT_ERR_NONE;
  int nGeomType = 0;
  m_Recordset.GetCollect("geom_type", nGeomType);

  switch (nGeomType) {
    case GTPoint:
      lRet = GetPoint(pGeom);
    default:
      break;
  }

  return lRet;
}

long AnnoFclsAdoLayer::AppendPoint(const Point *pPoint) {
  int nPoints = 1;

  if (nPoints < 0) return SMT_ERR_DB_OPER;

  RawPoint *pRawPoints = new RawPoint[nPoints];
  pRawPoints[0].x = pPoint->GetX();
  pRawPoints[0].y = pPoint->GetY();

  //////////////////////////////////////////////////////////////////////////
  m_Recordset.PutCollect("geom_type", _variant_t(GTPoint));
  m_Recordset.PutCollect("geom_pointnum", _variant_t(nPoints));

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

long AnnoFclsAdoLayer::GetPoint(Geometry *&pGeom) {
  int nPoints = 0;
  m_Recordset.GetCollect("geom_pointnum", nPoints);
  if (nPoints != 1) return SMT_ERR_DB_OPER;

  Point *pPoint = new Point();

  long lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
  if (lDataSize > 0) {
    _variant_t varBLOB;
    varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
    if (varBLOB.vt == (VT_ARRAY | VT_UI1)) {
      RawPoint *pRawPointBuf = NULL;
      SafeArrayAccessData(varBLOB.parray, (void **)&pRawPointBuf);
      pPoint->SetX(pRawPointBuf[0].x);
      pPoint->SetY(pRawPointBuf[0].y);
      SafeArrayUnaccessData(varBLOB.parray);
    }
  }

  pGeom = pPoint;

  return SMT_ERR_NONE;
}
}  // namespace _SDEAdo