/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <CommCtrl.h>
#include <windows.h>

#include <cstdlib>
#include <utility>

#include "deskgui/window.h"
#include "utils/throttle.h"

namespace deskgui {
  const wchar_t CLASS_NAME[] = L"deskgui Window Class";
  constexpr size_t kResizeThrottleInMs = 15;

  struct Window::Impl {
    WNDCLASS wc = {};
    HWND window;
    static inline HINSTANCE hInstance;

    static bool processWindowMessage(Window *window, HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK subclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                         [[maybe_unused]] UINT_PTR uIdSubclass,
                                         DWORD_PTR dwRefData);

    void registerWindowClass();

    Throttle throttle{kResizeThrottleInMs};
    float monitorScaleFactor_ = 1.f;
    COLORREF backgroundColor_;
  };

  inline float computeDpiScale(HWND hwnd) {
    float dpi = GetDpiForWindow(hwnd);
    return dpi / USER_DEFAULT_SCREEN_DPI;
  }

  // Process a common window message for the Window class. Return false to stop propagation of the
  // event
  inline bool Window::Impl::processWindowMessage(Window *window, HWND hwnd, UINT uMsg,
                                                 WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
      case WM_CREATE: {
        window->pImpl_->monitorScaleFactor_ = computeDpiScale(hwnd);
      } break;
      case WM_GETMINMAXINFO: {
        auto *info = reinterpret_cast<MINMAXINFO *>(lParam);
        if (window->maxSize_.first) {
          info->ptMaxTrackSize.x = static_cast<int>(window->maxSize_.first);
        }
        if (window->maxSize_.second) {
          info->ptMaxTrackSize.y = static_cast<int>(window->maxSize_.second);
        }
        if (window->minSize_.first) {
          info->ptMinTrackSize.x = static_cast<int>(window->minSize_.first);
        }
        if (window->minSize_.second) {
          info->ptMinTrackSize.y = static_cast<int>(window->minSize_.second);
        }
      } break;
      case WM_SHOWWINDOW: {
        window->emit<event::WindowShow>(static_cast<bool>(wParam));
      } break;
      case WM_SIZE: {
        window->pImpl_->throttle.trigger(
            [window]() { window->emit<event::WindowResize>(window->getSize()); });
      } break;
      case WM_EXITSIZEMOVE: {
        window->emit<event::WindowResize>(window->getSize());
      } break;
      case WM_DPICHANGED: {
        window->emit<event::WindowResize>(window->getSize());
      } break;
      case WM_GETDPISCALEDSIZE: {
        window->pImpl_->monitorScaleFactor_ = (float)wParam / USER_DEFAULT_SCREEN_DPI;
      } break;
      case WM_ERASEBKGND: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, CreateSolidBrush(window->pImpl_->backgroundColor_));
      } break;
    }
    return true;
  }

  // The main window procedure for the Window class
  inline LRESULT Window::Impl::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window *window;
    if (uMsg == WM_CREATE) {
      CREATESTRUCT *windowCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
      window = reinterpret_cast<Window *>(windowCreate->lpCreateParams);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    } else {
      window = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
      switch (uMsg) {
        case WM_CLOSE: {
          event::WindowClose closeEvent{};
          window->emit(closeEvent);
          if (closeEvent.isCancelled()) {
            return 0;
          }
          window->appHandler_->notifyWindowClosedFromUI(window->getName());
          break;
        }
        default:
          if (!processWindowMessage(window, hwnd, uMsg, wParam, lParam)) return 0;
          break;
      }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

  // The subclass window procedure attached to an external window
  inline LRESULT Window::Impl::subclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                            [[maybe_unused]] UINT_PTR uIdSubclass,
                                            DWORD_PTR dwRefData) {
    auto *window = reinterpret_cast<class Window *>(dwRefData);

    if (window) {
      if (!processWindowMessage(window, hwnd, uMsg, wParam, lParam)) return 0;
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
  }

  // Register the window class for the Window
  inline void Window::Impl::registerWindowClass() {
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

  inline std::wstring s2ws(const std::string &str) {
    int size_needed
        = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0],
                        size_needed);
    return wstr;
  }

  inline std::string ws2s(const std::wstring &wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()),
                                          nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), &str[0],
                        size_needed, nullptr, nullptr);
    return str;
  }
}  // namespace deskgui
