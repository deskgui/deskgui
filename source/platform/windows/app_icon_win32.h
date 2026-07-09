/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

// Resource id of the application icon embedded into the executable by the
// deskgui_target_icon() CMake helper (see cmake/app_icon/app_icon.rc.in).
//
// This value is shared between the generated .rc file and the window class
// registration code, so the window automatically loads the embedded icon for
// its title bar and task bar entry. Keep both in sync.
#define IDI_DESKGUI_APP_ICON 101
