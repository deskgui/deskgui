/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include "deskgui/webview.h"

#include <WebKit/WebKit.h>
#include <vector>

/**
 * CustomNavigationDelegate handles all WebKit navigation and message events.
 * It manages:
 * - Script message handling
 * - Navigation events
 * - URL scheme handling
 * - Context menu state
 */
@interface CustomNavigationDelegate : NSObject <WKNavigationDelegate, WKScriptMessageHandler, WKURLSchemeHandler>
@property (nonatomic) BOOL contextMenuEnabled;
- (instancetype)initWithWebview:(deskgui::Webview*)webview resources:(std::vector<deskgui::Resource>*)resources;
@end

/**
 * CustomUIDelegate handles all WebKit UI events.
 * It manages:
 * - Web view delegate events
 */
@interface CustomUIDelegate : NSObject <WKUIDelegate>
- (instancetype)initWithWebview:(deskgui::Webview*)webview;
@end

/**
 * CustomWebview extends WKWebView to add drag and drop functionality.
 * It handles:
 * - File drag and drop events
 * - Custom drag operations
 * - JavaScript event dispatching for drops
 */
@interface DragAndDropWebview : WKWebView <NSDraggingDestination>
- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration*)configuration;
@end

namespace deskgui {

extern NSString* const kSchemeUri;
extern NSString* const kScriptMessageCallback;

/**
 * Implementation details for the Webview class.
 * Contains the native WebKit components used by the Webview.
 */
struct Webview::Impl {
    WKWebView* webview = nullptr;        ///< The WebKit web view instance
    WKUserContentController* controller = nullptr;  ///< Controller for user content and scripts
    CustomUIDelegate* uiDelegate = nullptr;  ///< UI delegate for the webview
    WKWebViewConfiguration* configuration = nullptr;  ///< WebView configuration
    WKPreferences* preferences = nullptr;  ///< WebView preferences
    CustomNavigationDelegate* navigationDelegate = nullptr;  ///< Navigation delegate
};

} // namespace deskgui
