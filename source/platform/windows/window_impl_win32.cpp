/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <system_error>

#include "interfaces/app_impl.h"
#include "utils/strings.h"
#include "window_platform_win32.h"


using namespace deskgui;
using namespace deskgui::utils;

using Impl = Window::Impl;

Impl::Impl(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : platform_(std::make_unique<Impl::Platform>()), name_(name), appHandler_(appHandler) {

  if (nativeWindow == nullptr) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    platform_->registerWindowClass();

    platform_->windowHandle = CreateWindowEx(0,                    // Optional window styles.
                                             CLASS_NAME,           // Window class
                                             L"deskgui window",    // Window text
                                             WS_OVERLAPPEDWINDOW,  // Window style

                                             // Size and position
                                             kDefaultWindowRect.L, kDefaultWindowRect.T,
                                             kDefaultWindowRect.R - kDefaultWindowRect.L,
                                             kDefaultWindowRect.B - kDefaultWindowRect.T,

                                             nullptr,                    // Parent window
                                             nullptr,                    // Menu
                                             Impl::Platform::hInstance,  // Instance handle
                                             this  // Additional application data
    );

    if (!platform_->windowHandle) {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
    }
  } else {
    isExternalWindow_ = true;
    platform_->windowHandle = static_cast<HWND>(nativeWindow);
    SetWindowSubclass(platform_->windowHandle, &Impl::Platform::subclassProc, 1,
                      reinterpret_cast<DWORD_PTR>(this));
  }

  setMonitorScaleFactor(platform_->computeDpiScale(platform_->windowHandle));
}

Impl::~Impl() {
  if (IsWindow(platform_->windowHandle)) {
    if (!isExternalWindow_) {
      DestroyWindow(platform_->windowHandle);
    } else {
      RemoveWindowSubclass(platform_->windowHandle, &Platform::subclassProc, 1);
    }
    platform_->windowHandle = nullptr;
  }
}

void Impl::setTitle(const std::string& title) {
  SetWindowText(platform_->windowHandle, s2ws(title).c_str());
}

std::string Impl::getTitle() const {
  const int bufferSize = 256;
  WCHAR buffer[bufferSize];
  GetWindowText(platform_->windowHandle, buffer, bufferSize);

  return ws2s(buffer);
}

