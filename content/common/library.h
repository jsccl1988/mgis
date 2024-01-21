
// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef CONTENT_COMMON_LIBRARY_H
#define CONTENT_COMMON_LIBRARY_H

#include <string>

#include "base/file/file_path.h"
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "content/content.h"
#include "content/content_export.h"

namespace content {

class CONTENT_EXPORT Library {
 public:
  Library(const base::NameChar* name, const base::PathChar* path);
  virtual ~Library(void);

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

using Librarys = std::vector<Library*>;
class CONTENT_EXPORT LibraryManager {
 public:
  virtual ~LibraryManager(void);

 public:
  static LibraryManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  Library* LoadLibrary(const base::NameChar* name, const base::PathChar* path);
  void UnloadLibrary(const base::NameChar* name);

 protected:
  Librarys librarys_;

 private:
  LibraryManager(void);

 private:
  static LibraryManager* singleton_;
};
}  // namespace content

#endif  // CONTENT_COMMON_LIBRARY_H