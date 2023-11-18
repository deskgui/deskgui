/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <deskgui/events.h>

#include <any>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace deskgui {

  class EventBus {
  public:
    template <class EventType>
    [[maybe_unused]] UniqueId connect(std::function<void(EventType&)>&& listener) {
      std::unique_lock<std::shared_mutex> lock(mutex_);

      return connectHelper<EventType>([callback = std::move(listener)](auto event) {
        callback(*static_cast<EventType*>(event));
      });
    }

    template <class EventType>
    [[maybe_unused]] UniqueId connect(std::function<void(void)>&& listener) {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      return connectHelper<EventType>([callback = std::move(listener)](auto event) { callback(); });
    }

    template <class EventType, typename ClassType, typename MemberFunction>
    [[maybe_unused]] UniqueId connect(ClassType* instance, MemberFunction&& memberFunction) {
      std::unique_lock<std::shared_mutex> lock(mutex_);

      return connectHelper<EventType>([instance, memberFunction](auto event) {
        (instance->*memberFunction)(*static_cast<EventType*>(event));
      });
    }

    template <typename EventType> void disconnect(UniqueId id) {
      std::unique_lock<std::shared_mutex> lock(mutex_);

      const auto typeId = std::type_index(typeid(EventType));
      const auto it = connections_.find(typeId);
      if (it != connections_.end()) {
        it->second.erase(id);
      }
    }

    template <class EventType, typename = std::enable_if_t<!std::is_pointer_v<EventType>>>
    void emit(EventType&& event) {
      std::shared_lock<std::shared_mutex> lock(mutex_);

      const auto typeId = std::type_index(typeid(EventType));
      const auto it = connections_.find(typeId);
      if (it != connections_.end()) {
        for (const auto& connection : it->second) {
          const auto& callback = connection.second;
          callback(&event);
        }
      }
    }

    template <class EventType> [[nodiscard]] std::size_t connectionCount() const {
      std::shared_lock<std::shared_mutex> lock(mutex_);
      const auto typeId = std::type_index(typeid(EventType));
      const auto it = connections_.find(typeId);
      return (it != connections_.end()) ? it->second.size() : 0;
    }

    void clearConnections() {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      connections_.clear();
    }

  private:
    template <class EventType, class ListenerCallback>
    UniqueId connectHelper(ListenerCallback&& listener) {
      const auto id = EventListenerId::newId();
      const auto typeId = std::type_index(typeid(EventType));

      auto& callbacks = connections_[typeId];
      callbacks.try_emplace(id, std::forward<ListenerCallback>(listener));

      return id;
    }

    mutable std::shared_mutex mutex_;

    using EventCallback = std::function<void(void*)>;
    using Connections = std::unordered_map<UniqueId, EventCallback>;
    using EventConnections = std::unordered_map<std::type_index, Connections>;

    EventConnections connections_;
  };

}  // namespace deskgui
