/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <system_error>

#include "app_handler_darwin.h"
#include "window_darwin_impl.h"

using namespace deskgui;

@interface WindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic, assign) Window* window;
@property(nonatomic, assign) AppHandler* appHandler;
@property(nonatomic, assign) Throttle* throttle;
- (instancetype)initWithWindow:(Window*)window
                    appHandler:(AppHandler*)appHandler
                      throttle:(Throttle*)throttle;
@end

@implementation WindowDelegate

- (instancetype)initWithWindow:(Window*)window
                    appHandler:(AppHandler*)appHandler
                      throttle:(Throttle*)throttle {
  self = [super init];
  if (self) {
    _window = window;
    _appHandler = appHandler;
    _throttle = throttle;
  }
  return self;
}

- (void)windowDidLoad:(NSNotification*)notification {
  _window->emit(event::WindowShow{true});
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
  event::WindowClose closeEvent{};
  _window->emit(closeEvent);
  if (closeEvent.isCancelled()) {
    return FALSE;
  }
  _appHandler->notifyWindowClosedFromUI(_window->getName());
  return YES;
}

- (void)windowDidResize:(NSNotification*)notification {
  _throttle->trigger([=]() { _window->emit(event::WindowResize{_window->getSize()}); });
}

- (void)windowDidEndLiveResize:(NSNotification*)notification {
  _window->emit(event::WindowResize{_window->getSize()});
}

- (BOOL)windowShouldZoom:(NSWindow*)window toFrame:(NSRect)newFrame {
  return FALSE;
}
@end

Window::Window(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : name_(name), pImpl_{std::make_unique<Impl>()}, appHandler_(appHandler)     {
  if (nativeWindow == nullptr) {
    pImpl_->window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(kDefaultWindowRect.L, kDefaultWindowRect.T,
                                       kDefaultWindowRect.R - kDefaultWindowRect.L,
                                       kDefaultWindowRect.B - kDefaultWindowRect.T)
                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                            | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                    backing:NSBackingStoreBuffered
                      defer:NO];
    [pImpl_->window setTitle:@"deskgui Window"];
    [pImpl_->window center];
    [pImpl_->window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
  } else {
    isExternalWindow_ = true;
    if ([(__bridge id)nativeWindow isKindOfClass:[NSWindow class]]) {
      pImpl_->window = static_cast<NSWindow*>(nativeWindow);
    } else if ([(__bridge id)nativeWindow isKindOfClass:[NSView class]]) {
      pImpl_->window = [static_cast<NSView*>(nativeWindow) window];
    }
  }
  if (!pImpl_->window) {
    NSError* error = [NSError errorWithDomain:NSOSStatusErrorDomain code:-1 userInfo:nil];
    throw std::system_error(static_cast<int>(error.code), std::system_category(),
                            error.localizedDescription.UTF8String);
  }

  WindowDelegate* windowDelegate = [[WindowDelegate alloc] initWithWindow:this
                                                               appHandler:appHandler_
                                                                 throttle:&pImpl_->throttle];
  [pImpl_->window setDelegate:windowDelegate];
}

Window::~Window() {
  if (!isExternalWindow_ && pImpl_->window != nil) {
    [pImpl_->window close];
    [pImpl_->window release];
    pImpl_->window = nil;
  }
}

void Window::setTitle(const std::string& title) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([title, this] { return setTitle(title); });
  }
  NSString* nativeTitle = [NSString stringWithUTF8String:title.c_str()];
  [pImpl_->window setTitle:nativeTitle];
}

[[nodiscard]] std::string Window::getTitle() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getTitle(); });
  }
  NSString* nativeTitle = [pImpl_->window title];
  const char* utf8String = [nativeTitle UTF8String];
  std::string title(utf8String);
  return title;
}

void Window::setSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { return setSize(size); });
  }
  NSRect frame = pImpl_->window.frame;
  frame.size.width = size.first;
  frame.size.height = size.second;
  [pImpl_->window setFrame:frame display:YES];
}

[[nodiscard]] ViewSize Window::getSize() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getSize(); });
  }
  NSRect frame = pImpl_->window.contentView.frame;
  return {frame.size.width, frame.size.height};
}

void Window::setPosition(const ViewRect& position) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([position, this] { setPosition(position); });
  }
  NSRect frame;
  frame.origin.x = position.L;
  frame.origin.y = position.T;
  frame.size.width = position.R - position.L;
  frame.size.height = position.B - position.T;
  [pImpl_->window setFrame:frame display:YES];
}

[[nodiscard]] ViewRect Window::getPosition() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getPosition(); });
  }
  NSRect frame = pImpl_->window.frame;
  ViewRect position;
  position.L = frame.origin.x;
  position.T = frame.origin.y;
  position.R = frame.origin.x + frame.size.width;
  position.B = frame.origin.y + frame.size.height;
  return position;
}

void Window::setResizable(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this] { return setResizable(state); });
  }
  if (state) {
    pImpl_->window.styleMask |= NSWindowStyleMaskResizable;
  } else {
    pImpl_->window.styleMask &= ~NSWindowStyleMaskResizable;
  }
}

[[nodiscard]] bool Window::isResizable() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return isResizable(); });
  }

  NSUInteger styleMask = pImpl_->window.styleMask;
  return (styleMask & NSWindowStyleMaskResizable) == NSWindowStyleMaskResizable;
}

void Window::setDecorations(bool decorations) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return setDecorations(decorations); });
  }

  NSWindowStyleMask styleMask = [pImpl_->window styleMask];
  if (decorations) {
    styleMask |= NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                 | NSWindowStyleMaskMiniaturizable;
    styleMask &= ~NSWindowStyleMaskBorderless;
  } else {
    styleMask &= ~(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                   | NSWindowStyleMaskMiniaturizable);
    styleMask |= NSWindowStyleMaskBorderless;
  }
  [pImpl_->window setStyleMask:styleMask];
}

[[nodiscard]] bool Window::isDecorated() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return isDecorated(); });
  }
  NSWindowStyleMask styleMask = [pImpl_->window styleMask];
  return styleMask != NSWindowStyleMaskBorderless;
}

void Window::hide() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { hide(); });
  }
  [pImpl_->window orderOut:nil];
}

void Window::show() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { show(); });
  }
  [pImpl_->window makeKeyAndOrderFront:nil];
  [NSApp activateIgnoringOtherApps:YES];
}

void Window::center() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=]() { center(); });
  }
  [pImpl_->window center];
}

[[nodiscard]] void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->window); }

void Window::setMaxSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { setMinSize(size); });
  }
  maxSize_ = size;
  [pImpl_->window setContentMaxSize:NSMakeSize(size.first, size.second)];

  NSButton* button = [pImpl_->window standardWindowButton:NSWindowZoomButton];
  [button setHidden:YES];
  button.alphaValue = 0.0;
  [button setEnabled:NO];
  button.image = nil;
  button.alternateImage = nil;
}

void Window::setMinSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { setMaxSize(size); });
  }
  minSize_ = size;
  [pImpl_->window setContentMinSize:NSMakeSize(size.first, size.second)];
}

float Window::getMonitorScaleFactor() { return 1.f; } // not implemented yed
