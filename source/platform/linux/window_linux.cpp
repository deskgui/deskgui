/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <system_error>

#include "app_handler_linux.h"
#include "window_linux_impl.h"

using namespace deskgui;

Window::Window(const std::string& name, AppHandler* appHandler, void* nativeWindow)
    : pImpl_{std::make_unique<Impl>()}, name_(name), appHandler_(appHandler) {
  if (nativeWindow != nullptr) {
    isExternalWindow_ = true;
    pImpl_->window = GTK_WINDOW(nativeWindow);
  } else {
    gtk_init(nullptr, nullptr);

    pImpl_->window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

    gtk_window_set_default_size(pImpl_->window, kDefaultWindowRect.R, kDefaultWindowRect.B);
    gtk_window_set_resizable(pImpl_->window, false);
    gtk_window_set_position(pImpl_->window, GTK_WIN_POS_CENTER);
  }

  if (!pImpl_->window) {
    throw std::system_error(errno, std::generic_category());
  }

  g_signal_connect(G_OBJECT(pImpl_->window), "delete-event", G_CALLBACK(pImpl_->onDelete), this);
  g_signal_connect(G_OBJECT(pImpl_->window), "show", G_CALLBACK(pImpl_->onShow), this);
  g_signal_connect(G_OBJECT(pImpl_->window), "configure-event",
                   G_CALLBACK(pImpl_->onConfigureEvent), this);
}

Window::~Window() {
  if (!isExternalWindow_ && pImpl_->window != nullptr) {
    gtk_widget_destroy(GTK_WIDGET(pImpl_->window));
    pImpl_->window = nullptr;
  }
}

void Window::setTitle(const std::string& title) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([title, this] { return setTitle(title); });
  }
  gtk_window_set_title(pImpl_->window, title.c_str());
}

[[nodiscard]] std::string Window::getTitle() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { return getTitle(); });
  }

  return std::string(gtk_window_get_title(pImpl_->window));
}

void Window::setSize(const ViewSize& size, PixelsType type){
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, type, this] { setSize(size, type); });
  }

  auto newSize = size;
  if (type == PixelsType::kLogical) {
    newSize = ViewSize{size.first * monitorScaleFactor_, size.second * monitorScaleFactor_};
  }
  gtk_widget_set_size_request(GTK_WIDGET(pImpl_->window), newSize.first, newSize.second);
}

[[nodiscard]] ViewSize Window::getSize(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([type, this] { return getSize(type); });
  }
  gint width, height;
  gtk_window_get_size(pImpl_->window, &width, &height);

  auto size = ViewSize{static_cast<size_t>(width), static_cast<size_t>(height)};
  if (type == PixelsType::kLogical) {
    size.first /= monitorScaleFactor_;
    size.second /= monitorScaleFactor_;
  }
  return size;
}

void Window::setMaxSize(const ViewSize& size, PixelsType type) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, type, this] { setMaxSize(size, type); });
  }
  ViewSize adjustedSize = size;
  if (type == PixelsType::kLogical) {
    adjustedSize.first *= monitorScaleFactor_;
    adjustedSize.second *= monitorScaleFactor_;
  }

  maxSize_ = adjustedSize;
  ViewSize minSize = getMinSize(PixelsType::kPhysical);

  GdkGeometry hints;
  hints.min_height = minSize.second;
  hints.min_width = minSize.first;
  hints.max_height = maxSize_.first;
  hints.max_width = maxSize_.second;

  GdkWindowHints h
      = minSizeDefined_ ? GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE) : GDK_HINT_MAX_SIZE;
  gtk_window_set_geometry_hints(pImpl_->window, nullptr, &hints, h);
}

[[nodiscard]] ViewSize Window::getMaxSize(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, type]() { return getMaxSize(type); });
  }
  if (type == PixelsType::kLogical) {
    return ViewSize{maxSize_.first / monitorScaleFactor_, maxSize_.second / monitorScaleFactor_};
  } else {
    return maxSize_;
  }
}

