
// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef CONTENT_COMMON_DYNAMIC_LIBRARY_H
#define CONTENT_COMMON_DYNAMIC_LIBRARY_H

#include <string>

#include "base/file/file_path.h"
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "content/content.h"
#include "content/content_export.h"

namespace content {

class CONTENT_EXPORT DynamicLibrary {
 public:
  DynamicLibrary(const base::NameChar* name, const base::PathChar* path);
  virtual ~DynamicLibrary(void);

 public:
  inline const base::NameChar* GetName(void) const { return name_.c_str(); }
  inline const base::PathChar* GetPath(void) const { return path_.c_str(); }

  virtual bool Load(void);
  virtual bool Unload(void);

 protected:
  base::NameString name_;
  base::PathString path_;
  HMODULE dll_;
};

using DynamicLibrarys = std::vector<DynamicLibrary*>;
class CONTENT_EXPORT DynamicLibraryManager {
 public:
  virtual ~DynamicLibraryManager(void);

 public:
  static DynamicLibraryManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  DynamicLibrary* LoadDynamicLibrary(const base::NameChar* name,
                                     const base::PathChar* path);
  void UnloadDynamicLibrary(const base::NameChar* name);

 protected:
  DynamicLibrarys dynamic_librarys;

 private:
  DynamicLibraryManager(void);

 private:
  static DynamicLibraryManager* singleton_;
};
}  // namespace content

#endif  // CONTENT_COMMON_DYNAMIC_LIBRARY_H