/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#import <Cocoa/Cocoa.h>

#include <future>
#include <optional>

#include "deskgui/window.h"
#include "utils/throttle.h"

namespace deskgui {
  struct Window::Impl {
    NSWindow* window = nil;
    NSView* view = nil;
    id observer;
    bool isExternalWindow = false;
  };
}  // namespace deskgui
