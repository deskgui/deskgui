# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

# Include the FetchContent module to fetch external libraries
include(FetchContent)

add_library(PlatformWebview INTERFACE)

# Linux configuration
if (UNIX AND NOT APPLE)
  # Find and configure the required GTK and WebKit libraries using pkg-config
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(gtk REQUIRED gtk+-3.0 IMPORTED_TARGET)
  pkg_check_modules(webkit REQUIRED webkit2gtk-4.0 IMPORTED_TARGET)

  # Suppress deprecation warnings for the linked library
  if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(PkgConfig::webkit INTERFACE -Wno-deprecated-declarations)
  endif ()

  # Link the GTK and WebKit libraries to the target
  target_link_libraries(PlatformWebview INTERFACE PkgConfig::gtk PkgConfig::webkit)
endif()

# macOS configuration
if (APPLE)
  # Find and configure the required frameworks
  find_package(PkgConfig REQUIRED)
  
  # Link the necessary frameworks to the target
  target_link_libraries(PlatformWebview INTERFACE "-framework Webkit -framework Carbon -framework Cocoa")
  
  # Enable Objective-C and Objective-C++ languages
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

# Windows configuration
if (WIN32)
  # Include the nuget CMake file for downloading NuGet packages
  include(nuget)
  
  # Download and install the WebView2 NuGet package
  download_nuget_package(WebView2 "Microsoft.Web.WebView2" ${webview2_VERSION})
  
  # Add Unicode definitions for Windows compilation
  target_compile_definitions(PlatformWebview INTERFACE UNICODE=1 _UNICODE=1)
  
  # Include the WebView2 header files
  target_include_directories(PlatformWebview INTERFACE ${WebView2_PATH}/build/native/include)
  
  # Link the WebView2 library based on the architecture
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      target_link_libraries(PlatformWebview INTERFACE ${WebView2_PATH}/build/native/x64/WebView2LoaderStatic.lib)
  else()
      target_link_libraries(PlatformWebview INTERFACE ${WebView2_PATH}/build/native/x86/WebView2LoaderStatic.lib)
  endif()
  
  # Disable building packaging and tests for the Windows Implementation Library (WIL)
  option(WIL_BUILD_PACKAGING "" OFF)
  option(WIL_BUILD_TESTS  "" OFF)
  
  # Fetch and make the WIL library available
  FetchContent_Declare(wil GIT_REPOSITORY "https://github.com/microsoft/wil")
  FetchContent_MakeAvailable(wil)
  
  # Link the WIL library and comctl32 library to the target
  target_link_libraries(PlatformWebview INTERFACE WIL::WIL comctl32.lib Shlwapi)
endif()
