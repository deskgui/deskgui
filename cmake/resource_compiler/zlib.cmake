# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

include(FetchContent)

function(add_zlib_to_target target)
    # Check if zlib target already exists
    if(NOT TARGET zlib)
        message("-- Fetching zlib for resources compression...")
        FetchContent_Declare(
            zlib
            GIT_REPOSITORY https://github.com/madler/zlib.git
            GIT_TAG master # Use the desired branch, tag, or commit hash
            QUIET  # Hide output messages from FetchContent
        )

        FetchContent_MakeAvailable(zlib)  # Fetch and make zlib available
    endif()

    # Link the zlibstatic library to the target
    target_link_libraries(${target} PRIVATE zlibstatic)
    
    # Include both source and build directories of zlib
    target_include_directories(${target}
        PUBLIC
        ${zlib_SOURCE_DIR}
        ${zlib_BINARY_DIR}  # Include the build directory
    )
endfunction()
