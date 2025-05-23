/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#import <Cocoa/Cocoa.h>

#include "app_handler_darwin.h"


namespace deskgui {
  AppHandler::AppHandler(const std::string& name) : name_(name) {}

  void dispatchInMainQueue(std::function<void()> task) {
    dispatch_async(dispatch_get_main_queue(), ^{
      task();
    });
  }
}  // namespace deskgui
