/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "utils/webview_js.h"
#include "webview_darwin_impl.h"


using namespace deskgui;
using namespace deskgui::utils;

// Global constants
NSString* const deskgui::kSchemeUri = [NSString stringWithUTF8String:Webview::kProtocol];
NSString* const deskgui::kScriptMessageCallback = @"deskgui_callback";

@implementation CustomNavigationDelegate {
  deskgui::Webview* webview_;
  std::vector<deskgui::Resource>* resources_;
}

- (instancetype)initWithWebview:(deskgui::Webview*)webview
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
    NSData* data =
        [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted error:nil];
    NSString* jsonString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    webview_->onMessage([jsonString UTF8String]);
  }
}

- (void)webView:(WKWebView*)webView didFinishNavigation:(WKNavigation*)navigation {
  if (self.contextMenuEnabled) {
    [webView evaluateJavaScript:
                 @"window.removeEventListener('contextmenu', (event) => event.preventDefault());"
              completionHandler:nil];
  } else {
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
  std::string url = [urlString UTF8String];

  event::WebviewNavigationStarting event{url};
  webview_->emit(event);

  if (event.isCancelled()) {
    decisionHandler(WKNavigationActionPolicyCancel);
  } else {
    decisionHandler(WKNavigationActionPolicyAllow);
  }
}

- (void)webView:(WKWebView*)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
  if ([urlSchemeTask.request.URL.scheme isEqualToString:kSchemeUri] && resources_ != nullptr) {
    NSString* resourceURL = urlSchemeTask.request.URL.relativeString;
    std::string resourceURLString = [resourceURL UTF8String];
    std::string resourceScheme = resourceURLString.substr(Webview::kOrigin.size());

    size_t secondLastDotPos = resourceScheme.rfind('.', resourceScheme.rfind('.') - 1);
    if (secondLastDotPos != std::string::npos) {
      resourceScheme = resourceScheme.substr(secondLastDotPos + 1);
      size_t firstSlash = resourceScheme.find('/');
      if (firstSlash != std::string::npos) {
        resourceScheme = resourceScheme.substr(firstSlash + 1);
      }
    }

    auto it = std::find_if(
        resources_->begin(), resources_->end(),
        [&](const deskgui::Resource& resource) { return resource.scheme == resourceScheme; });

    if (it != resources_->end()) {
      NSData* resourceData = [NSData dataWithBytes:it->content.data() length:it->content.size()];
      NSString* mime = [NSString stringWithUTF8String:it->mime.c_str()];
      NSURLResponse* response = [[NSURLResponse alloc] initWithURL:urlSchemeTask.request.URL
                                                          MIMEType:mime
                                             expectedContentLength:resourceData.length
                                                  textEncodingName:nil];

      [urlSchemeTask didReceiveResponse:response];
      [urlSchemeTask didReceiveData:resourceData];
      [urlSchemeTask didFinish];
    }
    return;
  }
}

- (void)webView:(WKWebView*)webView stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
  // Todo
}

@end

@implementation CustomUIDelegate {
  deskgui::Webview* webview_;
}

- (instancetype)initWithWebview:(deskgui::Webview*)webview {
  self = [super init];
  if (self) {
    webview_ = webview;
  }
  return self;
}

- (WKWebView*)webView:(WKWebView*)webView
    createWebViewWithConfiguration:(WKWebViewConfiguration*)configuration
               forNavigationAction:(WKNavigationAction*)navigationAction
                    windowFeatures:(WKWindowFeatures*)windowFeatures {
  NSURL* url = navigationAction.request.URL;

  if (!url) {
    return nil;
  }

  // Emit the window requested event
  event::WebviewWindowRequested event(url.absoluteString.UTF8String);
  webview_->emit(event);

  // If the event was cancelled, don't load the URL
  if (event.isCancelled()) {
    return nil;
  }

  // Load the URL in the same webview
  [webView loadRequest:navigationAction.request];

  // Return nil to indicate that we didn't create a new webview
  return nil;
}

@end

@implementation DragAndDropWebview

- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration*)configuration {
  self = [super initWithFrame:frame configuration:configuration];
  if (self) {
    [self registerForDraggedTypes:@[ NSPasteboardTypeFileURL ]];
  }
  return self;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  return NSDragOperationCopy;
}

- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender {
  NSPasteboard* pasteboard = [sender draggingPasteboard];
  NSArray<NSURL*>* fileURLs =
      [pasteboard readObjectsForClasses:@[ [NSURL class] ]
                                options:@{NSPasteboardURLReadingFileURLsOnlyKey : @YES}];

  if (fileURLs.count == 0) {
    return NO;
  }

  // Convert NSArray of NSURLs to std::vector of paths
  std::vector<std::filesystem::path> paths;
  for (NSURL* fileURL in fileURLs) {
    paths.push_back([fileURL.path UTF8String]);
  }

  // Get drop location in view coordinates
  NSPoint dropPoint = [self convertPoint:[sender draggingLocation] fromView:nil];

  // Use the utility function to generate the JavaScript code
  std::string jsCode = createDropEventJS(paths, dropPoint.x, dropPoint.y);
  [self evaluateJavaScript:[NSString stringWithUTF8String:jsCode.c_str()] completionHandler:nil];

  return YES;
}

@end
