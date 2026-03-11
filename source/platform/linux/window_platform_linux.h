/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <gtk/gtk.h>

#include "interfaces/window_impl.h"
#include "utils/throttle.h"

namespace deskgui {
  constexpr size_t kResizeThrottleInMs = 15;

  struct Window::Impl::Platform {
    GtkWindow* window;
    GtkWidget* container;

    static gboolean onDelete(GtkWidget* widget, GdkEvent* event, Window::Impl* window);
    static gboolean onShow(GtkWidget* widget, Window::Impl* window);
    static gboolean onConfigureEvent(GtkWidget* widget, GdkEventConfigure* event,
                                     Window::Impl* window);
    static void onThemeChanged(GObject* settings, GParamSpec* pspec, Window::Impl* window);

    Throttle throttle{kResizeThrottleInMs};
  };
}  // namespace deskgui
