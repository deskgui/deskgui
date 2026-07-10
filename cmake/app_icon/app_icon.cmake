# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License
#
# deskgui_target_icon() embeds an application icon into a target at build time.
#
# Provide each platform's native icon format directly (no conversion is done):
#   - WINDOWS: a .ico file
#   - MACOS:   a .icns file (a .png also works for the runtime Dock icon)
#   - LINUX:   a .png file
#
# ICON is an optional fallback used for any platform without a specific entry.
#
# Usage:
#   deskgui_target_icon(
#     TARGET  my_app
#     WINDOWS assets/icon.ico
#     MACOS   assets/icon.icns
#     LINUX   assets/icon.png
#   )
#
# Per platform:
#   - Windows: the .ico is embedded via a generated .rc. This gives the .exe its
#     Explorer/desktop icon and, because deskgui loads it as the window class
#     icon, the title bar and task bar icons as well.
#   - macOS / Linux: the icon bytes are embedded in a generated C++ source and
#     registered with deskgui (deskgui::registerAppIcon), which applies them to
#     every window at creation time.
#   - macOS: sets the Dock / Cmd-Tab icon. For MACOSX_BUNDLE targets a .icns is
#     also set as the bundle icon, so Finder shows it too. (macOS windows have
#     no title bar icon.)
#   - Linux: on X11 this sets the task bar / Alt-Tab / window decoration icon
#     (_NET_WM_ICON). On Wayland runtime icons are ignored by the shell, and
#     launcher/menu icons always come from the icon theme: ship a .desktop file
#     with Icon= and an app id matching the binary for full desktop integration.

# The .rc.in / .cpp.in templates live next to this file.
set(_DESKGUI_APP_ICON_DIR "${CMAKE_CURRENT_LIST_DIR}")

# Must match IDI_DESKGUI_APP_ICON in source/platform/windows/app_icon_win32.h
set(_DESKGUI_APP_ICON_ID 101)

# Enable the RC language once, at module inclusion time (stable directory scope).
# Doing this inside deskgui_target_icon() breaks CMake's build-time regeneration
# because CMAKE_RC_COMPILER would not be cached. Enabling it here records it in
# the cache so .rc files compile on every generator.
if(WIN32)
  enable_language(RC)
endif()

function(deskgui_target_icon)
  set(oneValueArgs TARGET NAME ICON WINDOWS MACOS LINUX)
  cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})

  if(NOT ARG_TARGET)
    message(FATAL_ERROR "deskgui_target_icon: TARGET is required")
  endif()
  if(NOT TARGET ${ARG_TARGET})
    message(FATAL_ERROR "deskgui_target_icon: '${ARG_TARGET}' is not a target")
  endif()
  if(NOT ARG_NAME)
    set(ARG_NAME "${ARG_TARGET}")
  endif()

  # Pick this platform's icon (platform-specific first, then the ICON fallback).
  if(WIN32)
    set(_icon "${ARG_WINDOWS}")
  elseif(APPLE)
    set(_icon "${ARG_MACOS}")
  else()
    set(_icon "${ARG_LINUX}")
  endif()
  if(NOT _icon)
    set(_icon "${ARG_ICON}")
  endif()
  if(NOT _icon)
    # Nothing to do for this platform; not an error (icon may be optional).
    return()
  endif()

  get_filename_component(_icon_abs "${_icon}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  if(NOT EXISTS "${_icon_abs}")
    message(FATAL_ERROR "deskgui_target_icon: icon file not found: ${_icon_abs}")
  endif()

  get_filename_component(_icon_ext "${_icon_abs}" LAST_EXT)
  string(TOLOWER "${_icon_ext}" _icon_ext)

  set(_gen_dir "${CMAKE_BINARY_DIR}/deskgui_app_icon/${ARG_NAME}")
  file(MAKE_DIRECTORY "${_gen_dir}")

  if(WIN32)
    if(NOT _icon_ext STREQUAL ".ico")
      message(FATAL_ERROR "deskgui_target_icon: Windows needs a .ico file, got: ${_icon_abs}")
    endif()

    # rc wants forward slashes (or escaped backslashes) inside the quoted path.
    string(REPLACE "\\" "/" DESKGUI_ICON_PATH "${_icon_abs}")
    set(DESKGUI_ICON_ID "${_DESKGUI_APP_ICON_ID}")

    set(_rc_file "${_gen_dir}/app_icon.rc")
    configure_file("${_DESKGUI_APP_ICON_DIR}/app_icon.rc.in" "${_rc_file}" @ONLY)
    target_sources(${ARG_TARGET} PRIVATE "${_rc_file}")

  else()
    # macOS / Linux: embed the raw icon bytes and register them with deskgui.
    file(READ "${_icon_abs}" _hex HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," DESKGUI_ICON_BYTES "${_hex}")

    set(_src_file "${_gen_dir}/deskgui_app_icon.cpp")
    configure_file("${_DESKGUI_APP_ICON_DIR}/app_icon.cpp.in" "${_src_file}" @ONLY)
    target_sources(${ARG_TARGET} PRIVATE "${_src_file}")

    if(APPLE AND _icon_ext STREQUAL ".icns")
      # Also use it as the bundle icon for app-bundle targets.
      get_filename_component(_icns_name "${_icon_abs}" NAME)
      set_target_properties(${ARG_TARGET} PROPERTIES MACOSX_BUNDLE_ICON_FILE "${_icns_name}")
      target_sources(${ARG_TARGET} PRIVATE "${_icon_abs}")
      set_source_files_properties("${_icon_abs}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endif()
  endif()
endfunction()
