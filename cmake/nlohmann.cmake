# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

# Include the FetchContent module to fetch external libraries
include(FetchContent)

# Declare the JSON library using FetchContent_Declare
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)

# Make the JSON library available for the project
FetchContent_MakeAvailable(json)

# Link the JSON library to the target
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Include the JSON library's header files
target_include_directories(${PROJECT_NAME} PRIVATE ${json_PATH}/build/native/include)
