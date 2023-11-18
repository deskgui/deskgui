/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <nlohmann/json.hpp>

#include "app_handler_linux.h"
#include "webview_linux_impl.h"
#include "window_linux_impl.h"

using namespace deskgui;

Webview::Webview(const std::string& name, AppHandler* appHandler, void* window)
    : name_(name), appHandler_(appHandler), pImpl_(std::make_unique<Impl>()) {
  if (window == nullptr) {
    throw std::invalid_argument("Window is a nullptr");
  }

  GtkWindow* parentWindow = GTK_WINDOW(window);

  GtkScrolledWindow* scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
  gtk_container_add(GTK_CONTAINER(parentWindow), GTK_WIDGET(scrolledWindow));

  pImpl_->container = GTK_FIXED(gtk_fixed_new());
  gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(pImpl_->container));

  pImpl_->webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
  if (!pImpl_->webview) {
    throw std::runtime_error("Failed to create webview.");
  }
  int x, y;
  gtk_window_get_position(GTK_WINDOW(window), &x, &y);

  gtk_fixed_put(pImpl_->container, GTK_WIDGET(pImpl_->webview), kDefaultWindowRect.L,
                kDefaultWindowRect.T);

  gtk_widget_set_size_request(GTK_WIDGET(pImpl_->webview), kDefaultWindowRect.R,
                              kDefaultWindowRect.B);

  gtk_widget_grab_focus(GTK_WIDGET(pImpl_->webview));

  WebKitSettings* settings = webkit_web_view_get_settings(pImpl_->webview);
  webkit_settings_set_enable_javascript(settings, TRUE);

  g_signal_connect(pImpl_->webview, "load-changed", G_CALLBACK(pImpl_->onLoadChanged), this);
  g_signal_connect(pImpl_->webview, "decide-policy", G_CALLBACK(pImpl_->onNavigationRequest), this);

  WebKitUserContentManager* contentManager
      = webkit_web_view_get_user_content_manager(pImpl_->webview);
  webkit_user_content_manager_register_script_message_handler(contentManager, "messageHandler");
  g_signal_connect(contentManager, "script-message-received::messageHandler",
                   G_CALLBACK(pImpl_->onScriptMessageReceived), this);

  WebKitWebContext* context = webkit_web_view_get_context(pImpl_->webview);
  webkit_web_context_register_uri_scheme(
      context, name_.c_str(), (WebKitURISchemeRequestCallback)pImpl_->onCustomSchemeRequest, this,
      NULL);

  injectScript(R"(
                window.webview = {
                    async postMessage(message) 
                    {
                      window.webkit.messageHandlers.messageHandler.postMessage(JSON.stringify(message));
                    }
                };
                )");
  show(true);
}

Webview::~Webview() {
  pImpl_->container = nullptr;
  pImpl_->webview = nullptr;
}

void Webview::enableDevTools(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { enableDevTools(state); });
  }
  WebKitSettings* settings = webkit_web_view_get_settings(pImpl_->webview);
  webkit_settings_set_enable_developer_extras(settings, state);
}

void Webview::enableContextMenu([[maybe_unused]] bool state) {
  // not supported
}

void Webview::enableZoom(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { enableZoom(state); });
  }
  WebKitSettings* settings = webkit_web_view_get_settings(pImpl_->webview);
  webkit_settings_set_zoom_text_only(settings, state);
}

void Webview::enableAcceleratorKeys([[maybe_unused]] bool state) {
  // not supported
}

void Webview::resize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { resize(size); });
  }
  gtk_widget_set_size_request(GTK_WIDGET(pImpl_->webview), size.first, size.second);
}

void Webview::setPosition(const ViewRect& rect) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([rect, this] { setPosition(rect); });
  }
  gtk_fixed_move(GTK_FIXED(pImpl_->container), GTK_WIDGET(pImpl_->webview), rect.L, rect.T);
}

void Webview::show(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { show(state); });
  }
  if (state) {
    gtk_widget_show_all(GTK_WIDGET(pImpl_->webview));
  } else {
    gtk_widget_hide(GTK_WIDGET(pImpl_->webview));
  }
}

void Webview::navigate(const std::string& url) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([url, this] { navigate(url); });
  }
  webkit_web_view_load_uri(pImpl_->webview, url.c_str());
}

void Webview::loadFile(const std::string& path) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([path, this] { loadFile(path); });
  }
  std::string filePath = "file://" + path;
  webkit_web_view_load_uri(pImpl_->webview, filePath.c_str());
}

void Webview::loadHTMLString(const std::string& html) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([html, this] { loadHTMLString(html); });
  }
  webkit_web_view_load_html(pImpl_->webview, html.c_str(), NULL);
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
  navigate(name_ + "://" + resourceUrl);
}

void Webview::clearResources() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { clearResources(); });
  }
  resources_.clear();
}

[[nodiscard]] const std::string Webview::getUrl() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getUrl(); });
  }
  const gchar* uri = webkit_web_view_get_uri(pImpl_->webview);
  return std::string(uri ? uri : "");
}

void Webview::injectScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([script, this] { injectScript(script); });
  }
  WebKitUserContentManager* manager = webkit_web_view_get_user_content_manager(pImpl_->webview);
  webkit_user_content_manager_add_script(
      manager,
      webkit_user_script_new(script.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                             WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, nullptr, nullptr));
}

void Webview::executeScript(const std::string& script) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([script, this] { executeScript(script); });
  }
  webkit_web_view_run_javascript(pImpl_->webview, script.c_str(), nullptr, nullptr, nullptr);
}
