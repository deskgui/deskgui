/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <WebKit/WebKit.h>

#include <vector>

#include "interfaces/webview_impl.h"

/**
 * CustomNavigationDelegate handles all WebKit navigation and message events.
 * It manages:
 * - Script message handling
 * - Navigation events
 * - URL scheme handling
 * - Context menu state
 */
@interface CustomNavigationDelegate
    : NSObject <WKNavigationDelegate, WKScriptMessageHandler, WKURLSchemeHandler>
@property(nonatomic) BOOL contextMenuEnabled;
- (instancetype)initWithWebview:(deskgui::Webview::Impl*)webview
                      resources:(std::vector<deskgui::Resource>*)resources;
@end

/**
 * CustomUIDelegate handles all WebKit UI events.
 * It manages:
 * - Web view delegate events
 */
@interface CustomUIDelegate : NSObject <WKUIDelegate>
- (instancetype)initWithWebview:(deskgui::Webview::Impl*)webview;
@end

/**
 * CustomWebview extends WKWebView to add enhanced functionality.
 * It handles:
 * - File drag and drop events (when enabled)
 * - Custom drag operations
 * - JavaScript event dispatching for drops
 * - Keyboard shortcuts (copy, paste, select all, etc.)
 * - First responder status
 */
@interface CustomWebview : WKWebView <NSDraggingDestination>
- (instancetype)initWithFrame:(NSRect)frame
                configuration:(WKWebViewConfiguration*)configuration
            enableDragAndDrop:(BOOL)enableDragAndDrop;
- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;
- (BOOL)performKeyEquivalent:(NSEvent*)event;
@end

namespace deskgui {

  extern NSString* const kSchemeUri;
  extern NSString* const kScriptMessageCallback;

  /**
   * Implementation details for the Webview class.
   * Contains the native WebKit components used by the Webview.
   */
  struct Webview::Impl::Platform {
    void* parentWindow = nullptr;                     ///< Parent window (NSView*)
    WKWebView* webview = nullptr;                     ///< The WebKit web view instance
    WKUserContentController* controller = nullptr;    ///< Controller for user content and scripts
    CustomUIDelegate* uiDelegate = nullptr;           ///< UI delegate for the webview
    WKWebViewConfiguration* configuration = nullptr;  ///< WebView configuration
    WKPreferences* preferences = nullptr;             ///< WebView preferences
    CustomNavigationDelegate* navigationDelegate = nullptr;  ///< Navigation delegate
  };

}  // namespace deskgui
