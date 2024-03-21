/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#import <Cocoa/Cocoa.h>

#include "app_handler_darwin.h"
#include "deskgui/app.h"

using namespace deskgui;

void App::run() {
  if (isRunning_.load()) {
    return;
  }

  isRunning_.store(true);
  mainThreadId_ = std::this_thread::get_id();

  @autoreleasepool {
    [NSApplication sharedApplication];
    [NSApp run];
  }
}

void App::terminate() {
  if (!isMainThread()) {
    return runOnMainThread([=]() { terminate(); });
  }
    
  if(isRunning_.load()) {
    isRunning_.store(false);
    [NSApp terminate:nil];
  }
}
