/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/types.h>

#include <iostream>
#include <string>

namespace deskgui {
  namespace event {

    /**
     * @brief Base event class representing an event with optional cancellation support.
     */
    class Event {
    public:
      explicit Event(bool cancellable = false) : cancelled_(false), cancellable_(cancellable) {}
      Event(const Event& other) = delete;
      virtual ~Event() = default;

      // Method to prevent default behavior
      void preventDefault() {
        if (cancellable_) {
          cancelled_ = true;
        }
      }

      bool isCancelled() const { return cancelled_; }
      bool isCancellable() const { return cancellable_; }

    private:
      bool cancelled_;
      bool cancellable_;
    };

    /**
     * @brief Represents a window's visibility change event.
     * 
     * Triggered when a window is about to be shown or hidden, 
     * allowing developers to intercept and potentially cancel the visibility change.
     */
    struct WindowShow : Event {
      WindowShow(bool show) : Event(true), state(show) {}
      const bool state;  // True if the window is shown, false if hidden.
    };

    /**
     * @brief Represents a window resize event.
     * 
     * Fired when the window's size is about to change, providing 
     * details about the new dimensions and allowing potential intervention.
     */
    struct WindowResize : Event {
      WindowResize(const ViewSize& viewSize) : Event(true), size(viewSize) {}
      const ViewSize size;  // The new size of the window after resizing.
    };

    /**
     * @brief Represents a window close request event.
     * 
     * Triggered when the user attempts to close a window, 
     * allowing developers to potentially prevent the window from closing.
     */
    struct WindowClose : Event {
      WindowClose() : Event(true){}
    };

    // Webview events

    /**
     * @brief Represents a message received by the webview.
     * 
     * Fired when a message is sent from the web content to the application, 
     * enabling inter-process communication between web and native code.
     */
    struct WebviewOnMessage : Event {
      WebviewOnMessage(const std::string& msg) : Event(true), message(msg) {}
      const std::string message;
    };

    /**
     * @brief Represents the start of a webview navigation.
     * 
     * Triggered before the webview begins navigating to a new URL, 
     * allowing developers to inspect or potentially block the navigation.
     */
    struct WebviewNavigationStarting : Event {
      WebviewNavigationStarting(const std::string& urlArg) : Event(true), url(urlArg) {}
      const std::string url;
    };

    /**
     * @brief Represents the start of navigation within a webview frame.
     * 
     * Fired before a specific frame in the webview begins navigating, 
     * providing an opportunity to monitor or control frame-specific navigation.
     */
    struct WebviewFrameNavigationStarting : Event {
      WebviewFrameNavigationStarting(const std::string& urlArg) : Event(true), url(urlArg) {}
      const std::string url;
    };

    /**
     * @brief Represents a change in the webview's source URL.
     * 
     * Triggered when the webview's current source URL is modified, 
     * providing visibility into the navigation state of the webview.
     */
    struct WebviewSourceChanged : Event {
      WebviewSourceChanged(const std::string& src) : Event(false), source(src) {}
      const std::string source;  // The new source URL of the webview.
    };

    /**
     * @brief Represents the content loading state of a webview.
     * 
     * Fired to indicate whether the webview's content has been fully loaded, 
     * allowing developers to perform actions based on the loading status.
     */
    struct WebviewContentLoaded : Event {
      WebviewContentLoaded(bool loaded) : Event(false), state(loaded) {}
      const bool state;  // True if the webview content is fully loaded, false otherwise.
    };

    /**
     * @brief Represents a request to open a new window from within a WebView.
     * 
     * Fired when web content inside the WebView attempts to open a new window, 
     * such as through JavaScript's window.open() method. This event allows 
     * developers to intercept, modify, or prevent the window creation process.
     * 
     * @note This event is triggered by web-initiated window open requests, 
     * providing fine-grained control over new window spawning behavior.
     */
    struct WebviewWindowRequested: Event {
      WebviewWindowRequested(const std::string &url) : Event(true), url(url) {}
      const std::string url;  // The URL of the window requested to be opened.
    };

  }  // namespace event
}  // namespace deskgui
