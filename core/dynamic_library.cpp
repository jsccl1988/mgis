#include "core/dynamic_library.h"

#include "base/logging.h"

namespace core {
DynamicLibrary::DynamicLibrary(const char* name, const char* path) {
  strcpy(name_, name);
  strcpy(path_, path);
  dll_ = NULL;
}

DynamicLibrary::~DynamicLibrary() { UnLoad(); }

bool DynamicLibrary::Load() {
  LOG(INFO) << __FUNCTION__ << "loading" << name_;

  int nPathLen = strlen(path_) + MAX_PATH;
  char* pChPath = new char[nPathLen];
  sprintf_s(pChPath, nPathLen, "%s%s", path_, name_);
  dll_ = LoadLibrary(pChPath);
  if (!dll_) {
    std::string strErr = "   Loading ";
    strErr += name_;
    strErr += " error!";
    LOG(ERROR) << __FUNCTION__ << strErr;
    ::MessageBox(NULL, strErr.c_str(), "Gis - error", MB_OK | MB_ICONERROR);

    return false;
  }

  SMT_SAFE_DELETE_A(pChPath);

  return true;
}

void DynamicLibrary::UnLoad() {
  if (dll_ != NULL) {
    LOG(INFO) << __FUNCTION__ << "unloading" << name_;
    ::FreeLibrary(dll_);
    dll_ = NULL;
  }
}

DynamicLibraryManager* DynamicLibraryManager::singleton_ = NULL;

DynamicLibraryManager::DynamicLibraryManager(void) {
}

DynamicLibraryManager::~DynamicLibraryManager(void) {
  for (vDynamicLibraryPtrs::iterator it = dynamic_librarys.begin();
       it != dynamic_librarys.end(); ++it) {
    (*it)->UnLoad();
    SMT_SAFE_DELETE(*it);
  }

  dynamic_librarys.clear();
}

DynamicLibraryManager* DynamicLibraryManager::GetSingletonPtr(void) {
  if (singleton_ == NULL) {
    singleton_ = new DynamicLibraryManager();
  }
  return singleton_;
}

void DynamicLibraryManager::DestoryInstance(void) {
  CSLock cslock;
  ScopeCSLock scope(&cslock);

  SMT_SAFE_DELETE(singleton_);
}

//////////////////////////////////////////////////////////////////////////
DynamicLibrary* DynamicLibraryManager::LoadDynamicLibrary(const char* name,
                                                          const char* path) {
  DynamicLibrary* pLib = NULL;
  vDynamicLibraryPtrs::iterator i = dynamic_librarys.begin();
  while (i != dynamic_librarys.end()) {
    if (strcmp((*i)->GetName(), name) == 0) {
      pLib = *i;
      return pLib;
    }
    ++i;
  }

  pLib = new DynamicLibrary(name, path);
  if (pLib->Load())
    dynamic_librarys.push_back(pLib);
  else
    SMT_SAFE_DELETE(pLib);

  return pLib;
}

void DynamicLibraryManager::UnLoadDynamicLibrary(const char* name) {
  vDynamicLibraryPtrs::iterator i = dynamic_librarys.begin();
  while (i != dynamic_librarys.end()) {
    if (strcmp((*i)->GetName(), name) == 0) {
      (*i)->UnLoad();
      SMT_SAFE_DELETE(*i);
      dynamic_librarys.erase(i);
      break;
    }
    ++i;
  }
}
}  // namespace core