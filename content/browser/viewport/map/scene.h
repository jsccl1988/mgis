// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_MAP_SCENE_H_
#define CONTENT_BROWSER_VIEWPORT_MAP_SCENE_H_

#include <string>

#include "content/public/browser/scene_mode.h"
#include "content/public/browser/types.h"

namespace content {

class TraceLog;

// Mode, camera host, pick surface, and optional product presenters.
// session_wire links FakeScene. Product shells may register SceneImpl.
class Scene {
 public:
  virtual ~Scene() {}

  // Creates the scene child (or presenter children) under |parent_hwnd|.
  // Only kViewIdScene succeeds.
  virtual bool Attach(void* parent_hwnd,
                      const std::string& view_id,
                      int width,
                      int height,
                      TraceLog* trace) = 0;

  // Active-mode child after Attach. NULL when not attached.
  virtual void* child_hwnd() const = 0;

  // Child for |mode|. FakeScene returns the same HWND for every mode.
  virtual void* child_hwnd_for(SceneMode mode) const = 0;

  virtual void SetSceneMode(SceneMode mode, TraceLog* trace) = 0;
  virtual SceneMode scene_mode() const = 0;

  // Primitive mark for the current mode, or empty when nothing is hit.
  virtual std::string Pick(int x, int y) const = 0;

  // Layer ids registered on this scene (e.g. basemap, groundwater).
  virtual bool HasLayer(const std::string& id) const = 0;

  // Feature id that Pick would return for the current mode (m0 / g0).
  virtual bool HasEntity(const std::string& id) const = 0;

  virtual void Resize(int width, int height) {}
  virtual void Present() {}

  virtual void ArmTrackball() {}
  virtual void NotePointer(const InputEvent& event) {}

  virtual bool ExpectCamera(bool has_target,
                            double lon,
                            double lat,
                            double height,
                            std::string* error) const = 0;

  virtual bool ActivateTool(const std::string& tool_id,
                            const std::string& view_id) = 0;

  virtual bool OnPointer(const InputEvent& event, std::string* mark) = 0;

  virtual bool Commit(std::string* mark) { return false; }

  // Product HWND callbacks forward through this into GisContents::OnPointer.
  typedef bool (*InputForwardFn)(void* context, const InputEvent& event);
  // Inline default (same as Resize/Present). An out-of-line stub in
  // fake_scene.cc duplicated when SceneImpl also emitted Scene's vtable
  // (LNK2005 Scene::SetInputForwarder).
  virtual void SetInputForwarder(InputForwardFn fn, void* context) {
    (void)fn;
    (void)context;
  }
};

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_MAP_SCENE_H_
