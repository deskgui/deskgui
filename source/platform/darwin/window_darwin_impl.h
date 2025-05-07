/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include "deskgui/window.h"

#include <AppKit/AppKit.h>

/**
 * WindowDelegate handles all NSWindow delegate events.
 * It manages:
 * - Window show events
 * - Window close events
 * - Window resize events
 * - Window zoom events
 */
@interface WindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic, assign) deskgui::Window* window;
@property(nonatomic, assign) deskgui::AppHandler* appHandler;
- (instancetype)initWithWindow:(deskgui::Window*)window appHandler:(deskgui::AppHandler*)appHandler;
@end

/**
 * WindowObserver handles all NSWindow notifications.
 * It manages:
 * - Window show notifications
 * - Window close notifications
 * - Window resize notifications
 */
@interface WindowObserver : NSObject
@property(nonatomic, assign) deskgui::Window* window;
@property(nonatomic, assign) NSWindow* nativeWindow;
@property(nonatomic, assign) deskgui::AppHandler* appHandler;

- (instancetype)initWithWindow:(deskgui::Window*)window
                  nativeWindow:(NSWindow*)nativeWindow
                    appHandler:(deskgui::AppHandler*)appHandler;
- (void)stopObserving;
@end

namespace deskgui {

/**
 * Implementation details for the Window class.
 * Contains the native NSWindow components used by the Window.
 */
struct Window::Impl {
    NSWindow* window = nullptr;        ///< The NSWindow instance
    NSView* view = nullptr;           ///< The content view of the window
    id observer = nullptr;            ///< Window observer for notifications
};

} // namespace deskgui
