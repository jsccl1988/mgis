// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_SCRIPT_RUNTIME_H_
#define CONTENT_PUBLIC_SCRIPT_RUNTIME_H_

#include <string>
#include <vector>

#include "content/public/types.h"

namespace content {

// Gluing object for one suite run. The shell fills slots, then calls
// run_suite_script. P0 exposes the mark slot only. The mark list is also kept
// here so a caller can see what the suite wrote without installing a slot.
// The method bodies live under content/browser/glang.runtime/host.
class CapabilityHost {
 public:
  // |context| is an unowned embedder pointer passed back to |slot|.
  typedef void (*MarkSlot)(void* context, const std::string& name);
  typedef bool (*CommandSlot)(void* context,
                              const std::string& command_id,
                              const std::string& view_id);
  typedef bool (*PointerSlot)(void* context, const InputEvent& event);

  CapabilityHost();
  ~CapabilityHost();

  // Replaces the mark slot. |slot| may be NULL to record marks only.
  void set_mark_slot(MarkSlot slot, void* context);
  void set_command_slot(CommandSlot slot, void* context);
  void set_activate_slot(CommandSlot slot, void* context);
  void set_pointer_slot(PointerSlot slot, void* context);

  bool has_command_slot() const;
  bool has_activate_slot() const;
  bool has_pointer_slot() const;

  bool RunCommand(const std::string& command_id, const std::string& view_id);
  bool RunActivate(const std::string& command_id, const std::string& view_id);
  bool RunPointer(const InputEvent& event);

  // Appends |name| and, when a slot is set, invokes it. Empty names are
  // ignored so a bad caller cannot plant a blank mark.
  void mark(const std::string& name);

  bool has_mark(const std::string& name) const;
  const std::vector<std::string>& marks() const;

 private:
  CapabilityHost(const CapabilityHost&) = delete;
  CapabilityHost& operator=(const CapabilityHost&) = delete;

  MarkSlot mark_slot_;
  void* mark_context_;
  CommandSlot command_slot_;
  void* command_context_;
  CommandSlot activate_slot_;
  void* activate_context_;
  PointerSlot pointer_slot_;
  void* pointer_context_;
  std::vector<std::string> marks_;
};

// Loads the UTF-8 suite at |path_utf8| and applies it to |host|.
// The locked design runs one in-process CPython main(). The stand-in runner
// honors mark, command, activate_tool, and pointer. expect_* and capture fail
// closed. |error| may be NULL. On a parse failure no mark from this file is
// written. A slot that returns false does not roll back earlier marks.
bool run_suite_script(CapabilityHost& host,
                      const std::string& path_utf8,
                      std::string* error);

}  // namespace content

#endif  // CONTENT_PUBLIC_SCRIPT_RUNTIME_H_
