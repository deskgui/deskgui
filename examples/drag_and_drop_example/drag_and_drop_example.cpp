/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <deskgui/app.h>
#include <deskgui/resource_compiler.h>

using namespace deskgui;
using namespace deskgui::event;

int main() {
  App app("DragAndDropExample");
  auto window = app.createWindow("window");

  window->setTitle("Drag and Drop Example");
  window->setResizable(true);
  window->setMinSize({500, 500});
  window->setSize({800, 800});
  window->center();

  auto applyTheme = [&window](SystemTheme theme) {
    if (theme == SystemTheme::kDark) {
      window->setBackgroundColor(30, 30, 30);
      window->setTitleBarColor(45, 45, 45);
    } else {
      window->setBackgroundColor(249, 203, 103);
      window->setTitleBarColor(200, 150, 70);
    }
  };

  applyTheme(window->getSystemTheme());

  window->connect<WindowThemeChanged>(
      [&applyTheme](const WindowThemeChanged& event) { applyTheme(event.theme); });

  WebviewOptions options;
  options.setOption(WebviewOptions::kActivateNativeDragAndDrop, true);
  auto webview = window->createWebview("webview", options);

  webview->loadResources(getCompiledResources("drag_and_drop_example_web_resources"));
  webview->serveResource("index.html");
  webview->enableContextMenu(true);
  webview->enableDevTools(true);

  window->connect<WindowResize>(
      [&webview](const WindowResize& event) { webview->resize(event.size); });

  webview->connect<WebviewContentLoaded>([&window]() { window->show(); });

  app.run();
}
