
#ifndef GIS_CORE_DYNAMIC_LIBRARY_H
#define GIS_CORE_DYNAMIC_LIBRARY_H

#include <string>

#include "core/core.h"
#include "core/core_export.h"

namespace core {
const std::string g_strDynamicLibraryLog = "DynamicLibrary";
class GIS_CORE_EXPORT DynamicLibrary {
 public:
  DynamicLibrary(const char* name, const char* path);
  virtual ~DynamicLibrary(void);

 public:
  inline const char* GetName(void) const { return name_; }
  inline const char* GetPath(void) const { return path_; }

  virtual bool Load(void);
  virtual void UnLoad(void);

 protected:
  char name_[MAX_NAME_LENGTH];
  char path_[MAX_FILE_PATH];
  HMODULE dll_;
};

typedef std::vector<DynamicLibrary*> vDynamicLibraryPtrs;
class GIS_CORE_EXPORT DynamicLibraryManager {
 public:
  virtual ~DynamicLibraryManager(void);

 public:
  static DynamicLibraryManager* GetSingletonPtr(void);
  static void DestoryInstance(void);

 public:
  DynamicLibrary* LoadDynamicLibrary(const char* name, const char* path);
  void UnLoadDynamicLibrary(const char* name);

 protected:
  vDynamicLibraryPtrs dynamic_librarys;

 private:
  DynamicLibraryManager(void);

 private:
  static DynamicLibraryManager* singleton_;
};
}  // namespace core

#endif  // GIS_CORE_DYNAMIC_LIBRARY_H