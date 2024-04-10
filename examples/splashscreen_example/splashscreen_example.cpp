/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <deskgui/app.h>
#include <deskgui/resource_compiler.h>

#include <filesystem>
#include <iostream>

static const std::string kApplicationName = "SplashcreenExample";

static const std::string kLoadingWindowName = "loadingWindow";
static const std::string kLoadingViewName = "loadingView";

static const std::string kAppWindowName = "appWindow";
static const std::string kAppViewName = "appView";

using namespace deskgui;
using namespace deskgui::event;

int main() {
  App app(kApplicationName);

  // loading window
  auto loadingWindow = app.createWindow(kLoadingWindowName);
  // app window
  auto appWindow = app.createWindow(kAppWindowName);

  if (loadingWindow && appWindow) {
    loadingWindow->setTitle("Loading window!");
    loadingWindow->setSize({200, 200});
    loadingWindow->center();
    loadingWindow->setDecorations(false);
    loadingWindow->setResizable(false);

    auto loadingView = loadingWindow->createWebview(kLoadingViewName);

    // resize webview to window size
    loadingWindow->connect<WindowResize>(
        [&loadingView](const WindowResize& event) { loadingView->resize(event.size); });

    loadingView->loadResources(deskgui::getCompiledResources("splashscreen_resources"));
    loadingView->serveResource("loading.html");

    // show window when the content is loaded in the webview (to avoid blank screen)
    loadingView->connect<WebviewContentLoaded>(
        [&loadingWindow](const WebviewContentLoaded& event [[maybe_unused]]) {
          loadingWindow->show();
        });

    appWindow->setTitle("My App Window");
    appWindow->setMinSize({400, 400});
    appWindow->setSize({800, 600});
    appWindow->center();
    appWindow->setResizable(true);

    auto appView = appWindow->createWebview(kAppViewName);
    appWindow->connect<WindowResize>(
        [&appView](const WindowResize& event) { appView->resize(event.size); });

    appView->loadResources(deskgui::getCompiledResources("demo_resources"));
    appView->serveResource("helloworld.html");

    // simulate a loading delay, then destroy loading window and show the app window
    appView->connect<WebviewContentLoaded>(
        [&appWindow, &app](const WebviewContentLoaded& event [[maybe_unused]]) {
          std::thread sleep([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            app.destroyWindow(kLoadingWindowName);
            appWindow->show();
          });
          sleep.detach();
        });

    app.run();
  }
}
