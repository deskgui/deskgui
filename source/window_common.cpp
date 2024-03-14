/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "deskgui/window.h"

#ifdef _WIN32  // Windows
#  include "platform/windows/app_handler_windows.h"
#elif __APPLE__  // macOS (Darwin)
#  include "platform/darwin/app_handler_darwin.h"
#elif __linux__  // Linux
#  include "platform/linux/app_handler_linux.h"
#else
#  error "Unsupported operating system"
#endif

#include <system_error>

using namespace deskgui;

Webview* Window::createWebview(const std::string& name, const WebViewOptions& options) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return createWebview(name, options); });
  }
  std::lock_guard<std::mutex> lock(webviewsMutex_);

  try {
    auto result = webviews_.emplace(
        name, std::unique_ptr<Webview>(new Webview(name, appHandler_, getNativeWindow(), options)));
    if (result.second) {
      auto& webview = result.first->second;
      return webview.get();
    }
    return nullptr;
  } catch ([[maybe_unused]] const std::exception& e) {
    return nullptr;
  }
}

void Window::destroyWebview(const std::string& name) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { destroyWebview(name); });
  }
  std::lock_guard<std::mutex> lock(webviewsMutex_);

  auto it = webviews_.find(name);
  if (it != webviews_.end()) {
    webviews_.erase(it);
  }
}

Webview* Window::getWebview(const std::string& name) const {
  std::lock_guard<std::mutex> lock(webviewsMutex_);
  auto it = webviews_.find(name);
  if (it != webviews_.end()) {
    return it->second.get();
  } else {
    return nullptr;
  }
}

[[nodiscard]] ViewSize Window::getMaxSize() const { return maxSize_; }
[[nodiscard]] ViewSize Window::getMinSize() const { return minSize_; }
