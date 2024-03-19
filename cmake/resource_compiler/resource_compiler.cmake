# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

set(current_dir ${CMAKE_CURRENT_LIST_DIR})
set(resource_compiler_build ${CMAKE_BINARY_DIR}/resource_compiler)

# Define a CMake function to pack files into a resource library
macro(resource_compiler)
    set(oneValueArgs TARGET_NAME PACK_NAME ROOT_FOLDER OBFUSCATE)
    set(multiValueArgs RESOURCE_FILES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Set build directory
    set(build_dir ${CMAKE_BINARY_DIR}/${ARG_PACK_NAME})

    # Create the directory if it doesn't exist
    if(EXISTS ${build_dir})
        file(REMOVE_RECURSE ${build_dir})
    endif()

    file(MAKE_DIRECTORY ${build_dir})

    # Set current directory and build directory
    set(current_resource_compiler_build ${resource_compiler_build}/${ARG_TARGET_NAME})

    if(NOT EXISTS ${current_resource_compiler_build}/)
        file(MAKE_DIRECTORY ${current_resource_compiler_build})
    endif()

    # Paths for resource compiler
    set(resource_compiler_cpp ${current_resource_compiler_build}/resource_compiler.cpp)

    # Calculate the relative path from CMake script location to resources
    set(RELATIVE_RESOURCES_FILES "")

    foreach(resource_file ${ARG_RESOURCE_FILES})
        get_filename_component(abs_path ${resource_file} ABSOLUTE)
        file(RELATIVE_PATH relative_path ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_ROOT_FOLDER} ${abs_path})
        list(APPEND RELATIVE_RESOURCES_FILES ${relative_path})
    endforeach()

    find_package(Python COMPONENTS Interpreter)

    # Call generate_resource.py script
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env ${Python_EXECUTABLE} ${current_dir}/generate_resources.py -o ${build_dir} -p ${ARG_PACK_NAME} -r ${resource_compiler_cpp} -f ${RELATIVE_RESOURCES_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_ROOT_FOLDER}
    )

    # Create a static library with the packed resources (.cpp)
    file(GLOB_RECURSE resources CONFIGURE_DEPENDS ${build_dir}/*.cpp)
    add_library(${ARG_PACK_NAME} STATIC ${resources} ${resource_compiler_cpp})
    target_include_directories(${ARG_PACK_NAME} PRIVATE ${current_resource_compiler_build})
    target_include_directories(${ARG_PACK_NAME} PRIVATE ${current_dir}/../../include) # fix this include

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_target_properties(${ARG_PACK_NAME} PROPERTIES FOLDER "resources")

    # Link the resource library to the target
    target_link_libraries(${ARG_TARGET_NAME} PRIVATE ${ARG_PACK_NAME})
    target_compile_definitions(${ARG_PACK_NAME} PRIVATE COMPILED_RESOURCES_ENABLED)
    target_compile_definitions(${ARG_TARGET_NAME} PRIVATE COMPILED_RESOURCES_ENABLED)
    set_target_properties(${ARG_PACK_NAME} PROPERTIES CXX_STANDARD 17)
    set_target_properties(${ARG_TARGET_NAME} PROPERTIES CXX_STANDARD 17)
endmacro()
