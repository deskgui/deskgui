/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <system_error>

#include "app_handler_windows.h"
#include "utils/strings.h"
#include "window_windows_impl.h"


using namespace deskgui;
using namespace deskgui::utils;
Window::Window(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : name_(name), pImpl_{std::make_unique<Impl>()}, appHandler_(appHandler) {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

  if (nativeWindow == nullptr) {
    pImpl_->registerWindowClass();

    pImpl_->windowHandle = CreateWindowEx(0,                    // Optional window styles.
                                          CLASS_NAME,           // Window class
                                          L"deskgui window",    // Window text
                                          WS_OVERLAPPEDWINDOW,  // Window style

                                          // Size and position
                                          kDefaultWindowRect.L, kDefaultWindowRect.T,
                                          kDefaultWindowRect.R - kDefaultWindowRect.L,
                                          kDefaultWindowRect.B - kDefaultWindowRect.T,

                                          nullptr,                  // Parent window
                                          nullptr,                  // Menu
                                          Window::Impl::hInstance,  // Instance handle
                                          this                      // Additional application data
    );

    if (!pImpl_->windowHandle) {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
    }
  } else {
    isExternalWindow_ = true;
    pImpl_->windowHandle = static_cast<HWND>(nativeWindow);
    SetWindowSubclass(pImpl_->windowHandle, &Impl::subclassProc, 1,
                      reinterpret_cast<DWORD_PTR>(this));
  }

  setMonitorScaleFactor(pImpl_->computeDpiScale(pImpl_->windowHandle));
}

Window::~Window() {
  if (IsWindow(pImpl_->windowHandle)) {
    if (!isExternalWindow_) {
      DestroyWindow(pImpl_->windowHandle);
    } else {
      RemoveWindowSubclass(pImpl_->windowHandle, &Impl::subclassProc, 1);
    }
    pImpl_->windowHandle = nullptr;
  }
}

void Window::setTitle(const std::string& title) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, title] { setTitle(title); });
  }
  SetWindowText(pImpl_->windowHandle, s2ws(title).c_str());
}

[[nodiscard]] std::string Window::getTitle() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { return getTitle(); });
  }
  const int bufferSize = 256;
  WCHAR buffer[bufferSize];
  GetWindowText(pImpl_->windowHandle, buffer, bufferSize);

  return ws2s(buffer);
}

void Window::setSize(const ViewSize& size, PixelsType type) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, size, type] { setSize(size, type); });
  }
  auto modifiedSize = size;
  if (type == PixelsType::kLogical) {
    modifiedSize = ViewSize{size.first * monitorScaleFactor_, size.second * monitorScaleFactor_};
  }

  RECT windowRect = {0, 0, modifiedSize.first, modifiedSize.second};
  AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, 0);

  int windowWidth = windowRect.right - windowRect.left;
  int windowHeight = windowRect.bottom - windowRect.top;
  SetWindowPos(pImpl_->windowHandle, nullptr, windowRect.left, windowRect.top, windowWidth,
               windowHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

[[nodiscard]] ViewSize Window::getSize(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, type] { return getSize(type); });
  }

  RECT rect;
  GetClientRect(pImpl_->windowHandle, &rect);
  auto size = ViewSize{rect.right - rect.left, rect.bottom - rect.top};

  if (type == PixelsType::kLogical) {
    size.first /= monitorScaleFactor_;
    size.second /= monitorScaleFactor_;
  }
  return size;
}

void Window::setMaxSize(const ViewSize& size, PixelsType type) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, size, type]() { setMaxSize(size, type); });
  }
  ViewSize adjustedSize = size;
  if (type == PixelsType::kLogical) {
    adjustedSize.first *= monitorScaleFactor_;
    adjustedSize.second *= monitorScaleFactor_;
  }

  maxSize_ = adjustedSize;
  maxSizeDefined_ = true;

  LONG windowStyle = GetWindowLong(pImpl_->windowHandle, GWL_STYLE);
  windowStyle &= ~WS_MAXIMIZEBOX;
  SetWindowLong(pImpl_->windowHandle, GWL_STYLE, windowStyle);
}

[[nodiscard]] ViewSize Window::getMaxSize(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, type]() { return getMaxSize(type); });
  }
  if (type == PixelsType::kLogical) {
    return ViewSize{maxSize_.first / monitorScaleFactor_, maxSize_.second / monitorScaleFactor_};
  } else {
    return maxSize_;
  }
}

void Window::setMinSize(const ViewSize& size, PixelsType type) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, size, type]() { setMinSize(size, type); });
  }
  ViewSize adjustedSize = size;
  if (type == PixelsType::kLogical) {
    adjustedSize.first *= monitorScaleFactor_;
    adjustedSize.second *= monitorScaleFactor_;
  }

  minSize_ = adjustedSize;
  minSizeDefined_ = true;
}

