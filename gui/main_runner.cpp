// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/main_runner.h"

#include "base/path/base_paths.h"
#include "content/control/map_box/map_view_impl.h"
#include "gui/base/thread/message_loop.h"
#include "gui/base/thread/thread_manager.h"
#include "gui/content_impl/app/task_runner_impl.h"
#include "gui/content_impl/map_content_factory.h"
#include "gui/content_impl/map_content_manager.h"

CAppModule _Module;

namespace gui {
static const std::string kWorkerThread = "Worker";

MainRunner::MainRunner() {}
MainRunner::~MainRunner() {}

bool MainRunner::Init() {
  _Module.Init(NULL, ::GetModuleHandle(NULL));
  _Module.AddMessageLoop(&ui_message_loop_);

  // thread
  if (!CreateThreads()) {
    return false;
  }

  bool result = false;
  content::MapContent* content = nullptr;
  MapContentFactory factory;
  task_runner_ = new TaskRunnerImpl();
  if (factory.CreateMapContent(&content) && content) {
    result = gui::MapContentManager::Instance()->Init(
        this, content, task_runner_->AsWeakPtr());
  }

  content::MapView::CreateParams params;
  content::MapView::Preferences preferences;
  params.parent_hwnd = NULL;
  map_view_ = reinterpret_cast<content::MapView*>(new content::MapViewImpl());
  map_view_->Create(params, preferences);

  return true;
}

bool MainRunner::Run() {
  ui_message_loop_.Run();
  return true;
}

bool MainRunner::Stop() {
  if (!DestroyThreads()) {
    return false;
  }

  _Module.RemoveMessageLoop();
  _Module.Term();

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