void Window::setMinSize(const ViewSize& size, PixelsType type) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, type, this] { setMinSize(size, type); });
  }

  ViewSize adjustedSize = size;
  if (type == PixelsType::kLogical) {
    adjustedSize.first *= monitorScaleFactor_;
    adjustedSize.second *= monitorScaleFactor_;
  }

  minSize_ = adjustedSize;
  minSizeDefined_ = true;

  ViewSize maxSize = getMaxSize(PixelsType::kPhysical);

  GdkGeometry hints;
  hints.min_width = minSize_.first;
  hints.min_height = minSize_.second;
  hints.max_height = maxSize.first;
  hints.max_width = maxSize.second;

  GdkWindowHints h
      = maxSizeDefined_ ? GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE) : GDK_HINT_MIN_SIZE;
  gtk_window_set_geometry_hints(pImpl_->window, nullptr, &hints, h);
}

[[nodiscard]] ViewSize Window::getMinSize(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, type]() { return getMinSize(type); });
  }
  if (type == PixelsType::kLogical) {
    return ViewSize{minSize_.first / monitorScaleFactor_, minSize_.second / monitorScaleFactor_};
  } else {
    return minSize_;
  }
}

void Window::setPosition(const ViewRect& position, PixelsType type) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([position, type, this] { setPosition(position, type); });
  }
  int width = position.R - position.L;
  int height = position.B - position.T;

  if (type == PixelsType::kLogical) {
    width *= monitorScaleFactor_;
    height *= monitorScaleFactor_;
  }
  gtk_window_resize(pImpl_->window, width, height);
  gtk_window_move(pImpl_->window, position.L, position.T);
}

[[nodiscard]] ViewRect Window::getPosition(PixelsType type) const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([type, this] { return getPosition(type); });
  }
  gint x, y, width, height;
  gtk_window_get_position(pImpl_->window, &x, &y);
  gtk_window_get_size(pImpl_->window, &width, &height);

  if (type == PixelsType::kLogical) {
    x /= monitorScaleFactor_;
    y /= monitorScaleFactor_;
    width /= monitorScaleFactor_;
    height /= monitorScaleFactor_;
  }
  return {static_cast<size_t>(x), static_cast<size_t>(y), static_cast<size_t>(x + width),
          static_cast<size_t>(y + height)};
}

void Window::setResizable(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this] { return setResizable(state); });
  }
  gtk_window_set_resizable(pImpl_->window, state);
}

[[nodiscard]] bool Window::isResizable() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { return isResizable(); });
  }

  return gtk_window_get_resizable(pImpl_->window) != FALSE;
}

void Window::setDecorations(bool decorations) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([decorations, this] { setDecorations(decorations); });
  }
  gtk_window_set_decorated(GTK_WINDOW(pImpl_->window), decorations);
}

[[nodiscard]] bool Window::isDecorated() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { return isDecorated(); });
  }
  return gtk_window_get_decorated(GTK_WINDOW(pImpl_->window));
}

void Window::hide() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { hide(); });
  }
  gtk_widget_hide(GTK_WIDGET(pImpl_->window));
}

void Window::show() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this] { show(); });
  }
  gtk_widget_show_all(GTK_WIDGET(pImpl_->window));
}

void Window::center() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this]() { center(); });
  }
  int windowWidth, windowHeight;
  gtk_window_get_size(pImpl_->window, &windowWidth, &windowHeight);

  int x = (gdk_screen_width() - windowWidth) / 2;
  int y = (gdk_screen_height() - windowHeight) / 2;

  gtk_window_move(pImpl_->window, x, y);
}

void Window::enable(bool state){
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, state]() { enable(state); });
  }

  gtk_widget_set_sensitive(pImpl_->window, state ? TRUE : FALSE);

  if (state) {
    gtk_window_present(GTK_WINDOW(pImpl_->window));
  }
}

void Window::setBackgroundColor(int red, int green, int blue) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([this, red, green, blue]() { setBackgroundColor(red, green, blue); });
  }
  GdkColor color;
  color.red = red * 256;
  color.green = green * 256;
  color.blue = blue * 256;

  gtk_widget_modify_bg(GTK_WIDGET(pImpl_->window), GTK_STATE_NORMAL, &color);
}

[[nodiscard]] void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->window); }
