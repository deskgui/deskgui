/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <gtk/gtk.h>

#include "deskgui/window.h"
#include "utils/throttle.h"


namespace deskgui {
  constexpr size_t kResizeThrottleInMs = 15;

  struct Window::Impl {
    GtkWindow* window;
    GtkWidget* container;

    static inline gboolean onDelete(GtkWidget* widget, GdkEvent* event, Window* window);
    static inline gboolean onShow(GtkWidget* widget, Window* window);
    static inline gboolean onConfigureEvent(GtkWidget* widget, GdkEventConfigure* event,
                                            Window* window);

    Throttle throttle{kResizeThrottleInMs};
  };

  // Callback function for the "on-delete" signal
  inline gboolean Window::Impl::onDelete(GtkWidget* widget, [[maybe_unused]] GdkEvent* event,
                                         Window* window) {
    if (window) {
      event::WindowClose closeEvent{};
      window->emit(closeEvent);
      if (closeEvent.isCancelled()) {
        return TRUE;
      }
      window->appHandler_->notifyWindowClosedFromUI(window->getName());
    }

    return FALSE;
  }

  // Callback function for the "show" signal
  inline gboolean Window::Impl::onShow(GtkWidget* widget, Window* window) {
    if (window) {
      gboolean shown = gtk_widget_get_visible(widget);
      window->emit(event::WindowShow{shown ? true : false});
    }
    return FALSE;
  }

  // Callback function for the "configure-event" signal
  inline gboolean Window::Impl::onConfigureEvent(GtkWidget* widget,
                                                 [[maybe_unused]] GdkEventConfigure* event,
                                                 Window* window) {
    if (window) {
      window->pImpl_->throttle.trigger(
          [window]() { window->emit<event::WindowResize>({window->getSize()}); });
    }
    return FALSE;
  }
}  // namespace deskgui
