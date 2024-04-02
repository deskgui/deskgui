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

  static const inline UINT WM_SAFE_CALL = RegisterWindowMessageW(L"safe_call");
  using MainThreadTask = std::function<void()>;

  template <typename Task> auto AppHandler::runOnMainThread(Task&& task) {
    using ResultType = typename std::invoke_result_t<Task>;

    std::promise<ResultType> result;
    auto taskWrapper = MainThreadTask([task_ = std::forward<Task>(task), &result]() mutable {
      if constexpr (std::is_void_v<ResultType>) {
        task_();
        result.set_value();
      } else {
        result.set_value(task_());
      }
    });

    auto future = result.get_future();

    PostMessage(messageOnlyWindow_, WM_SAFE_CALL, 0, reinterpret_cast<LPARAM>(&taskWrapper));

    return future.get();
  }
}  // namespace deskgui
