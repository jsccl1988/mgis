#include "bl_style.h"

#include "base/logging.h"

namespace core {
Style::Style(void) {
  strcpy(name_, "Default");
  m_stType = ST_PenDesc;
}

Style::Style(const char *name, const PenDesc &penDesc,
                   const BrushDesc &brushDesc,
                   const AnnotationDesc &annoDesc,
                   const SymbolDesc &symbolDesc)
    : m_stPenDesc(penDesc),
      m_stBrushDesc(brushDesc),
      m_stAnnoDesc(annoDesc),
      m_stSymbolDesc(symbolDesc) {
  SetStyleName(name);
}

Style::~Style() {}

Style *Style::Clone(const char *szNewName) const {
  Style *pNewStyle = new Style(szNewName, m_stPenDesc, m_stBrushDesc,
                                     m_stAnnoDesc, m_stSymbolDesc);
  if (pNewStyle != NULL) {
    pNewStyle->SetStyleType(m_stType);
    return pNewStyle;
  } else
    return NULL;
}

StyleTable::StyleTable(void) {
  m_nStyleCount = 0;
  m_pStyleNames = NULL;
}

StyleTable::~StyleTable(void) {
  int index = 0;
  while (index < m_nStyleCount) {
    free(m_pStyleNames[index]);
    index++;
  }

  free(m_pStyleNames);

  m_nStyleCount = 0;
}

StyleTable *StyleTable::Clone(void) const {
  StyleTable *pStyleTable = new StyleTable();
  if (pStyleTable == NULL) return NULL;

  if (m_nStyleCount < 1) return NULL;

  pStyleTable->m_pStyleNames = new char *[m_nStyleCount];

  int i = 0;
  while (i < m_nStyleCount) {
    pStyleTable->m_pStyleNames[i] = strdup(m_pStyleNames[i]);
    i++;
  }

  pStyleTable->m_nStyleCount = m_nStyleCount;

  return pStyleTable;
}

//////////////////////////////////////////////////////////////////////////
int StyleTable::FindStyleNameIndex(const char *stylename) const {
  int index = 0;
  while (index < m_nStyleCount) {
    if (strcmp(m_pStyleNames[index], stylename) == 0) return index;
    index++;
  }

  return -1;
}

int StyleTable::AddStyle(const char *stylename) {
  int index = FindStyleNameIndex(stylename);
  if (index > 0 && index < m_nStyleCount) return ERR_NONE;

  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  if (pStyleMgr->GetStyle(stylename) != NULL) {
    m_pStyleNames =
        (char **)realloc(m_pStyleNames, sizeof(void *) * (m_nStyleCount + 1));
    m_pStyleNames[m_nStyleCount] = strdup(stylename);
    m_nStyleCount++;

    return ERR_NONE;
  }

  return ERR_FAILURE;
}

void StyleTable::RemoveStyle(const char *stylename) {
  int index = FindStyleNameIndex(stylename);
  if (index != -1 && index < m_nStyleCount) {
    SAFE_DELETE_A(m_pStyleNames[index]);
    memmove(m_pStyleNames + index, m_pStyleNames + index + 1,
            sizeof(void *) * (m_nStyleCount - index - 1));
    m_nStyleCount--;
  }
}

Style *StyleTable::GetStyle(const char *stylename) {
  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  int index = FindStyleNameIndex(stylename);
  if (index != -1) {
    Style *pStyle = pStyleMgr->GetStyle(stylename);
    if (NULL != pStyle) return pStyle;
  }

  return pStyleMgr->GetDefaultStyle();
}

const Style *StyleTable::GetStyle(const char *stylename) const {
  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  int index = FindStyleNameIndex(stylename);
  if (index != -1) {
    Style *pStyle = pStyleMgr->GetStyle(stylename);
    if (NULL != pStyle) return pStyle;
  }

  return pStyleMgr->GetDefaultStyle();
}

Style *StyleTable::GetStyle(int index) {
  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  if (index > -1 && index < m_nStyleCount) {
    Style *pStyle = pStyleMgr->GetStyle(m_pStyleNames[index]);
    if (pStyle != NULL) return pStyle;
  }

  return pStyleMgr->GetDefaultStyle();
}

const Style *StyleTable::GetStyle(int index) const {
  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  if (index > -1 && index < m_nStyleCount) {
    Style *pStyle = pStyleMgr->GetStyle(m_pStyleNames[index]);
    if (pStyle != NULL) return pStyle;
  }

  return pStyleMgr->GetDefaultStyle();
}

const char *StyleTable::GetStyleName(int index) {
  if (m_pStyleNames && index != -1 && index < m_nStyleCount)
    return m_pStyleNames[index];
  else
    return "";
}

StyleManager *StyleManager::singleton_ = NULL;

StyleManager *StyleManager::GetSingletonPtr(void) {
  if (singleton_ == NULL) {
    singleton_ = new StyleManager();
  }
  return singleton_;
}

void StyleManager::DestoryInstance(void) { SAFE_DELETE(singleton_); }

//////////////////////////////////////////////////////////////////////////
StyleManager::StyleManager(void) {
  m_pDefaultStyle = NULL;

  DestroyAllStyle();
}

StyleManager::~StyleManager(void) { DestroyAllStyle(); }

//////////////////////////////////////////////////////////////////////////

void StyleManager::SetDefaultStyle(const char *defName, PenDesc &penDesc,
                                      BrushDesc &brushDesc,
                                      AnnotationDesc &annoDesc,
                                      SymbolDesc &symbolDesc) {
  m_pDefaultStyle =
      new Style(defName, penDesc, brushDesc, annoDesc, symbolDesc);
  m_StylePtrList.push_back(m_pDefaultStyle);
}

void StyleManager::DestroyAllStyle() {
  StylePtrList::iterator i = m_StylePtrList.begin();

  while (i != m_StylePtrList.end()) {
    SAFE_DELETE(*i);
    ++i;
  }
  m_StylePtrList.clear();
}

Style *StyleManager::CreateStyle(const char *defName, PenDesc &penDesc,
                                       BrushDesc &brushDesc,
                                       AnnotationDesc &annoDesc,
                                       SymbolDesc &symbolDesc) {
  Style *pStyle = NULL;
  pStyle = GetStyle(defName);
  if (!pStyle) {
    pStyle = new Style(defName, penDesc, brushDesc, annoDesc, symbolDesc);
    m_StylePtrList.push_back(pStyle);
  }

  return pStyle;
}

void StyleManager::DestroyStyle(const char *name) {
  Style *pStyle = NULL;
  StylePtrList::iterator i = m_StylePtrList.begin();

  while (i != m_StylePtrList.end()) {
    if (strcmp((**i).GetStyleName(), name) == 0) {
      SAFE_DELETE(*i);
      m_StylePtrList.erase(i);
      break;
    }
    ++i;
  }
}

void StyleManager::DestroyStyle(Style *pStyle) {
  DestroyStyle(pStyle->GetStyleName());
}

Style *StyleManager::GetStyle(const char *name) {
  Style *pStyle = NULL;
  StylePtrList::iterator i = m_StylePtrList.begin();

  while (i != m_StylePtrList.end()) {
    if (strcmp((**i).GetStyleName(), name) == 0) {
      pStyle = *i;
      break;
    }
    ++i;
  }
  return pStyle;
}

Style *StyleManager::GetDefaultStyle(void) {
  if (m_pDefaultStyle)
    return m_pDefaultStyle;
  else
    return NULL;
}
}  // namespace core