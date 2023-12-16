#include "rd3d_base.h"

namespace _3Drd {
Material::Material(void) { ; }

// set
void Material::SetDiffuseValue(const Color& diffuse) { m_cDiffuse = diffuse; }

void Material::SetSpecularValue(const Color& specular) {
  m_cSpecular = specular;
}

void Material::SetAmbientValue(const Color& ambient) { m_cAmbient = ambient; }

void Material::SetEmissiveValue(const Color& emissive) {
  m_cEmissive = emissive;
}

void Material::SetShininessValue(float shininess) { m_fShininess = shininess; }

// get
const Color& Material::GetDiffuseValue(void) { return m_cDiffuse; }

const Color& Material::GetSpecularValue(void) { return m_cSpecular; }

const Color& Material::GetAmbientValue(void) { return m_cAmbient; }

const Color& Material::GetEmissiveValue(void) { return m_cEmissive; }

float Material::GetShininessValue(void) { return m_fShininess; }

}  // namespace _3Drd