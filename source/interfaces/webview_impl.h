/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/event_bus.h>
#include <deskgui/webview.h>

#include <mutex>
#include <unordered_map>

namespace deskgui {

  class Webview::Impl {
  public:
    class Platform;

    explicit Impl(const std::string& name, AppHandler* appHandler, void* window,
                  const WebviewOptions& options);
    ~Impl();

    void initialize(const WebviewOptions& options);

    /**
     * Defaults used when no custom scheme/host is provided via WebviewOptions.
     */
    static constexpr auto kDefaultProtocol = "webview";
    static constexpr auto kDefaultHost = "localhost";

    /**
     * Per-instance scheme metadata, derived from WebviewOptions during
     * construction. See WebviewOptions::kCustomSchemeProtocol /
     * WebviewOptions::kCustomSchemeHost.
     */
    [[nodiscard]] inline const std::string& getProtocol() const { return protocol_; }
    [[nodiscard]] inline const std::string& getOrigin() const { return origin_; }

    [[nodiscard]] inline std::string getName() const { return name_; }

    [[nodiscard]] bool isReady() const;
    void onReady(std::function<void()> callback);
    void notifyReady();  // Internal: called when initialization completes

    // Settings
    void enableDevTools(bool state);
    void enableContextMenu(bool state);
    void enableZoom(bool state);
    void enableAcceleratorKeys(bool state);

    // View
    void setPosition(const ViewRect& rect);
    void show(bool state);
    void resize(const ViewSize& size);

    // Content
    void navigate(const std::string& url);
    void loadFile(const std::string& path);
    void loadHTMLString(const std::string& html);
    void loadResources(Resources&& resources);
    void serveResource(const std::string& resourceUrl);
    void clearResources();
    [[nodiscard]] std::string getUrl();

    // Functionality
    void addCallback(const std::string& key, MessageCallback callback);
    void removeCallback(const std::string& key);
    void postMessage(const std::string& message);
    void injectScript(const std::string& script);
    void executeScript(const std::string& script);
    void onMessage(const std::string& message);

    [[nodiscard]] inline AppHandler* application() const { return appHandler_; }
    [[nodiscard]] inline EventBus& events() { return events_; }

  private:
    void applySchemeOptions(const WebviewOptions& options);

    std::unique_ptr<Platform> platform_{nullptr};
    std::string name_;
    std::unordered_map<std::string, MessageCallback> callbacks_;
    AppHandler* appHandler_{nullptr};
    Resources resources_;
    EventBus events_;
    mutable std::mutex readyMutex_;
    bool isReady_ = false;
    std::vector<std::function<void()>> readyCallbacks_;
    std::string protocol_;
    std::string origin_;
  };

}  // namespace deskgui
