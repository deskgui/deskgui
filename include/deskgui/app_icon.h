/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <cstddef>

namespace deskgui {

  /**
   * @brief Registers the application icon from raw, encoded image bytes (PNG).
   *
   * This is normally called automatically by the C++ source that the
   * `deskgui_target_icon()` CMake helper generates from your icon file. The
   * generated file invokes this function from a static initializer, so the icon
   * is registered before `main()` runs and every window created afterwards picks
   * it up automatically.
   *
   * The pointed-to data must outlive the application (the generated array has
   * static storage duration). Only the last registered icon is kept.
   *
   * @param data Pointer to the encoded image bytes (PNG).
   * @param size Number of bytes.
   * @return Always true, so it can be used to initialize a static bool.
   */
  bool registerAppIcon(const unsigned char* data, std::size_t size);

  namespace internal {
    /// @return Pointer to the registered icon bytes, or nullptr if none.
    const unsigned char* appIconData();
    /// @return Size of the registered icon bytes, or 0 if none.
    std::size_t appIconSize();
  }  // namespace internal

}  // namespace deskgui
