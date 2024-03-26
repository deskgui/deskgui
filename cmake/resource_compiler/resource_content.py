# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

import os

from mime_types import MIME_TYPE_MAP

# Resource creation
def get_binary_data_from_file(file_path):
    with open(file_path, "rb") as f:
        return f.read()

def generate_binary_array(name, binary_data):
    '''
    Generate an array representation of the binary data as a C-style.

    Args:
        name (str): The name of the array.
        binary_data (bytes): The binary data to convert.

    Returns:
        str: A string representation of the binary data as a C-style array.
    '''
    cpp_content = f"constexpr std::array<unsigned char, {len(binary_data)}> {name} = {{\n"

    # Group bytes into chunks of 12
    byte_lines = [", ".join(f"0x{byte:02X}" for byte in binary_data[i:i+12]) for i in range(0, len(binary_data), 12)]
    cpp_content += "        " + ",\n        ".join(byte_lines) + "\n"
    
    cpp_content += f"    }};\n\n"
    return cpp_content

def generate_resource_cpp_content(pack_name: str, resource_file_path: str):
    '''
    Generate the C++ content for a resource file.

    Args:
        pack_name (str): The name of the resource pack.
        resource_file_path (str): The path to the resource file.

    Returns:
        str: The generated C++ content for the resource file.
    '''
    resource_file_name, file_extension = os.path.splitext(
        os.path.basename(resource_file_path)
    )
    binary_data = get_binary_data_from_file(resource_file_path)

    resource_data_name = resource_file_name.replace(".", "_").replace("-", "_")
    resource_data_array_name = f"{resource_data_name}_resource"

    cpp_content = f'#include "deskgui/resource_compiler.h"\n\n'
    cpp_content += f"using namespace deskgui;\n\n"
    
    cpp_content += generate_binary_array(resource_data_array_name, binary_data)

    cpp_content += f"Resource mount_{pack_name}_{resource_data_name}() {{\n"
    cpp_content += f'    std::vector<unsigned char> resource({resource_data_array_name}.begin(), {resource_data_array_name}.end());\n'
    cpp_content += f'    return {{"{resource_file_path}", resource, "{MIME_TYPE_MAP.get(file_extension, "application/octet-stream")}"}};\n'

    cpp_content += f"}}\n"
    return cpp_content

def generate_resource_cpp_file(output_dir, pack_name, resource_file):
    '''
    Generate a C++ file containing the resource content.

    Args:
        output_dir (str): The directory where the C++ file will be generated.
        pack_name (str): The name of the resource pack.
        resource_file (str): The path to the resource file.

    Returns:
        str: The C++ method name of the mounted resource.

    '''
    cpp_content = generate_resource_cpp_content(pack_name, resource_file)

    resource_file_name, _ = os.path.splitext(os.path.basename(resource_file))
    cpp_file_name = f"{pack_name}_{resource_file_name}.cpp"
    cpp_file_path = os.path.join(output_dir, cpp_file_name)

    with open(cpp_file_path, "w") as cpp_file:
        cpp_file.write(cpp_content)
    return f'mount_{pack_name}_{resource_file_name.replace(".", "_").replace("-", "_")}'
