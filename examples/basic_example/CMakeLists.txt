# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

cmake_minimum_required(VERSION 3.14...3.22)

set(source basic_example.cpp)
get_filename_component(executable_name ${source} NAME_WE)
add_executable(${executable_name} MACOSX_BUNDLE ${source})
set_target_properties(${executable_name} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${executable_name} PRIVATE deskgui)

# Set macOS bundle properties
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.${executable_name}")
set(MACOSX_BUNDLE_BUNDLE_NAME ${executable_name})