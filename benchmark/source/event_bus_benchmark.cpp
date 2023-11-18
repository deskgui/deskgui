#include <deskgui/event_bus.h>

#include <catch2/catch_all.hpp>
#include <exception>

TEST_CASE("EventBus Benchmark") {
  deskgui::EventBus eventBus;

  struct Event1 {
    int value;
  };

  struct Event2 {
    int value;
  };

  constexpr int kNumOfConnections = 1000;

  for (int i = 0; i < kNumOfConnections; ++i) {
    eventBus.connect<Event1>([](const Event1 &) {});
  }

  eventBus.emit(Event1{1});

  BENCHMARK("Emit event to " + std::to_string(kNumOfConnections) + " connections") {
    eventBus.emit(Event1{1});
  };

  BENCHMARK("Emit non-listened event to " + std::to_string(kNumOfConnections) + " connections") {
    eventBus.emit(Event2{2});
  };
}
