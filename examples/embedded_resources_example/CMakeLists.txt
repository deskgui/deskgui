# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

cmake_minimum_required(VERSION 3.14...3.22)

set(source embedded_resources_example.cpp)
get_filename_component(executable_name ${source} NAME_WE)
add_executable(${executable_name} MACOSX_BUNDLE ${source})
set_target_properties(${executable_name} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${executable_name} PRIVATE deskgui)

# embed resources
include(resource_compiler/resource_compiler)

# Specify the assets
file(GLOB_RECURSE ASSETS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/*)

# Set macOS bundle properties
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.${executable_name}")
set(MACOSX_BUNDLE_BUNDLE_NAME ${executable_name})

resource_compiler(
    TARGET_NAME  ${executable_name}
    RESOURCE_FILES "${ASSETS_FILES}"
    PACK_NAME "web_resources"
    ROOT_FOLDER "/assets"
)
