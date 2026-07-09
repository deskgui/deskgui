# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

cmake_minimum_required(VERSION 3.14...3.22)

set(source app_icon_example.cpp)
get_filename_component(executable_name ${source} NAME_WE)
add_executable(${executable_name} MACOSX_BUNDLE ${source})
set_target_properties(${executable_name} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${executable_name} PRIVATE deskgui)

# Embed the application icon into the executable / bundle.
# Each platform gets its native icon format.
deskgui_target_icon(
  TARGET  ${executable_name}
  WINDOWS ${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico
  MACOS   ${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.icns
  LINUX   ${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png
)

# Set macOS bundle properties
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.${executable_name}")
set(MACOSX_BUNDLE_BUNDLE_NAME ${executable_name})
