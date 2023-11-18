# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

include(FetchContent)

# Set the version and URL of the Catch2 package
set(CATCH2_VERSION 3.3.2)
set(CATCH2_URL "https://github.com/catchorg/Catch2/archive/v${CATCH2_VERSION}.tar.gz")

# Fetch the Catch2 package
FetchContent_Declare(
  catch2
  URL ${CATCH2_URL}
)

# Add the Catch2 library
FetchContent_MakeAvailable(catch2)

enable_testing()
include(CTest)
include(Catch)

# ---- Options ----

option(ENABLE_COMPILER_WARNINGS "Enable compiler warnings" OFF)
option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- hide targets in IDE ----
set_target_properties(Continuous Experimental Nightly NightlyMemoryCheck Catch2 Catch2WithMain PROPERTIES FOLDER test_dependencies)
