# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

# Include the FetchContent module to fetch external libraries
include(FetchContent)

set(RAPIDJSON_URL "https://github.com/Tencent/rapidjson/archive/master.tar.gz")

set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_CXX11 OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_CXX17 ON CACHE BOOL "" FORCE)

# Declare the JSON library using FetchContent_Declare
FetchContent_Declare(RapidJSON
    URL ${RAPIDJSON_URL}
)

# Make the JSON library available for the project
FetchContent_MakeAvailable(RapidJSON)
