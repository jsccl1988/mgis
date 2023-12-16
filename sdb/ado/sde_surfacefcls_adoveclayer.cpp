#include "sde_surfacefcls_adoveclayer.h"
#include "smt_api.h"
#include "smt_logmanager.h"
#include "geo_api.h"

using namespace core;
using namespace core;
using namespace _GIS;

namespace _SDEAdo
{
	SurfaceFclsAdoLayer::SurfaceFclsAdoLayer(DataSource *pOwnerDs):AdoVecLayer(pOwnerDs)
	{

	}

	SurfaceFclsAdoLayer::~SurfaceFclsAdoLayer(void)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool SurfaceFclsAdoLayer::Create(void)
	{
       bool bRet = true;

	   SetDefaultAttFields();
	   SetDefaultGeomFields();
	   SetTableFields();

	   //////////////////////////////////////////////////////////////////////////
	   sprintf(m_szLayerTableName,"%s_SurfaceFcls",m_szLayerName);
	   LayerInfo info;
	   sprintf(info.szArchiveName,m_szLayerTableName);
	   sprintf(info.szName,m_szLayerName);
	   info.unFeatureType = FtSurface;

	   return ((AdoDataSource *)m_pOwnerDs)->CreateLayerTable(info,m_pTableFieldCollect);
	}

	//////////////////////////////////////////////////////////////////////////
	void SurfaceFclsAdoLayer::SetDefaultAttFields(void)
	{
		SMT_SAFE_DELETE(m_pAtt);
		m_pAtt = new Attribute();

		Field smtFld;

		smtFld.SetName("ID");
		smtFld.SetType(VarType::Integer);
		m_pAtt->AddField(smtFld);

		smtFld.SetName("area");
		smtFld.SetType(VarType::Real);
		m_pAtt->AddField(smtFld);
	}

	void SurfaceFclsAdoLayer::SetDefaultGeomFields(void)
	{
		SMT_SAFE_DELETE(m_pGeomFieldCollect);
		m_pGeomFieldCollect = new FieldCollect();

		Field smtFld;
		//////////////////////////////////////////////////////////////////////////
		//geom
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

		smtFld.SetName("geom_polygonrings");
		smtFld.SetType(VarType::Integer);
		m_pGeomFieldCollect->AddField(smtFld);

		smtFld.SetName("geom_pointnums");
		smtFld.SetType(VarType::Binary);
		m_pGeomFieldCollect->AddField(smtFld);

		smtFld.SetName("geom_points");
		smtFld.SetType(VarType::Binary);
		m_pGeomFieldCollect->AddField(smtFld);
	}

	void SurfaceFclsAdoLayer::SetTableFields(void)
	{
		SMT_SAFE_DELETE(m_pTableFieldCollect);
		m_pTableFieldCollect = new FieldCollect();

		Field smtFld;

		smtFld.SetName("ID");
		smtFld.SetType(VarType::Integer);
		m_pTableFieldCollect->AddField(smtFld);
		//////////////////////////////////////////////////////////////////////////
		//geom
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

		smtFld.SetName("geom_polygonrings");
		smtFld.SetType(VarType::Integer);
		m_pTableFieldCollect->AddField(smtFld);

		smtFld.SetName("geom_pointnums");
		smtFld.SetType(VarType::Binary);
		m_pTableFieldCollect->AddField(smtFld);

		smtFld.SetName("geom_points");
		smtFld.SetType(VarType::Binary);
		m_pTableFieldCollect->AddField(smtFld);
		//////////////////////////////////////////////////////////////////////////
		smtFld.SetName("style");
		smtFld.SetType(VarType::Binary);
		m_pTableFieldCollect->AddField(smtFld);
		//////////////////////////////////////////////////////////////////////////
		smtFld.SetName("area");
		smtFld.SetType(VarType::Real);
		m_pTableFieldCollect->AddField(smtFld);
	}

