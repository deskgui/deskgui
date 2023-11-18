/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <string>

namespace deskgui {
  // Defines the size of a view, represented by width and height.
  using ViewSize = std::pair<std::size_t, std::size_t>;

  // Represents the rectangle boundaries of a view.
  struct ViewRect {
    std::size_t L;  // Left coordinate of the rectangle.
    std::size_t T;  // Top coordinate of the rectangle.
    std::size_t R;  // Right coordinate of the rectangle.
    std::size_t B;  // Bottom coordinate of the rectangle.
    friend bool operator==(const ViewRect& lhs, const ViewRect& rhs) {
      return (lhs.L == rhs.L) && (lhs.T == rhs.T) && (lhs.R == rhs.R) && (lhs.B == rhs.B);
    }
  };

  // Represents the default rectangle for a window.
  static const ViewRect kDefaultWindowRect = {0, 0, 600, 600};

  // Callback function type for receiving messages.
  using MessageCallback = std::function<void(std::string)>;

  typedef size_t UniqueId;

  struct UniqueIdGenerator {
    static UniqueId newId() {
      static std::atomic<UniqueId> registerId{0};
      return registerId.fetch_add(1);
    }
  };

  struct EventListenerId : public UniqueIdGenerator {};
}  // namespace deskgui
