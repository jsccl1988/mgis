// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/main_runner.h"

#include "base/path/base_paths.h"
#include "gui/base/thread/message_loop.h"
#include "gui/base/thread/thread_manager.h"
#include "gui/content_impl/app/task_runner_impl.h"
#include "gui/content_impl/manager/map_content_manager.h"

namespace gui {
static const std::string kWorkerThread = "Worker";

MainRunner::MainRunner() {}
MainRunner::~MainRunner() {}

bool MainRunner::Init() {
  // thread
  if (!CreateThreads()) {
    return false;
  }

  task_runner_ = new TaskRunnerImpl();
  return gui::MapContentManager::Instance()->Init(task_runner_->AsWeakPtr());
}

bool MainRunner::Run() {
  gui::MapContentManager::Instance()->Run();
  return true;
}

bool MainRunner::Stop() {
  if (!DestroyThreads()) {
    return false;
  }

  gui::MapContentManager::Instance()->Destroy();

  return true;
}

bool MainRunner::CreateThreads() {
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
  ThreadManager::RemoveAllMessageLoop();
  worker_thread_.reset(nullptr);

  return true;
}
}  // namespace gui
