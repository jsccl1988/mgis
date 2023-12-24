// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/base/thread/message_loop.h"

#include "gui/base/thread/thread_local_pointer.h"
#include "gui/base/thread/thread_manager.h"

namespace gui {
const wchar_t kWndClass[] = L"MessagePumpWindow";
#define WM_TASK (WM_USER + 1001)
#define WM_QUIT_MESSAGELOOP (WM_USER + 1002)

class Task {
 public:
  explicit Task(const std::function<void(void)>& handler) : handler_(handler) {}

 public:
  void Run() {
    if (handler_) {
      handler_();
    }
  }

  void Destroy() { delete this; }

 protected:
  std::function<void(void)> handler_;
};

ThreadLocalPointer<MessageLoop> g_tls_message_loop;
MessageLoop::MessageLoop(ThreadId thread_id) : thread_id_(thread_id) {
  g_tls_message_loop.Set(this);
  InitMessageWnd();
}

MessageLoop::~MessageLoop() {
  DestroyWindow(message_wnd_);
  UnregisterClass(kWndClass, GetModuleHandle(NULL));
  ThreadManager::RemoveMessageLoop(thread_id_);
}

void MessageLoop::PostTask(const std::function<void(void)>& handler,
                           unsigned int delay) {
  PostMessage(message_wnd_, WM_TASK, (WPARAM)(new Task(handler)),
              (LPARAM)delay);
}

void MessageLoop::Quit() {
  PostMessage(message_wnd_, WM_QUIT_MESSAGELOOP, NULL, NULL);
}

void MessageLoop::Run() {
  MSG msg;
  BOOL result;

  while (result = GetMessage(&msg, NULL, 0, 0)) {
    if (result == -1) {
      continue;
    }

    if (msg.message == WM_QUIT_MESSAGELOOP) {
      break;
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

MessageLoop* MessageLoop::Current() { return g_tls_message_loop.Get(); }

void MessageLoop::InitMessageWnd() {
  HINSTANCE hinst = GetModuleHandle(NULL);

  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = WndProcThunk;
  wc.hInstance = hinst;
  wc.lpszClassName = kWndClass;
  RegisterClassEx(&wc);

  message_wnd_ =
      CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hinst, 0);
}

// static
LRESULT CALLBACK MessageLoop::WndProcThunk(HWND hwnd, UINT message,
                                           WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_TASK: {
      Task* task = reinterpret_cast<Task*>(wparam);
      unsigned int delay = (unsigned int)lparam;
      task->Run();
      task->Destroy();
    } break;
  }

  return DefWindowProc(hwnd, message, wparam, lparam);
}
}  // namespace gui
