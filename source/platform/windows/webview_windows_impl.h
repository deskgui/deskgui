/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <wil/com.h>
#include <wil/stl.h>
#include <wil/win32_helpers.h>
#include <wrl.h>

#include <atomic>
#include <optional>

#include "deskgui/webview.h"

namespace deskgui {

  struct Webview::Impl {
    bool createWebviewInstance(HWND hWnd);

    wil::com_ptr<ICoreWebView2> webview_;
    wil::com_ptr<ICoreWebView2Controller> webviewController_;

    const std::string rootScheme_ = "https://localhost/";
    std::optional<EventRegistrationToken> webResourceRequestedToken_;
  };

  inline bool Webview::Impl::createWebviewInstance(HWND hWnd) {
    using namespace Microsoft::WRL;

    ComPtr options = Make<CoreWebView2EnvironmentOptions>();

    // Uncomment the following line to enable remote debugging
    // options->put_AdditionalBrowserArguments(L"--remote-debugging-port=9222");
    options->put_AdditionalBrowserArguments(L"--disable-gpu");

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
      return false;
    }

    std::wstring temp;
    wil::GetEnvironmentVariableW(L"TEMP", temp);

    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    flag.test_and_set();

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, temp.c_str(), options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [&]([[maybe_unused]] HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
              env->CreateCoreWebView2Controller(
                  hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [&]([[maybe_unused]] HRESULT result,
                                ICoreWebView2Controller* controller) -> HRESULT {
                              if (controller) {
                                webviewController_ = controller;
                                webviewController_->get_CoreWebView2(&webview_);
                              }
                              flag.clear();  // Clear the flag when WebView is ready
                              return S_OK;
                            })
                            .Get());
              return S_OK;
            })
            .Get());

    MSG msg = {};
    while (flag.test_and_set() && GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if (!webviewController_ || !webview_) {
      return false;
    }

    wil::com_ptr<ICoreWebView2Settings> settings;
    webview_->get_Settings(&settings);

    // Enable web messages and scripts
    settings->put_IsWebMessageEnabled(true);
    settings->put_IsScriptEnabled(true);

    // Disable default settings
    settings->put_AreDevToolsEnabled(false);
    settings->put_AreDefaultContextMenusEnabled(false);
    settings->put_IsZoomControlEnabled(false);
    settings->put_AreDefaultScriptDialogsEnabled(false);
    settings->put_AreHostObjectsAllowed(false);
    settings->put_IsStatusBarEnabled(false);

    if (auto settings3 = settings.try_query<ICoreWebView2Settings3>(); settings3) {
      settings3->put_AreBrowserAcceleratorKeysEnabled(false);
    }

    return true;
  }

}  // namespace deskgui
