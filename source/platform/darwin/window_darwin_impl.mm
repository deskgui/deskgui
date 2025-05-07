/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "window_darwin_impl.h"
#include "app_handler_darwin.h"

using namespace deskgui;

@implementation WindowDelegate

- (instancetype)initWithWindow:(Window*)window appHandler:(AppHandler*)appHandler {
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

@implementation WindowObserver

- (instancetype)initWithWindow:(Window*)window
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
