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
- (instancetype)window:(Window*)window appHandler:(AppHandler*)appHandler;
@end

@implementation WindowDelegate

- (instancetype)window:(Window*)window appHandler:(AppHandler*)appHandler {
    self = [super init];
    if (self) {
        _window = window;
        _appHandler = appHandler;
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
    _window->emit(event::WindowResize{_window->getSize(PixelsType::kPhysical)});
}

- (BOOL)windowShouldZoom:(NSWindow*)window toFrame:(NSRect)newFrame {
    return FALSE;
}
@end

@interface WindowObserver : NSObject
@property(nonatomic, assign) Window* window;
@property(nonatomic, assign) NSWindow* nativeWindow;
@property(nonatomic, assign) AppHandler* appHandler;

- (instancetype)window:(Window*)window
          nativeWindow:(NSWindow*)nativeWindow
            appHandler:(AppHandler*)appHandler;
@end

@implementation WindowObserver

- (instancetype)window:(Window*)window
          nativeWindow:(NSWindow*)nativeWindow
            appHandler:(AppHandler*)appHandler {
    self = [super init];
    if (self) {
        _window = window;
        _nativeWindow = nativeWindow;
        _appHandler = appHandler;
        
        // Register for notifications
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidLoadNotification:)
                                                     name:NSWindowDidBecomeKeyNotification
                                                   object:_nativeWindow];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowWillCloseNotification:)
                                                     name:NSWindowWillCloseNotification
                                                   object:_nativeWindow];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidResizeNotification:)
                                                     name:NSWindowDidResizeNotification
                                                   object:_nativeWindow];
    }
    return self;
}

- (void)dealloc {
    [self stopObserving];
    [super dealloc];
}

- (void)stopObserving {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    _window = nil;
}

- (void)windowDidLoadNotification:(NSNotification*)notification {
    _window->emit(event::WindowShow{true});
}

- (void)windowWillCloseNotification:(NSNotification*)notification {
    _window->emit(event::WindowClose{});
}

- (void)windowDidResizeNotification:(NSNotification*)notification {
    _window->emit(event::WindowResize{_window->getSize()});
}

@end

Window::Window(const std::string& name, AppHandler* appHandler, void* nativeWindow)
: name_(name), pImpl_{std::make_unique<Impl>()}, appHandler_(appHandler) {
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
        pImpl_->view = [pImpl_->window contentView];
        WindowDelegate* windowDelegate = [[WindowDelegate alloc] window:this appHandler:appHandler_];
        [pImpl_->window setDelegate:windowDelegate];
    } else {
        isExternalWindow_ = true;
        
        if ([(__bridge id)nativeWindow isKindOfClass:[NSWindow class]]) {
            pImpl_->window = static_cast<NSWindow*>(nativeWindow);
            pImpl_->view = [pImpl_->window contentView];
        } else if ([(__bridge id)nativeWindow isKindOfClass:[NSView class]]) {
            NSView* view = static_cast<NSView*>(nativeWindow);
            pImpl_->window = [view window];
            pImpl_->view = view;
        }
        
        pImpl_->observer = [[WindowObserver alloc] window:this
                                             nativeWindow:pImpl_->window
                                               appHandler:appHandler_];
    }
    if (!pImpl_->window && !pImpl_->view) {
        NSError* error = [NSError errorWithDomain:NSOSStatusErrorDomain code:-1 userInfo:nil];
        throw std::system_error(static_cast<int>(error.code), std::system_category(),
                                error.localizedDescription.UTF8String);
    }
}

Window::~Window() {
    if (!isExternalWindow_ && pImpl_->window != nil) {
        [pImpl_->window close];
        pImpl_->window = nil;
    }
    
    if (pImpl_->observer != nil) {
        auto observer = (WindowObserver*)pImpl_->observer;
        [observer stopObserving];
        observer = nil;
    }
}

void Window::setTitle(const std::string& title) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([title, this] { setTitle(title); });
    }
    NSString* nativeTitle = [NSString stringWithUTF8String:title.c_str()];
    [pImpl_->window setTitle:nativeTitle];
}

[[nodiscard]] std::string Window::getTitle() const {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([this] { return getTitle(); });
    }
    NSString* nativeTitle = [pImpl_->window title];
    const char* utf8String = [nativeTitle UTF8String];
    std::string title(utf8String);
    return title;
}

void Window::setSize(const ViewSize& size, PixelsType type) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([size, type, this] { setSize(size, type); });
    }
    auto newSize = size;
    if (type == PixelsType::kLogical) {
        newSize = ViewSize{size.first * monitorScaleFactor_, size.second * monitorScaleFactor_};
    }
    
    NSRect frame = pImpl_->window.frame;
    frame.size.width = newSize.first;
    frame.size.height = newSize.second;
    [pImpl_->window setFrame:frame display:YES];
}

