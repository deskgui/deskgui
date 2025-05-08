/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <shlwapi.h>

#include <iostream>

#include "app_handler_windows.h"
#include "js/drop.h"
#include "utils/strings.h"
#include "webview_windows_impl.h"

using namespace deskgui;
using namespace deskgui::utils;
using namespace Microsoft::WRL;

Webview::Webview(const std::string& name, AppHandler* appHandler, void* window,
                 const WebviewOptions& options)
    : pImpl_(std::make_unique<Impl>()), appHandler_(appHandler), name_(name) {
  if (window == nullptr) {
    throw std::invalid_argument("Window is a nullptr");
  }

  if (!pImpl_->createWebviewInstance(appHandler_->getName(), static_cast<HWND>(window), options)) {
    throw std::exception("Cannot initialize webview");
  }

  // Message received event
  pImpl_->webview->add_WebMessageReceived(
      Callback<ICoreWebView2WebMessageReceivedEventHandler>(
          [this]([[maybe_unused]] ICoreWebView2* sender,
                 ICoreWebView2WebMessageReceivedEventArgs* args) {
            if (pImpl_->handleDragAndDrop(args)) {
              return S_OK;
            }

            wil::unique_cotaskmem_string message;
            args->get_WebMessageAsJson(&message);
            onMessage(ws2s(message.get()));
            return S_OK;
          })
          .Get(),
      nullptr);

  // Handle the navigation starting event
  pImpl_->webview->add_NavigationStarting(
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
  pImpl_->webview->add_FrameNavigationStarting(
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
  pImpl_->webview->add_NavigationCompleted(
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
  pImpl_->webview->add_SourceChanged(
      Callback<ICoreWebView2SourceChangedEventHandler>(
          [=]([[maybe_unused]] ICoreWebView2* sender,
              [[maybe_unused]] ICoreWebView2SourceChangedEventArgs* args) -> HRESULT {
            emit(event::WebviewSourceChanged(getUrl()));
            return S_OK;
          })
          .Get(),
      nullptr);

  pImpl_->webview->add_NewWindowRequested(
      Callback<ICoreWebView2NewWindowRequestedEventHandler>(
          [=](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
            wil::unique_cotaskmem_string uri;
            args->get_Uri(&uri);

            event::WebviewWindowRequested event(ws2s(uri.get()));
            emit(event);

            if (event.isCancelled()) {
              args->put_Handled(true);
              return S_OK;
            }

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

  if (options.getOption<bool>(WebviewOptions::kActivateNativeDragAndDrop)) {
    injectScript(js::kWindowsDropListener);
  }

  enableAcceleratorKeys(false);
  show(true);
}

Webview::~Webview() {}

void Webview::enableDevTools(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state] { enableDevTools(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview->get_Settings(&settings);
  settings->put_AreDevToolsEnabled(state);
  pImpl_->webview->Reload();
}

void Webview::enableContextMenu(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state] { enableContextMenu(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview->get_Settings(&settings);
  settings->put_AreDefaultContextMenusEnabled(state);
  pImpl_->webview->Reload();
}

void Webview::enableZoom(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state] { enableZoom(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview->get_Settings(&settings);
  settings->put_IsZoomControlEnabled(state);
  pImpl_->webview->Reload();
}

void Webview::enableAcceleratorKeys(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state] { enableAcceleratorKeys(state); });
  }

  if (state) {
    if (pImpl_->acceleratorKeysToken) {
      pImpl_->webviewController->remove_AcceleratorKeyPressed(pImpl_->acceleratorKeysToken.value());
      pImpl_->acceleratorKeysToken.reset();
    }
  } else {
    if (!pImpl_->acceleratorKeysToken) {
      pImpl_->acceleratorKeysToken = EventRegistrationToken();

      pImpl_->webviewController->add_AcceleratorKeyPressed(
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
          &pImpl_->acceleratorKeysToken.value());
    }
  }
}

void Webview::resize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { resize(size); });
  }
  if (pImpl_->webviewController) {
    pImpl_->webviewController->put_Bounds(
        RECT{0, 0, static_cast<LONG>(size.first), static_cast<LONG>(size.second)});
  }
}

void Webview::setPosition(const ViewRect& rect) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, rect] { setPosition(rect); });
  }
  if (pImpl_->webviewController) {
    pImpl_->webviewController->put_Bounds(RECT{static_cast<LONG>(rect.L), static_cast<LONG>(rect.T),
                                               static_cast<LONG>(rect.R),
                                               static_cast<LONG>(rect.B)});
  }
}

void Webview::show(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state] { show(state); });
  }
  if (pImpl_->webviewController) {
    pImpl_->webviewController->put_IsVisible(static_cast<BOOL>(state));
  }
}

void Webview::navigate(const std::string& url) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, url] { navigate(url); });
  }

  pImpl_->webview->Navigate(s2ws(url).c_str());
}

void Webview::loadFile(const std::string& path) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, path] { loadFile(path); });
  }
  std::string filePath = "file://" + path;
  pImpl_->webview->Navigate(s2ws(filePath).c_str());
}

void Webview::loadHTMLString(const std::string& html) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, html] { loadHTMLString(html); });
  }

  pImpl_->webview->NavigateToString(s2ws(html).c_str());
}

void Webview::loadResources(Resources&& resources) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread(
        [&resources, this]() { loadResources(std::move(resources)); });
  }

  resources_ = std::move(resources);

  if (!pImpl_->webResourceRequestedToken) {
    pImpl_->webResourceRequestedToken = EventRegistrationToken();

    pImpl_->webview->AddWebResourceRequestedFilter((Webview::kWOrigin + L"*").c_str(),
                                                   COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

    pImpl_->webview->add_WebResourceRequested(
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
              auto it = std::find_if(resources_.begin(), resources_.end(),
                                     [&](const Resource& resource) {
                                       return (Webview::kOrigin + resource.scheme) == requestedUrl;
                                     });

              if (it != resources_.end()) {
                auto webview2 = pImpl_->webview.try_query<ICoreWebView2_2>();
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
        &pImpl_->webResourceRequestedToken.value());
  }
}

void Webview::serveResource(const std::string& resourceUrl) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, resourceUrl] { serveResource(resourceUrl); });
  }
  navigate(Webview::kOrigin + resourceUrl);
}

void Webview::clearResources() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { clearResources(); });
  }

  resources_.clear();

  if (pImpl_->webResourceRequestedToken) {
    pImpl_->webview->remove_WebResourceRequested(pImpl_->webResourceRequestedToken.value());
    pImpl_->webview->RemoveWebResourceRequestedFilter((Webview::kWOrigin + L"*").c_str(),
                                                      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
    pImpl_->webResourceRequestedToken.reset();
  }
}

[[nodiscard]] const std::string Webview::getUrl() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { return getUrl(); });
  }

  wil::unique_cotaskmem_string url;
  pImpl_->webview->get_Source(&url);
  return ws2s(url.get());
}

void Webview::injectScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, script] { injectScript(script); });
  }

  pImpl_->webview->AddScriptToExecuteOnDocumentCreated(s2ws(script).c_str(), nullptr);
}

void Webview::executeScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, script] { executeScript(script); });
  }
  pImpl_->webview->ExecuteScript(s2ws(script).c_str(), nullptr);
}
