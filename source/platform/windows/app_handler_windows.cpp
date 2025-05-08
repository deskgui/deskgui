/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "app_handler_windows.h"

namespace deskgui {
  static LRESULT CALLBACK MessageOnlyWindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_SAFE_CALL) {
      auto task = reinterpret_cast<MainThreadTask*>(lParam);
      if (task) {
        (*task)();
      }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

  AppHandler::AppHandler(const std::string& name) : name_(name) {
    // Register class for the message-only window
    WNDCLASSEX wc = {sizeof(WNDCLASSEX),        0,       MessageOnlyWindow, 0,       0,
                     GetModuleHandle(nullptr),  nullptr, nullptr,           nullptr, nullptr,
                     L"MessageOnlyWindowClass", nullptr};
    RegisterClassEx(&wc);

    // Create the message-only window
    messageOnlyWindow_ = CreateWindowEx(0, L"MessageOnlyWindowClass", L"MessageOnlyWindow", 0, 0, 0,
                                        0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
  }
}  // namespace deskgui
