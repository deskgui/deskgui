/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "app_handler_darwin.h"
#include "webview_darwin_impl.h"

using namespace deskgui;

static const std::string kSchemeUriString = "deskgui://";
extern NSString* const kSchemeUri = @"deskgui";
extern NSString* const kScriptMessageCallback = @"deskgui_callback";

@interface CustomNavigationDelegate
    : NSObject <WKScriptMessageHandler, WKNavigationDelegate, WKURLSchemeHandler> {
  Webview* webview_;
  std::vector<deskgui::Resource>* resources_;
}
@property(nonatomic, assign) BOOL contextMenuEnabled;
@end

@implementation CustomNavigationDelegate

- (instancetype)initWithWebview:(Webview*)webview
                      resources:(std::vector<deskgui::Resource>*)resources {
  self = [super init];
  if (self) {
    webview_ = webview;
    resources_ = resources;
  }
  return self;
}

- (void)userContentController:(nonnull WKUserContentController*)userContentController
      didReceiveScriptMessage:(nonnull WKScriptMessage*)message {
  if ([[message name] isEqualToString:kScriptMessageCallback]) {
    NSDictionary* dict = (NSDictionary*)message.body;
    NSData* data = [NSJSONSerialization dataWithJSONObject:dict
                                                   options:NSJSONWritingPrettyPrinted
                                                     error:nil];
    NSString* jsonString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    webview_->onMessage([jsonString UTF8String]);
  }
}

- (void)webView:(WKWebView*)webView didFinishNavigation:(WKNavigation*)navigation {
  if (self.contextMenuEnabled) {
    // Restore the original state if context menu is enabled
    [webView evaluateJavaScript:
                 @"window.removeEventListener('contextmenu', (event) => event.preventDefault());"
              completionHandler:nil];
  } else {
    // Disable the context menu
    [webView evaluateJavaScript:
                 @"window.addEventListener('contextmenu', (event) => event.preventDefault());"
              completionHandler:nil];
  }
  webview_->emit(event::WebviewContentLoaded{true});
}

- (void)webView:(WKWebView*)webView didCommitNavigation:(WKNavigation*)navigation {
  if (webView.URL) {
    NSString* urlString = [webView.URL absoluteString];
    const char* urlCString = [urlString UTF8String];
    std::string url = urlCString ? urlCString : "";
    webview_->emit(event::WebviewSourceChanged{url});
  }
}

- (void)webView:(WKWebView*)webView
    decidePolicyForNavigationAction:(WKNavigationAction*)navigationAction
                    decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler {
  NSString* urlString = navigationAction.request.URL.absoluteString;
  std::string url = [urlString UTF8String];  // Convert NSString to std::string

  event::WebviewNavigationStarting event{url};
  webview_->emit(event);

  if (event.isCancelled()) {
    decisionHandler(WKNavigationActionPolicyCancel);
  } else {
    decisionHandler(WKNavigationActionPolicyAllow);
  }
}

- (void)webView:(WKWebView*)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
  // Handle custom URL scheme requests

  if ([urlSchemeTask.request.URL.scheme isEqualToString:kSchemeUri] && resources_ != nullptr) {
    // Get the resource data for the requested URL
    NSString* resourceURL = urlSchemeTask.request.URL.relativeString;
    std::string resourceURLString = [resourceURL UTF8String];
    std::string resourceScheme = resourceURLString.substr(kSchemeUriString.size());

    size_t secondLastDotPos = resourceScheme.rfind('.', resourceScheme.rfind('.') - 1);
    if (secondLastDotPos != std::string::npos) {
      resourceScheme = resourceScheme.substr(secondLastDotPos + 1);
      size_t firstSlash = resourceScheme.find('/');
      if (firstSlash != std::string::npos) {
        resourceScheme = resourceScheme.substr(firstSlash + 1);
      }
    }

    // Check if the requested URL matches the resource you want to load
    auto it = std::find_if(
        resources_->begin(), resources_->end(),
        [&](const deskgui::Resource& resource) { return resource.scheme == resourceScheme; });

    if (it != resources_->end()) {
      NSData* resourceData = [NSData dataWithBytes:it->content.data() length:it->content.size()];
      NSString* mime = [NSString stringWithUTF8String:it->mime.c_str()];
      // Create a NSURLResponse with the resource data
      NSURLResponse* response = [[NSURLResponse alloc] initWithURL:urlSchemeTask.request.URL
                                                          MIMEType:mime
                                             expectedContentLength:resourceData.length
                                                  textEncodingName:nil];

      // Send the response and resource data to the URL scheme task
      [urlSchemeTask didReceiveResponse:response];
      [urlSchemeTask didReceiveData:resourceData];
      [urlSchemeTask didFinish];
    }
    return;
  }
}

