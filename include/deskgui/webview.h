/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/webview_options.h>
#include <deskgui/app_handler.h>
#include <deskgui/event_bus.h>
#include <deskgui/resource_compiler.h>
#include <deskgui/types.h>

#include <memory>
#include <unordered_map>
#include <vector>

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
  class Webview : public EventBus {
    struct Impl;

  private:
    friend class Window;
    /**
     * @brief Constructs a Webview object.
     *
     * @param AppHandler Pointer to the application handler, which ensures thread safety for Webview
     * operations.
     *
     * @param window Pointer to the native window.
     *               - On Windows, it should be of type HWND.
     *               - On MacOS, it should be of type NSWindow.
     *               - On Linux, it should be of type GtkWindow.
     */
    explicit Webview(const std::string& name, AppHandler* appHandler, void* window, const WebViewOptions& options);

  public:
    /**
     * @brief Destroys the Webview object.
     */
    ~Webview();

    const std::string& getName() const;

    // Settings

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
    void loadResources(const Resources& resources);

    /**
     * @brief Serves a resource identified by its URL scheme.
     *
     * For example: "index.html", "src/assets/image.png"
     *
     * @param resourceScheme The URL of the resource to be served.
     */
    void serveResource(const std::string& resourceScheme);

    /**
     * @brief Clears all the resources that have been loaded into the application.
     */
    void clearResources();

    /**
     * @brief Gets the current URL of the web view.
     *
     * @return The current URL.
     */
    [[nodiscard]] const std::string getUrl();

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
     * @brief A callback function to handle incoming messages from the webview.
     *
     * @param message A message received from the webview.
     */
    void onMessage(const std::string& message);

    /**
     * @brief Resizes the web view to the specified size.
     *
     * @param size The new size of the web view.
     */
    void resize(const ViewSize& size);

  private:
    // Pointer to the implementation
    std::unique_ptr<Impl> pImpl_{nullptr};

    // App delegate
    AppHandler* appHandler_{nullptr};

    std::unordered_map<std::string, MessageCallback> callbacks_;
    Resources resources_;

    std::string name_;
  };

}  // namespace deskgui
