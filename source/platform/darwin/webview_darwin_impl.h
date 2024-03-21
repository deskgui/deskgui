/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <WebKit/WebKit.h>

#include "deskgui/webview.h"

namespace deskgui {
  struct Webview::Impl {
    WKWebView* webview;
    WKUserContentController* controller;
  };
}  // namespace deskgui
