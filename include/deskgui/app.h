/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/app_handler.h>
#include <deskgui/window.h>

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace deskgui {
  /**
   * @class App
   * @brief The main application class responsible for running the deskgui application.
   *
   * @param name The name of the application. Defaults to "deskgui" if not provided.
   *
   */
  class App : public AppHandler {
  public:
    class Impl;

    explicit App(const std::string& name = "deskgui");
    ~App() final;

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
    void destroyWindow(const std::string& name) override;

    /**
     * Get the window with the specified name.
     * The caller should not assume ownership of the returned pointer.
     *
     * @param name The name of the window to retrieve.
     * @return A pointer to the window if found, otherwise nullptr.
     */
    Window* getWindow(const std::string& name) const;

    /**
     * @brief Gets the name of the application.
     *
     * @return The name of the application.
     */
    [[nodiscard]] std::string_view getName() const override;

    /**
     * @brief Starts the main event loop of the application.
     *
     * This method enters the main event loop and continues running until the application is
     * terminated. It processes messages from windows and tasks posted to the main thread.
     */
    void run();

    /**
     * @brief Terminates the application's main event loop and destroys all windows.
     *
     * This method terminates the application's main event loop and cleans up any resources
     * associated with it. All windows created by the application will be destroyed.
     */
    void terminate();

    /**
     * @brief Checks if the application is currently running.
     *
     * @return True if the application is currently running, false otherwise.
     */
    [[nodiscard]] bool isRunning() const;

    /**
     * @brief Checks if the current thread is the main thread.
     *
     * @return True if the current thread is the main thread, false otherwise.
     */
    [[nodiscard]] bool isMainThread() const override;

  private:
    std::unique_ptr<Impl> impl_{nullptr};

    /**
     * @brief Posts a task to the main thread's message loop
     *
     * @tparam Task The type of the task function to be posted.
     * @param task The task function to be posted.
     * @return The result of the task function, if applicable.
     */
    void dispatch(DispatchTask&& task) const override;

    /**
     * @brief Gets a pointer to the application handler.
     *
     * @return A pointer to the application handler.
     */
    inline AppHandler* getHandler() { return this; }
  };

}  // namespace deskgui
