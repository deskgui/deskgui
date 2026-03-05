/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <deskgui/types.h>
#include <wil/com.h>
#include <wil/stl.h>
#include <wil/win32_helpers.h>
#include <wrl.h>

#include <atomic>
#include <cstdlib>
#include <functional>
#include <optional>
#include <utility>

#include "interfaces/webview_impl.h"

namespace deskgui {

  class Webview::Impl::Platform {
  public:
    Platform() = default;
    ~Platform() = default;

    bool createWebviewInstance(std::string_view appName, HWND hWnd, const WebviewOptions& options);

    HRESULT onCreateEnvironmentCompleted(ICoreWebView2Environment* environment, HWND hWnd);
    void onCreateCoreWebView2ControllerCompleted(ICoreWebView2Controller* controller);

    bool handleDragAndDrop(ICoreWebView2WebMessageReceivedEventArgs* event);

    wil::com_ptr<ICoreWebView2> webview;
    wil::com_ptr<ICoreWebView2Controller> webviewController;

    std::optional<EventRegistrationToken> webResourceRequestedToken;
    std::optional<EventRegistrationToken> acceleratorKeysToken;

    bool ephemeralSession_ = false;
    bool asyncMode_ = false;
    Webview::Impl* webviewImpl_ = nullptr;
    WebviewOptions options_;
    std::atomic_flag creationFlag_ = ATOMIC_FLAG_INIT;
  };

}  // namespace deskgui
