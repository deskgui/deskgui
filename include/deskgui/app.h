/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */
#pragma once

#include <deskgui/app_handler.h>
#include <deskgui/window.h>

#include <atomic>
#include <iostream>
#include <unordered_map>

namespace deskgui {

  /**
   * @class App
   * @brief The main application class responsible for running the deskgui application.
   *
   */
  class App : private AppHandler {
  public:
    App() = default;
    ~App() = default;

    /**
     * @brief Starts the main event loop of the application.
     *
     * This method enters the main event loop and continues running until the application is
     * terminated. It processes messages from windows and tasks posted to the main thread.
     */
    void run();

    /**
     * @brief Checks if the application is currently running.
     *
     * @return True if the application is currently running, false otherwise.
     */
    [[nodiscard]] inline bool isRunning() const { return isRunning_.load(); }

    /**
     * @brief Terminates the application's main event loop and destroys all windows.
     *
     * This method terminates the application's main event loop and cleans up any resources
     * associated with it. All windows created by the application will be destroyed.
     */
    void terminate();

    /**
     * Create a new window with the specified name and position it with the given rect.
     *
     * @param name The name of the window.
     * @param nativeWindow Pointer to the native window.
     *                     - On Windows, it should be of type HWND.
     *                     - On MacOS, it should be of type NSWindow.
     *                     - On Linux, it should be of type GtkWindow.
     * @remarks The nativeWindow parameter is optional and defaults to nullptr if not provided.
     *          Ensure that the nativeWindow is of the correct type for the target platform.
     *          Improper usage or invalid nativeWindow types may result in undefined behavior.
     * @return A pointer to the created window.
     */
    [[nodiscard]] Window* createWindow(const std::string& name, void* nativeWindow = nullptr);

    /**
     * Destroy the window with the specified name.
     *
     * This method will destroy and deallocate the window object associated with the given name.
     * After calling this method, the window will be removed from the application and its
     * resources will be released, including its webviews.
     *
     * @param name The name of the window to be destroyed.
     */
    void destroyWindow(const std::string& name);

    /**
     * Get the window with the specified name.
     * The caller should not assume ownership of the returned pointer.
     *
     * @param name The name of the window to retrieve.
     * @return A pointer to the window if found, otherwise nullptr.
     */
    Window* getWindow(const std::string& name) const;

  private:
    /**
     * @brief Override method for notifying the application that a window with the specified
     * name was closed from the user interface.
     *
     * The `notifyWindowClosedFromUI` method is called when a window with the given name is
     * closed by the user through the user interface.
     *
     * The method destroys the window instance associated with the given
     * name, releasing all resources used by the window, including its webviews. It is essential
     * to ensure proper cleanup and resource management.
     *
     * @param name The name or identifier of the closed window.
     */
    void notifyWindowClosedFromUI(const std::string& name) override;

    /**
     * @brief Gets a pointer to the application handler.
     *
     * @return A pointer to the application handler.
     */
    inline AppHandler* getHandler() { return this; }

    std::atomic<bool> isRunning_{false};  // Atomic flag to indicate if the application is running.

    // Collection of windows
    std::unordered_map<std::string, std::unique_ptr<Window>> windows_;
    mutable std::mutex windowsMutex_;
  };

}  // namespace deskgui
