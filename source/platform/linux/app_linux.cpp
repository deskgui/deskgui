/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "app_handler_linux.h"
#include "deskgui/app.h"

using namespace deskgui;

void App::run() {
  if (isRunning_.load()) {
    return;
  }
  isRunning_.store(true);

  mainThreadId_ = std::this_thread::get_id();
  gtk_main();
}

void App::terminate() {
  if (!isMainThread()) {
    return runOnMainThread([=]() { terminate(); });
  }

  if (isRunning_.load()) {
    isRunning_.store(false);
    gtk_main_quit();
  }
}
