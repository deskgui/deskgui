/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>

namespace deskgui {
  using DispatchTask = std::function<void()>;

  class AppHandler {
  public:
    AppHandler() = default;
    virtual ~AppHandler() = default;

    /**
     * @brief Destroys the window with the specified name.
     *
     * @param name The name or identifier of the window being destroyed.
     */
    virtual void destroyWindow(const std::string& name) = 0;

    /**
     * @brief Checks if the current thread is the main thread.
     *
     * @return True if the current thread is the main thread running the app instance, false
     * otherwise.
     */
    virtual bool isMainThread() const = 0;

    /**
     * @brief Gets the name of the application.
     *
     * @return The name of the application.
     */
    [[nodiscard]] virtual std::string_view getName() const = 0;

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

    template <typename Task> auto dispatchOnMainThread(Task&& task) const {
      using ResultType = typename std::invoke_result_t<Task>;
      std::promise<ResultType> result;
      auto taskWrapper = DispatchTask([task_ = std::forward<Task>(task), &result]() mutable {
        if constexpr (std::is_void_v<ResultType>) {
          task_();
          result.set_value();
        } else {
          result.set_value(task_());
        }
      });

      auto future = result.get_future();

      dispatch(std::move(taskWrapper));

      return future.get();
    };

  protected:
    /**
     * @brief Posts a task to the main thread's message loop
     *
     * @tparam Task The type of the task function to be posted.
     * @param task The task function to be posted.
     */
    virtual void dispatch(DispatchTask&& task) const = 0;
  };

}  // namespace deskgui