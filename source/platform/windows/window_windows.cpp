/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <system_error>

#include "app_handler_windows.h"
#include "window_windows_impl.h"

using namespace deskgui;

Window::Window(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : pImpl_{std::make_unique<Impl>()}, name_(name), appHandler_(appHandler) {
  if (nativeWindow == nullptr) {
    pImpl_->registerWindowClass();

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    pImpl_->window = CreateWindowEx(0,                    // Optional window styles.
                                    CLASS_NAME,           // Window class
                                    L"deskgui window",    // Window text
                                    WS_OVERLAPPEDWINDOW,  // Window style

                                    // Size and position
                                    kDefaultWindowRect.L, kDefaultWindowRect.T,
                                    kDefaultWindowRect.R - kDefaultWindowRect.L,
                                    kDefaultWindowRect.B - kDefaultWindowRect.T,

                                    nullptr,            // Parent window
                                    nullptr,            // Menu
                                    pImpl_->hInstance,  // Instance handle
                                    this                // Additional application data
    );

    if (!pImpl_->window) {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
    }
  } else {
    pImpl_->window = static_cast<HWND>(nativeWindow);
    SetWindowSubclass(pImpl_->window, &Impl::subclassProc, 1, reinterpret_cast<DWORD_PTR>(this));
  }
}

Window::~Window() {
  if (IsWindow(pImpl_->window)) {
    DestroyWindow(pImpl_->window);
    pImpl_->window = nullptr;
  }
}

void Window::setTitle(const std::string& title) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { setTitle(title); });
  }
  SetWindowText(pImpl_->window, s2ws(title).c_str());
}

[[nodiscard]] std::string Window::getTitle() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getTitle(); });
  }
  const int bufferSize = 256;
  WCHAR buffer[bufferSize];
  GetWindowText(pImpl_->window, buffer, bufferSize);

  return ws2s(buffer);
}

void Window::setSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { setSize(size); });
  }
  RECT windowRect;
  GetWindowRect(pImpl_->window, &windowRect);

  int width = size.first * pImpl_->displayScaleFactor_;
  int height = size.second * pImpl_->displayScaleFactor_;

  // Set window position and size
  SetWindowPos(pImpl_->window, nullptr, windowRect.right, windowRect.left, width, height,
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

[[nodiscard]] ViewSize Window::getSize() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getSize(); });
  }

  RECT rect;
  GetWindowRect(pImpl_->window, &rect);
  return ViewSize{rect.right - rect.left, rect.bottom - rect.top};
}

void Window::setPosition(const ViewRect& position) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { setPosition(position); });
  }
  RECT r;
  r.left = position.L * pImpl_->displayScaleFactor_;
  r.top = position.T * pImpl_->displayScaleFactor_;
  r.right = position.R * pImpl_->displayScaleFactor_;
  r.bottom = position.B * pImpl_->displayScaleFactor_;
  SetWindowPos(pImpl_->window, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top,
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

[[nodiscard]] ViewRect Window::getPosition() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getPosition(); });
  }
  RECT windowRect;
  GetWindowRect(pImpl_->window, &windowRect);
  return ViewRect{static_cast<size_t>(windowRect.left), static_cast<size_t>(windowRect.top),
                  static_cast<size_t>(windowRect.right), static_cast<size_t>(windowRect.bottom)};
}

void Window::setResizable(bool resizable) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { setResizable(resizable); });
  }

  auto currentStyle = GetWindowLong(pImpl_->window, GWL_STYLE);
  if (resizable) {
    currentStyle |= (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  } else {
    currentStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  }
  SetWindowLong(pImpl_->window, GWL_STYLE, currentStyle);
}

[[nodiscard]] bool Window::isResizable() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return isResizable(); });
  }

  auto currentStyle = GetWindowLong(pImpl_->window, GWL_STYLE);
  return (currentStyle & (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) != 0;
}

void Window::setDecorations(bool decorations) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { setDecorations(decorations); });
  }

  LONG style = GetWindowLong(pImpl_->window, GWL_STYLE);
  if (decorations) {
    style |= WS_OVERLAPPEDWINDOW;
  } else {
    style &= ~WS_OVERLAPPEDWINDOW;
  }
  SetWindowLong(pImpl_->window, GWL_STYLE, style);
}

[[nodiscard]] bool Window::isDecorated() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return isDecorated(); });
  }
  LONG style = GetWindowLong(pImpl_->window, GWL_STYLE);
  return (style & WS_OVERLAPPEDWINDOW) != 0;
}

void Window::hide() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=]() { hide(); });
  }
  ShowWindow(pImpl_->window, SW_HIDE);
}

void Window::show() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=]() { show(); });
  }
  ShowWindow(pImpl_->window, SW_SHOW);
}

void Window::center() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=]() { center(); });
  }
  // Get the screen resolution or work area size
  RECT screenRect;
  GetWindowRect(pImpl_->window, &screenRect);

  int xPos = (GetSystemMetrics(SM_CXSCREEN) - screenRect.right) / 2;
  int yPos = (GetSystemMetrics(SM_CYSCREEN) - screenRect.bottom) / 2;

  // Set the new window position
  SetWindowPos(pImpl_->window, NULL, xPos, yPos, 0, 0,
               SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->window); }

void Window::setMaxSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=]() { setMaxSize(size); });
  }
  maxSize_ = {size.first * pImpl_->displayScaleFactor_, size.second * pImpl_->displayScaleFactor_};

  LONG windowStyle = GetWindowLong(pImpl_->window, GWL_STYLE);
  windowStyle &= ~WS_MAXIMIZEBOX;
  SetWindowLong(pImpl_->window, GWL_STYLE, windowStyle);
}

void Window::setMinSize(const ViewSize& size) {
  minSize_ = {size.first * pImpl_->displayScaleFactor_, size.second * pImpl_->displayScaleFactor_};
}

float Window::getDisplayScaleFactor() { return pImpl_->displayScaleFactor_; }
