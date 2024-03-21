/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <chrono>
#include <functional>

namespace deskgui {
  using Func = std::function<void()>;

  /**
   * Throttle - A utility class to throttle function calls, limiting their frequency
   *            to prevent rapid consecutive invocations within a specified time period.
   *
   * The Throttle blocks any function call that occurs in less than the specified period,
   * effectively reducing the rate at which the function is executed. This is useful for
   * optimizing performance in certain scenarios, such as reducing flickering when resizing.
   */
  class Throttle {
  public:
    /**
     * Constructor - Creates a Throttle with the specified time period.
     *
     * @param period The time period in milliseconds. Function calls will be
     *               throttled if they occur within this time frame.
     */
    Throttle(size_t period) : period_(period){}
    ~Throttle() = default;

    /**
     * trigger - Triggers the provided function if the time elapsed since the
     *           last invocation is greater than the specified period.
     *
     * If the time since the last invocation is less than the period, the
     * function call is blocked, effectively throttling consecutive calls.
     *
     * @param f The function to be executed.
     */
    void trigger(const Func& f) {
      auto now = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - created).count() > period_) {
        f();
        created = now;
      }
    }

  private:
    size_t period_ = 0;
    std::chrono::high_resolution_clock::time_point created
        = std::chrono::high_resolution_clock::now();
  };
}  // namespace deskgui
