// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_IMPL_MAP_CONTENT_IMPL_H
#define CONTENT_CONTROL_IMPL_MAP_CONTENT_IMPL_H

#include "content/content.h"
#include "content/impl/app/process_runner_impl.h"
#include "content/impl/map_view_impl.h"
#include "content/public/app/process_runner.h"
#include "content/public/map_content.h"
#include "content/public/map_view.h"

namespace content {
class MapContentImpl : public MapContent {
 public:
  MapContentImpl() {}

  virtual bool GetVersion(std::string* version) {
    if (version) {
      *version = kVersion;
      return true;
    }

    return false;
  }

  virtual bool Create(const base::WeakPtr<MapContentClient>& map_content_client,
                      const base::WeakPtr<TaskRunner>& task_runner,
                      const MapContent::Preferences& preferences,
                      ProcessRunner** process_runner,
                      MapView** map_view) {
    map_content_client_ = map_content_client;
    task_runner_ = task_runner;
    preferences_ = preferences;

    if (process_runner) {
      *process_runner = new content::ProcessRunnerImpl();
      process_runner_ = *process_runner;
    }

    if (map_view) {
      *map_view = new content::MapViewImpl();
      map_view_ = *map_view;
    }

    return true;
  }
  virtual bool Destroy() {
    return true;
  }

  bool GetProcessRunner(ProcessRunner** process_runner) {
    if (process_runner) {
      *process_runner = process_runner_.get();
      return true;
    }

    return false;
  }

  bool GetMapView(MapView** map_view) {
    if (map_view) {
      *map_view = map_view_.get();
      return true;
    }

    return false;
  }

 private:
  MapContent::Preferences preferences_;
  base::WeakPtr<MapContentClient> map_content_client_;
  base::WeakPtr<TaskRunner> task_runner_;

  base::ScopedRefPtr<content::MapView> map_view_;
  base::ScopedRefPtr<content::ProcessRunner> process_runner_;

 protected:
  friend class base::RefCountedThreadSafe<MapContentImpl>;
  virtual ~MapContentImpl() {}
};
}  // namespace content

#endif  // CONTENT_CONTROL_IMPL_MAP_CONTENT_IMPL_H
