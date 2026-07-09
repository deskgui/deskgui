/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <deskgui/app.h>

#include <chrono>
#include <thread>

using namespace deskgui;

int main() {
  App app;

  // The application icon is embedded at build time with the CMake helper,
  // using each platform's native icon format:
  //
  //   deskgui_target_icon(
  //     TARGET app_icon_example
  //     WINDOWS assets/icon.ico
  //     MACOS   assets/icon.icns
  //     LINUX   assets/icon.png)
  //
  // No runtime code is needed. On Windows the icon shows in the title bar,
  // task bar and the .exe itself; on macOS in the Dock; on Linux on the window.
  auto window = app.createWindow("Window");
  window->setTitle("deskgui - app icon example");
  window->setSize({800, 600});
  window->center();

  auto webview = window->createWebview("Webview");
  webview->loadHTMLString(
      "<html><body style='font-family:sans-serif;padding:2rem;margin:0;"
      "background:#1e1e2e;color:#f5f5f5;height:100vh'>"
      "<h1>Look at the title bar / task bar icon \xF0\x9F\x91\x80</h1>"
      "<p>This window's icon was embedded at build time via "
      "<code style='color:#89b4fa'>deskgui_target_icon()</code>.</p>"
      "</body></html>");

  webview->connect<event::WebviewContentLoaded>([window]() {
    window->show();
    window->focus();
  });

  window->connect<event::WindowResize>(
      [&webview](const event::WindowResize& event) { webview->resize(event.size); });

  app.run();
}
