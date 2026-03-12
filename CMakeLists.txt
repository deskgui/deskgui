# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

cmake_minimum_required(VERSION 3.14...3.22)

# ---- Project ----

project(
  deskgui
  VERSION 0.1.0
  LANGUAGES CXX
)

if(APPLE)
  enable_language(OBJC)
endif()

# ---- Properties ----

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
option(BUILD_EXAMPLES "Enable building project examples" ON)
option(BUILD_TESTS_AND_BENCHMARKS "Build tests and benchmarks" ON)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add source files ----

# Headers
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# Sources
set(sources
    "${CMAKE_CURRENT_SOURCE_DIR}/source/app.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/webview.cpp"
    )
    
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)
set(PLATFORM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/platform/${SYSTEM_NAME}")
file(GLOB_RECURSE platform_sources CONFIGURE_DEPENDS "${PLATFORM_SOURCE_DIR}/*.cpp" "${PLATFORM_SOURCE_DIR}/*.h" "${PLATFORM_SOURCE_DIR}/*.mm")
source_group("Platform Files" FILES ${platform_sources})
list(APPEND sources ${platform_sources})

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME} ${headers} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 LINKER_LANGUAGE CXX)

if(APPLE)
  set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_OPTIONS "-fobjc-arc")
endif()

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(webview2_VERSION "1.0.2592.51" CACHE STRING "The WebView2 version to use" FORCE)
include(platform_webview)
include(rapidjson)

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)
target_include_directories(
  ${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/source ${RapidJSON_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE PlatformWebview)

if(BUILD_EXAMPLES)
  add_subdirectory(examples examples)
endif()

if(BUILD_TESTS_AND_BENCHMARKS)
  include(catch2)
  add_subdirectory(test test)
  add_subdirectory(benchmark benchmark)
endif()

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)
