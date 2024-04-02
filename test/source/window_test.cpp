#define DOCTEST_WINDOW_TEST
#define NOMINMAX

#include <deskgui/app.h>

#include <thread>

#include "catch2/catch_all.hpp"

deskgui::ViewSize pixelsToDips(deskgui::ViewSize size, float scale) {
  return {size.first / scale, size.second / scale};
}

TEST_CASE("Window test") {
  deskgui::App app;
  auto window = app.createWindow("window");

  SECTION("Get native window") { CHECK(window->getNativeWindow() != nullptr); }

  SECTION("Set and get title") {
    std::string expectedTitle = "Window tests";
    window->setTitle(expectedTitle);
    CHECK(expectedTitle == window->getTitle());
  }

  SECTION("Set and get size") {
    deskgui::ViewSize expectedSize = {600, 600};
    window->setSize(expectedSize);
    CHECK(expectedSize == pixelsToDips(window->getSize(), window->getMonitorScaleFactor()));
  }

  SECTION("Set and get max size") {
    deskgui::ViewSize expectedSize = {600, 600};
    window->setMaxSize(expectedSize);
    CHECK(expectedSize == pixelsToDips(window->getMaxSize(), window->getMonitorScaleFactor()));
  }

  SECTION("Set and get min size") {
    deskgui::ViewSize expectedSize = {600, 600};
    window->setMinSize(expectedSize);
    CHECK(expectedSize == pixelsToDips(window->getMinSize(), window->getMonitorScaleFactor()));
  }

  SECTION("Set resizable") {
    window->setResizable(true);
    CHECK(window->isResizable());

    window->setResizable(false);
    CHECK_FALSE(window->isResizable());
  }

  SECTION("Set and get window position") {
    deskgui::ViewRect expectedPosition{200, 100, 500, 600};
    window->setPosition(expectedPosition);
    auto position = window->getPosition();
    auto scale = window->getMonitorScaleFactor();
    position.L /= scale;
    position.T /= scale;
    position.R /= scale;
    position.B /= scale;
    CHECK(expectedPosition == position);
  }

  SECTION("Set and get decorated") {
    window->setDecorations(true);
    CHECK(window->isDecorated());
    window->setDecorations(false);
    CHECK_FALSE(window->isDecorated());
  }
}
