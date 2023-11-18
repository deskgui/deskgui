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

void Webview::addCallback(const std::string& name, MessageCallback callback) {
  callbacks_.emplace(name, callback);
  auto script = "window['" + name + "'] = function(message) { const name = '" + name + "';" +
                R"(
                    window.webview.postMessage({
                              name: name,
                              message: message,
                            });
                    }
                )";
  injectScript(script);
  executeScript(script);
}

void Webview::removeCallback(const std::string& name) {
  callbacks_.erase(name);
  auto script = "delete window['" + name + "']";
  injectScript(script);
  executeScript(script);
}

void Webview::onMessage(const std::string& message) {
  auto json = nlohmann::json::parse(message);
  if (json.is_object()) {
    if (json.contains("name") || json.contains("message")) {
      auto callback = callbacks_.find(json["name"]);
      if (callback != callbacks_.end()) {
        callback->second(json["message"].dump());
      }
      emit(deskgui::event::WebviewOnMessage{message});
    }
  }
}
