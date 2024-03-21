/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "app_handler_windows.h"
#include "deskgui/app.h"

using namespace deskgui;

void App::run() {
  if (isRunning_.load()) {
    return;
  }
  isRunning_.store(true);

  mainThreadId_ = std::this_thread::get_id();
  eventHandle_ = CreateEvent(NULL, FALSE, FALSE, NULL);

  MSG msg = {};
  while (isRunning_.load()) {
    MsgWaitForMultipleObjects(1, &eventHandle_, FALSE, INFINITE, QS_ALLINPUT);

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    MainThreadTask task;
    while (queue.pop(task)) {
      task();
    }
  }
}

void App::terminate() {
  if (!isMainThread()) {
    return runOnMainThread([=]() { terminate(); });
  }
  isRunning_.store(false);
}
