/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "interfaces/webview_impl.h"
#include "utils/dispatch.h"

using namespace deskgui;

Webview::Webview(const std::string& name, AppHandler* appHandler, void* window,
                 const WebviewOptions& options)
    : impl_(std::make_shared<Impl>(name, appHandler, window, options)), events_(&impl_->events()) {}

Webview::~Webview() = default;

std::string Webview::getName() const { return utils::dispatch<&Impl::getName>(impl_); }

bool Webview::isReady() const { return utils::dispatch<&Impl::isReady>(impl_); }

void Webview::onReady(std::function<void()> callback) {
  utils::dispatch<&Impl::onReady>(impl_, callback);
}

bool Webview::Impl::isReady() const {
  std::lock_guard<std::mutex> lock(readyMutex_);
  return isReady_;
}

void Webview::Impl::onReady(std::function<void()> callback) {
  bool shouldCallNow = false;
  {
    std::lock_guard<std::mutex> lock(readyMutex_);
    if (isReady_) {
      shouldCallNow = true;
    } else {
      readyCallbacks_.push_back(std::move(callback));
    }
  }
  if (shouldCallNow) {
    callback();
  }
}

void Webview::Impl::notifyReady() {
  std::vector<std::function<void()>> callbacksToInvoke;
  {
    std::lock_guard<std::mutex> lock(readyMutex_);
    if (isReady_) {
      return;
    }
    isReady_ = true;
    callbacksToInvoke = std::move(readyCallbacks_);
  }
  for (auto& callback : callbacksToInvoke) {
    callback();
  }
}

void Webview::Impl::addCallback(const std::string& key, MessageCallback callback) {
  callbacks_.try_emplace(key, callback);
}

void Webview::addCallback(const std::string& key, MessageCallback callback) {
  if (!isReady()) return;
  auto script = "window['" + key + "'] = function(payload) { const key = '" + key + "';" +
                R"(
                    window.webview.postMessage({
                              key: key,
                              payload: payload,
                            });
                    }
                )";
  utils::dispatch<&Impl::addCallback>(impl_, key, callback);
  injectScript(script);
  executeScript(script);
}

void Webview::Impl::removeCallback(const std::string& key) { callbacks_.erase(key); }

void Webview::removeCallback(const std::string& key) {
  if (!isReady()) return;
  auto script = "delete window['" + key + "']";
  utils::dispatch<&Impl::removeCallback>(impl_, key);
  injectScript(script);
  executeScript(script);
}

void Webview::postMessage(const std::string& message) {
  if (!isReady()) return;
  executeScript("window.webview.onMessage('" + message + "');");
}

void Webview::Impl::onMessage(const std::string& message) {
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
  events().emit(deskgui::event::WebviewOnMessage{message});
}

void Webview::Impl::applySchemeOptions(const WebviewOptions& options) {
  protocol_ = options.hasOption(WebviewOptions::kCustomSchemeProtocol)
                  ? options.getOption<std::string>(WebviewOptions::kCustomSchemeProtocol)
                  : std::string{kDefaultProtocol};
  std::string host = options.hasOption(WebviewOptions::kCustomSchemeHost)
                         ? options.getOption<std::string>(WebviewOptions::kCustomSchemeHost)
                         : std::string{kDefaultHost};
  if (protocol_.empty()) protocol_ = kDefaultProtocol;
  if (host.empty()) host = kDefaultHost;
  origin_ = protocol_ + "://" + host + "/";
}

// Settings methods
void Webview::enableDevTools(bool state) {
  if (!isReady()) return;
  utils::dispatch<&Impl::enableDevTools>(impl_, state);
}

void Webview::enableContextMenu(bool state) {
  if (!isReady()) return;
  utils::dispatch<&Impl::enableContextMenu>(impl_, state);
}

void Webview::enableZoom(bool state) {
  if (!isReady()) return;
  utils::dispatch<&Impl::enableZoom>(impl_, state);
}

void Webview::enableAcceleratorKeys(bool state) {
  if (!isReady()) return;
  utils::dispatch<&Impl::enableAcceleratorKeys>(impl_, state);
}

// View methods
void Webview::setPosition(const ViewRect& rect) {
  if (!isReady()) return;
  utils::dispatch<&Impl::setPosition>(impl_, rect);
}

void Webview::show(bool state) {
  if (!isReady()) return;
  utils::dispatch<&Impl::show>(impl_, state);
}

void Webview::resize(const ViewSize& size) {
  if (!isReady()) return;
  utils::dispatch<&Impl::resize>(impl_, size);
}

// Content methods
void Webview::navigate(const std::string& url) {
  if (!isReady()) return;
  utils::dispatch<&Impl::navigate>(impl_, url);
}

void Webview::loadFile(const std::string& path) {
  if (!isReady()) return;
  utils::dispatch<&Impl::loadFile>(impl_, path);
}

void Webview::loadHTMLString(const std::string& html) {
  if (!isReady()) return;
  utils::dispatch<&Impl::loadHTMLString>(impl_, html);
}

void Webview::loadResources(Resources&& resources) {
  if (!isReady()) return;
  utils::dispatch<&Impl::loadResources>(impl_, std::move(resources));
}

void Webview::serveResource(const std::string& resourceUrl) {
  if (!isReady()) return;
  utils::dispatch<&Impl::serveResource>(impl_, resourceUrl);
}

void Webview::clearResources() {
  if (!isReady()) return;
  utils::dispatch<&Impl::clearResources>(impl_);
}

std::string Webview::getUrl() {
  if (!isReady()) return {};
  return utils::dispatch<&Impl::getUrl>(impl_);
}

// Functionality methods
void Webview::injectScript(const std::string& script) {
  if (!isReady()) return;
  utils::dispatch<&Impl::injectScript>(impl_, script);
}

void Webview::executeScript(const std::string& script) {
  if (!isReady()) return;
  utils::dispatch<&Impl::executeScript>(impl_, script);
}
