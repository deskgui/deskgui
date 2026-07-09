/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include <deskgui/app_icon.h>

namespace {
  const unsigned char* g_appIconData = nullptr;
  std::size_t g_appIconSize = 0;
}  // namespace

namespace deskgui {

  bool registerAppIcon(const unsigned char* data, std::size_t size) {
    g_appIconData = data;
    g_appIconSize = size;
    return true;
  }

  namespace internal {
    const unsigned char* appIconData() { return g_appIconData; }
    std::size_t appIconSize() { return g_appIconSize; }
  }  // namespace internal

}  // namespace deskgui
