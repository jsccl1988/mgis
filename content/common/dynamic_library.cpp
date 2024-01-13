// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#include "content/common/dynamic_library.h"

#include "base/logging.h"
#include "base/util/string_util.h"

namespace content {
DynamicLibrary::DynamicLibrary(const base::NameChar* name,
                               const base::PathChar* path) {
  name_ = name;
  path_ = path;
  dll_ = nullptr;
}

DynamicLibrary::~DynamicLibrary() { Unload(); }

bool DynamicLibrary::Load() {
  LOG(INFO) << __FUNCTION__ << "loading" << base::UTF16ToUTF8(name_);

  size_t path_length = path_.length() + MAX_PATH;
  auto path = std::make_unique<base::PathChar[]>(path_length);
  swprintf_s(path.get(), path_length, L"%s%s", path_.c_str(), name_.c_str());
  dll_ = ::LoadLibrary(path.get());
  if (!dll_) {
    std::string err = "   Loading ";
    err += base::UTF16ToUTF8(name_);
    err += " error!";
    LOG(ERROR) << __FUNCTION__ << err;
    return false;
  }

  return true;
}

bool DynamicLibrary::Unload() {
  if (dll_ != NULL) {
    LOG(INFO) << __FUNCTION__ << "unloading"
              << base::UTF16ToUTF8(name_).c_str();
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

DynamicLibrary* DynamicLibraryManager::LoadDynamicLibrary(
    const base::NameChar* name, const base::PathChar* path) {
  DynamicLibrary* lib = NULL;
  DynamicLibrarys::iterator i = dynamic_librarys.begin();
  while (i != dynamic_librarys.end()) {
    if (wcscmp((*i)->GetName(), name) == 0) {
      lib = *i;
      return lib;
    }
    ++i;
  }

  lib = new DynamicLibrary(name, path);
  if (lib->Load()) {
    dynamic_librarys.push_back(lib);
  } else {
    SAFE_DELETE(lib);
  }

  return lib;
}

void DynamicLibraryManager::UnloadDynamicLibrary(const base::NameChar* name) {
  DynamicLibrarys::iterator i = dynamic_librarys.begin();
  while (i != dynamic_librarys.end()) {
    if (wcscmp((*i)->GetName(), name) == 0) {
      (*i)->Unload();
      SAFE_DELETE(*i);
      dynamic_librarys.erase(i);
      break;
    }
    ++i;
  }
}
}  // namespace content