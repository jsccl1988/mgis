// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/app/main_runner.h"

#include "gui/app/frame.h"
#include "gui/base/path/base_paths.h"
#include "gui/base/thread/message_loop.h"
#include "gui/base/thread/thread_manager.h"
#include "gui/content_impl/app/task_runner_impl.h"
#include "gui/content_impl/map_content_factory.h"
#include "gui/content_impl/map_content_manager.h"


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
  content::MapContent* content = nullptr;
  MapContentFactory factory;
  task_runner_ = new TaskRunnerImpl();
  if (factory.CreateMapContent(&content) && content) {
    result = gui::MapContentManager::Instance()->Init(
        this, content, task_runner_->AsWeakPtr());
  }

  Frame* frame = new Frame();
  frame->CreateEx();
  frame->ShowWindow(SW_MAXIMIZE);
  frame->Show();

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

  if (!DestroyThreads()) {
    return false;
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
  ui_message_loop_.reset(nullptr);
  worker_thread_.reset(nullptr);

  return true;
}
}  // namespace gui
