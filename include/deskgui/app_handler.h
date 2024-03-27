/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#ifdef WIN32
#  include "queue.h"
#endif

#include <atomic>
#include <functional>
#include <iostream>
#include <thread>

namespace deskgui {
  /**
   * @class AppHandler
   * @brief The base class for handling main thread operations and synchronizing windows and
   * webviews within the application instance.
   *
   * The `AppHandler` class provides essential methods to determine if the current thread is the
   * main thread, facilitating safe execution of tasks on the main thread's message loop.
   * Additionally, it manages window synchronization and keeps track of open windows in the
   * application.
   * 
   * @param name The name associated with this Application.
   * 
   */
  class AppHandler {
  public:
    explicit AppHandler(const std::string& name) : name_{name} {}
    virtual ~AppHandler() = default;

    /**
     * @brief Get the name associated with this Application.
     *
     *
     * @return A constant reference to the name of the app.
     */
    inline const std::string& getName() const { return name_; }

    /**
     * @brief Checks if the current thread is the main thread.
     *
     * @return True if the current thread is the main thread running the app instance, false
     * otherwise.
     */
    inline bool isMainThread() const { return std::this_thread::get_id() == mainThreadId_; }

    /**
     * @brief Posts a task to the main thread's message loop in a thread-safe manner.
     *
     * This method posts a task function to be executed on the main thread's message loop.
     * The task function should not have any side effects on shared resources.
     *
     * @tparam Task The type of the task function to be posted.
     * @param task The task function to be posted.
     * @return The result of the task function, if applicable.
     */
    template <typename Task> auto runOnMainThread(Task&& task);

    /**
     * @brief Get the count of currently open windows in the application.
     *
     * @note This count represents the number of active windows at the time of the function call.
     *
     * @return The count of currently open windows.
     */
    inline size_t getOpenWindowsCount() const { return openedWindows_.load(); }

    /**
     * @brief Notifies the `AppHandler` about a window being closed from the user interface.
     *
     * @param name The name or identifier of the window being closed.
     */
    virtual void notifyWindowClosedFromUI(const std::string& name) = 0;

  protected:
    // App name
    std::string name_;

    // ID of the main thread
    std::thread::id mainThreadId_ = std::this_thread::get_id();

    // Number of open windows
    std::atomic<size_t> openedWindows_ = 0;

#ifdef WIN32
    using MainThreadTask = std::function<void()>;  // windows only
    Queue<MainThreadTask> queue;                   // windows only
    void* eventHandle_ = nullptr;                  // windows only
#endif
  };
}  // namespace deskgui
