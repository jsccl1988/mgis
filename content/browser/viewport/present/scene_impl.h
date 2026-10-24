// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_VIEWPORT_PRESENT_SCENE_IMPL_H_
#define CONTENT_BROWSER_VIEWPORT_PRESENT_SCENE_IMPL_H_

#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "content/browser/viewport/map/scene.h"

namespace content {

class ScenePresenter;

// Product scene: MapLibre (or mercator) for 2D; UE pipe then FlyCube for 3D.
class SceneImpl : public Scene {
 public:
  SceneImpl();
  ~SceneImpl() override;

  bool Attach(void* parent_hwnd,
              const std::string& view_id,
              int width,
              int height,
              TraceLog* trace) override;
  void* child_hwnd() const override;
  void* child_hwnd_for(SceneMode mode) const override;
  void SetSceneMode(SceneMode mode, TraceLog* trace) override;
  SceneMode scene_mode() const override;
  std::string Pick(int x, int y) const override;
  bool HasLayer(const std::string& id) const override;
  bool HasEntity(const std::string& id) const override;
  void Resize(int width, int height) override;
  void Present() override;
  void ArmTrackball() override;
  void NotePointer(const InputEvent& event) override;
  bool ExpectCamera(bool has_target,
                    double lon,
                    double lat,
                    double height,
                    std::string* error) const override;
  bool ActivateTool(const std::string& tool_id,
                    const std::string& view_id) override;
  bool OnPointer(const InputEvent& event, std::string* mark) override;
  bool Commit(std::string* mark) override;
  void SetInputForwarder(InputForwardFn fn, void* context) override;

 private:
  SceneImpl(const SceneImpl&) = delete;
  SceneImpl& operator=(const SceneImpl&) = delete;

  ScenePresenter* ActivePresenter() const;
  bool HasPrimitive(const std::string& id) const;
  void BindPresenterPointers();
  static void OnHwndPointer(void* user,
                            int kind,
                            int action,
                            double x,
                            double y,
                            double wheel_dx,
                            double wheel_dy);

  SceneMode mode_;
  scoped_ptr<ScenePresenter> presenter_2d_;
  scoped_ptr<ScenePresenter> presenter_3d_;
  std::vector<std::string> primitive_ids_;
  std::string tool_id_;
  int draft_points_;
  bool trackball_armed_;
  double orbit_delta_;
  int width_;
  int height_;
  InputForwardFn input_forward_;
  void* input_context_;
  bool pointer_down_;
  bool pointer_right_;
  double last_x_;
  double last_y_;
};

Scene* CreateProductScene();

}  // namespace content

#endif  // CONTENT_BROWSER_VIEWPORT_PRESENT_SCENE_IMPL_H_
