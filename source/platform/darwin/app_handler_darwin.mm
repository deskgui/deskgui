/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "platform/darwin/app_handler_darwin.h"

#import <Cocoa/Cocoa.h>

namespace deskgui {
  void dispatchInMainQueue(std::function<void()> task) {
    dispatch_async(dispatch_get_main_queue(), ^{
      task();
    });
  }
}  // namespace deskgui
