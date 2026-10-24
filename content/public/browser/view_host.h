// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_BROWSER_VIEW_HOST_H_
#define CONTENT_PUBLIC_BROWSER_VIEW_HOST_H_

#include <string>

#include "content/public/browser/trace_log.h"
#include "content/public/browser/types.h"

namespace content {

// Which child renderer a view slot may attach. MapLibre and UE are named so
// the shell can ask for them; this slice only creates a FlyCube child.
enum ViewBackend {
  kViewBackendFlyCube = 1,
  kViewBackendMapLibre = 2,
  kViewBackendUe = 3,
};

// Shell-supplied child window for one backend. Pointers are not owned.
// |create| returns the child HWND, or NULL on failure.
typedef void* (*ViewChildCreateFn)(void* parent_hwnd,
                                   int width,
                                   int height,
                                   void* user);
typedef void (*ViewChildResizeFn)(void* child, int width, int height, void* user);
typedef void (*ViewChildDestroyFn)(void* child, void* user);
typedef void (*ViewChildPresentFn)(void* child, void* user);

struct ViewBackendHooks {
  ViewBackendHooks();

  ViewChildCreateFn create;
  ViewChildResizeFn resize;
  ViewChildDestroyFn destroy;
  ViewChildPresentFn present;
  void* user;
};

// Parent HWND in, child HWND out. The shell repositions the child.
// This type does not know views::Widget. Main thread only.
class ViewHost {
 public:
  class Delegate {
   public:
    virtual ~Delegate();
    // |event| already happened on the child. Return whether the router
    // consumed it. The host does not publish EventBus itself.
    virtual bool OnChildGesture(const InputEvent& event) = 0;

   protected:
    Delegate();

   private:
    Delegate(const Delegate&) = delete;
    Delegate& operator=(const Delegate&) = delete;
  };

  ViewHost();
  ~ViewHost();

  void set_delegate(Delegate* delegate);
  void set_trace(TraceLog* trace);

  // Replaces hooks for |backend|. Empty hooks clear the backend.
  // FlyCube without hooks still attaches a labeled placeholder child.
  static void SetBackendHooks(ViewBackend backend, const ViewBackendHooks& hooks);

  // Snapshot of hooks last set for |backend|. Empty when unset or out of range.
  static ViewBackendHooks GetBackendHooks(ViewBackend backend);

  // Non-NULL only while Attach is inside a backend |create| callback.
  static ViewHost* AttachingHost();

  // |parent_hwnd| is the aura window. |width| and |height| are pixels.
  bool Attach(void* parent_hwnd,
              const std::string& view_id,
              ViewBackend backend,
              int width,
              int height);
  void Resize(int width, int height);
  void Present();
  void Detach();

  // Forwards a sample that the child already produced (FlyCube hook path).
  bool NotifyGesture(const InputEvent& event);

  void* child_hwnd() const;
  ViewBackend backend() const;
  const std::string& view_id() const;

 private:
  ViewHost(const ViewHost&) = delete;
  ViewHost& operator=(const ViewHost&) = delete;

  Delegate* delegate_;
  TraceLog* trace_;
  void* child_;
  ViewBackend backend_;
  std::string view_id_;
  bool factory_owned_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_VIEW_HOST_H_
