// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/main_runner.h"

#include "gui/base/path/base_paths.h"
#include "gui/base/thread/message_loop.h"
#include "gui/base/thread/thread_manager.h"


namespace gui {
static const std::string kWorkerThread = "Worker";

MainRunner::MainRunner() {}
MainRunner::~MainRunner() { Stop(); }

bool MainRunner::Init() {
  if (ui_message_loop_) {
    return true;
  }

  // thread
  if (!CreateThreads()) {
    return false;
  }

  bool result = false;
  // web render
  content::MapContent* content = NULL;
  MapContentFactory factory;
  task_runner_ = new TaskRunnerImpl();
  if (factory.CreateMapContent(&content) && content) {
    result = gui::MapContentManager::Instance()->Init(this, content,
                                                     task_runner_->AsWeakPtr());
  }

  return true;
}

bool MainRunner::Run() {
  if (!ui_message_loop_) {
    return false;
  }

  ui_message_loop_->Run();

  return true;
}

bool MainRunner::Stop() {
  if (!ui_message_loop_) {
    return false;
  }

  if (process_singleton_) {
    process_singleton_->Cleanup();
    process_singleton_ = NULL;
  }

  if (!DestroyThreads()) {
    return false;
  }

  return true;
}

bool MainRunner::InitProcessSingleton() {
  string16 user_data_dir;
  if (!PathProvider(gui::DIR_APP_DATA_HBGUI, &user_data_dir)) return false;
  process_singleton_ = new ProcessSingleton(this, user_data_dir);

  // When another process is running, use that process instead of starting a
  // new one. NotifyOtherProcess will currently give the other process up to
  // 20 seconds to respond. Note that this needs to be done before we attempt
  // to read the profile.
  notify_result_ = process_singleton_->NotifyOtherProcessOrCreate();
  switch (notify_result_) {
    case ProcessSingleton::PROCESS_NONE:
      // No process already running, fall through to starting a new one.
      return true;

    case ProcessSingleton::PROCESS_NOTIFIED:
      return false;

    case ProcessSingleton::PROFILE_IN_USE:
      return false;

    case ProcessSingleton::LOCK_ERROR:
      return false;
    default:
      break;
  }

  return true;
}

bool MainRunner::CreateThreads() {
  // ui message loop
  ui_message_loop_.reset(new MessageLoop(::GetCurrentThreadId()));
  ThreadManager::SetMessageLoop(ThreadManager::UI, ui_message_loop_.get());

  // worker thread
  worker_thread_.reset(new gui::Thread());
  gui::Thread::Options io_opt(0, true);
  io_opt.thread_name_ = kWorkerThread.c_str();
  worker_thread_->StartWithOptions(io_opt);
  ThreadManager::SetMessageLoop(ThreadManager::WORKER,
                                worker_thread_->GetMessageLoop());

  return true;
}

bool MainRunner::DestroyThreads() {
  ui_message_loop_->Quit();
  ThreadManager::RemoveAllMessageLoop();
  ui_message_loop_.reset(NULL);
  worker_thread_.reset(NULL);

  return true;
}

// Inherit from ProcessSingleton::Delegate
bool MainRunner::OnNotify(const gui::CommandLine& command_line,
                          const string16& current_directory) {
  uint32 new_id = kUint32Max;
  if (!Browser::Instance()->ProcessCommand(command_line)) {
    Browser::Instance()->NewWindow(kUint32Max, L"https://www.baidu.com",
                                   &new_id);
  }

  return false;
}
}  // namespace gui
