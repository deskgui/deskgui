/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <system_error>

#include "window_platform_linux.h"

using namespace deskgui;

using Impl = Window::Impl;

Impl::Impl(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : platform_(std::make_unique<Impl::Platform>()), name_(name), appHandler_(appHandler) {
  if (nativeWindow != nullptr) {
    isExternalWindow_ = true;
    platform_->window = GTK_WINDOW(nativeWindow);
  } else {
    gtk_init(nullptr, nullptr);

    platform_->window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

    gtk_window_set_default_size(platform_->window, kDefaultWindowRect.R, kDefaultWindowRect.B);
    gtk_window_set_resizable(platform_->window, false);
    gtk_window_set_position(platform_->window, GTK_WIN_POS_CENTER);
  }

  if (!platform_->window) {
    throw std::system_error(errno, std::generic_category());
  }

  g_signal_connect(G_OBJECT(platform_->window), "delete-event", G_CALLBACK(platform_->onDelete),
                   this);
  g_signal_connect(G_OBJECT(platform_->window), "show", G_CALLBACK(platform_->onShow), this);
  g_signal_connect(G_OBJECT(platform_->window), "configure-event",
                   G_CALLBACK(platform_->onConfigureEvent), this);
  g_signal_connect(gtk_settings_get_default(), "notify::gtk-theme-name",
                   G_CALLBACK(platform_->onThemeChanged), this);
}

Impl::~Impl() {
  if (!isExternalWindow_ && platform_->window != nullptr) {
    gtk_widget_destroy(GTK_WIDGET(platform_->window));
    platform_->window = nullptr;
  }
}

void Impl::setTitle(const std::string& title) {
  gtk_window_set_title(platform_->window, title.c_str());
}

std::string Impl::getTitle() const { return std::string(gtk_window_get_title(platform_->window)); }

void Impl::setSize(const ViewSize& size, PixelsType type) {
  auto newSize = size;
  if (type == PixelsType::kLogical) {
    newSize = ViewSize{size.first * monitorScaleFactor_, size.second * monitorScaleFactor_};
  }
  gtk_widget_set_size_request(GTK_WIDGET(platform_->window), newSize.first, newSize.second);
}

ViewSize Impl::getSize(PixelsType type) const {
  gint width, height;
  gtk_window_get_size(platform_->window, &width, &height);

  auto size = ViewSize{static_cast<size_t>(width), static_cast<size_t>(height)};
  if (type == PixelsType::kLogical) {
    size.first /= monitorScaleFactor_;
    size.second /= monitorScaleFactor_;
  }
  return size;
}

void Impl::setMaxSize(const ViewSize& size, PixelsType type) {
  ViewSize logicalSize = size;
  if (type == PixelsType::kPhysical) {
    logicalSize.first /= monitorScaleFactor_;
    logicalSize.second /= monitorScaleFactor_;
  }

  maxSize_ = logicalSize;
  maxSizeDefined_ = true;

  auto physicalMax = getMaxSize(PixelsType::kPhysical);
  auto physicalMin = getMinSize(PixelsType::kPhysical);

  GdkGeometry hints;
  hints.min_height = physicalMin.second;
  hints.min_width = physicalMin.first;
  hints.max_height = physicalMax.second;
  hints.max_width = physicalMax.first;

  GdkWindowHints h
      = minSizeDefined_ ? GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE) : GDK_HINT_MAX_SIZE;
  gtk_window_set_geometry_hints(platform_->window, nullptr, &hints, h);
}

ViewSize Impl::getMaxSize(PixelsType type) const {
  if (type == PixelsType::kPhysical) {
    return ViewSize{maxSize_.first * monitorScaleFactor_, maxSize_.second * monitorScaleFactor_};
  }
  return maxSize_;
}

