/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <future>

#include "deskgui/app_handler.h"

namespace deskgui {

  void dispatchInMainQueue(std::function<void()> task);

  template <typename Task> auto AppHandler::runOnMainThread(Task&& task) {
    using ResultType = typename std::invoke_result_t<Task>;

    std::promise<ResultType> result;
    auto taskWrapper = [task = std::forward<Task>(task), &result]() mutable {
      if constexpr (std::is_void_v<ResultType>) {
        task();
        result.set_value();
      } else {
        result.set_value(task());
      }
    };

    auto future = result.get_future();
    dispatchInMainQueue(taskWrapper);
    return future.get();
  }
}  // namespace deskgui
