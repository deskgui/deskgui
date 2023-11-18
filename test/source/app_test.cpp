#define DOCTEST_WEBVIEW_TEST

#include <deskgui/app.h>

#include "catch2/catch_all.hpp"

using namespace deskgui;

TEST_CASE("App tests") {
  App app;

  SECTION("Create window") {
    constexpr auto windowName = "test_window";
    auto window = app.createWindow(windowName);
    REQUIRE(app.getWindow(windowName) == window);
  }
}
