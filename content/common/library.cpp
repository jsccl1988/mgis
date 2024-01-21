// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#include "content/common/library.h"

#include "base/logging.h"
#include "base/util/string_util.h"


namespace content {
Library::Library(const base::NameChar* name, const base::PathChar* path) {
  name_ = name;
  path_ = path;
  dll_ = nullptr;
}

Library::~Library() { Unload(); }

bool Library::Load() {
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

bool Library::Unload() {
  if (dll_ != NULL) {
    LOG(INFO) << __FUNCTION__ << "unloading"
              << base::UTF16ToUTF8(name_).c_str();
    ::FreeLibrary(dll_);
    dll_ = NULL;
  }

  return true;
}

LibraryManager::~LibraryManager(void) {
  for (Librarys::iterator it = librarys_.begin(); it != librarys_.end(); ++it) {
    (*it)->Unload();
    SAFE_DELETE(*it);
  }

  librarys_.clear();
}

Library* LibraryManager::LoadLibrary(const base::NameChar* name,
                                     const base::PathChar* path) {
  Library* lib = NULL;
  Librarys::iterator i = librarys_.begin();
  while (i != librarys_.end()) {
    if (wcscmp((*i)->GetName(), name) == 0) {
      lib = *i;
      return lib;
    }
    ++i;
  }

  lib = new Library(name, path);
  if (lib->Load()) {
    librarys_.push_back(lib);
  } else {
    SAFE_DELETE(lib);
  }

  return lib;
}

void LibraryManager::UnloadLibrary(const base::NameChar* name) {
  Librarys::iterator i = librarys_.begin();
  while (i != librarys_.end()) {
    if (wcscmp((*i)->GetName(), name) == 0) {
      (*i)->Unload();
      SAFE_DELETE(*i);
      librarys_.erase(i);
      break;
    }
    ++i;
  }
}
}  // namespace content