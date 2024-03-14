/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <deskgui/app.h>
#include <deskgui/resource_compiler.h>

#include <filesystem>

using namespace deskgui;
using namespace deskgui::event;

int main() {
  App app;
  auto window = app.createWindow("window");

  window->setTitle("Embedded web resources example");
  window->setResizable(true);
  window->setMinSize({500, 500});
  window->setSize({800, 800});
  window->center();

  WebViewOptions options;
  options.setOption(WebViewOptions::kRemoteDebuggingPort, 9222);

  auto webview = window->createWebview("webview", options);

  webview->loadResources(getCompiledResources("web_resources"));
  webview->serveResource("index.html");
  // webview->serveResource("src/lenna.png"); //try loading a png!

  window->connect<WindowResize>(
      [&webview](const WindowResize& event) { webview->resize(event.size); });

  webview->connect<WebviewSourceChanged>([&window]() { window->show(); });

  app.run();
}
