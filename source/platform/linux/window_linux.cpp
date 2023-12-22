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
  if (pImpl_->window != nullptr) {
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
    return appHandler_->runOnMainThread([=] { return getTitle(); });
  }

  return std::string(gtk_window_get_title(pImpl_->window));
}

void Window::setSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { return setSize(size); });
  }
  gtk_widget_set_size_request(GTK_WIDGET(pImpl_->window), size.first, size.second);
}

[[nodiscard]] ViewSize Window::getSize() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getSize(); });
  }
  gint width, height;
  gtk_window_get_size(pImpl_->window, &width, &height);
  return {static_cast<size_t>(width), static_cast<size_t>(height)};
}

void Window::setPosition(const ViewRect& position) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([position, this] { setPosition(position); });
  }
  int width = position.R - position.L;
  int height = position.B - position.T;
  gtk_window_resize(pImpl_->window, width, height);
  gtk_window_move(pImpl_->window, position.L, position.T);
}

[[nodiscard]] ViewRect Window::getPosition() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return getPosition(); });
  }
  gint L, T, W, H;
  gtk_window_get_position(pImpl_->window, &L, &T);
  gtk_window_get_size(pImpl_->window, &W, &H);
  return {static_cast<size_t>(L), static_cast<size_t>(T), static_cast<size_t>(L + W),
          static_cast<size_t>(T + H)};
}

void Window::setResizable(bool state) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([state, this] { return setResizable(state); });
  }
  gtk_window_set_resizable(pImpl_->window, state);
}

[[nodiscard]] bool Window::isResizable() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return isResizable(); });
  }

  return gtk_window_get_resizable(pImpl_->window) != FALSE;
}

void Window::setDecorations(bool decorations) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { setDecorations(decorations); });
  }
  gtk_window_set_decorated(GTK_WINDOW(pImpl_->window), decorations);
}

[[nodiscard]] bool Window::isDecorated() const {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { return isDecorated(); });
  }
  return gtk_window_get_decorated(GTK_WINDOW(pImpl_->window));
}

void Window::hide() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { hide(); });
  }
  gtk_widget_hide(GTK_WIDGET(pImpl_->window));
}

void Window::show() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=] { show(); });
  }
  gtk_widget_show_all(GTK_WIDGET(pImpl_->window));
}

void Window::center() {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([=]() { center(); });
  }
  int windowWidth, windowHeight;
  gtk_window_get_size(pImpl_->window, &windowWidth, &windowHeight);

  int x = (gdk_screen_width() - windowWidth) / 2;
  int y = (gdk_screen_height() - windowHeight) / 2;

  gtk_window_move(pImpl_->window, x, y);
}

[[nodiscard]] void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->window); }

void Window::setMinSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { setMinSize(size); });
  }

  minSize_ = size;
  minSizeDefined_ = true;

  ViewSize maxSize = getMaxSize();

  GdkGeometry hints;
  hints.min_width = size.first;
  hints.min_height = size.second;
  hints.max_height = maxSize.first;
  hints.max_width = maxSize.second;

  GdkWindowHints h
      = maxSizeDefined_ ? GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE) : GDK_HINT_MIN_SIZE;
  gtk_window_set_geometry_hints(pImpl_->window, nullptr, &hints, h);
}

void Window::setMaxSize(const ViewSize& size) {
  if (!appHandler_->isMainThread()) {
    return appHandler_->runOnMainThread([size, this] { setMaxSize(size); });
  }
  maxSize_ = size;
  maxSizeDefined_ = true;
  ViewSize minSize = getMinSize();

  GdkGeometry hints;
  hints.min_height = minSize.second;
  hints.min_width = minSize.first;
  hints.max_height = size.first;
  hints.max_width = size.second;

  GdkWindowHints h
      = minSizeDefined_ ? GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE) : GDK_HINT_MAX_SIZE;
  gtk_window_set_geometry_hints(pImpl_->window, nullptr, &hints, h);
}

float Window::getDisplayScaleFactor() { return 1.f; } // not implemented yed