[[nodiscard]] ViewSize Window::getMinSize(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, type]() { return getMinSize(type); });
  }
  if (type == PixelsType::kLogical) {
    return ViewSize{minSize_.first / monitorScaleFactor_, minSize_.second / monitorScaleFactor_};
  } else {
    return minSize_;
  }
}

void Window::setPosition(const ViewRect& position, PixelsType type) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, position, type] { setPosition(position, type); });
  }
  RECT r;
  r.left = position.L;
  r.top = position.T;
  r.right = position.R;
  r.bottom = position.B;

  if (type == PixelsType::kLogical) {
    r.left *= monitorScaleFactor_;
    r.top *= monitorScaleFactor_;
    r.right *= monitorScaleFactor_;
    r.bottom *= monitorScaleFactor_;
  }
  SetWindowPos(pImpl_->windowHandle, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top,
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

[[nodiscard]] ViewRect Window::getPosition(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    // Run on main thread if not already on it
    return appHandler_->runOnMainThread([this, type] { return getPosition(type); });
  }

  RECT windowRect;
  GetWindowRect(pImpl_->windowHandle, &windowRect);

  ViewRect position{static_cast<size_t>(windowRect.left), static_cast<size_t>(windowRect.top),
                    static_cast<size_t>(windowRect.right), static_cast<size_t>(windowRect.bottom)};

  if (type == PixelsType::kLogical) {
    position.L /= monitorScaleFactor_;
    position.T /= monitorScaleFactor_;
    position.R /= monitorScaleFactor_;
    position.B /= monitorScaleFactor_;
  }

  return position;
}

void Window::setResizable(bool resizable) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, resizable] { setResizable(resizable); });
  }

  auto currentStyle = GetWindowLong(pImpl_->windowHandle, GWL_STYLE);
  if (resizable) {
    currentStyle |= (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  } else {
    currentStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  }
  SetWindowLong(pImpl_->windowHandle, GWL_STYLE, currentStyle);
}

[[nodiscard]] bool Window::isResizable() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { return isResizable(); });
  }

  auto currentStyle = GetWindowLong(pImpl_->windowHandle, GWL_STYLE);
  return (currentStyle & (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) != 0;
}

void Window::setDecorations(bool decorations) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, decorations] { setDecorations(decorations); });
  }

  LONG style = GetWindowLong(pImpl_->windowHandle, GWL_STYLE);
  if (decorations) {
    style |= WS_OVERLAPPEDWINDOW;
  } else {
    style &= ~WS_OVERLAPPEDWINDOW;
  }
  SetWindowLong(pImpl_->windowHandle, GWL_STYLE, style);
}

[[nodiscard]] bool Window::isDecorated() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { return isDecorated(); });
  }
  LONG style = GetWindowLong(pImpl_->windowHandle, GWL_STYLE);
  return (style & WS_OVERLAPPEDWINDOW) != 0;
}

void Window::hide() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this]() { hide(); });
  }
  ShowWindow(pImpl_->windowHandle, SW_HIDE);
}

void Window::show() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this]() { show(); });
  }
  ShowWindow(pImpl_->windowHandle, SW_SHOW);
}

void Window::center() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this]() { center(); });
  }
  RECT windowRect;
  GetWindowRect(pImpl_->windowHandle, &windowRect);

  // Calculate the window width and height
  int windowWidth = windowRect.right - windowRect.left;
  int windowHeight = windowRect.bottom - windowRect.top;

  // Get the screen dimensions
  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);

  // Get the taskbar height
  APPBARDATA appBarData;
  appBarData.cbSize = sizeof(appBarData);
  UINT taskbarHeight = 0;
  if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData)) {
    taskbarHeight = appBarData.rc.bottom - appBarData.rc.top;
  }

  // Calculate the window position to center it, considering the taskbar
  int yPos = screenHeight - windowHeight - taskbarHeight;
  yPos = (yPos < 0) ? 0 : yPos;
  yPos /= 2;

  // Calculate the window position to center it, considering the taskbar
  int xPos = (screenWidth - windowWidth) / 2;

  // Set the new window position
  SetWindowPos(pImpl_->windowHandle, nullptr, xPos, yPos, 0, 0,
               SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void Window::enable(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state]() { enable(state); });
  }

  EnableWindow(pImpl_->windowHandle, state ? TRUE : FALSE);

  if (state) {
    SetForegroundWindow(pImpl_->windowHandle);
  }
}

void Window::setBackgroundColor(int red, int green, int blue) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread(
        [this, red, green, blue]() { setBackgroundColor(red, green, blue); });
  }
  pImpl_->backgroundColor_ = RGB(red, green, blue);
  InvalidateRect(pImpl_->windowHandle, nullptr, TRUE);
}

[[nodiscard]] void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->windowHandle); }

[[nodiscard]] void* Window::getContentView() { return static_cast<void*>(pImpl_->windowHandle); }