	void SurfaceFclsAdoLayer::GetFeature(Feature *pFeature)
	{
        if (!pFeature)
			return;

		int       nID = 0;
		double	  dbfArea = 0.;
		Style *pStyle = NULL;

		int nGridID = 0;
		Envelope env;
		m_Recordset.GetCollect("ID",nID);
		m_Recordset.GetCollect("mbr_xmin",env.MinX);
		m_Recordset.GetCollect("mbr_ymin",env.MinY);
		m_Recordset.GetCollect("mbr_xmax",env.MaxX);
		m_Recordset.GetCollect("mbr_ymax",env.MaxY);
		m_Recordset.GetCollect("grid_id",nGridID);
		GetStyle(pStyle) ;
		//////////////////////////////////////////////////////////////////////////
		Geometry *pGeom = NULL;
		GetGeom(pGeom);

		m_Recordset.GetCollect("area",dbfArea);

		//////////////////////////////////////////////////////////////////////////
		pFeature->SetID(nID);
		pFeature->SetFeatureType(FeatureType::FtSurface);
		pFeature->SetStyleDirectly(pStyle);
		pFeature->SetGeometryDirectly(pGeom);
		pFeature->SetFieldValue(pFeature->GetFieldIndexByName("area"),dbfArea);
	}

	//////////////////////////////////////////////////////////////////////////
	long SurfaceFclsAdoLayer::AppendFeature(const Feature *pFeature,bool bclone)
	{
		long lRet = SMT_ERR_NONE;
		//////////////////////////////////////////////////////////////////////////
		//db
		if (!IsOpen())
			if (!Open(m_szLayerTableName))
				return SMT_ERR_DB_OPER;

		int nGridID = 0;
		Envelope env;

		const Geometry *pGeom = pFeature->GetGeometryRef();
		const Attribute *pAtt = pFeature->GetAttributeRef();
		const Style	*pStyle = pFeature->GetStyle();

		pGeom->GetEnvelope(&env);

		m_Recordset.AddNew();
		m_Recordset.PutCollect("ID",_variant_t(pFeature->GetID()));
		m_Recordset.PutCollect("mbr_xmin",_variant_t(env.MinX));
		m_Recordset.PutCollect("mbr_ymin",_variant_t(env.MinY));
		m_Recordset.PutCollect("mbr_xmax",_variant_t(env.MaxX));
		m_Recordset.PutCollect("mbr_ymax",_variant_t(env.MaxY));
		m_Recordset.PutCollect("grid_id",_variant_t(nGridID));
		//////////////////////////////////////////////////////////////////////////
		lRet = AppendStyle(pStyle);

		//////////////////////////////////////////////////////////////////////////
		lRet = AppendGeom(pGeom);

		//////////////////////////////////////////////////////////////////////////
		m_Recordset.PutCollect("area",_variant_t(((Surface*)pFeature->GetGeometryRef())->GetArea()));

		if (lRet != SMT_ERR_DB_OPER && !m_Recordset.Update())
		{
			SMT_SAFE_DELETE(pFeature);
			return SMT_ERR_DB_OPER;
		}

		//mem
		m_pMemLayer->AppendFeature(pFeature,bclone);
		CalEnvelope();

		return lRet;
	}

	//////////////////////////////////////////////////////////////////////////
	long SurfaceFclsAdoLayer::AppendFeatureBatch(const Feature *pFeature,bool bclone)
	{
		long lRet = SMT_ERR_NONE;
		//////////////////////////////////////////////////////////////////////////
		//db
		if (!IsOpen())
			if (!Open(m_szLayerTableName))
				return SMT_ERR_DB_OPER;

		int nGridID = 0;
		Envelope env;

		const Geometry *pGeom = pFeature->GetGeometryRef();
		const Attribute *pAtt = pFeature->GetAttributeRef();
		const Style	*pStyle = pFeature->GetStyle();

		pGeom->GetEnvelope(&env);

		m_Recordset.AddNew();
		m_Recordset.PutCollect("ID",_variant_t(pFeature->GetID()));
		m_Recordset.PutCollect("mbr_xmin",_variant_t(env.MinX));
		m_Recordset.PutCollect("mbr_ymin",_variant_t(env.MinY));
		m_Recordset.PutCollect("mbr_xmax",_variant_t(env.MaxX));
		m_Recordset.PutCollect("mbr_ymax",_variant_t(env.MaxY));
		m_Recordset.PutCollect("grid_id",_variant_t(nGridID));

		//////////////////////////////////////////////////////////////////////////
		lRet = AppendStyle(pStyle);
		//////////////////////////////////////////////////////////////////////////
		lRet = AppendGeom(pGeom);
		//////////////////////////////////////////////////////////////////////////
		m_Recordset.PutCollect("area",_variant_t(((Surface*)pFeature->GetGeometryRef())->GetArea()));

		//mem
		m_pMemLayer->AppendFeature(pFeature,bclone);

		return lRet;
	}


