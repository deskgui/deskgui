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
  constexpr size_t kResizeThrottleInMs = 15;

  struct Window::Impl {
    NSWindow* window;
    Throttle throttle{kResizeThrottleInMs};
  };
}  // namespace deskgui
