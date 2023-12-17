#ifndef _GIS_API_H
#define _GIS_API_H

#include "gis_sde.h"
#include "smt_core.h"


using namespace Smt_Core;
using namespace Smt_GIS;
//////////////////////////////////////////////////////////////////////////
//
long CORE_EXPORT CopyLayer(SmtLayer* pTarLayer, SmtLayer* pSrcLayer,
                              bool bClone = true, bool bCheckFeaType = false);
long CORE_EXPORT CopyLayer(SmtVectorLayer* pTarLayer,
                              SmtVectorLayer* pSrcLayer, bool bClone = true,
                              bool bCheckFeaType = false);
long CORE_EXPORT CopyLayer(SmtRasterLayer* pTarLayer,
                              SmtRasterLayer* pSrcLayer, bool bClone = true,
                              bool bCheckFeaType = false);

long CORE_EXPORT Points2MultiPoint(SmtVectorLayer* pLayer, string strStyle);

long CORE_EXPORT GetQueryRs(int geomType, int feaType);
#endif  //_GIS_API_H