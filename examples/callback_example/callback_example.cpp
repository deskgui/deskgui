/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <deskgui/app.h>

#include <filesystem>
#include <iostream>

static const std::string kWindowName = "window";
static const std::string kWebviewName = "webview";

using namespace deskgui::event;

struct MessageDeserialization {
  void process(const WebviewOnMessage& event) { std::cout << event.message << std::endl; }
};

int main() {
  deskgui::App app;

  auto window = app.createWindow(kWindowName);
  window->setTitle("My awesome webview!");
  window->setResizable(true);
  window->setMinSize({500, 500});
  window->setSize({800, 800});
  window->center();

  auto webview = window->createWebview(kWebviewName);
  webview->enableContextMenu(true);
  webview->enableDevTools(true);
  
  window->connect<WindowResize>(
      [&webview](const WindowResize& event) { webview->resize(event.size); });

  // This C++ callback is exposed as a global JavaScript function, `window.counter_value()`.
  // When `window.counter_value()` is called in JavaScript, this callback will be triggered.
  webview->addCallback("counter_value", [](std::string message) {
    std::cout << "Counter value message " << message << std::endl;
  });

  webview->addCallback("counter_reset", [=](std::string message) {
    std::cout << "Counter reset " << message << std::endl;
    // notify frontend that reset is processed...
    webview->postMessage("Counter reset received on the C++ side! <3");
  });

  // We can listen to all types of messages (including attached callbacks) by connecting to the
  // WebviewOnMessage event.
  MessageDeserialization msgCallback;
  webview->connect<WebviewOnMessage>(&msgCallback, &MessageDeserialization::process);

  std::filesystem::path assetsPath;

#ifdef WEBVIEW_CONTENT_DIRECTORY
  assetsPath = WEBVIEW_CONTENT_DIRECTORY;
#endif

  assetsPath.append("index.html");
  webview->loadFile(assetsPath.u8string());

  webview->connect<WebviewContentLoaded>([&](const WebviewContentLoaded&) { window->show(); });

  app.run();
}
