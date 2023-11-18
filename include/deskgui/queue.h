/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace deskgui {

  template <typename T> class Queue {
  private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;

  public:
    bool push(T&& value) {
      std::unique_lock<std::mutex> lock(mutex_);
      queue_.push(std::move(value));
      return true;
    }

    // Pop an element from the queue, blocking if the queue is empty
    bool pop(T& result) {
      std::unique_lock<std::mutex> lock(mutex_);

      if (!queue_.empty()) {
        result = std::move(queue_.front());
        queue_.pop();
        return true;
      }

      return false;  // Queue was empty
    }

    void clear() {
      std::lock_guard<std::mutex> lock(mutex_);
      std::queue<T> emptyQueue;
      queue_.swap(emptyQueue);
    }

    // Check if the queue is empty
    bool empty() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return queue_.empty();
    }

    // Get the size of the queue
    size_t size() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return queue_.size();
    }
  };
}  // namespace deskgui
