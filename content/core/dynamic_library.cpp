#include "content/core/dynamic_library.h"

#include "base/logging.h"

namespace content {
DynamicLibrary::DynamicLibrary(const char* name, const char* path) {
  name_ = name;
  path_ = path;
  dll_ = nullptr;
}

DynamicLibrary::~DynamicLibrary() { Unload(); }

bool DynamicLibrary::Load() {
  LOG(INFO) << __FUNCTION__ << "loading" << name_;

  size_t path_length = path_.length()+ MAX_PATH;
  auto path = std::make_unique<char[]>(path_length);
  sprintf_s(path.get(), path_length, "%s%s", path_.c_str(), name_.c_str());
  // dll_ = ::LoadLibrary(path);
  if (!dll_) {
    std::string err = "   Loading ";
    err += name_;
    err += " error!";
    LOG(ERROR) << __FUNCTION__ << err;
    return false;
  }

  return true;
}

bool DynamicLibrary::Unload() {
  if (dll_ != NULL) {
    LOG(INFO) << __FUNCTION__ << "unloading" << name_;
    ::FreeLibrary(dll_);
    dll_ = NULL;
  }

  return true;
}

DynamicLibraryManager::~DynamicLibraryManager(void) {
  for (DynamicLibrarys::iterator it = dynamic_librarys.begin();
       it != dynamic_librarys.end(); ++it) {
    (*it)->Unload();
    SAFE_DELETE(*it);
  }

  dynamic_librarys.clear();
}

DynamicLibrary* DynamicLibraryManager::LoadDynamicLibrary(const char* name,
                                                          const char* path) {
  DynamicLibrary* pLib = NULL;
  DynamicLibrarys::iterator i = dynamic_librarys.begin();
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
    SAFE_DELETE(pLib);

  return pLib;
}

void DynamicLibraryManager::UnloadDynamicLibrary(const char* name) {
  DynamicLibrarys::iterator i = dynamic_librarys.begin();
  while (i != dynamic_librarys.end()) {
    if (strcmp((*i)->GetName(), name) == 0) {
      (*i)->Unload();
      SAFE_DELETE(*i);
      dynamic_librarys.erase(i);
      break;
    }
    ++i;
  }
}
}  // namespace content