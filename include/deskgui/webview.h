/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/app_handler.h>
#include <deskgui/event_bus.h>
#include <deskgui/resource_compiler.h>
#include <deskgui/types.h>
#include <deskgui/webview_options.h>

namespace deskgui {
  class Window;

  /**
   * @class Webview
   * @brief The Webview class represents a web view widget.
   *
   * It provides functionality for loading and displaying web content.
   * The class supports various web-related operations, such as navigating to URLs, loading local
   * files, loading embedded resources and executing scripts. Additionally, it allows for
   * interaction between JavaScript and native code by adding and removing callback functions.
   */
  class Webview {
  private:
    friend class Window;

    /**
     * @brief Constructs a Webview object.
     *
     * @param name The name of the webview.
     * @param appHandler Pointer to the application handler, which ensures thread safety for Webview
     * operations.
     * @param window Pointer to the native window.
     *               - On Windows, it should be of type HWND.
     *               - On MacOS, it should be of type NSWindow.
     *               - On Linux, it should be of type GtkWindow.
     * @param options Webview options.
     */
    Webview(const std::string& name, AppHandler* appHandler, void* window,
            const WebviewOptions& options);

  public:
    class Impl;

    /**
     * @brief Destroys the Webview object.
     */
    ~Webview();

    /**
     * @brief Get the name associated with this Webview.
     *
     *
     * @return A constant reference to the name of the webview.
     */
    [[nodiscard]] std::string getName() const;

    /**
     * @brief Checks if the webview is ready for use.
     *
     * On Windows, webview creation is asynchronous. This method returns true
     * when the WebView2 environment and controller are fully initialized.
     * On macOS and Linux, this typically returns true immediately.
     *
     * @return true if the webview is ready, false otherwise.
     */
    [[nodiscard]] bool isReady() const;

    /**
     * @brief Attaches a callback to be invoked when the webview becomes ready.
     *
     * If the webview is already ready, the callback is invoked immediately.
     * Otherwise, the callback will be invoked asynchronously when initialization completes.
     *
     * @param callback The callback function to invoke when ready.
     */
    void onReady(std::function<void()> callback);

    /**
     * @brief Enables or disables the developer tools.
     *
     * @param state True to enable, false to disable.
     */
    void enableDevTools(bool state);

    /**
     * @brief Enables or disables the context menu.
     *
     * @param state True to enable, false to disable.
     */
    void enableContextMenu(bool state);

    /**
     * @brief Enables or disables zooming.
     *
     * @param state True to enable, false to disable.
     */
    void enableZoom(bool state);

    /**
     * @brief Enables or disables accelerator keys.
     *
     * @param state True to enable, false to disable.
     */
    void enableAcceleratorKeys(bool state);

    // View

    /**
     * @brief Sets the position of the web view.
     *
     * @param rect The position and size of the web view.
     */
    void setPosition(const ViewRect& rect);

    /**
     * @brief Shows or hides the web view.
     *
     * @param state True to show, false to hide.
     */
    void show(bool state);

    // Content

    /**
     * @brief Navigates to the specified URL.
     *
     * @param url The URL to navigate to.
     */
    void navigate(const std::string& url);

    /**
     * @brief Loads a local file URL.
     *
     * When loading a document via a file path, the web content is retrieved from static files on
     * disk. For example: "home/some_path/index.html"
     *
     * @param path The file path to load.
     */
    void loadFile(const std::string& path);

    /**
     * @brief Sets the HTML content of the web view.
     *
     * @param html The HTML content.
     */
    void loadHTMLString(const std::string& html);

    /**
     * @brief Loads custom resources and integrates them into your web content.
     *
     * @param resources Resources vector object.
     */
    void loadResources(Resources&& resources);

    /**
     * @brief Serves a resource identified by its URL scheme.
     *
     * For example: "index.html", "src/assets/image.png"
     *
     * @param resourceUrl The URL of the resource to be served.
     */
    void serveResource(const std::string& resourceUrl);

    /**
     * @brief Clears all the resources that have been loaded into the application.
     */
    void clearResources();

    /**
     * @brief Gets the current URL of the web view.
     *
     * @return The current URL.
     */
    [[nodiscard]] std::string getUrl();

    // Functionality

    /**
     * @brief Injects a script into the web view.
     *
     * @param script The script to inject.
     */
    void injectScript(const std::string& script);

    /**
     * @brief Executes a script in the web view.
     *
     * @param script The script to execute.
     */
    void executeScript(const std::string& script);

    /**
     * @brief Adds a callback function with the specified name.
     *
     * The callback is exposed as a global JavaScript function accessible via
     * window.<callback-key>(message).
     *
     * @param key The name (key) of the callback.
     * @param callback The callback function to be invoked when the JavaScript function is called.
     */
    void addCallback(const std::string& key, MessageCallback callback);

    /**
     * @brief Removes the callback function for the specified key.
     *
     * @param key The key of the callback.
     */
    void removeCallback(const std::string& key);

    /**
     * @brief Sends a message to the webview.
     *
     * @param message The message to send.
     */
    void postMessage(const std::string& message);

    /**
     * @brief Resizes the web view to the specified size.
     *
     * @param size The new size of the web view.
     */
    void resize(const ViewSize& size);

    /**
     * @brief Connects a listener to a webview event type.
     *
     * Allows registering callbacks_ for webview-specific events. See the WebviewEvents
     * namespace for available event types that can be listened to.
     *
     * Example:
     * @code{.cpp}
     * webview->connect<WebviewSourceChanged>([](const WebviewSourceChanged& event) {
     *   // Handle webview content change
     * });
     * @endcode
     *
     * @tparam EventType The type of webview event to listen for
     * @tparam Callable The type of the callable object (lambda, function, etc.)
     * @param listener The callable object to be called when the event is emitted
     * @return A unique ID that can be used to disconnect the listener later
     */
    template <class EventType, typename Callable>
    [[maybe_unused]] UniqueId connect(Callable&& listener) {
      return events_->connect<EventType>(std::forward<Callable>(listener));
    }

    /**
     * @brief Disconnects a listener
     *
     * @tparam EventType The type of event the listener was connected to.
     * @param id The unique ID returned when the listener was connected.
     */
    template <typename EventType> void disconnect(UniqueId id) {
      events_->disconnect<EventType>(id);
    }

  private:
    std::shared_ptr<Impl> impl_{nullptr};

    EventBus* events_;
  };

}  // namespace deskgui
