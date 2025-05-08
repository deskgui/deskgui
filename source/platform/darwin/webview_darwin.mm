/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "app_handler_darwin.h"
#include "webview_darwin_impl.h"

using namespace deskgui;

Webview::Webview(const std::string& name, AppHandler* appHandler, void* window,
                 [[maybe_unused]] const WebviewOptions& options)
    : pImpl_(std::make_unique<Impl>()), appHandler_(appHandler), name_(name) {
  if (window == nullptr) {
    throw std::invalid_argument("Window is a nullptr");
  }

  pImpl_->controller = [[WKUserContentController alloc] init];

  pImpl_->configuration = [[WKWebViewConfiguration alloc] init];
  pImpl_->configuration.userContentController = pImpl_->controller;

  pImpl_->preferences = [[WKPreferences alloc] init];
  pImpl_->configuration.preferences = pImpl_->preferences;

  pImpl_->navigationDelegate =
      [[CustomNavigationDelegate alloc] initWithWebview:this resources:&resources_];
  [pImpl_->controller addScriptMessageHandler:pImpl_->navigationDelegate
                                         name:kScriptMessageCallback];
  [pImpl_->configuration setURLSchemeHandler:pImpl_->navigationDelegate forURLScheme:kSchemeUri];

  auto pool = pImpl_->configuration.processPool;
  SEL selector = NSSelectorFromString(@"_registerURLSchemeAsSecure:");
  if ([pool respondsToSelector:selector]) {
    NSMethodSignature* signature = [pool methodSignatureForSelector:selector];
    if (signature) {
      NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
      [invocation setTarget:pool];
      [invocation setSelector:selector];
      id schemeUri = kSchemeUri;
      [invocation setArgument:&schemeUri atIndex:2];
      [invocation invoke];
    }
  }

  const auto nativeDragAndDrop
      = options.getOption<bool>(WebviewOptions::kActivateNativeDragAndDrop);
  if (nativeDragAndDrop) {
    pImpl_->webview =
        [[DragAndDropWebview alloc] initWithFrame:CGRectZero configuration:pImpl_->configuration];
  } else {
    pImpl_->webview =
        [[WKWebView alloc] initWithFrame:CGRectZero configuration:pImpl_->configuration];
  }
  [pImpl_->webview setNavigationDelegate:pImpl_->navigationDelegate];

  // Set up UI delegate
  pImpl_->uiDelegate = [[CustomUIDelegate alloc] initWithWebview:this];
  [pImpl_->webview setUIDelegate:pImpl_->uiDelegate];

  [pImpl_->webview setFrame:[(__bridge id)window frame]];
  [pImpl_->webview setValue:@NO forKey:@"drawsBackground"];

  pImpl_->webview.translatesAutoresizingMaskIntoConstraints = YES;
  [(__bridge id)window addSubview:pImpl_->webview];

  injectScript(R"(
              window.webview = {
                  async postMessage(message)
                  {
                    webkit.messageHandlers.deskgui_callback.postMessage(message);
                  }
              };
              )");
  show(true);
}

Webview::~Webview() {
  [pImpl_->webview removeFromSuperview];
  [pImpl_->controller removeScriptMessageHandlerForName:kScriptMessageCallback];
}

void Webview::enableDevTools(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this]() { enableDevTools(state); });
  }
  [pImpl_->preferences setValue:@(state) forKey:@"developerExtrasEnabled"];
}

void Webview::enableContextMenu(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this]() { enableContextMenu(state); });
  }
  CustomNavigationDelegate* navigationDelegate
      = static_cast<CustomNavigationDelegate*>(pImpl_->webview.navigationDelegate);
  navigationDelegate.contextMenuEnabled = state;
  [pImpl_->webview reload];
}

void Webview::enableZoom(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this]() { enableZoom(state); });
  }
  [pImpl_->webview setAllowsMagnification:[@(state ? YES : NO) boolValue]];
}

void Webview::enableAcceleratorKeys([[maybe_unused]] bool state) {
  // TODO
}

void Webview::resize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this]() { resize(size); });
  }
  [pImpl_->webview setFrameSize:NSMakeSize(size.first, size.second)];
}

void Webview::setPosition(const ViewRect& rect) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([rect, this]() { setPosition(rect); });
  }
  [pImpl_->webview setFrame:NSMakeRect(rect.L, rect.T, rect.R - rect.L, rect.B - rect.T)];
}

void Webview::show(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this]() { show(state); });
  }
  [pImpl_->webview setHidden:!state];
}

void Webview::navigate(const std::string& url) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([url, this]() { navigate(url); });
  }
  NSURL* nsURL = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
  NSURLRequest* request = [NSURLRequest requestWithURL:nsURL];
  [pImpl_->webview loadRequest:request];
}

void Webview::loadFile(const std::string& path) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([path, this]() { loadFile(path); });
  }
  NSURL* nsurl =
      [NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()] isDirectory:FALSE];
  [pImpl_->webview loadFileURL:nsurl
       allowingReadAccessToURL:[nsurl URLByDeletingLastPathComponent]];
}

[[nodiscard]] const std::string Webview::getUrl() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this]() { return getUrl(); });
  }
  NSURL* currentURL = pImpl_->webview.URL;
  if (currentURL) {
    return std::string(currentURL.absoluteString.UTF8String);
  }
  return "";
}

void Webview::loadResources(Resources&& resources) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread(
        [&resources, this]() { loadResources(std::move(resources)); });
  }
  resources_ = std::move(resources);
}

void Webview::serveResource(const std::string& resourceUrl) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([resourceUrl, this] { serveResource(resourceUrl); });
  }
  navigate(Webview::kOrigin + resourceUrl);
}

void Webview::clearResources() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { clearResources(); });
  }
  resources_.clear();
}

void Webview::loadHTMLString(const std::string& html) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([html, this]() { loadHTMLString(html); });
  }
  [pImpl_->webview loadHTMLString:[NSString stringWithUTF8String:html.c_str()] baseURL:nil];
}

void Webview::injectScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([script, this]() { injectScript(script); });
  }
  WKUserScript* script1 =
      [[WKUserScript alloc] initWithSource:[NSString stringWithUTF8String:script.c_str()]
                             injectionTime:WKUserScriptInjectionTimeAtDocumentStart
                          forMainFrameOnly:YES];
  [pImpl_->controller addUserScript:script1];
}

void Webview::executeScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([script, this]() { executeScript(script); });
  }
  [pImpl_->webview evaluateJavaScript:[NSString stringWithUTF8String:script.c_str()]
                    completionHandler:nil];
}
