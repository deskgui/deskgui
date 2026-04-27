/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "webview_platform_linux.h"

namespace deskgui {

  using Platform = Webview::Impl::Platform;

  gboolean Platform::onNavigationRequest(WebKitWebView* webview, WebKitPolicyDecision* decision,
                                         WebKitPolicyDecisionType decisionType,
                                         Webview::Impl* impl) {
    if (!impl) return FALSE;

    switch (decisionType) {
      case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION: {
        WebKitNavigationPolicyDecision* navigationDecision
            = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
        WebKitURIRequest* uriRequest
            = webkit_navigation_policy_decision_get_request(navigationDecision);

        const gchar* uri = webkit_uri_request_get_uri(uriRequest);

        event::WebviewNavigationStarting event{std::string(uri)};
        impl->events().emit(event);

        if (event.isCancelled()) {
          return TRUE;
        }
      }
    }

    return FALSE;  // stop further signal emission
  }

  void Platform::onLoadChanged(WebKitWebView* webview, WebKitLoadEvent loadEvent,
                               Webview::Impl* impl) {
    if (!impl || !webview) return;

    if (loadEvent == WEBKIT_LOAD_COMMITTED) {
      const gchar* uri = webkit_web_view_get_uri(webview);
      impl->events().emit(event::WebviewSourceChanged(std::string(uri)));
    } else if (loadEvent == WEBKIT_LOAD_FINISHED) {
      impl->events().emit(event::WebviewContentLoaded(true));
    }
  }

  void Platform::onScriptMessageReceived([[maybe_unused]] WebKitUserContentManager* manager,
                                         WebKitJavascriptResult* message, Webview::Impl* impl) {
    if (!impl) return;

    char* s;
    JSCValue* value = webkit_javascript_result_get_js_value(message);
    s = jsc_value_to_string(value);
    impl->onMessage(s);
    g_free(s);
  }

  void Platform::onCustomSchemeRequest(WebKitURISchemeRequest* request, gpointer userData) {
    Webview::Impl* impl = static_cast<Webview::Impl*>(userData);

    if (!impl || !request) return;

    const gchar* uri = webkit_uri_scheme_request_get_uri(request);

    auto it = std::find_if(impl->resources_.begin(), impl->resources_.end(),
                           [&](const Resource& resource) {
                             return (impl->getOrigin() + resource.scheme) == std::string(uri);
                           });

    if (it != impl->resources_.end()) {
      GBytes* bytes = g_bytes_new(it->content.data(), it->content.size());

      if (!bytes) {
        GError* error = nullptr;
        g_set_error(&error, g_quark_from_static_string("webview"), 1,
                    "Requested resource for webview is not supported");
        webkit_uri_scheme_request_finish_error(request, error);
        g_clear_error(&error);
        return;
      }

      GInputStream* inputStream = G_INPUT_STREAM(g_memory_input_stream_new_from_bytes(bytes));

      webkit_uri_scheme_request_finish(request, inputStream, it->content.size(), it->mime.c_str());
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
