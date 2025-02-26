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

  MSG msg = {};
  while (isRunning_.load()) {
    WaitMessage();

    // Process messages
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        App::terminate();
        break;
      };
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

void App::terminate() {
  if (!isMainThread()) {
    return runOnMainThread([this]() { terminate(); });
  }
  isRunning_.store(false);
}