void Impl::setMinSize(const ViewSize& size, PixelsType type) {
  ViewSize logicalSize = size;
  if (type == PixelsType::kPhysical) {
    logicalSize.first /= monitorScaleFactor_;
    logicalSize.second /= monitorScaleFactor_;
  }

  minSize_ = logicalSize;
  minSizeDefined_ = true;

  auto physicalMin = getMinSize(PixelsType::kPhysical);
  auto physicalMax = getMaxSize(PixelsType::kPhysical);

  GdkGeometry hints;
  hints.min_width = physicalMin.first;
  hints.min_height = physicalMin.second;
  hints.max_height = physicalMax.second;
  hints.max_width = physicalMax.first;

  GdkWindowHints h
      = maxSizeDefined_ ? GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE) : GDK_HINT_MIN_SIZE;
  gtk_window_set_geometry_hints(platform_->window, nullptr, &hints, h);
}

ViewSize Impl::getMinSize(PixelsType type) const {
  if (type == PixelsType::kPhysical) {
    return ViewSize{minSize_.first * monitorScaleFactor_, minSize_.second * monitorScaleFactor_};
  }
  return minSize_;
}

void Impl::setPosition(const ViewRect& position, PixelsType type) {
  int width = position.R - position.L;
  int height = position.B - position.T;

  if (type == PixelsType::kLogical) {
    width *= monitorScaleFactor_;
    height *= monitorScaleFactor_;
  }
  gtk_window_resize(platform_->window, width, height);
  gtk_window_move(platform_->window, position.L, position.T);
}

ViewRect Impl::getPosition(PixelsType type) const {
  gint x, y, width, height;
  gtk_window_get_position(platform_->window, &x, &y);
  gtk_window_get_size(platform_->window, &width, &height);

  if (type == PixelsType::kLogical) {
    x /= monitorScaleFactor_;
    y /= monitorScaleFactor_;
    width /= monitorScaleFactor_;
    height /= monitorScaleFactor_;
  }
  return {static_cast<size_t>(x), static_cast<size_t>(y), static_cast<size_t>(x + width),
          static_cast<size_t>(y + height)};
}

void Impl::setResizable(bool state) { gtk_window_set_resizable(platform_->window, state); }

bool Impl::isResizable() const { return gtk_window_get_resizable(platform_->window) != FALSE; }

void Impl::setDecorations(bool decorations) {
  gtk_window_set_decorated(GTK_WINDOW(platform_->window), decorations);
}

bool Impl::isDecorated() const { return gtk_window_get_decorated(GTK_WINDOW(platform_->window)); }

void Impl::hide() { gtk_widget_hide(GTK_WIDGET(platform_->window)); }

void Impl::show() { gtk_widget_show_all(GTK_WIDGET(platform_->window)); }

void Impl::center() {
  int windowWidth, windowHeight;
  gtk_window_get_size(platform_->window, &windowWidth, &windowHeight);

  int x = (gdk_screen_width() - windowWidth) / 2;
  int y = (gdk_screen_height() - windowHeight) / 2;

  gtk_window_move(platform_->window, x, y);
}

void Impl::enable(bool state) {
  gtk_widget_set_sensitive(GTK_WIDGET(platform_->window), state ? TRUE : FALSE);

  if (state) {
    gtk_window_present(GTK_WINDOW(platform_->window));
  }
}

void Impl::setBackgroundColor(int red, int green, int blue) {
  GdkColor color;
  color.red = red * 256;
  color.green = green * 256;
  color.blue = blue * 256;

  gtk_widget_modify_bg(GTK_WIDGET(platform_->window), GTK_STATE_NORMAL, &color);
}

void Impl::setTitleBarColor(int red, int green, int blue) {
  GtkCssProvider* provider = gtk_css_provider_new();
  char css[256];
  snprintf(css, sizeof(css), "headerbar { background-color: rgb(%d, %d, %d); }", red, green, blue);
  gtk_css_provider_load_from_data(provider, css, -1, nullptr);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

SystemTheme Impl::getSystemTheme() const {
  gchar* themeName = nullptr;
  g_object_get(gtk_settings_get_default(), "gtk-theme-name", &themeName, nullptr);
  bool isDark = themeName && g_strstr_len(themeName, -1, "dark") != nullptr;
  g_free(themeName);
  return isDark ? SystemTheme::kDark : SystemTheme::kLight;
}

void* Impl::getNativeWindow() { return static_cast<void*>(platform_->window); }

void* Impl::getContentView() { return static_cast<void*>(platform_->window); }
