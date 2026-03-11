/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/window.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace deskgui {
  /**
   * @class Window
   * @brief Represents a native window with functionality for managing window properties and
   * behavior.
   *
   * The Window class is used to create and manage a native window for displaying web content.
   * It provides methods to set and retrieve window properties such as size, title, position, and
   * decoration. Additionally, it supports event handling for window resize and show/hide events.
   */
  class Window::Impl {
  public:
    class Platform;

    explicit Impl(const std::string& name, AppHandler* appHandler, void* nativeWindow = nullptr);
    ~Impl();

    [[nodiscard]] Webview* createWebview(const std::string& name, const WebviewOptions& options);
    void destroyWebview(const std::string& name);
    [[nodiscard]] Webview* getWebview(const std::string& name) const;

    [[nodiscard]] inline std::string getName() const { return name_; }

    void setTitle(const std::string& title);
    [[nodiscard]] std::string getTitle() const;

    void setSize(const ViewSize& size, PixelsType type = PixelsType::kLogical);
    [[nodiscard]] ViewSize getSize(PixelsType type = PixelsType::kLogical) const;

    void setMaxSize(const ViewSize& size, PixelsType type = PixelsType::kLogical);
    [[nodiscard]] ViewSize getMaxSize(PixelsType type = PixelsType::kLogical) const;

    void setMinSize(const ViewSize& size, PixelsType type = PixelsType::kLogical);
    [[nodiscard]] ViewSize getMinSize(PixelsType type = PixelsType::kLogical) const;

    void setPosition(const ViewRect& position, PixelsType type = PixelsType::kLogical);
    [[nodiscard]] ViewRect getPosition(PixelsType type = PixelsType::kLogical) const;

    void setResizable(bool resizable);
    [[nodiscard]] bool isResizable() const;

    void setDecorations(bool decorations);
    [[nodiscard]] bool isDecorated() const;

    void hide();
    void show();
    void center();
    void enable(bool state);
    inline void close() { appHandler_->destroyWindow(getName()); }

    void setBackgroundColor(int red, int green, int blue);
    void setTitleBarColor(int red, int green, int blue);

    [[nodiscard]] SystemTheme getSystemTheme() const;

    [[nodiscard]] void* getNativeWindow();
    [[nodiscard]] void* getContentView();

    inline void setMonitorScaleFactor(float scaleFactor) { monitorScaleFactor_ = scaleFactor; }
    [[nodiscard]] inline float getMonitorScaleFactor() const { return monitorScaleFactor_; }

    [[nodiscard]] inline AppHandler* application() const { return appHandler_; }
    [[nodiscard]] inline EventBus& events() { return events_; }
    [[nodiscard]] inline Platform* platform() { return platform_.get(); }


  private:
    std::unique_ptr<Platform> platform_{nullptr};

    std::unordered_map<std::string, std::shared_ptr<Webview>> webviews_;

    // Window name
    std::string name_;

    AppHandler* appHandler_{nullptr};

    // Window sizes (logical pixels)
    ViewSize minSize_;
    ViewSize maxSize_;
    bool minSizeDefined_{false};
    bool maxSizeDefined_{false};
    bool isExternalWindow_{false};

    float monitorScaleFactor_ = 1.f;

    EventBus events_;
  };

}  // namespace deskgui