[[nodiscard]] ViewSize Window::getSize(PixelsType type) const {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([type, this] { return getSize(type); });
    }
    NSRect frame = pImpl_->view.frame;
    auto size = ViewSize{frame.size.width, frame.size.height};
    if (type == PixelsType::kLogical) {
        size.first /= monitorScaleFactor_;
        size.second /= monitorScaleFactor_;
    }
    return size;
}

void Window::setMaxSize(const ViewSize& size, PixelsType type) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([size, type, this] { setMinSize(size, type); });
    }
    ViewSize adjustedSize = size;
    if (type == PixelsType::kLogical) {
        adjustedSize.first *= monitorScaleFactor_;
        adjustedSize.second *= monitorScaleFactor_;
    }
    
    maxSize_ = adjustedSize;
    maxSizeDefined_ = true;
    
    [pImpl_->window setContentMaxSize:NSMakeSize(size.first, size.second)];
    
    NSButton* button = [pImpl_->window standardWindowButton:NSWindowZoomButton];
    [button setHidden:YES];
    button.alphaValue = 0.0;
    [button setEnabled:NO];
    button.image = nil;
    button.alternateImage = nil;
}

[[nodiscard]] ViewSize Window::getMaxSize(PixelsType type) const {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([type, this]() { return getMaxSize(type); });
    }
    if (type == PixelsType::kLogical) {
        return ViewSize{maxSize_.first / monitorScaleFactor_, maxSize_.second / monitorScaleFactor_};
    } else {
        return maxSize_;
    }
}

void Window::setMinSize(const ViewSize& size, PixelsType type) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([size, type, this] { setMaxSize(size, type); });
    }
    ViewSize adjustedSize = size;
    if (type == PixelsType::kLogical) {
        adjustedSize.first *= monitorScaleFactor_;
        adjustedSize.second *= monitorScaleFactor_;
    }
    
    minSize_ = adjustedSize;
    [pImpl_->window setContentMinSize:NSMakeSize(size.first, size.second)];
}

[[nodiscard]] ViewSize Window::getMinSize(PixelsType type) const {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([this, type]() { return getMinSize(type); });
    }
    if (type == PixelsType::kLogical) {
        return ViewSize{minSize_.first / monitorScaleFactor_, minSize_.second / monitorScaleFactor_};
    } else {
        return minSize_;
    }
}

void Window::setPosition(const ViewRect& position, PixelsType type) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([position, type, this] { setPosition(position, type); });
    }
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
    
    [pImpl_->window setFrame:frame display:YES];
}

[[nodiscard]] ViewRect Window::getPosition(PixelsType type) const {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([type, this] { return getPosition(type); });
    }
    NSRect frame = pImpl_->window.frame;
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

void Window::setResizable(bool state) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([state, this] { return setResizable(state); });
    }
    if (state) {
        pImpl_->window.styleMask |= NSWindowStyleMaskResizable;
    } else {
        pImpl_->window.styleMask &= ~NSWindowStyleMaskResizable;
    }
    
    NSButton* zoomButton = [pImpl_->window standardWindowButton:NSWindowZoomButton];
    if (zoomButton) {
        [zoomButton setHidden:!state];
        [zoomButton setEnabled:state];
    }
}

[[nodiscard]] bool Window::isResizable() const {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([this] { return isResizable(); });
    }
    
    NSUInteger styleMask = pImpl_->window.styleMask;
    return (styleMask & NSWindowStyleMaskResizable) == NSWindowStyleMaskResizable;
}

void Window::setDecorations(bool decorations) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread(
                                            [decorations, this] { return setDecorations(decorations); });
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
        return appHandler_->runOnMainThread([this] { return isDecorated(); });
    }
    NSWindowStyleMask styleMask = [pImpl_->window styleMask];
    return styleMask != NSWindowStyleMaskBorderless;
}

void Window::hide() {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([this] { hide(); });
    }
    [pImpl_->window orderOut:nil];
}

void Window::show() {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([this] { show(); });
    }
    [pImpl_->window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

void Window::center() {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread([this]() { center(); });
    }
    [pImpl_->window center];
}

void Window::enable(bool state){
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state]() { enable(state); });
  }

  [pImpl_->window setIgnoresMouseEvents:state ? NO: YES];

  if (state) {
    [pImpl_->window makeKeyAndOrderFront:nil];
  }
}

void Window::setBackgroundColor(int red, int green, int blue) {
    if (!appHandler_->isMainThread()) {
        return appHandler_->runOnMainThread(
                                            [this, red, green, blue]() { setBackgroundColor(red, green, blue); });
    }
    NSColor* color = [NSColor colorWithCalibratedRed:red / 255.0
                                               green:green / 255.0
                                                blue:blue / 255.0
                                               alpha:1.0];
    [pImpl_->view setBackgroundColor:color];
}

[[nodiscard]] void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->window); }

[[nodiscard]] void* Window::getContentView() { return static_cast<void*>(pImpl_->view); }
