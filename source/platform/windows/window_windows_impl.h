/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <windows.h>
#include <CommCtrl.h>

#include "deskgui/window.h"
#include "utils/throttle.h"


namespace deskgui {
  const wchar_t CLASS_NAME[] = L"deskgui Window Class";
  constexpr size_t kResizeThrottleInMs = 15;

  struct Window::Impl {
    WNDCLASS wc = {};
    HWND windowHandle;
    static inline HINSTANCE hInstance;

    static bool processWindowMessage(Window *window, HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK subclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                         [[maybe_unused]] UINT_PTR uIdSubclass,
                                         DWORD_PTR dwRefData);

    void registerWindowClass();
    float computeDpiScale(HWND hwnd);

    Throttle throttle{kResizeThrottleInMs};
    COLORREF backgroundColor_;
  };
}  // namespace deskgui
