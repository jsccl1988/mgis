#include "gt_iacommandreceiver.h"

#include "bl_api.h"
#include "gis_map.h"
#include "smt_api.h"


using namespace _GIS;

namespace _GroupTool {
IACommandReceiver::IACommandReceiver(LPRENDERDEVICE pRenderDevice,
                                     Map *pOperMap, Feature *pFeature,
                                     IAType iaType)
    : m_pRenderDevice(pRenderDevice),
      m_pOperMap(pOperMap),
      m_pFeature(pFeature),
      m_iaType(iaType) {}

IACommandReceiver::~IACommandReceiver() {}

bool IACommandReceiver::Action(bool is_undo) {
  switch (m_iaType) {
    case eIA_AppendFeature: {
      if (is_undo) {
        m_pFeature = m_pFeature->Clone();
        m_pOperMap->DeleteFeature(m_pFeature);
      } else {
        m_pOperMap->AppendFeature(m_pFeature, true);
      }
    } break;
    case eIA_DeleteFeature: {
      if (is_undo) {
        m_pOperMap->AppendFeature(m_pFeature, true);
      } else {
        m_pFeature = m_pFeature->Clone();
        m_pOperMap->DeleteFeature(m_pFeature);
      }
    } break;
    case eIA_ModifyFeature: {
      if (is_undo) {
        m_pOperMap->UpdateFeature(m_pFeature);
      }
    } break;
    default:
      break;
  }

  // refresh
  fRect frt;
  Envelope envelope;
  float fMargin = 5. / m_pRenderDevice->GetBlc();
  const Geometry *pGeom = m_pFeature->GetGeometryRef();

  pGeom->GetEnvelope(&envelope);
  envelope.Merge(envelope.MinX - fMargin, envelope.MinY - fMargin);
  envelope.Merge(envelope.MaxX + fMargin, envelope.MaxY + fMargin);
  EnvelopeToRect(frt, envelope);

  m_pRenderDevice->Refresh(m_pOperMap, frt);

  return true;
}
}  // namespace _GroupTool