/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "window_platform_linux.h"

using namespace deskgui;

using Platform = Window::Impl::Platform;

// Callback function for the "on-delete" signal
gboolean Platform::onDelete(GtkWidget* widget, [[maybe_unused]] GdkEvent* event,
                            Window::Impl* window) {
  if (window) {
    event::WindowClose closeEvent{};
    window->events().emit(closeEvent);
    if (closeEvent.isCancelled()) {
      return TRUE;
    }
    window->close();
  }

  return FALSE;
}

// Callback function for the "show" signal
gboolean Platform::onShow(GtkWidget* widget, Window::Impl* window) {
  if (window) {
    gboolean shown = gtk_widget_get_visible(widget);
    window->events().emit(event::WindowShow{shown ? true : false});
  }
  return FALSE;
}

// Callback function for the "configure-event" signal
gboolean Platform::onConfigureEvent(GtkWidget* widget, [[maybe_unused]] GdkEventConfigure* event,
                                    Window::Impl* window) {
  if (window) {
    window->platform()->throttle.trigger([window]() {
      event::WindowResize resizeEvent({window->getSize()});
      window->events().emit(resizeEvent);
    });
  }
  return FALSE;
}

// Callback function for theme change
void Platform::onThemeChanged(GObject* settings, GParamSpec*, Window::Impl* window) {
  if (!window) return;
  gchar* themeName = nullptr;
  g_object_get(settings, "gtk-theme-name", &themeName, nullptr);
  bool isDark = themeName && g_strstr_len(themeName, -1, "dark") != nullptr;
  g_free(themeName);
  auto theme = isDark ? SystemTheme::kDark : SystemTheme::kLight;
  window->events().emit(event::WindowThemeChanged{theme});
}
