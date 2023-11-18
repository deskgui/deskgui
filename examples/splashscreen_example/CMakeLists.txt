# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

cmake_minimum_required(VERSION 3.14...3.22)

set(source splashscreen_example.cpp)
get_filename_component(executable_name ${source} NAME_WE)
add_executable(${executable_name} MACOSX_BUNDLE ${source})
set_target_properties(${executable_name} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${executable_name} PRIVATE deskgui)

# embed resources
include(resource_compiler/resource_compiler)

resource_compiler(
    TARGET_NAME  ${executable_name}
    RESOURCE_FILES "assets/loading.html"
    PACK_NAME "splashscreen_resources"
    COMPRESS TRUE
    ROOT_FOLDER "/assets"
)

resource_compiler(
    TARGET_NAME  ${executable_name}
    RESOURCE_FILES "assets/helloworld.html"
    PACK_NAME "demo_resources"
    COMPRESS TRUE
    ROOT_FOLDER "/assets"
)