void Impl::setSize(const ViewSize& size, PixelsType type) {
  auto modifiedSize = size;
  if (type == PixelsType::kLogical) {
    modifiedSize = ViewSize{size.first * monitorScaleFactor_, size.second * monitorScaleFactor_};
  }

  RECT windowRect = {0, 0, modifiedSize.first, modifiedSize.second};
  AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, 0);

  int windowWidth = windowRect.right - windowRect.left;
  int windowHeight = windowRect.bottom - windowRect.top;
  SetWindowPos(platform_->windowHandle, nullptr, windowRect.left, windowRect.top, windowWidth,
               windowHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

ViewSize Impl::getSize(PixelsType type) const {
  RECT rect;
  GetClientRect(platform_->windowHandle, &rect);
  auto size = ViewSize{rect.right - rect.left, rect.bottom - rect.top};

  if (type == PixelsType::kLogical) {
    size.first /= monitorScaleFactor_;
    size.second /= monitorScaleFactor_;
  }
  return size;
}

void Impl::setMaxSize(const ViewSize& size, PixelsType type) {
  ViewSize logicalSize = size;
  if (type == PixelsType::kPhysical) {
    logicalSize.first /= monitorScaleFactor_;
    logicalSize.second /= monitorScaleFactor_;
  }

  maxSize_ = logicalSize;
  maxSizeDefined_ = true;

  LONG windowStyle = GetWindowLong(platform_->windowHandle, GWL_STYLE);
  windowStyle &= ~WS_MAXIMIZEBOX;
  SetWindowLong(platform_->windowHandle, GWL_STYLE, windowStyle);
}

ViewSize Impl::getMaxSize(PixelsType type) const {
  if (type == PixelsType::kPhysical) {
    return ViewSize{maxSize_.first * monitorScaleFactor_, maxSize_.second * monitorScaleFactor_};
  }
  return maxSize_;
}

void Impl::setMinSize(const ViewSize& size, PixelsType type) {
  ViewSize logicalSize = size;
  if (type == PixelsType::kPhysical) {
    logicalSize.first /= monitorScaleFactor_;
    logicalSize.second /= monitorScaleFactor_;
  }

  minSize_ = logicalSize;
  minSizeDefined_ = true;
}

ViewSize Impl::getMinSize(PixelsType type) const {
  if (type == PixelsType::kPhysical) {
    return ViewSize{minSize_.first * monitorScaleFactor_, minSize_.second * monitorScaleFactor_};
  }
  return minSize_;
}

void Impl::setPosition(const ViewRect& position, PixelsType type) {
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
  SetWindowPos(platform_->windowHandle, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top,
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

ViewRect Impl::getPosition(PixelsType type) const {
  RECT windowRect;
  GetWindowRect(platform_->windowHandle, &windowRect);

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

void Impl::setResizable(bool resizable) {
  auto currentStyle = GetWindowLong(platform_->windowHandle, GWL_STYLE);
  if (resizable) {
    currentStyle |= (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  } else {
    currentStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  }
  SetWindowLong(platform_->windowHandle, GWL_STYLE, currentStyle);
}

bool Impl::isResizable() const {
  auto currentStyle = GetWindowLong(platform_->windowHandle, GWL_STYLE);
  return (currentStyle & (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) != 0;
}

void Impl::setDecorations(bool decorations) {
  LONG style = GetWindowLong(platform_->windowHandle, GWL_STYLE);
  if (decorations) {
    style |= WS_OVERLAPPEDWINDOW;
  } else {
    style &= ~WS_OVERLAPPEDWINDOW;
  }
  SetWindowLong(platform_->windowHandle, GWL_STYLE, style);
}

bool Impl::isDecorated() const {
  LONG style = GetWindowLong(platform_->windowHandle, GWL_STYLE);
  return (style & WS_OVERLAPPEDWINDOW) != 0;
}

void Impl::hide() { ShowWindow(platform_->windowHandle, SW_HIDE); }

void Impl::show() { ShowWindow(platform_->windowHandle, SW_SHOW); }

void Impl::center() {
  RECT windowRect;
  GetWindowRect(platform_->windowHandle, &windowRect);

  int windowWidth = windowRect.right - windowRect.left;
  int windowHeight = windowRect.bottom - windowRect.top;

  // Get the work area of the monitor the window is currently on
  HMONITOR monitor = MonitorFromWindow(platform_->windowHandle, MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitorInfo = {};
  monitorInfo.cbSize = sizeof(monitorInfo);
  GetMonitorInfo(monitor, &monitorInfo);

  auto &workArea = monitorInfo.rcWork;
  int xPos = workArea.left + (workArea.right - workArea.left - windowWidth) / 2;
  int yPos = workArea.top + (workArea.bottom - workArea.top - windowHeight) / 2;

  SetWindowPos(platform_->windowHandle, nullptr, xPos, yPos, 0, 0,
               SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void Impl::enable(bool state) {
  EnableWindow(platform_->windowHandle, state ? TRUE : FALSE);

  if (state) {
    SetForegroundWindow(platform_->windowHandle);
  }
}

void Impl::setBackgroundColor(int red, int green, int blue) {
  platform_->backgroundColor = RGB(red, green, blue);
  InvalidateRect(platform_->windowHandle, nullptr, TRUE);
}

void* Impl::getNativeWindow() { return static_cast<void*>(platform_->windowHandle); }

void* Impl::getContentView() { return static_cast<void*>(platform_->windowHandle); }