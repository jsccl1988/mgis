#include "rd3d_base.h"

namespace _3Drd {
Light::Light(void) {
  m_fCutoffAngle = 180.f;
  m_fExponent = 0.0f;
  m_fAttenuationConstant = 1.f;
  m_fAttenuationLinear = 0.f;
  m_fAttenuationQuadric = 0.f;
  m_fRange = 1000;
  m_fThetaAngle = 180;
  m_fPhiAngle = 45;
}

// set
void Light::SetType(LIGHTTYPE type) { m_Type = type; }

void Light::SetDiffuseValue(const Color& diffuse) { m_cDiffuse = diffuse; }

void Light::SetSpecularValue(const Color& specular) { m_cSpecular = specular; }

void Light::SetAmbientValue(const Color& ambient) { m_cAmbient = ambient; }

void Light::SetPoistion(const Vector4& position) { m_vPosition = position; }

void Light::SetDirection(const Vector4& direction) { m_vDirection = direction; }

// get

LIGHTTYPE Light::GetType() { return m_Type; }

const Color& Light::GetDiffuseValue(void) { return m_cDiffuse; }

const Color& Light::GetSpecularValue(void) { return m_cSpecular; }

const Color& Light::GetAmbientValue(void) { return m_cAmbient; }

const Vector4& Light::GetPosition(void) { return m_vPosition; }

const Vector4& Light::GetDirection(void) { return m_vDirection; }
}  // namespace _3Drd