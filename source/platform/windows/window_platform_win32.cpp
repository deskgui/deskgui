/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <cstdlib>
#include <system_error>
#include <utility>

#include "interfaces/app_impl.h"
#include "window_platform_win32.h"

using namespace deskgui;

using Platform = Window::Impl::Platform;

float Platform::computeDpiScale(HWND hwnd) {
  auto dpi = static_cast<float>(GetDpiForWindow(hwnd));
  return dpi / USER_DEFAULT_SCREEN_DPI;
}

bool Platform::processWindowMessage(Window::Impl *window, HWND hwnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam) {
  switch (uMsg) {
    case WM_SHOWWINDOW: {
      event::WindowShow showEvent(static_cast<bool>(wParam));
      window->events().emit(showEvent);
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
      event::WindowResize resizeEvent(window->getSize(PixelsType::kPhysical));
      window->events().emit(resizeEvent);
    } break;
    case WM_SIZE: {
      window->platform()->throttle.trigger([window]() {
        event::WindowResize resizeEvent(window->getSize(PixelsType::kPhysical));
        window->events().emit(resizeEvent);
      });
    } break;
    case WM_DPICHANGED: {
      window->setMonitorScaleFactor(window->platform()->computeDpiScale(hwnd));
    } break;
    case WM_SETTINGCHANGE: {
      if (lParam && wcscmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0) {
        // Read current theme from registry
        DWORD useLightTheme = 1;
        DWORD size = sizeof(useLightTheme);
        RegGetValueW(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     L"AppsUseLightTheme", RRF_RT_DWORD, nullptr, &useLightTheme, &size);
        auto theme = useLightTheme ? SystemTheme::kLight : SystemTheme::kDark;
        window->events().emit(event::WindowThemeChanged{theme});
      }
    } break;
    case WM_ERASEBKGND: {
      auto hdc = reinterpret_cast<HDC>(wParam);
      RECT rc;
      GetClientRect(hwnd, &rc);
      FillRect(hdc, &rc, CreateSolidBrush(window->platform()->backgroundColor));
      return true;
    };
  }
  return false;
}

LRESULT CALLBACK Platform::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Window::Impl *window;
  if (uMsg == WM_CREATE) {
    CREATESTRUCT *windowCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    window = reinterpret_cast<Window::Impl *>(windowCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
  } else {
    window = reinterpret_cast<Window::Impl *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if (window) {
    if (uMsg == WM_CLOSE) {
      event::WindowClose closeEvent{};
      window->events().emit(closeEvent);
      if (closeEvent.isCancelled()) {
        return 0;
      }
      window->close();
    } else {
      if (processWindowMessage(window, hwnd, uMsg, wParam, lParam)) return 0;
    }
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Platform::subclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                        [[maybe_unused]] UINT_PTR uIdSubclass,
                                        DWORD_PTR dwRefData) {
  if (auto *window = reinterpret_cast<class Window::Impl *>(dwRefData); window != nullptr) {
    if (processWindowMessage(window, hwnd, uMsg, wParam, lParam)) return 0;
  }
  return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

void Platform::registerWindowClass() {
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