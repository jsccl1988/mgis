// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef PLUGIN_API_MVIEW_PLUGIN_API_H_
#define PLUGIN_API_MVIEW_PLUGIN_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Stable C ABI version 1. Plugin DLLs include only this header. */

#define MVIEW_PLUGIN_API_VERSION 1

#if defined(MVIEW_PLUGIN_IMPLEMENTATION) && defined(_WIN32)
#define MVIEW_PLUGIN_EXPORT __declspec(dllexport)
#else
#define MVIEW_PLUGIN_EXPORT
#endif

typedef struct MviewPluginInputEvent {
  int kind;   /* 0 mouse, 1 wheel, 2 key */
  int action; /* 0 none, 1 move, 2 press, 3 release */
  double x_px;
  double y_px;
  double wheel_dy;
  int modifiers;
} MviewPluginInputEvent;

typedef struct MviewPluginInteraction {
  void* user;
  const char* (*id)(void* user);
  int (*on_input)(void* user, const MviewPluginInputEvent* event);
  void (*destroy)(void* user);
} MviewPluginInteraction;

typedef MviewPluginInteraction* (*MviewPluginCreateInteractionFn)(void* user_data);

typedef struct MviewPluginApi {
  int version;
  void* host;
  int (*contribute_command)(void* host, const char* command_id);
  int (*contribute_interaction_factory)(void* host,
                                        const char* factory_id,
                                        MviewPluginCreateInteractionFn create,
                                        void* user_data);
  int (*contribute_menu_item)(void* host,
                              const char* menu_id,
                              const char* label_utf8,
                              const char* command_id);
  int (*contribute_dock_panel)(void* host,
                               const char* panel_id,
                               const char* title_utf8);
  int (*run_processing)(void* host,
                        const char* algorithm_id,
                        char* error,
                        int error_cap);
} MviewPluginApi;

/* Sole required DLL export. Returns 0 on success. */
MVIEW_PLUGIN_EXPORT int mview_plugin_contribute(MviewPluginApi* api);

/* Optional: host may call this to run a contributed command. Returns 0 on
 * success. Sample implements writing the harness mark. */
MVIEW_PLUGIN_EXPORT int mview_plugin_execute(const char* command_id);

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_API_MVIEW_PLUGIN_API_H_ */
