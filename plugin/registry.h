// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef PLUGIN_REGISTRY_H_
#define PLUGIN_REGISTRY_H_

#include <string>
#include <vector>

namespace plugin {

// One plugin.json manifest kept in process. No DLL is loaded for the record.
struct Manifest {
  std::string id;
  std::string name;
  std::vector<std::string> commands;
};

// In-process catalog of plugin.json files under a directory.
// Scan reads the directory itself and its immediate child directories.
// Find pointers are invalidated by the next Scan or Register call.
class Registry {
 public:
  Registry();
  ~Registry();

  // Replaces the catalog with readable plugin.json files under |directory_utf8|.
  // Returns false when the directory cannot be opened, and then leaves the
  // previous catalog in place. A malformed manifest is skipped.
  bool Scan(const std::string& directory_utf8);

  // Copies |manifest| into the catalog. Returns false when id is empty,
  // name is empty, or id is already present.
  bool Register(const Manifest& manifest);

  const Manifest* Find(const std::string& id) const;

  std::vector<Manifest> List() const;

 private:
  std::vector<Manifest> records_;

  Registry(const Registry&);
  Registry& operator=(const Registry&);
};

}  // namespace plugin

#endif  // PLUGIN_REGISTRY_H_
