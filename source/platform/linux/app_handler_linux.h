/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <gtk/gtk.h>

#include <future>

#include "deskgui/app_handler.h"

namespace deskgui {

  template <typename Task> auto AppHandler::runOnMainThread(Task&& task) {
    using ResultType = typename std::invoke_result_t<Task>;
    using TaskFunction = std::function<void()>;

    std::promise<ResultType> result;
    TaskFunction taskWrapper = [task = std::forward<Task>(task), &result]() mutable {
      if constexpr (std::is_void_v<ResultType>) {
        task();
        result.set_value();
      } else {
        result.set_value(task());
      }
    };

    auto future = result.get_future();
    g_idle_add(
        [](gpointer user_data) -> gboolean {
          TaskFunction* task = static_cast<TaskFunction*>(user_data);
          (*task)();
          delete task;
          return G_SOURCE_REMOVE;
        },
        new TaskFunction(taskWrapper));
    return future.get();
  }
}  // namespace deskgui