	long  SurfaceFclsAdoLayer::UpdateFeature(const Feature *pFeature)
	{
		//////////////////////////////////////////////////////////////////////////
		//db
		Feature *pFea = m_pMemLayer->GetFeature();
		if (NULL != pFea)
		{
			char szSQL[TEMP_BUFFER_SIZE];
			sprintf_s(szSQL,TEMP_BUFFER_SIZE,"select * from %s where ID = %d;",m_szLayerTableName,pFea->GetID());
			if (m_Recordset.Open(szSQL))
			{
				m_Recordset.MoveFirst();
				if (m_Recordset.IsOpen() && !m_Recordset.IsEOF() && m_Recordset.GetRecordCount() == 1)
				{
					int nGridID = 0;
					Envelope env;

					const Geometry *pGeom = pFeature->GetGeometryRef();
					const Attribute *pAtt = pFeature->GetAttributeRef();
					const Style	*pStyle = pFeature->GetStyle();

					pGeom->GetEnvelope(&env);

					m_Recordset.AddNew();
					m_Recordset.PutCollect("ID",_variant_t(pFeature->GetID()));
					m_Recordset.PutCollect("mbr_xmin",_variant_t(env.MinX));
					m_Recordset.PutCollect("mbr_ymin",_variant_t(env.MinY));
					m_Recordset.PutCollect("mbr_xmax",_variant_t(env.MaxX));
					m_Recordset.PutCollect("mbr_ymax",_variant_t(env.MaxY));
					m_Recordset.PutCollect("grid_id",_variant_t(nGridID));
					//////////////////////////////////////////////////////////////////////////
					AppendStyle(pStyle);

					//////////////////////////////////////////////////////////////////////////
					AppendGeom(pGeom);

					//////////////////////////////////////////////////////////////////////////
					m_Recordset.PutCollect("area",_variant_t(((Surface*)pFeature->GetGeometryRef())->GetArea()));

					if (!m_Recordset.Update())
						return SMT_ERR_DB_OPER;

					//mem
					m_pMemLayer->UpdateFeature(pFeature);
					CalEnvelope();
				}
			}
		}

		return SMT_ERR_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	long   SurfaceFclsAdoLayer::AppendStyle(const Style *pStyle)
	{
		char   *pBuf = (char*)pStyle;
		int    nLen  = sizeof(Style);

		VARIANT varBLOB;
		SAFEARRAY *pSa = NULL;
		SAFEARRAYBOUND rgSaBound[1];
		if(pBuf)
		{
			rgSaBound[0].lLbound = 0;
			rgSaBound[0].cElements = nLen;

			char *pSABuf = NULL;
			pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
			if(SafeArrayAccessData(pSa,(void **)&pSABuf) == NOERROR)
			{
				memcpy(pSABuf,pBuf,nLen);
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

    long   SurfaceFclsAdoLayer::AppendGeom(const Geometry *pGeom)
	{
		long lRet = SMT_ERR_NONE;
        GeometryType type  = pGeom->GetGeometryType();

		switch(type)
		{
		case GTPolygon:
			lRet = AppendPolygon((Polygon*)pGeom);
			break;

		case GTFan:
			lRet = AppendFan((Fan*)pGeom);
			break;

		default:
			break;
		}

		return lRet;
	}

	//////////////////////////////////////////////////////////////////////////
	long   SurfaceFclsAdoLayer::GetStyle(Style *&pStyle)
	{
		pStyle = new Style();
		long lDataSize = m_Recordset.GetField("style")->ActualSize;
		if(lDataSize > 0)
		{
			_variant_t varBLOB;
			varBLOB = m_Recordset.GetField("style")->GetChunk(lDataSize);
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				Style *pBuf = NULL;
				SafeArrayAccessData(varBLOB.parray,(void **)&pBuf);
				memcpy(pStyle,pBuf,sizeof(Style));
				SafeArrayUnaccessData (varBLOB.parray);
			}
		}

		return SMT_ERR_NONE;
	}

	long   SurfaceFclsAdoLayer::GetGeom(Geometry *&pGeom)
	{
		long lRet = SMT_ERR_NONE;
		int nGeomType = 0;
		m_Recordset.GetCollect("geom_type",nGeomType);

		switch(nGeomType)
		{

		case GTPolygon:
			lRet = GetPolygon(pGeom);
			break;

		case GTFan:
			lRet = GetFan(pGeom);
			break;

		default:
			break;
		}

		return lRet;
	}

	//////////////////////////////////////////////////////////////////////////
	long   SurfaceFclsAdoLayer::AppendPolygon(const Polygon *pPolygon)
	{
		 int nRings = pPolygon->GetNumInteriorRings()+1;

		 //////////////////////////////////////////////////////////////////////////
		 m_Recordset.PutCollect("geom_type",_variant_t(GTPolygon));
		 m_Recordset.PutCollect("geom_polygonrings",_variant_t(nRings));

		 //////////////////////////////////////////////////////////////////////////
		 PutCollectLinearingNums(pPolygon);
		 PutCollectLinearings(pPolygon);

		 return SMT_ERR_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	long   SurfaceFclsAdoLayer::PutCollectLinearingNums(const Polygon *pPolygon)
	{
		int nInteriorRings = pPolygon->GetNumInteriorRings();

		int *pRingPointNums = new int [nInteriorRings+1];

		pRingPointNums[0] = pPolygon->GetExteriorRing()->GetNumPoints();
		for(int i = 0; i < nInteriorRings ; i ++)
			pRingPointNums[i+1] = pPolygon->GetInteriorRing(i)->GetNumPoints();

		char   *pBuf = (char*)pRingPointNums;
		int    nLen  = sizeof(int)*(nInteriorRings+1);
		VARIANT varBLOB;
		SAFEARRAY *pSa = NULL;
		SAFEARRAYBOUND rgSaBound[1];
		if(pBuf)
		{
			rgSaBound[0].lLbound = 0;
			rgSaBound[0].cElements = nLen;

			char *pSABuf = NULL;
			pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
			if(SafeArrayAccessData(pSa,(void **)&pSABuf) == NOERROR)
			{
				memcpy(pSABuf,pBuf,nLen);
			}
			SafeArrayUnaccessData(pSa);

			/*pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
			for (long i = 0; i < (long)nLen; i++)
				SafeArrayPutElement (pSa, &i, pBuf++); */

			varBLOB.vt = VT_ARRAY | VT_UI1;
			varBLOB.parray = pSa;
			m_Recordset.GetField("geom_pointnums")->AppendChunk(varBLOB);
		}

		SMT_SAFE_DELETE_A(pRingPointNums);

		return SMT_ERR_NONE;
	}

	long   SurfaceFclsAdoLayer::PutCollectLinearings(const Polygon *pPolygon)
	{
		int nInteriorRings = pPolygon->GetNumInteriorRings();
		int nPoints  = pPolygon->GetExteriorRing()->GetNumPoints();

		for(int i = 0; i < nInteriorRings ; i ++)
			nPoints += pPolygon->GetInteriorRing(i)->GetNumPoints();

		int nLen = sizeof(RawPoint)*nPoints;

		VARIANT varBLOB;
		SAFEARRAY *pSa = NULL;
		SAFEARRAYBOUND rgSaBound[1];

		rgSaBound[0].lLbound = 0;
		rgSaBound[0].cElements = nLen;
		pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);

		long nSaEleIndex = 0;
		PutCollectLinearing(pPolygon->GetExteriorRing(),pSa,nSaEleIndex);

		for (int i = 0 ; i < nInteriorRings ; i++)
		  PutCollectLinearing(pPolygon->GetInteriorRing(i),pSa,nSaEleIndex);

		varBLOB.vt = VT_ARRAY | VT_UI1;
		varBLOB.parray = pSa;
		m_Recordset.GetField("geom_points")->AppendChunk(varBLOB);

		return SMT_ERR_NONE;
	}

	long   SurfaceFclsAdoLayer::PutCollectLinearing(const LinearRing *pLinearRing,SAFEARRAY *pSa,long &index)
	{
		int nPoints = pLinearRing->GetNumPoints();
		if (nPoints < 0)
			return SMT_ERR_DB_OPER;

		RawPoint *pRawPoints = new RawPoint[nPoints];
		pLinearRing->GetPoints(pRawPoints);

		char   *pBuf = (char*)pRawPoints;
		long    nEndIndex = index + sizeof(RawPoint)*nPoints;

		if(pBuf)
		{
			for (; index < nEndIndex; index++)
				SafeArrayPutElement (pSa, &index, pBuf++);
		}

		SMT_SAFE_DELETE_A(pRawPoints);

		return SMT_ERR_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	long   SurfaceFclsAdoLayer::AppendFan(const Fan *pFan)
	{
		//////////////////////////////////////////////////////////////////////////
		m_Recordset.PutCollect("geom_type",_variant_t(GTFan));
		m_Recordset.PutCollect("geom_polygonrings",_variant_t(1));

		//////////////////////////////////////////////////////////////////////////
		int    nPoints = 3;
		char   *pBuf = (char*)(&nPoints);
		int    nLen  = sizeof(int);
		VARIANT varBLOB;
		SAFEARRAY *pSa;
		SAFEARRAYBOUND rgSaBound[1];
		if(pBuf)
		{
			rgSaBound[0].lLbound = 0;
			rgSaBound[0].cElements = nLen;

			pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
			for (long i = 0; i < (long)nLen; i++)
				SafeArrayPutElement (pSa, &i, pBuf++);

			varBLOB.vt = VT_ARRAY | VT_UI1;
			varBLOB.parray = pSa;
			m_Recordset.GetField("geom_pointnums")->AppendChunk(varBLOB);
		}
		SafeArrayDestroy(pSa);

		//////////////////////////////////////////////////////////////////////////
		const Arc * pArc =pFan->GetArc();
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

		pBuf = (char*)pRawPoints;
		nLen  = sizeof(RawPoint)*3;

		if(pBuf)
		{
			rgSaBound[0].lLbound = 0;
			rgSaBound[0].cElements = nLen;
			pSa = SafeArrayCreate(VT_UI1, 1, rgSaBound);
			for (long i = 0; i < (long)nLen; i++)
				SafeArrayPutElement (pSa, &i, pBuf++);

			varBLOB.vt = VT_ARRAY | VT_UI1;
			varBLOB.parray = pSa;
			m_Recordset.GetField("geom_points")->AppendChunk(varBLOB);
		}

		SafeArrayDestroy(pSa);
		SMT_SAFE_DELETE_A(pRawPoints);

		return SMT_ERR_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	long   SurfaceFclsAdoLayer::GetPolygon(Geometry *&pGeom)
	{
		  int nRings = 0,nInteriorRings = 0;
		  m_Recordset.GetCollect("geom_polygonrings",nRings);
		  nInteriorRings = nRings - 1;

		  Polygon *pPolygon = new Polygon();
		  pPolygon->AddRingDirectly(new LinearRing());

		  for (int i = 0; i < nInteriorRings ; i ++)
			  pPolygon->AddRingDirectly(new LinearRing());

		  GetCollectLinearingNums(pPolygon);
		  GetCollectLinearings(pPolygon);

		  pGeom = pPolygon;

		  return SMT_ERR_NONE;
	}

	long   SurfaceFclsAdoLayer::GetCollectLinearingNums(Polygon *pPloygon)
	{
		int nInteriorRings = pPloygon->GetNumInteriorRings();
		long lDataSize = m_Recordset.GetField("geom_pointnums")->ActualSize;
		if(lDataSize > 0)
		{
			_variant_t varBLOB;
			varBLOB = m_Recordset.GetField("geom_pointnums")->GetChunk(lDataSize);
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				char *pBuf = NULL;
				SafeArrayAccessData(varBLOB.parray,(void **)&pBuf);
				int nPoints = 0;
				memcpy(&nPoints,pBuf,sizeof(int));
				pBuf += sizeof(int);
				pPloygon->GetExteriorRing()->SetNumPoints(nPoints);

				for (int index = 0; index < nInteriorRings ; index ++)
				{
					memcpy(&nPoints,pBuf,sizeof(int));
					pPloygon->GetInteriorRing(index)->SetNumPoints(nPoints);
					pBuf += sizeof(int);
				}

				SafeArrayUnaccessData (varBLOB.parray);
			}
		}

		return SMT_ERR_NONE;
	}

	long  SurfaceFclsAdoLayer::GetCollectLinearings(Polygon *pPloygon)
	{
		int nInteriorRings = pPloygon->GetNumInteriorRings();
		long lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
		if(lDataSize > 0)
		{
			_variant_t varBLOB;
			varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				char *pBuf = NULL;
				SafeArrayAccessData(varBLOB.parray,(void **)&pBuf);
				RawPoint *pRawPointBuf = (RawPoint*)pBuf;

				GetCollectLinearing(pPloygon->GetExteriorRing(),pRawPointBuf);
				for (int  i = 0; i < nInteriorRings ; i++)
				  GetCollectLinearing(pPloygon->GetInteriorRing(i),pRawPointBuf);

				SafeArrayUnaccessData (varBLOB.parray);
			}
		}
        return SMT_ERR_NONE;
	}

	long   SurfaceFclsAdoLayer::GetCollectLinearing(LinearRing *pLinearRing,RawPoint *&pRawPointBuf)
	{
		int nPoints = pLinearRing->GetNumPoints();
		pLinearRing->SetPoints(nPoints,pRawPointBuf);
		pRawPointBuf += nPoints;

		return SMT_ERR_NONE;
	}


	long   SurfaceFclsAdoLayer::GetFan(Geometry *&pGeom)
	{
		long lDataSize = m_Recordset.GetField("geom_polygonrings")->ActualSize;
		if (lDataSize != sizeof(int))
			return SMT_ERR_DB_OPER;

		//get point nums
		int nPoints = 0;
		lDataSize = m_Recordset.GetField("geom_pointnums")->ActualSize;
		if(lDataSize > 0)
		{
			_variant_t varBLOB;
			varBLOB = m_Recordset.GetField("geom_pointnums")->GetChunk(lDataSize);
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				char *pBuf = NULL;
				SafeArrayAccessData(varBLOB.parray,(void **)&pBuf);
				memcpy(&nPoints,pBuf,sizeof(int));
				pBuf += sizeof(int);
				SafeArrayUnaccessData (varBLOB.parray);
			}
		}

		if (nPoints != 3)
			return SMT_ERR_DB_OPER;


		Arc *pArc = new Arc();
		RawPoint *pRawPoints = new RawPoint[3];

		lDataSize = m_Recordset.GetField("geom_points")->ActualSize;
		if(lDataSize > 0)
		{
			_variant_t varBLOB;
			varBLOB = m_Recordset.GetField("geom_points")->GetChunk(lDataSize);
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				char *pBuf = NULL;
				RawPoint *pRawPoints1 = pRawPoints;
				SafeArrayAccessData(varBLOB.parray,(void **)&pBuf);
				for (int index = 0; index < nPoints ; index ++)
				{
					memcpy(pRawPoints1,pBuf,sizeof(RawPoint));
					pRawPoints1 ++;
					pBuf +=sizeof(RawPoint);
				}

				SafeArrayUnaccessData (varBLOB.parray);
			}
		}

		pArc->SetCenterPoint(pRawPoints[0]);
		pArc->SetStartPoint(pRawPoints[1]);
		pArc->SetEndPoint(pRawPoints[2]);

		float r = Distance(pRawPoints[0].x,pRawPoints[0].y,pRawPoints[1].x,pRawPoints[1].y);
		pArc->SetRadius(r);

		SMT_SAFE_DELETE_A(pRawPoints);

		Fan * pFan = new Fan();
		pFan->SetArcDirectly(pArc);
		pGeom = pFan;

		return SMT_ERR_NONE;
	}
}