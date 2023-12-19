/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "deskgui/webview.h"
#include "nlohmann/json.hpp"

using namespace deskgui;

const std::string& Webview::getName() const { return name_; }

void Webview::addCallback(const std::string& key, MessageCallback callback) {
  callbacks_.emplace(key, callback);
  auto script = "window['" + key + "'] = function(payload) { const key = '" + key + "';" +
                R"(
                    window.webview.postMessage({
                              key: key,
                              payload: payload,
                            });
                    }
                )";
  injectScript(script);
  executeScript(script);
}

void Webview::removeCallback(const std::string& key) {
  callbacks_.erase(key);
  auto script = "delete window['" + key + "']";
  injectScript(script);
  executeScript(script);
}

void Webview::postMessage(const std::string& message) {
  executeScript("window.webview.onMessage('" + message + "');");
}

void Webview::onMessage(const std::string& message) {
  auto json = nlohmann::json::parse(message);
  if (json.is_object()) {
    if (json.contains("key") || json.contains("payload")) {
      auto callback = callbacks_.find(json["key"]);
      if (callback != callbacks_.end()) {
        callback->second(json["payload"].dump());
      }
    }
  }
  emit(deskgui::event::WebviewOnMessage{message});
}
