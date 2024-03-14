/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include <algorithm>

#include "deskgui/webview.h"

namespace deskgui {
  struct Webview::Impl {
    WebKitWebView* webview;
    GtkFixed* container;

    static gboolean onNavigationRequest(WebKitWebView* webview, WebKitPolicyDecision* decision,
                                        WebKitPolicyDecisionType decisionType, Webview* self);
    static void onLoadChanged(WebKitWebView* webview, WebKitLoadEvent loadEvent, Webview* self);
    static void onScriptMessageReceived(WebKitUserContentManager* manager,
                                        WebKitJavascriptResult* message, Webview* self);
    static void onCustomSchemeRequest(WebKitURISchemeRequest* request, gpointer userData);
  };

  inline gboolean Webview::Impl::onNavigationRequest(WebKitWebView* webview,
                                                     WebKitPolicyDecision* decision,
                                                     WebKitPolicyDecisionType decisionType,
                                                     Webview* self) {
    if (!self) return FALSE;

    switch (decisionType) {
      case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION: {
        WebKitNavigationPolicyDecision* navigationDecision
            = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
        WebKitURIRequest* uriRequest
            = webkit_navigation_policy_decision_get_request(navigationDecision);

        const gchar* uri = webkit_uri_request_get_uri(uriRequest);

        event::WebviewNavigationStarting event{std::string(uri)};
        self->emit(event);

        if (event.isCancelled()) {
          return TRUE;
        }
      }
    }

    return FALSE;  // stop further signal emission
  }

  inline void Webview::Impl::onLoadChanged(WebKitWebView* webview, WebKitLoadEvent loadEvent,
                                           Webview* self) {
    if (!self || !webview) return;

    if (loadEvent == WEBKIT_LOAD_COMMITTED) {
      const gchar* uri = webkit_web_view_get_uri(webview);
      self->emit(event::WebviewSourceChanged(std::string(uri)));
    } else if (loadEvent == WEBKIT_LOAD_FINISHED) {
      self->emit(event::WebviewContentLoaded(true));
    }
  }

  inline void Webview::Impl::onScriptMessageReceived(
      [[maybe_unused]] WebKitUserContentManager* manager, WebKitJavascriptResult* message,
      Webview* self) {
    if (!self) return;

    char* s;
    JSCValue* value = webkit_javascript_result_get_js_value(message);
    s = jsc_value_to_string(value);
    self->onMessage(s);
    g_free(s);
  }

  inline void Webview::Impl::onCustomSchemeRequest(WebKitURISchemeRequest* request,
                                                   gpointer userData) {
    Webview* self = static_cast<Webview*>(userData);

    if (!self || !request) return;

    const gchar* uri = webkit_uri_scheme_request_get_uri(request);

    auto schemeRoot = self->getName() + "://";
    std::transform(schemeRoot.begin(), schemeRoot.end(), schemeRoot.begin(), ::tolower);

    auto it = std::find_if(self->resources_.begin(), self->resources_.end(),
                           [&](const Resource& resource) {
                             return (schemeRoot + resource.scheme) == std::string(uri);
                           });

    if (it != self->resources_.end()) {
      GBytes* bytes = g_bytes_new(it->data, it->size);

      if (!bytes) {
        GError* error = nullptr;
        g_set_error(&error, g_quark_from_static_string("webview"), 1,
                    "Requested resource for webview is not supported");
        webkit_uri_scheme_request_finish_error(request, error);
        g_clear_error(&error);
        return;
      }

      GInputStream* inputStream = G_INPUT_STREAM(g_memory_input_stream_new_from_bytes(bytes));

      webkit_uri_scheme_request_finish(request, inputStream, it->data.size(), it->mime.c_str());
      g_object_unref(inputStream);
      g_bytes_unref(bytes);
      return;
    }

    GError* error = nullptr;
    g_set_error(&error, g_quark_from_static_string("webview"), 1,
                "Cannot load requested resource for webview");
    webkit_uri_scheme_request_finish_error(request, error);
    g_clear_error(&error);
  }

}  // namespace deskgui
