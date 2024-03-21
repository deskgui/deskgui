/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <shlwapi.h>

#include <iostream>

#include "app_handler_windows.h"
#include "webview_windows_impl.h"
#include "window_windows_impl.h"

using namespace deskgui;
using namespace Microsoft::WRL;

Webview::Webview(const std::string& name, AppHandler* appHandler, void* window,
                 const WebviewOptions& options)
    : name_(name), appHandler_(appHandler), pImpl_(std::make_unique<Impl>()) {
  if (window == nullptr) {
    throw std::invalid_argument("Window is a nullptr");
  }

  if (!pImpl_->createWebviewInstance(static_cast<HWND>(window), options)) {
    throw std::exception("Cannot initialize webview");
  }

  // Message received event
  pImpl_->webview_->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                               [=]([[maybe_unused]] ICoreWebView2* sender,
                                                   ICoreWebView2WebMessageReceivedEventArgs* args) {
                                                 wil::unique_cotaskmem_string jsonString;
                                                 args->get_WebMessageAsJson(&jsonString);
                                                 onMessage(ws2s(jsonString.get()).c_str());
                                                 return S_OK;
                                               })
                                               .Get(),
                                           nullptr);

  // Handle the navigation starting event
  pImpl_->webview_->add_NavigationStarting(
      Callback<ICoreWebView2NavigationStartingEventHandler>(
          [=](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
            wil::unique_cotaskmem_string uri;
            args->get_Uri(&uri);
            event::WebviewNavigationStarting event(ws2s(uri.get()));
            emit(event);

            if (event.isCancelled()) {
              sender->Stop();
            }
            return S_OK;
          })
          .Get(),
      nullptr);

  // Handle frame navigation starting event (if needed)
  pImpl_->webview_->add_FrameNavigationStarting(
      Callback<ICoreWebView2NavigationStartingEventHandler>(
          [=](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
            wil::unique_cotaskmem_string uri;
            args->get_Uri(&uri);

            event::WebviewFrameNavigationStarting event(ws2s(uri.get()));
            emit(event);

            if (event.isCancelled()) {
              sender->Stop();
            }
            return S_OK;
          })
          .Get(),
      nullptr);

  // Handle navigation completed event
  pImpl_->webview_->add_NavigationCompleted(
      Callback<ICoreWebView2NavigationCompletedEventHandler>(
          [=]([[maybe_unused]] ICoreWebView2* sender,
              ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
            BOOL success;
            args->get_IsSuccess(&success);
            emit(event::WebviewContentLoaded(static_cast<bool>(success)));
            return S_OK;
          })
          .Get(),
      nullptr);

  // Handle source changed event
  pImpl_->webview_->add_SourceChanged(
      Callback<ICoreWebView2SourceChangedEventHandler>(
          [=]([[maybe_unused]] ICoreWebView2* sender,
              [[maybe_unused]] ICoreWebView2SourceChangedEventArgs* args) -> HRESULT {
            emit(event::WebviewSourceChanged(getUrl()));
            return S_OK;
          })
          .Get(),
      nullptr);

  injectScript(R"(
                window.webview = {
                    async postMessage(message) 
                    {
                        window.chrome.webview.postMessage(message);
                    }
                };
                )");
  enableAcceleratorKeys(false);
  show(true);
}

Webview::~Webview() {}

void Webview::enableDevTools(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { enableDevTools(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  settings->put_AreDevToolsEnabled(state);
  pImpl_->webview_->Reload();
}

void Webview::enableContextMenu(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { enableContextMenu(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  settings->put_AreDefaultContextMenusEnabled(state);
  pImpl_->webview_->Reload();
}

void Webview::enableZoom(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { enableZoom(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  settings->put_IsZoomControlEnabled(state);
  pImpl_->webview_->Reload();
}

void Webview::enableAcceleratorKeys(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { enableAcceleratorKeys(state); });
  }

  if (state) {
    if (pImpl_->acceleratorKeysToken_) {
      pImpl_->webviewController_->remove_AcceleratorKeyPressed(
          pImpl_->acceleratorKeysToken_.value());
      pImpl_->acceleratorKeysToken_.reset();
    }
  } else {
    if (!pImpl_->acceleratorKeysToken_) {
      pImpl_->acceleratorKeysToken_ = EventRegistrationToken();

      pImpl_->webviewController_->add_AcceleratorKeyPressed(
          Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
              [this](ICoreWebView2Controller* sender,
                     ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT {
                wil::com_ptr<ICoreWebView2AcceleratorKeyPressedEventArgs2> args2;
                args->QueryInterface(IID_PPV_ARGS(&args2));
                if (args2) {
                  args2->put_IsBrowserAcceleratorKeyEnabled(FALSE);
                }
                return S_OK;
              })
              .Get(),
          &pImpl_->acceleratorKeysToken_.value());
    }
  }
}

void Webview::resize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { resize(size); });
  }
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_Bounds(
        RECT{0, 0, static_cast<LONG>(size.first), static_cast<LONG>(size.second)});
  }
}

