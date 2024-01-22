/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "deskgui/webview.h"

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
  rapidjson::Document doc;
  doc.Parse(message.c_str());
  if (!doc.HasParseError() && doc.IsObject()) {
    if (doc.HasMember("key") && doc.HasMember("payload")) {
      const auto& key = doc["key"];
      if (key.IsString()) {
            std::string keyStr = key.GetString();
            auto callback = callbacks_.find(keyStr);
            if (callback != callbacks_.end()) {
                const auto& payload = doc["payload"];
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                payload.Accept(writer);
                callback->second(buffer.GetString());
            }
        }
    }
  }
  emit(deskgui::event::WebviewOnMessage{message});
}
