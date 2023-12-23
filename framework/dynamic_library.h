
#ifndef FRAMEWORK_EXPORT_DYNAMIC_LIBRARY_H
#define FRAMEWORK_EXPORT_DYNAMIC_LIBRARY_H

#include <string>

#include "base/memory/singleton.h"

#include "framework/framework.h"
#include "framework/framework_export.h"

namespace framework {
class FRAMEWORK_EXPORT DynamicLibrary {
 public:
  DynamicLibrary(const char* name, const char* path);
  virtual ~DynamicLibrary(void);

 public:
  inline const char* GetName(void) const { return name_.c_str(); }
  inline const char* GetPath(void) const { return path_.c_str(); }

  virtual bool Load(void);
  virtual bool Unload(void);

 protected:
  std::string name_;
  std::string path_;
  HMODULE dll_;
};

using DynamicLibrarys = std::vector<DynamicLibrary*> ;
class FRAMEWORK_EXPORT DynamicLibraryManager {
 public:
  virtual ~DynamicLibraryManager(void);

 public:
  static DynamicLibraryManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  DynamicLibrary* LoadDynamicLibrary(const char* name, const char* path);
  void UnloadDynamicLibrary(const char* name);

 protected:
  DynamicLibrarys dynamic_librarys;

 private:
  DynamicLibraryManager(void);

 private:
  static DynamicLibraryManager* singleton_;
};
}  // namespace framework

#endif  // FRAMEWORK_EXPORT_DYNAMIC_LIBRARY_H