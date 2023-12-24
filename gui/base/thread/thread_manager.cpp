// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/base/thread/thread_manager.h"

#include <map>

namespace gui {
// static
base::Lock ThreadManager::callers_lock_;
std::map<ThreadManager::ID, MessageLoop *> ThreadManager::id_2_message_loop_;

ThreadManager::ThreadManager() {}

ThreadManager::~ThreadManager() {}

// static
bool ThreadManager::SetMessageLoop(ID identifier, MessageLoop *message_loop) {
  MessageLoop *temp;
  if (GetMessageLoop(identifier, &temp) && temp) return false;

  base::AutoLock lock(&callers_lock_);
  id_2_message_loop_[identifier] = message_loop;

  return true;
}

void ThreadManager::RemoveMessageLoop(ID identifier) {
  base::AutoLock lock(&callers_lock_);
  id_2_message_loop_.erase(identifier);
}

void ThreadManager::RemoveMessageLoop(ThreadId thread_id) {
  base::AutoLock lock(&callers_lock_);
  std::map<ID, MessageLoop *>::iterator iter = id_2_message_loop_.begin();
  while (iter != id_2_message_loop_.end()) {
    if (iter->second && iter->second->GetThreadID() == thread_id) {
      id_2_message_loop_.erase(iter);
      break;
    }
  }
}

void ThreadManager::RemoveAllMessageLoop() {
  base::AutoLock lock(&callers_lock_);
  id_2_message_loop_.clear();
}

bool ThreadManager::CurrentlyOn(ID identifier) {
  MessageLoop *message_loop = nullptr;
  GetMessageLoop(identifier, &message_loop);
  if (message_loop) {
    return message_loop->GetThreadID() == ::GetCurrentThreadId();
  }

  return false;
}

bool ThreadManager::GetMessageLoop(ID identifier, MessageLoop **message_loop) {
  base::AutoLock lock(&callers_lock_);
  std::map<ID, MessageLoop *>::iterator iter = id_2_message_loop_.end();

  iter = id_2_message_loop_.find(identifier);
  if (iter != id_2_message_loop_.end()) {
    *message_loop = iter->second;
    return true;
  }

  return false;
}

bool ThreadManager::PostTask(ID identifier,
                             const std::function<void(void)> &handler,
                             int64_t delay) {
  MessageLoop *message_loop;
  if (!GetMessageLoop(identifier, &message_loop) || !message_loop) return false;

  message_loop->PostTask(handler, delay);

  return true;
}
}  // namespace gui
