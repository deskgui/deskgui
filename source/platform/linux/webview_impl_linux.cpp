/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "webview_platform_linux.h"

using namespace deskgui;

using Impl = Webview::Impl;

Impl::Impl(const std::string& name, AppHandler* appHandler, void* window,
           const WebviewOptions& options)
    : platform_(std::make_unique<Impl::Platform>()), name_(name), appHandler_(appHandler) {
  if (window == nullptr) {
    throw std::invalid_argument("Window is a nullptr");
  }

  // Create GTK container hierarchy
  GtkWindow* parentWindow = GTK_WINDOW(window);
  GtkScrolledWindow* scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
  gtk_container_add(GTK_CONTAINER(parentWindow), GTK_WIDGET(scrolledWindow));
  platform_->container = GTK_FIXED(gtk_fixed_new());
  gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(platform_->container));

  initialize(options);
}

Impl::~Impl() {
  platform_->container = nullptr;
  platform_->webview = nullptr;
}

void Impl::initialize(const WebviewOptions& options) {
  // Create webview (with ephemeral context if requested)
  const bool ephemeralSession = options.hasOption(WebviewOptions::kEphemeralSession)
      && options.getOption<bool>(WebviewOptions::kEphemeralSession);
  if (ephemeralSession) {
    WebKitWebContext* ephemeralContext = webkit_web_context_new_ephemeral();
    platform_->webview = WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(ephemeralContext));
    g_object_unref(ephemeralContext);
  } else {
    platform_->webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
  }

  if (!platform_->webview) {
    throw std::runtime_error("Failed to create webview.");
  }

  // Set initial position and size
  gtk_fixed_put(platform_->container, GTK_WIDGET(platform_->webview), kDefaultWindowRect.L,
                kDefaultWindowRect.T);
  gtk_widget_set_size_request(GTK_WIDGET(platform_->webview), kDefaultWindowRect.R,
                              kDefaultWindowRect.B);
  gtk_widget_grab_focus(GTK_WIDGET(platform_->webview));

  // Configure settings
  WebKitSettings* settings = webkit_web_view_get_settings(platform_->webview);
  webkit_settings_set_enable_javascript(settings, TRUE);

  // Connect navigation signals
  g_signal_connect(platform_->webview, "load-changed", G_CALLBACK(platform_->onLoadChanged), this);
  g_signal_connect(platform_->webview, "decide-policy", G_CALLBACK(platform_->onNavigationRequest),
                   this);

  // Set up message handler for JS communication
  WebKitUserContentManager* contentManager
      = webkit_web_view_get_user_content_manager(platform_->webview);
  webkit_user_content_manager_register_script_message_handler(contentManager, "messageHandler");
  g_signal_connect(contentManager, "script-message-received::messageHandler",
                   G_CALLBACK(platform_->onScriptMessageReceived), this);

  // Register custom URI scheme for local resources
  WebKitWebContext* context = webkit_web_view_get_context(platform_->webview);
  webkit_web_context_register_uri_scheme(
      context, Impl::kProtocol, (WebKitURISchemeRequestCallback)platform_->onCustomSchemeRequest,
      this, NULL);

  // Inject JS bridge
  injectScript(R"(
                window.webview = {
                    async postMessage(message) 
                    {
                      if (typeof window.webkit === 'undefined' || !window.webkit.messageHandlers?.messageHandler) return;
                      return window.webkit.messageHandlers.messageHandler.postMessage(JSON.stringify(message));
                    }
                };
                )");

  show(true);
  notifyReady();
}

void Impl::enableDevTools(bool state) {
  WebKitSettings* settings = webkit_web_view_get_settings(platform_->webview);
  webkit_settings_set_enable_developer_extras(settings, state);
}

void Impl::enableContextMenu([[maybe_unused]] bool state) {
  // not supported
}

void Impl::enableZoom(bool state) {
  WebKitSettings* settings = webkit_web_view_get_settings(platform_->webview);
  webkit_settings_set_zoom_text_only(settings, state);
}

void Impl::enableAcceleratorKeys([[maybe_unused]] bool state) {
  // not supported
}

void Impl::resize(const ViewSize& size) {
  gtk_widget_set_size_request(GTK_WIDGET(platform_->webview), size.first, size.second);
}

void Impl::setPosition(const ViewRect& rect) {
  gtk_fixed_move(GTK_FIXED(platform_->container), GTK_WIDGET(platform_->webview), rect.L, rect.T);
}

void Impl::show(bool state) {
  if (state) {
    gtk_widget_show_all(GTK_WIDGET(platform_->webview));
  } else {
    gtk_widget_hide(GTK_WIDGET(platform_->webview));
  }
}

void Impl::navigate(const std::string& url) {
  webkit_web_view_load_uri(platform_->webview, url.c_str());
}

void Impl::loadFile(const std::string& path) {
  std::string filePath = "file://" + path;
  webkit_web_view_load_uri(platform_->webview, filePath.c_str());
}

void Impl::loadHTMLString(const std::string& html) {
  webkit_web_view_load_html(platform_->webview, html.c_str(), NULL);
}

void Impl::loadResources(Resources&& resources) { resources_ = std::move(resources); }

void Impl::serveResource(const std::string& resourceUrl) { navigate(Impl::kOrigin + resourceUrl); }

void Impl::clearResources() { resources_.clear(); }

std::string Impl::getUrl() {
  const gchar* uri = webkit_web_view_get_uri(platform_->webview);
  return std::string(uri ? uri : "");
}

void Impl::injectScript(const std::string& script) {
  WebKitUserContentManager* manager = webkit_web_view_get_user_content_manager(platform_->webview);
  webkit_user_content_manager_add_script(
      manager,
      webkit_user_script_new(script.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                             WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, nullptr, nullptr));
}

void Impl::executeScript(const std::string& script) {
  webkit_web_view_run_javascript(platform_->webview, script.c_str(), nullptr, nullptr, nullptr);
}
