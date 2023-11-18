#define DOCTEST_WINDOW_TEST

#include <deskgui/app.h>

#include <thread>

#include "catch2/catch_all.hpp"

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
    CHECK(expectedSize == window->getSize());
  }

  SECTION("Set and get max size") {
    deskgui::ViewSize expectedSize = {600, 600};
    window->setMaxSize(expectedSize);
    CHECK(expectedSize == window->getMaxSize());
  }

  SECTION("Set and get min size") {
    deskgui::ViewSize expectedSize = {600, 600};
    window->setMinSize(expectedSize);
    CHECK(expectedSize == window->getMinSize());
  }

  SECTION("Set resizable") {
    window->setResizable(true);
    CHECK(window->isResizable());

    window->setResizable(false);
    CHECK_FALSE(window->isResizable());
  }

  SECTION("Set and get window position") {
    deskgui::ViewRect position{200, 100, 500, 600};
    window->setPosition(position);
    CHECK(position == window->getPosition());
  }

  SECTION("Set and get decorated") {
    window->setDecorations(true);
    CHECK(window->isDecorated());
    window->setDecorations(false);
    CHECK_FALSE(window->isDecorated());
  }
}
