/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/app_handler.h>
#include <deskgui/event_bus.h>
#include <deskgui/types.h>
#include <deskgui/webview.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace deskgui {
  class App;

  /**
   * @class Window
   * @brief Represents a native window with functionality for managing window properties and
   * behavior.
   *
   * The Window class is used to create and manage a native window for displaying web content.
   * It provides methods to set and retrieve window properties such as size, title, position, and
   * decoration. Additionally, it supports event handling for window resize and show/hide events.
   */
  class Window : public EventBus {
    struct Impl;

  private:
    friend class App;
    /**
     * @brief Constructs a Window object.
     *
     * @param appDelegate A weak pointer to the application delegate for handling main thread
     * operations.
     * @param nativeWindow Pointer to the native window.
     *                     - On Windows, it should be of type HWND.
     *                     - On MacOS, it should be of type NSWindow.
     *                     - On Linux, it should be of type GtkWindow.
     * @remarks The nativeWindow parameter is optional and defaults to nullptr if not provided.
     *          Ensure that the nativeWindow is of the correct type for the target platform.
     *          Improper usage or invalid nativeWindow types may result in undefined behavior.
     */
    explicit Window(const std::string& name, AppHandler* appHandler, void* nativeWindow = nullptr);

  public:
    /**
     * @brief Destroys the Window object.
     */
    ~Window();

    /**
     * @brief Get the name associated with this Window.
     *
     *
     * @return A constant reference to the name of the window.
     */
    inline const std::string& getName() const { return name_; }

    /**
     * Create a new Webview with the specified name and position it with the given rect.
     *
     * @param name The name of the Webview.
     * @return A pointer to the created Webview.
     */
    Webview* createWebview(const std::string& name, const WebviewOptions& options = {});

    /**
     * Destroy the Webview with the specified name.
     *
     * This method will destroy and deallocate the Webview object associated with the given name.
     * After calling this method, the Webview will be removed from the application and its resources
     * will be released.
     *
     * @param name The name of the Webview to be destroyed.
     */
    void destroyWebview(const std::string& name);

    /**
     * Get the Webview with the specified name.
     * The caller should not assume ownership of the returned pointer.
     *
     * @param name The name of the Webview to retrieve.
     * @return A pointer to the Webview if found, otherwise nullptr.
     */
    Webview* getWebview(const std::string& name) const;

    // Window Style

    /**
     * @brief Sets the title of the window.
     *
     * @param title The window title.
     */
    void setTitle(const std::string& title);

    /**
     * @brief Gets the title of the window.
     *
     * @return The window title.
     */
    [[nodiscard]] std::string getTitle() const;

    /**
     * @brief Sets the size of the window.
     *
     * @param size The window size.
     * @param type The type of pixels used for the size. Default is logical pixels.
     *             It can be either logical or physical.
     */
    void setSize(const ViewSize& size, PixelsType type = PixelsType::kLogical);

    /**
     * @brief Gets the size of the window.
     *
     * @param type The type of pixels used for the size. Default is logical pixels.
     *             It can be either logical or physical.
     * @return The window size.
     */
    [[nodiscard]] ViewSize getSize(PixelsType type = PixelsType::kLogical) const;

    /**
     * @brief Sets the maximum size of the window.
     *
     * Sets the maximum window size in logical or physical units.
     *
     * @param size The maximum window size.
     * @param type The type of pixels used for the size. Default is logical pixels.
     *             It can be either logical or physical.
     */
    void setMaxSize(const ViewSize& size, PixelsType type = PixelsType::kLogical);

    /**
     * @brief Gets the maximum size of the window.
     *
     * Retrieves the maximum window size in logical or physical units.
     *
     * @param type The type of pixels used for the size. Default is logical pixels.
     *             It can be either logical or physical.
     * @return The maximum window size.
     */
    [[nodiscard]] ViewSize getMaxSize(PixelsType type = PixelsType::kLogical) const;

    /**
     * @brief Sets the minimum size of the window.
     *
     * Sets the minimum window size in logical or physical units.
     *
     * @param size The minimum window size.
     * @param type The type of pixels used for the size. Default is logical pixels.
     *             It can be either logical or physical.
     */
    void setMinSize(const ViewSize& size, PixelsType type = PixelsType::kLogical);

    /**
     * @brief Gets the minimum size of the window.
     *
     * Retrieves the minimum window size in logical or physical units.
     *
     * @param type The type of pixels used for the size. Default is logical pixels.
     *             It can be either logical or physical.
     * @return The minimum window size.
     */
    [[nodiscard]] ViewSize getMinSize(PixelsType type = PixelsType::kLogical) const;

    /**
     * @brief Sets the position of the window.
     *
     * Sets the position of the window.
     *
     * @param position The position of the window.
     * @param type The type of pixels used for the position. Default is logical pixels.
     *             It can be either logical or physical.
     */
    void setPosition(const ViewRect& position, PixelsType type = PixelsType::kLogical);

    /**
     * @brief Gets the position of the window.
     *
     * Retrieves the position of the window.
     *
     * @param type The type of pixels used for the position. Default is logical pixels.
     *             It can be either logical or physical.
     * @return The position of the window.
     */
    [[nodiscard]] ViewRect getPosition(PixelsType type = PixelsType::kLogical) const;

    /**
     * @brief Sets whether the window is resizable.
     *
     * @param resizable True to make the window resizable, false otherwise.
     */
    void setResizable(bool resizable);

    /**
     * @brief Checks if the window is resizable.
     *
     * @return True if the window is resizable, false otherwise.
     */
    [[nodiscard]] bool isResizable() const;

    /**
     * @brief Sets whether the window has decorations such as borders and title bar.
     *
     * @param decorations True to enable window decorations, false to disable them.
     *                    Enabling decorations will show window borders, title bar, etc.
     *                    Disabling decorations will remove borders and title bar, making
     *                    the window appear borderless and more minimalistic.
     */
    void setDecorations(bool decorations);

    /**
     * @brief Checks if the window has decorations such as borders and title bar.
     *
     * @return True if the window has decorations (borders, title bar, etc.), false if it is
     *         borderless and more minimalistic without decorations.
     */
    [[nodiscard]] bool isDecorated() const;

    /**
     * @brief Hides the window.
     */
    void hide();

    /**
     * @brief Shows the window.
     */
    void show();

    /**
     * @brief Centers the window.
     */
    void center();

    /**
     * @brief Sets the background color of the window.
     *
     * This method sets the background color of the window to the specified RGB color.
     *
     * @param red The intensity of the red component of the color (0-255).
     * @param green The intensity of the green component of the color (0-255).
     * @param blue The intensity of the blue component of the color (0-255).
     */
    void setBackgroundColor(int red, int green, int blue);

    /**
     * @brief Gets the native window handle.
     *
     * @return The native window handle.
     *         - On Windows, it should be of type HWND.
     *         - On MacOS, it should be of type NSWindow.
     *         - On Linux, it should be of type GtkWindow.
     */
    [[nodiscard]] void* getNativeWindow();

    /**
     * @brief Sets the monitor scale factor.
     *
     * Sets the scaling factor representing the DPI (dots per inch) scale or display pixel density
     * for the current screen or display. This factor is used to scale the window content to match
     * the display resolution and pixel density.
     *
     * @param scaleFactor The display scale factor.
     */
    void setMonitorScaleFactor(float scaleFactor);

    /**
     * @brief Retrieves the display scale factor.
     *
     * Retrieves the scaling factor representing the DPI (dots per inch) scale
     * or display pixel density for the current screen or display.
     *
     * @return The display scale factor.
     */
    float getMonitorScaleFactor() const;

  private:
    // Window name
    std::string name_;

    // Pointer to the implementation
    std::unique_ptr<Impl> pImpl_{nullptr};

    // App delegate
    AppHandler* appHandler_{nullptr};

    // Collection of webviews
    std::unordered_map<std::string, std::unique_ptr<Webview>> webviews_;
    mutable std::mutex webviewsMutex_;

    // Window sizes (physical size)
    ViewSize minSize_;
    ViewSize maxSize_;
    bool minSizeDefined_{false};
    bool maxSizeDefined_{false};

    float monitorScaleFactor_ = 1.f;
  };

}  // namespace deskgui
