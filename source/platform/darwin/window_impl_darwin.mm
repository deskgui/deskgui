/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <system_error>

#include "window_platform_darwin.h"

using namespace deskgui;

using Impl = Window::Impl;

Impl::Impl(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : name_(name), platform_(std::make_unique<Impl::Platform>()), appHandler_(appHandler) {
  if (nativeWindow == nullptr) {
    platform_->window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(kDefaultWindowRect.L, kDefaultWindowRect.T,
                                       kDefaultWindowRect.R - kDefaultWindowRect.L,
                                       kDefaultWindowRect.B - kDefaultWindowRect.T)
                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                            | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                    backing:NSBackingStoreBuffered
                      defer:NO];
    [platform_->window setTitle:@"deskgui Window"];
    [platform_->window center];
    [platform_->window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    platform_->view = [platform_->window contentView];
    WindowDelegate* windowDelegate = [[WindowDelegate alloc] initWithWindow:this];
    [platform_->window setDelegate:windowDelegate];
  } else {
    isExternalWindow_ = true;

    if ([(__bridge id)nativeWindow isKindOfClass:[NSWindow class]]) {
      platform_->window = (__bridge NSWindow*)nativeWindow;
      platform_->view = [platform_->window contentView];
    } else if ([(__bridge id)nativeWindow isKindOfClass:[NSView class]]) {
      NSView* view = (__bridge NSView*)nativeWindow;
      platform_->window = [view window];
      platform_->view = view;
    }

    platform_->observer =
        [[WindowObserver alloc] initWithWindow:this nativeWindow:platform_->window];
  }
  if (!platform_->window && !platform_->view) {
    NSError* error = [NSError errorWithDomain:NSOSStatusErrorDomain code:-1 userInfo:nil];
    throw std::system_error(static_cast<int>(error.code), std::system_category(),
                            error.localizedDescription.UTF8String);
  }
}

Impl::~Impl() {
  if (!isExternalWindow_ && platform_->window != nil) {
    [platform_->window close];
    platform_->window = nil;
  }

  if (platform_->observer != nil) {
    auto observer = (WindowObserver*)platform_->observer;
    [observer stopObserving];
    observer = nil;
  }
}

void Impl::setTitle(const std::string& title) {
  NSString* nativeTitle = [NSString stringWithUTF8String:title.c_str()];
  [platform_->window setTitle:nativeTitle];
}

std::string Impl::getTitle() const {
  NSString* nativeTitle = [platform_->window title];
  const char* utf8String = [nativeTitle UTF8String];
  std::string title(utf8String);
  return title;
}

void Impl::setSize(const ViewSize& size, PixelsType type) {
  auto newSize = size;
  if (type == PixelsType::kLogical) {
    newSize = ViewSize{size.first * monitorScaleFactor_, size.second * monitorScaleFactor_};
  }

  NSRect frame = platform_->window.frame;
  frame.size.width = newSize.first;
  frame.size.height = newSize.second;
  [platform_->window setFrame:frame display:YES];
}

ViewSize Impl::getSize(PixelsType type) const {
  NSRect frame = platform_->view.frame;
  auto size = ViewSize{frame.size.width, frame.size.height};
  if (type == PixelsType::kLogical) {
    size.first /= monitorScaleFactor_;
    size.second /= monitorScaleFactor_;
  }
  return size;
}

void Impl::setMaxSize(const ViewSize& size, PixelsType type) {
  ViewSize logicalSize = size;
  if (type == PixelsType::kPhysical) {
    logicalSize.first /= monitorScaleFactor_;
    logicalSize.second /= monitorScaleFactor_;
  }

  maxSize_ = logicalSize;
  maxSizeDefined_ = true;

  auto physicalMax = getMaxSize(PixelsType::kPhysical);
  [platform_->window setContentMaxSize:NSMakeSize(physicalMax.first, physicalMax.second)];

  NSButton* button = [platform_->window standardWindowButton:NSWindowZoomButton];
  [button setHidden:YES];
  button.alphaValue = 0.0;
  [button setEnabled:NO];
  button.image = nil;
  button.alternateImage = nil;
}

ViewSize Impl::getMaxSize(PixelsType type) const {
  if (type == PixelsType::kPhysical) {
    return ViewSize{maxSize_.first * monitorScaleFactor_, maxSize_.second * monitorScaleFactor_};
  }
  return maxSize_;
}

void Impl::setMinSize(const ViewSize& size, PixelsType type) {
  ViewSize logicalSize = size;
  if (type == PixelsType::kPhysical) {
    logicalSize.first /= monitorScaleFactor_;
    logicalSize.second /= monitorScaleFactor_;
  }

  minSize_ = logicalSize;
  minSizeDefined_ = true;

  auto physicalMin = getMinSize(PixelsType::kPhysical);
  [platform_->window setContentMinSize:NSMakeSize(physicalMin.first, physicalMin.second)];
}

ViewSize Impl::getMinSize(PixelsType type) const {
  if (type == PixelsType::kPhysical) {
    return ViewSize{minSize_.first * monitorScaleFactor_, minSize_.second * monitorScaleFactor_};
  }
  return minSize_;
}

void Impl::setPosition(const ViewRect& position, PixelsType type) {
  NSRect frame;
  frame.origin.x = position.L;
  frame.origin.y = position.T;
  frame.size.width = position.R - position.L;
  frame.size.height = position.B - position.T;

  if (type == PixelsType::kLogical) {
    frame.origin.x *= monitorScaleFactor_;
    frame.origin.y *= monitorScaleFactor_;
    frame.size.width *= monitorScaleFactor_;
    frame.size.height *= monitorScaleFactor_;
  }

  [platform_->window setFrame:frame display:YES];
}

ViewRect Impl::getPosition(PixelsType type) const {
  NSRect frame = platform_->window.frame;
  ViewRect position;
  position.L = frame.origin.x;
  position.T = frame.origin.y;
  position.R = frame.origin.x + frame.size.width;
  position.B = frame.origin.y + frame.size.height;

  if (type == PixelsType::kLogical) {
    position.L /= monitorScaleFactor_;
    position.T /= monitorScaleFactor_;
    position.R /= monitorScaleFactor_;
    position.B /= monitorScaleFactor_;
  }

  return position;
}

void Impl::setResizable(bool state) {
  if (state) {
    platform_->window.styleMask |= NSWindowStyleMaskResizable;
  } else {
    platform_->window.styleMask &= ~NSWindowStyleMaskResizable;
  }

  NSButton* zoomButton = [platform_->window standardWindowButton:NSWindowZoomButton];
  if (zoomButton) {
    [zoomButton setHidden:!state];
    [zoomButton setEnabled:state];
  }
}

bool Impl::isResizable() const {
  NSUInteger styleMask = platform_->window.styleMask;
  return (styleMask & NSWindowStyleMaskResizable) == NSWindowStyleMaskResizable;
}

void Impl::setDecorations(bool decorations) {
  NSWindowStyleMask styleMask = [platform_->window styleMask];
  if (decorations) {
    styleMask |= NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                 | NSWindowStyleMaskMiniaturizable;
    styleMask &= ~NSWindowStyleMaskBorderless;
  } else {
    styleMask &= ~(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                   | NSWindowStyleMaskMiniaturizable);
    styleMask |= NSWindowStyleMaskBorderless;
  }
  [platform_->window setStyleMask:styleMask];
}

bool Impl::isDecorated() const {
  NSWindowStyleMask styleMask = [platform_->window styleMask];
  return styleMask != NSWindowStyleMaskBorderless;
}

void Impl::hide() { [platform_->window orderOut:nil]; }

void Impl::show() {
  [platform_->window makeKeyAndOrderFront:nil];
  [NSApp activateIgnoringOtherApps:YES];
}

void Impl::center() { [platform_->window center]; }

void Impl::enable(bool state) {
  [platform_->window setIgnoresMouseEvents:state ? NO : YES];

  if (state) {
    [platform_->window makeKeyAndOrderFront:nil];
  }
}

void Impl::setBackgroundColor(int red, int green, int blue) {
  NSColor* color =
      [NSColor colorWithCalibratedRed:red / 255.0 green:green / 255.0 blue:blue / 255.0 alpha:1.0];
  [platform_->view setWantsLayer:YES];
  [platform_->view.layer setBackgroundColor:color.CGColor];
}

void* Impl::getNativeWindow() { return (__bridge void*)platform_->window; }

void* Impl::getContentView() { return (__bridge void*)platform_->view; }
