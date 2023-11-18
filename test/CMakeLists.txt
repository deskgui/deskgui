# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

cmake_minimum_required(VERSION 3.14...3.22)

project(deskgui_tests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_COMPILER_WARNINGS "Enable compiler warnings" OFF)
option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- Create binary ----
file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} Catch2::Catch2WithMain deskgui)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# ---- compiler warnings ----
if(ENABLE_COMPILER_WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(deskgui PRIVATE -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(deskgui PRIVATE /W4 /WX)
  endif()
endif()

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(deskgui PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(deskgui PUBLIC -fprofile-arcs -ftest-coverage)
endif()

catch_discover_tests(${PROJECT_NAME})
