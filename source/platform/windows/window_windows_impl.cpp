/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <cstdlib>
#include <system_error>
#include <utility>

#include "window_windows_impl.h"

using namespace deskgui;

float Window::Impl::computeDpiScale(HWND hwnd) {
  float dpi = static_cast<float>(GetDpiForWindow(hwnd));
  return dpi / USER_DEFAULT_SCREEN_DPI;
}

bool Window::Impl::processWindowMessage(Window *window, HWND hwnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam) {
  switch (uMsg) {
    case WM_SHOWWINDOW: {
      window->emit<event::WindowShow>(static_cast<bool>(wParam));
    } break;
    case WM_GETMINMAXINFO: {
      auto *info = reinterpret_cast<MINMAXINFO *>(lParam);
      auto maxSize = window->getMaxSize(PixelsType::kPhysical);
      auto minSize = window->getMinSize(PixelsType::kPhysical);

      if (maxSize.first) info->ptMaxTrackSize.x = maxSize.first;
      if (maxSize.second) info->ptMaxTrackSize.y = maxSize.second;
      if (minSize.first) info->ptMinTrackSize.x = minSize.first;
      if (minSize.second) info->ptMinTrackSize.y = minSize.second;
      return true;
    } break;
    case WM_EXITSIZEMOVE: {
      window->emit<event::WindowResize>(window->getSize(PixelsType::kPhysical));
    } break;
    case WM_SIZE: {
      window->pImpl_->throttle.trigger([window]() {
        window->emit<event::WindowResize>(window->getSize(PixelsType::kPhysical));
      });
    } break;
    case WM_DPICHANGED: {
      window->setMonitorScaleFactor(window->pImpl_->computeDpiScale(hwnd));

      RECT *suggestedRect = reinterpret_cast<RECT *>(lParam);

      SetWindowPos(hwnd, NULL, suggestedRect->left, suggestedRect->top,
                   suggestedRect->right - suggestedRect->left,
                   suggestedRect->bottom - suggestedRect->top, SWP_NOZORDER | SWP_NOACTIVATE);
    } break;
    case WM_ERASEBKGND: {
      auto hdc = reinterpret_cast<HDC>(wParam);
      RECT rc;
      GetClientRect(hwnd, &rc);
      FillRect(hdc, &rc, CreateSolidBrush(window->pImpl_->backgroundColor_));
      return true;
    } break;
  }
  return false;
}

LRESULT CALLBACK Window::Impl::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Window *window;
  if (uMsg == WM_CREATE) {
    CREATESTRUCT *windowCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    window = reinterpret_cast<Window *>(windowCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
  } else {
    window = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if (window) {
    if (uMsg == WM_CLOSE) {
      event::WindowClose closeEvent{};
      window->emit(closeEvent);
      if (closeEvent.isCancelled()) {
        return 0;
      }
      window->appHandler_->notifyWindowClosedFromUI(window->getName());
    } else {
      if (processWindowMessage(window, hwnd, uMsg, wParam, lParam)) return 0;
    }
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window::Impl::subclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                            [[maybe_unused]] UINT_PTR uIdSubclass,
                                            DWORD_PTR dwRefData) {
  if (auto *window = reinterpret_cast<class Window *>(dwRefData); window != nullptr) {
    if (processWindowMessage(window, hwnd, uMsg, wParam, lParam)) return 0;
  }
  return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

void Window::Impl::registerWindowClass() {
  if (!hInstance) {
    hInstance = GetModuleHandleW(nullptr);

    wc.lpfnWndProc = windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
    }
  }
}