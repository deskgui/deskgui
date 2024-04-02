#define DOCTEST_WEBVIEW_TEST
#define NOMINMAX

#include <deskgui/app.h>

#include "catch2/catch_all.hpp"

using namespace deskgui;

TEST_CASE("Webview tests") {
  App app;
  auto window = app.createWindow("window");
  auto webview = window->createWebview("webview");

  SECTION("Navigate to url") {
    std::string url = "http://localhost/";
    webview->navigate(url);
    webview->connect<event::WebviewSourceChanged>([&app]() { app.terminate(); });
    app.run();
    CHECK(url == webview->getUrl());
  }

  SECTION("Load file") {
    std::string file = "file.html";
    webview->loadFile(file);
    webview->connect<event::WebviewSourceChanged>([&app]() { app.terminate(); });
    app.run();
    CHECK("file://" + file + "/" == webview->getUrl());
  }
}