- (void)webView:(WKWebView *)webView stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
    // Todo
}

@end

Webview::Webview(const std::string& name, AppHandler* appHandler, void* window, const WebviewOptions& options)
    : name_(name), appHandler_(appHandler), pImpl_(std::make_unique<Impl>()) {
  if (window == nullptr) {
    throw std::invalid_argument("Window is a nullptr");
  }
  // Create a WKUserContentController
  pImpl_->controller = [[WKUserContentController alloc] init];

  // Configure the WKWebView with the user content controller
  WKWebViewConfiguration* configuration = [[WKWebViewConfiguration alloc] init];
  configuration.userContentController = pImpl_->controller;

  WKPreferences* preferences = [[WKPreferences alloc] init];
  configuration.preferences = preferences;

  // Script handler
  CustomNavigationDelegate* scriptHandler =
      [[CustomNavigationDelegate alloc] initWithWebview:this resources:&resources_];
  [pImpl_->controller addScriptMessageHandler:scriptHandler name:kScriptMessageCallback];
  [configuration setURLSchemeHandler:scriptHandler forURLScheme:kSchemeUri];

  // Create a WKWebView
  pImpl_->webview = [[WKWebView alloc] initWithFrame:CGRectZero configuration:configuration];
  [pImpl_->webview setNavigationDelegate:scriptHandler];

  // Add the WKWebView to the native window or view
  NSWindow* nativeWindow = static_cast<NSWindow*>(window);
  NSView* view = nativeWindow.contentView;

  [view addSubview:pImpl_->webview];

  pImpl_->webview.translatesAutoresizingMaskIntoConstraints = YES;
  [pImpl_->webview setFrame:[view frame]];

  // Add Auto Layout constraints
  // [pImpl_->webview.leadingAnchor constraintEqualToAnchor:view.leadingAnchor].active = YES;
  // [pImpl_->webview.trailingAnchor constraintEqualToAnchor:view.trailingAnchor].active = YES;
  // [pImpl_->webview.topAnchor constraintEqualToAnchor:view.topAnchor].active = YES;
  // [pImpl_->webview.bottomAnchor constraintEqualToAnchor:view.bottomAnchor].active = YES;

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
  [pImpl_->webview release];
  pImpl_->webview = nullptr;

  [pImpl_->controller release];
  pImpl_->controller = nullptr;
};

void Webview::enableDevTools(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this]() { enableDevTools(state); });
  }
  [pImpl_->webview.configuration.preferences setValue:@(state) forKey:@"developerExtrasEnabled"];
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
  // Load the specified URL in the WKWebView
  NSURL* nsURL = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
  NSURLRequest* request = [NSURLRequest requestWithURL:nsURL];
  [pImpl_->webview loadRequest:request];
}

void Webview::loadFile(const std::string& path) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([path, this]() { loadFile(path); });
  }
  NSURL* nsurl = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()]
                            isDirectory:FALSE];
  [pImpl_->webview loadFileURL:nsurl
       allowingReadAccessToURL:[nsurl URLByDeletingLastPathComponent]];
}

[[nodiscard]] const std::string Webview::getUrl() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=]() { return getUrl(); });
  }
  // Get the current URL of the WKWebView
  NSURL* currentURL = pImpl_->webview.URL;
  if (currentURL) {
    return std::string(currentURL.absoluteString.UTF8String);
  }
  return "";
}

void Webview::loadResources(const Resources& resources) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([resources, this]() { loadResources(resources); });
  }

  resources_ = resources;
}

void Webview::serveResource(const std::string& resourceUrl) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([resourceUrl, this] { serveResource(resourceUrl); });
  }
  navigate(kSchemeUriString + resourceUrl);
}

void Webview::clearResources() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { clearResources(); });
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

  // this script adds a function IPlugSendMsg that is used to call the platform webview messaging
  // function in JS
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
};
