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
     * @brief Event structure representing the visibility state of a window.
     */
    struct WindowShow : Event {
      WindowShow(bool show) : Event(true), state(show) {}
      const bool state;  // True if the window is shown, false if hidden.
    };

    /**
     * @brief Event structure representing a window resizing event.
     */
    struct WindowResize : Event {
      WindowResize(const ViewSize& viewSize) : Event(true), size(viewSize) {}
      const ViewSize size;  // The new size of the window after resizing.
    };

    struct WindowClose : Event {
      WindowClose() : Event(true){}
    };

    // Webview events

    struct WebviewOnMessage : Event {
      WebviewOnMessage(const std::string& msg) : Event(true), message(msg) {}
      const std::string message;
    };

    struct WebviewNavigationStarting : Event {
      WebviewNavigationStarting(const std::string& urlArg) : Event(true), url(urlArg) {}
      const std::string url;
    };

    struct WebviewFrameNavigationStarting : Event {
      WebviewFrameNavigationStarting(const std::string& urlArg) : Event(true), url(urlArg) {}
      const std::string url;
    };

    /**
     * @brief Event structure representing a change in the webview's source URL.
     */
    struct WebviewSourceChanged : Event {
      WebviewSourceChanged(const std::string& src) : Event(false), source(src) {}
      const std::string source;  // The new source URL of the webview.
    };

    /**
     * @brief Event structure representing the loading state of a webview.
     */
    struct WebviewContentLoaded : Event {
      WebviewContentLoaded(bool loaded) : Event(false), state(loaded) {}
      const bool state;  // True if the webview content is fully loaded, false otherwise.
    };

  }  // namespace event
}  // namespace deskgui
