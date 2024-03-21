/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "deskgui/app.h"

#ifdef _WIN32
#  include "platform/windows/app_handler_windows.h"
#elif __APPLE__
#  include "platform/darwin/app_handler_darwin.h"
#elif __linux__
#  include "platform/linux/app_handler_linux.h"
#else
#  error "Unsupported operating system"
#endif

#include <exception>

using namespace deskgui;

Window* App::createWindow(const std::string& name, void* nativeWindow) {
  if (!isMainThread()) {
    return runOnMainThread([=]() { return createWindow(name, nativeWindow); });
  }
  std::lock_guard<std::mutex> lock(windowsMutex_);
  try {
    auto result = windows_.emplace(
        name, std::unique_ptr<Window>(new Window(name, getHandler(), nativeWindow)));
    if (result.second) {
      openedWindows_.fetch_add(1);
      auto& window = result.first->second;
      return window.get();
    }
    return nullptr;

  } catch ([[maybe_unused]] const std::exception& e) {
    return nullptr;
  }
}

void App::destroyWindow(const std::string& name) {
  if (!isMainThread()) {
    return runOnMainThread([=]() { destroyWindow(name); });
  }
  std::lock_guard<std::mutex> lock(windowsMutex_);

  auto it = windows_.find(name);
  if (it != windows_.end()) {
    openedWindows_.fetch_sub(1);
    windows_.erase(it);
  }

  if (openedWindows_ == 0) {
    // Terminate the application when there are no more windows
    terminate();
  }
}

Window* App::getWindow(const std::string& name) const {
  std::lock_guard<std::mutex> lock(windowsMutex_);
  auto it = windows_.find(name);
  if (it != windows_.end()) {
    return it->second.get();
  } else {
    return nullptr;
  }
}

void App::notifyWindowClosedFromUI(const std::string& name) { destroyWindow(name); }
