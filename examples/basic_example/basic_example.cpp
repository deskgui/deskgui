/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <deskgui/app.h>

using namespace deskgui;

int main() {
  // Create app 🏠
  App app;

  // Create window 🖥️
  auto window = app.createWindow("Window");
  window->setTitle("My awesome app!");
  window->setMinSize({400, 400});
  window->setSize({800, 800});

  // Create webview 🧩
  auto webview = window->createWebview("Webview");
  webview->navigate("https://www.google.com");

  // Show window when webview content is ready to avoid white screen 🧭
  webview->connect<deskgui::event::WebviewSourceChanged>([&window]() { window->show(); });

  // Listen window resize event to resize webview 📐
  window->connect<deskgui::event::WindowResize>(
      [&webview](const deskgui::event::WindowResize& event) { webview->resize(event.size); });

  // Run the application! 🚀
  app.run();
}
