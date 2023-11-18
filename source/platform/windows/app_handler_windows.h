/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <windows.h>

#include <future>

#include "deskgui/app_handler.h"

namespace deskgui {
  template <typename Task> auto AppHandler::runOnMainThread(Task&& task) {
    using ResultType = typename std::invoke_result_t<Task>;

    std::promise<ResultType> result;
    auto taskWrapper = MainThreadTask([task = std::forward<Task>(task), &result]() mutable {
      if constexpr (std::is_void_v<ResultType>) {
        task();
        result.set_value();
      } else {
        result.set_value(task());
      }
    });

    auto future = result.get_future();

    queue.push(std::move(taskWrapper));
    SetEvent(eventHandle_);

    return future.get();
  }
}  // namespace deskgui
