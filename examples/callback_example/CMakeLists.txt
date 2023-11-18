# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

cmake_minimum_required(VERSION 3.14...3.22)

set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(source callback_example.cpp)
get_filename_component(executable_name ${source} NAME_WE)
add_executable(${executable_name} MACOSX_BUNDLE ${source})
set_target_properties(${executable_name} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${executable_name} PRIVATE deskgui)
target_compile_definitions(${executable_name} PUBLIC WEBVIEW_CONTENT_DIRECTORY="${ASSETS_DIR}") # Fix me
