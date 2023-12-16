#include "sde_mem.h"
#include "smt_api.h"

namespace _SDEMem
{
   MemDataSource::MemDataSource(void)
   {

   }

   MemDataSource::~MemDataSource(void)
   {

   }

   bool MemDataSource::Create(void)
   {
	   //////////////////////////////////////////////////////////////////////////
	   //...

	   return true;
   }

   bool MemDataSource::Open()
   {
	   m_bOpen = true;
	   return true;
   }

   DataSource * MemDataSource::Clone(void) const
   {
         MemDataSource *pDs = new MemDataSource();
		 pDs->SetInfo(m_dsInfo);

		 return (DataSource*)pDs;
   }

   bool MemDataSource::Close()
   {
       return true;
   }

   VectorLayer	*MemDataSource::CreateVectorLayer(const char *pszName,fRect &lyrRect,FeatureType ftType)
   {
	   MemVecLayer *pLayer = new MemVecLayer(this);
	   pLayer->SetLayerFeatureType(ftType);
	   pLayer->SetLayerName(pszName);
	   pLayer->SetLayerRect(lyrRect);
	   if (!pLayer->Create())
		   SMT_SAFE_DELETE(pLayer);

		LayerInfo info;
		sprintf(info.szArchiveName,"%s%s",m_dsInfo.szUrl,pszName);
		strcpy(info.szName,pszName);
		info.unFeatureType = ftType;

		m_vLayerInfos.push_back(info);

       return (VectorLayer*)pLayer;
   }

   VectorLayer	*MemDataSource::OpenVectorLayer(const char *pszLayerFileName)
   {
	   MemVecLayer *pLayer = new MemVecLayer(this);
	   if(pLayer->Open(pszLayerFileName))
	   {
		   pLayer->SetLayerName(pszLayerFileName);
		   pLayer->Fetch();
		   return (VectorLayer*)pLayer;
	   }

       SMT_SAFE_DELETE(pLayer);

	   return NULL;
   }

   bool  MemDataSource::DeleteVectorLayer(const char *pszName)
   {
	   return true;
   }

   //////////////////////////////////////////////////////////////////////////
   RasterLayer* MemDataSource::CreateRasterLayer(const char *szName,fRect &lyrRect,long lImageCode)
   {
	   MemRasLayer *pLayer = new MemRasLayer(this);
	   pLayer->SetLayerName(szName);
	   pLayer->SetLayerRect(lyrRect);
	   if (!pLayer->Create())
		   SMT_SAFE_DELETE(pLayer);

	   LayerInfo info;
	   sprintf(info.szArchiveName,"%s%s",m_dsInfo.szUrl,szName);
	   strcpy(info.szName,szName);
	   info.unFeatureType = Layer_Ras;

	   m_vLayerInfos.push_back(info);

	   return (RasterLayer*)pLayer;
   }

   RasterLayer* MemDataSource::OpenRasterLayer(const char *szName)
   {
	   MemRasLayer *pLayer = new MemRasLayer(this);
	   if(pLayer->Open(szName))
	   {
		   pLayer->SetLayerName(szName);
		   pLayer->Fetch();
		   return (RasterLayer*)pLayer;
	   }

	   SMT_SAFE_DELETE(pLayer);

	   return NULL;
   }

   bool MemDataSource::DeleteRasterLayer(const char *szName)
   {
		return true;
   }

   //////////////////////////////////////////////////////////////////////////
   TileLayer* MemDataSource::CreateTileLayer(const char *szName,fRect &lyrRect,long lImageCode)
   {
	   MemTileLayer *pLayer = new MemTileLayer(this);
	   pLayer->SetLayerName(szName);
	   pLayer->SetLayerRect(lyrRect);
	   if (!pLayer->Create())
		   SMT_SAFE_DELETE(pLayer);

	   LayerInfo info;
	   sprintf(info.szArchiveName,"%s%s",m_dsInfo.szUrl,szName);
	   strcpy(info.szName,szName);
	   info.unFeatureType = Layer_Tile;

	   m_vLayerInfos.push_back(info);

	   return (TileLayer*)pLayer;
   }

   TileLayer* MemDataSource::OpenTileLayer(const char *szName)
   {
	   MemTileLayer *pLayer = new MemTileLayer(this);
	   if(pLayer->Open(szName))
	   {
		   pLayer->SetLayerName(szName);
		   pLayer->Fetch();
		   return (TileLayer*)pLayer;
	   }

	   return NULL;
   }

   bool MemDataSource::DeleteTileLayer(const char *szName)
   {
		return true;
   }
}