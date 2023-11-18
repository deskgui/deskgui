# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

# Function to download a NuGet package
# Parameters:
# - OUTPUT_PACKAGE_NAME: The name of the output variable to store the package path
# - PACKAGE_NAME: The name of the NuGet package to download
# - PACKAGE_VERSION: The version of the NuGet package to download
function(download_nuget_package OUTPUT_PACKAGE_NAME PACKAGE_NAME PACKAGE_VERSION)
    # Find the NuGet executable
    find_program(NUGET_EXECUTABLE nuget)

    # Print a status message indicating the package being downloaded
    message(STATUS "[Package Download] Downloading package: ${PACKAGE_NAME} [${PACKAGE_VERSION}]")

    # Check if NuGet executable is not found
    if (NUGET_EXECUTABLE STREQUAL "NUGET_EXECUTABLE-NOTFOUND")
        # Check if NuGet-CLI file does not exist, and download it if needed
        if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/nuget/nuget-cli.exe")
            message(STATUS "[Package Download] NuGet-CLI not found, downloading...")
            file(DOWNLOAD "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe" "${CMAKE_CURRENT_BINARY_DIR}/nuget/nuget-cli.exe")
        endif()
        # Set the path to NuGet-CLI executable
        set(NUGET_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/nuget/nuget-cli.exe")
    endif()

    # Execute a process to get NuGet version information
    execute_process(COMMAND ${NUGET_EXECUTABLE} help OUTPUT_VARIABLE NUGET_INFO)
    # Extract the NuGet version using regex
    string(REGEX MATCH "NuGet Version: ([0-9.]+)" _ "${NUGET_INFO}")
    # Print a status message indicating the NuGet version found
    message(STATUS "[Package Download] NuGet found at: ${NUGET_EXECUTABLE} [${CMAKE_MATCH_1}]")

    # Execute a process to get NuGet sources
    execute_process(COMMAND ${NUGET_EXECUTABLE} sources OUTPUT_VARIABLE NUGET_SOURCES)
    
    # Check if the official NuGet sources are missing and add them if needed
    if (NOT "${NUGET_SOURCES}" MATCHES "https:\/\/api\.nuget\.org\/v3\/index\.json")
        message(WARNING "[Package Download] Official NuGet sources missing!")
        execute_process(COMMAND ${NUGET_EXECUTABLE} sources Add -Name "nuget.org" -Source "https://api.nuget.org/v3/index.json")
    endif()

    # Execute a process to install the specified NuGet package
    execute_process(COMMAND ${NUGET_EXECUTABLE} install ${PACKAGE_NAME} -Version ${PACKAGE_VERSION} -ExcludeVersion -OutputDirectory "${CMAKE_CURRENT_BINARY_DIR}/packages" OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY)
    
    # Set the output package path variable
    set(${OUTPUT_PACKAGE_NAME}_PATH "${CMAKE_CURRENT_BINARY_DIR}/packages/${PACKAGE_NAME}" PARENT_SCOPE)
endfunction()
