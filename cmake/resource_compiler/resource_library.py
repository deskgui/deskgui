# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

import os

def find_extern_resources_code(content, pack_name):
    '''
    Find the start and end positions of the extern resources code block for a specific package.

    Parameters:
        content (str): The content to search within.
        pack_name (str): The name of the package.

    Returns:
        tuple: A tuple containing the start and end positions of the extern resources code block.
    '''
    return content.find(f"// Start extern {pack_name} resources\n"), content.find(
        f"// End extern {pack_name} resources\n"
    )

def find_add_resources_code(content, pack_name):
    '''
    Finds the start and end indices of the previously defined resources for a given pack_name.

    Args:
        content (str): The content to search in.
        pack_name (str): The name of the resource pack.

    Returns:
        tuple: A tuple containing the start and end indices of the previously defined resources.
    '''
    return content.find(f"    // Start add {pack_name} resources\n"), content.find(
        f"    // End add {pack_name} resources\n"
    )

def generate_extern_resources_code(pack_name, mount_methods):
    '''
    Generates the code to declare external resources for a given resource pack.

    Args:
        pack_name (str): The name of the resource pack.
        mount_methods (list): A list of mount methods for the resource pack.

    Returns:
        str: The generated code to declare external resources.
    '''
    content = f"// Start extern {pack_name} resources\n"
    content += (
        "\n".join([f"extern Resource {method}();" for method in mount_methods]) + "\n"
    )
    content += f"// End extern {pack_name} resources\n"

    return content
def mount_compiled_resources(pack_name, mount_methods):
    '''
    This method adds the code to mount a package's resources.

    Args:
        pack_name (str): The name of the package.
        mount_methods (list): A list of mount methods for the package's resources.

    Returns:
        str: The generated code to mount the package's resources.
    '''
    content = f"    // Start add {pack_name} resources\n"
    content += f'    if(name == "{pack_name}") {{\n'
    for method in mount_methods:
        content += f"        resources.emplace_back({method}());\n"
    content += f"    }}\n"
    content += f"    // End add {pack_name} resources\n"
    return content


def create_library_cpp(pack_name, mount_methods):
    '''
    Creates a new cpp file to compile resource content.

    Args:
        pack_name (str): The name of the resource pack.
        mount_methods (list): A list of mount methods.

    Returns:
        str: The content of the cpp file.
    '''
    cpp_content = f'#include "deskgui/resource_compiler.h"\n\n'
    cpp_content += f"using namespace deskgui;\n\n"

    cpp_content += generate_extern_resources_code(pack_name, mount_methods)

    cpp_content += (
        f"Resources deskgui::getCompiledResources(const std::string& name) {{\n"
    )
    cpp_content += f"    Resources resources;\n"
    cpp_content += mount_compiled_resources(pack_name, mount_methods)
    cpp_content += f"    return resources;\n"
    cpp_content += f"}}\n"

    return cpp_content

def extend_library_cpp(existing_content, pack_name, mount_methods):
    '''
    Extends the library cpp file in case there is another package already defined.
    This is needed since we can mount multiple packages in the same library.

    Args:
        existing_content (str): The existing content of the library cpp file.
        pack_name (str): The name of the package to be extended.
        mount_methods (list): A list of mount methods for the package.

    Returns:
        str: The updated content of the library cpp file after extending the package.
    '''
    extern_start, extern_end = find_extern_resources_code(existing_content, pack_name)

    if extern_start != -1 and extern_end != -1:
        new_content = existing_content[:extern_start]
        new_content += generate_extern_resources_code(pack_name, mount_methods)
        new_content += existing_content[
            extern_end + len(f"// End extern {pack_name} resources\n") :
        ]

        resources_start, resources_end = find_add_resources_code(new_content, pack_name)
        if resources_start != -1 and resources_end != -1:
            content = new_content
            new_content = content[:resources_start]
            new_content += mount_compiled_resources(pack_name, mount_methods)
            new_content += content[
                resources_end + len(f"    // End add {pack_name} resources\n") :
            ]
    else:
        start_marker = f"using namespace deskgui;\n\n"
        end_marker = "    return resources;"
        start_position = existing_content.find(start_marker)
        end_position = existing_content.find(end_marker)

        if start_position != -1 and end_position != -1:
            new_content = existing_content[: start_position + len(start_marker)]
            new_content += generate_extern_resources_code(pack_name, mount_methods)
            new_content += existing_content[
                start_position + len(start_marker) : end_position
            ]
            new_content += mount_compiled_resources(pack_name, mount_methods)
            new_content += existing_content[end_position:]

    # If no modification is made, return the original content
    return new_content

def generate_library_cpp(pack_name, resource_compiler_cpp, mount_methods):
    '''
    Generate the API C++ code for mounting different resource packages, allowing them to be accessed from a C++ executable. This code serves as the access point to the resources.

    Args:
        pack_name (str): The name of the resource package.
        resource_compiler_cpp (str): The path to the resource compiler C++ file.
        mount_methods (list): A list of mount methods.

    Returns:
        str: The generated API C++ code.

    Raises:
        FileNotFoundError: If the resource compiler C++ file does not exist.
    '''
    if os.path.exists(resource_compiler_cpp):
        with open(resource_compiler_cpp, "r") as cpp_file:
            existing_content = cpp_file.read()

        cpp_content = extend_library_cpp(existing_content, pack_name, mount_methods)
    else:
        cpp_content = create_library_cpp(pack_name, mount_methods)

    with open(resource_compiler_cpp, "w") as cpp_file:
        cpp_file.write(cpp_content)