void Webview::setPosition(const ViewRect& rect) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { setPosition(rect); });
  }
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_Bounds(
        RECT{static_cast<LONG>(rect.L), static_cast<LONG>(rect.T), static_cast<LONG>(rect.R),
             static_cast<LONG>(rect.B)});
  }
}

void Webview::show(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { show(state); });
  }
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_IsVisible(static_cast<BOOL>(state));
  }
}

void Webview::navigate(const std::string& url) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { navigate(url); });
  }

  pImpl_->webview_->Navigate(s2ws(url).c_str());
}

void Webview::loadFile(const std::string& path) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { loadFile(path); });
  }
  std::string filePath = "file://" + path;
  pImpl_->webview_->Navigate(s2ws(filePath).c_str());
}

void Webview::loadHTMLString(const std::string& html) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { loadHTMLString(html); });
  }

  pImpl_->webview_->NavigateToString(s2ws(html).c_str());
}

void Webview::loadResources(Resources&& resources) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([&resources, this]() { loadResources(std::move(resources)); });
  }

  resources_ = std::move(resources);

  if (!pImpl_->webResourceRequestedToken_) {
    pImpl_->webResourceRequestedToken_ = EventRegistrationToken();

    pImpl_->webview_->AddWebResourceRequestedFilter((s2ws(pImpl_->rootScheme_) + L"*").c_str(),
                                                    COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

    pImpl_->webview_->add_WebResourceRequested(
        Callback<ICoreWebView2WebResourceRequestedEventHandler>(
            [=](ICoreWebView2* sender,
                ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
              // Get the request object
              wil::com_ptr<ICoreWebView2WebResourceRequest> request;
              HRESULT hr = args->get_Request(&request);
              if (FAILED(hr)) {
                return hr;
              }

              // Get the URL of the requested resource
              wil::unique_cotaskmem_string url;
              hr = request->get_Uri(&url);
              if (FAILED(hr)) {
                return hr;
              }

              std::string requestedUrl = ws2s(url.get());

              // Check if the requested URL matches the resource you want to load
              auto it = std::find_if(
                  resources_.begin(), resources_.end(), [&](const Resource& resource) {
                    return (pImpl_->rootScheme_ + resource.scheme) == requestedUrl;
                  });

              if (it != resources_.end()) {
                auto webview2 = pImpl_->webview_.try_query<ICoreWebView2_2>();
                if (webview2) {
                  wil::com_ptr<ICoreWebView2Environment> env;
                  webview2->get_Environment(&env);

                  // Create an IStream object from the content
                  wil::com_ptr<IStream> contentStream
                      = SHCreateMemStream(reinterpret_cast<const BYTE*>(it->content.data()),
                                          static_cast<UINT>(it->content.size()));

                  wil::com_ptr<ICoreWebView2WebResourceResponse> response;
                  hr = env->CreateWebResourceResponse(contentStream.get(), 200, L"OK",
                                                      s2ws("Content-Type:" + it->mime).c_str(),
                                                      &response);
                  if (FAILED(hr)) {
                    return hr;
                  }

                  args->put_Response(response.get());
                }
              }

              return S_OK;
            })
            .Get(),
        &pImpl_->webResourceRequestedToken_.value());
  }
}

void Webview::serveResource(const std::string& resourceScheme) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { serveResource(resourceScheme); });
  }
  navigate(pImpl_->rootScheme_ + resourceScheme);
}

void Webview::clearResources() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { clearResources(); });
  }

  resources_.clear();

  if (pImpl_->webResourceRequestedToken_) {
    pImpl_->webview_->remove_WebResourceRequested(pImpl_->webResourceRequestedToken_.value());
    pImpl_->webview_->RemoveWebResourceRequestedFilter((s2ws(pImpl_->rootScheme_) + L"*").c_str(),
                                                       COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
    pImpl_->webResourceRequestedToken_.reset();
  }
}

[[nodiscard]] const std::string Webview::getUrl() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getUrl(); });
  }

  wil::unique_cotaskmem_string url;
  pImpl_->webview_->get_Source(&url);
  return ws2s(url.get());
}

void Webview::injectScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { injectScript(script); });
  }

  pImpl_->webview_->AddScriptToExecuteOnDocumentCreated(s2ws(script).c_str(), nullptr);
}

void Webview::executeScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { executeScript(script); });
  }
  pImpl_->webview_->ExecuteScript(s2ws(script).c_str(), nullptr);
}
