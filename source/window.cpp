/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "interfaces/window_impl.h"
#include "utils/dispatch.h"

using namespace deskgui;

Webview* Window::Impl::createWebview(const std::string& name, const WebviewOptions& options) {
  try {
    auto result = webviews_.emplace(
        name, std::unique_ptr<Webview>(new Webview(name, appHandler_, getContentView(), options)));
    if (result.second) {
      return result.first->second.get();
    }
    return nullptr;
  } catch ([[maybe_unused]] const std::exception& e) {
    return nullptr;
  }
}

void Window::Impl::destroyWebview(const std::string& name) { webviews_.erase(name); }

Webview* Window::Impl::getWebview(const std::string& name) const {
  if (auto it = webviews_.find(name); it != webviews_.end()) {
    return it->second.get();
  } else {
    return nullptr;
  }
}

Webview* Window::createWebview(const std::string& name, const WebviewOptions& options) {
  return utils::dispatch<&Impl::createWebview>(impl_, name, options);
}

void Window::destroyWebview(const std::string& name) {
  utils::dispatch<&Impl::destroyWebview>(impl_, name);
}

Webview* Window::getWebview(const std::string& name) const {
  return utils::dispatch<&Impl::getWebview>(impl_, name);
}

Window::Window(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : impl_(std::make_shared<Impl>(name, appHandler, nativeWindow)), events_(&impl_->events()) {}

Window::~Window() = default;

std::string Window::getName() const { return utils::dispatch<&Impl::getName>(impl_); }

// Title methods
void Window::setTitle(const std::string& title) { utils::dispatch<&Impl::setTitle>(impl_, title); }

std::string Window::getTitle() const { return utils::dispatch<&Impl::getTitle>(impl_); }

// Size methods
void Window::setSize(const ViewSize& size, PixelsType type) {
  utils::dispatch<&Impl::setSize>(impl_, size, type);
}

ViewSize Window::getSize(PixelsType type) const {
  return utils::dispatch<&Impl::getSize>(impl_, type);
}

void Window::setMaxSize(const ViewSize& size, PixelsType type) {
  utils::dispatch<&Impl::setMaxSize>(impl_, size, type);
}

ViewSize Window::getMaxSize(PixelsType type) const {
  return utils::dispatch<&Impl::getMaxSize>(impl_, type);
}

void Window::setMinSize(const ViewSize& size, PixelsType type) {
  utils::dispatch<&Impl::setMinSize>(impl_, size, type);
}

ViewSize Window::getMinSize(PixelsType type) const {
  return utils::dispatch<&Impl::getMinSize>(impl_, type);
}

// Position methods
void Window::setPosition(const ViewRect& position, PixelsType type) {
  utils::dispatch<&Impl::setPosition>(impl_, position, type);
}

ViewRect Window::getPosition(PixelsType type) const {
  return utils::dispatch<&Impl::getPosition>(impl_, type);
}

// Behavior methods
void Window::setResizable(bool resizable) {
  utils::dispatch<&Impl::setResizable>(impl_, resizable);
}

bool Window::isResizable() const { return utils::dispatch<&Impl::isResizable>(impl_); }

void Window::setDecorations(bool decorations) {
  utils::dispatch<&Impl::setDecorations>(impl_, decorations);
}

bool Window::isDecorated() const { return utils::dispatch<&Impl::isDecorated>(impl_); }

// Visibility methods
void Window::hide() { utils::dispatch<&Impl::hide>(impl_); }

void Window::show() { utils::dispatch<&Impl::show>(impl_); }

void Window::center() { utils::dispatch<&Impl::center>(impl_); }

void Window::enable(bool state) { utils::dispatch<&Impl::enable>(impl_, state); }

void Window::close() { utils::dispatch<&Impl::close>(impl_); }

// Background methods
void Window::setBackgroundColor(int red, int green, int blue) {
  utils::dispatch<&Impl::setBackgroundColor>(impl_, red, green, blue);
}

void Window::setTitleBarColor(int red, int green, int blue) {
  utils::dispatch<&Impl::setTitleBarColor>(impl_, red, green, blue);
}

SystemTheme Window::getSystemTheme() const {
  return utils::dispatch<&Impl::getSystemTheme>(impl_);
}

// Handle methods
void* Window::getNativeWindow() const { return utils::dispatch<&Impl::getNativeWindow>(impl_); }

void* Window::getContentView() const { return utils::dispatch<&Impl::getContentView>(impl_); }

// Monitor scale factor methods
void Window::setMonitorScaleFactor(float scaleFactor) {
  utils::dispatch<&Impl::setMonitorScaleFactor>(impl_, scaleFactor);
}

float Window::getMonitorScaleFactor() const {
  return utils::dispatch<&Impl::getMonitorScaleFactor>(impl_);
}