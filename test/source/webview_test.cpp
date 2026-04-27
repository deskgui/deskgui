#define DOCTEST_WEBVIEW_TEST
#define NOMINMAX

#include <deskgui/app.h>

#include "catch2/catch_all.hpp"

using namespace deskgui;

TEST_CASE("Webview tests") {
  App app;
  auto window = app.createWindow("window");
  auto webview = window->createWebview("Webview");

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

TEST_CASE("WebviewOptions custom scheme keys round-trip") {
  WebviewOptions options;
  options.setOption(WebviewOptions::kCustomSchemeProtocol, std::string{"app"});
  options.setOption(WebviewOptions::kCustomSchemeHost, std::string{"custom"});

  REQUIRE(options.hasOption(WebviewOptions::kCustomSchemeProtocol));
  REQUIRE(options.hasOption(WebviewOptions::kCustomSchemeHost));
  CHECK(options.getOption<std::string>(WebviewOptions::kCustomSchemeProtocol) == "app");
  CHECK(options.getOption<std::string>(WebviewOptions::kCustomSchemeHost) == "custom");
}

namespace {

  // Drive the app loop until a WebviewNavigationStarting matching `expectedPrefix`
  // is observed (the initial pre-navigation, if any, is filtered out so we capture
  // the URL produced by serveResource specifically).
  std::string captureServeResourceUrl(App& app, Webview* webview, const std::string& resource,
                                      const std::string& expectedPrefix) {
    std::string capturedUrl;
    webview->connect<event::WebviewNavigationStarting>(
        [&app, &capturedUrl, expectedPrefix](const event::WebviewNavigationStarting& e) {
          if (e.url.rfind(expectedPrefix, 0) != 0) {
            return;
          }
          capturedUrl = e.url;
          app.terminate();
        });
    webview->serveResource(resource);
    app.run();
    return capturedUrl;
  }

}  // namespace

TEST_CASE("Webview serveResource uses the configured scheme origin") {
  SECTION("Default origin is webview://localhost/") {
    App app("WebviewSchemeTestDefault");
    auto window = app.createWindow("window");
    auto webview = window->createWebview("Webview");

    const auto url = captureServeResourceUrl(app, webview, "index.html", "webview://");
    CHECK(url == "webview://localhost/index.html");
  }

  SECTION("Custom protocol and host produce <proto>://<host>/<path>") {
    WebviewOptions options;
    options.setOption(WebviewOptions::kCustomSchemeProtocol, std::string{"app"});
    options.setOption(WebviewOptions::kCustomSchemeHost, std::string{"custom"});

    App app("WebviewSchemeTestCustom");
    auto window = app.createWindow("window");
    auto webview = window->createWebview("Webview", options);

    const auto url = captureServeResourceUrl(app, webview, "index.html", "app://");
    CHECK(url == "app://custom/index.html");
  }

  SECTION("Empty option values fall back to defaults") {
    WebviewOptions options;
    options.setOption(WebviewOptions::kCustomSchemeProtocol, std::string{});
    options.setOption(WebviewOptions::kCustomSchemeHost, std::string{});

    App app("WebviewSchemeTestEmpty");
    auto window = app.createWindow("window");
    auto webview = window->createWebview("Webview", options);

    const auto url = captureServeResourceUrl(app, webview, "index.html", "webview://");
    CHECK(url == "webview://localhost/index.html");
  }
}
