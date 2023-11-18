# deskgui - A powerful and flexible C++ library to create web-based desktop applications.
# Copyright (c) 2023 deskgui
# MIT License

import argparse
import os
import zlib

MIME_TYPE_MAP = {
    ".txt": "text/plain",
    ".html": "text/html",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".png": "image/png",
    ".mp3": "audio/mpeg",
    ".ogg": "audio/ogg",
    ".wav": "audio/wav",
    ".mp4": "video/mp4",
    ".bin": "application/octet-stream",
    ".aac": "audio/aac",
    ".abw": "application/x-abiword",
    ".arc": "application/octet-stream",
    ".avi": "video/x-msvideo",
    ".azw": "application/vnd.amazon.ebook",
    ".bz": "application/x-bzip",
    ".bz2": "application/x-bzip2",
    ".csh": "application/x-csh",
    ".css": "text/css",
    ".csv": "text/csv",
    ".doc": "application/msword",
    ".epub": "application/epub+zip",
    ".gif": "image/gif",
    ".ico": "image/x-icon",
    ".ics": "text/calendar",
    ".jar": "application/java-archive",
    ".js": "application/javascript",
    ".json": "application/json",
    ".mid": "audio/midi",
    ".midi": "audio/midi",
    ".mpeg": "video/mpeg",
    ".mpkg": "application/vnd.apple.installer+xml",
    ".odp": "application/vnd.oasis.opendocument.presentation",
    ".ods": "application/vnd.oasis.opendocument.spreadsheet",
    ".odt": "application/vnd.oasis.opendocument.text",
    ".oga": "audio/ogg",
    ".ogv": "video/ogg",
    ".ogx": "application/ogg",
    ".pdf": "application/pdf",
    ".ppt": "application/vnd.ms-powerpoint",
    ".rar": "application/x-rar-compressed",
    ".rtf": "application/rtf",
    ".sh": "application/x-sh",
    ".svg": "image/svg+xml",
    ".swf": "application/x-shockwave-flash",
    ".tar": "application/x-tar",
    ".tif": "image/tiff",
    ".tiff": "image/tiff",
    ".ttf": "font/ttf",
    ".vsd": "application/vnd.visio",
    ".wav": "audio/x-wav",
    ".weba": "audio/webm",
    ".webm": "video/webm",
    ".webp": "image/webp",
    ".woff": "font/woff",
    ".woff2": "font/woff2",
    ".xhtml": "application/xhtml+xml",
    ".xls": "application/vnd.ms-excel",
    ".xml": "application/xml",
    ".xul": "application/vnd.mozilla.xul+xml",
    ".zip": "application/zip",
    ".3gp": "video/3gpp",
    ".3g2": "video/3gpp2",
    ".7z": "application/x-7z-compressed"
}

# Resource creation
def convert_to_binary(file_path):
    with open(file_path, "rb") as f:
        return f.read()
def generate_resource_cpp_content(resource_file_path, compress):
    resource_file_name, file_extension = os.path.splitext(os.path.basename(resource_file_path))
    binary_data = convert_to_binary(resource_file_path)
    if compress:
        binary_data = zlib.compress(binary_data)

    resource_data_name = resource_file_name.replace(".", "_").replace("-", "_")

    cpp_content = f'#include "deskgui/resource_compiler.h"\n\n'
    cpp_content += f'using namespace deskgui;\n\n'
    cpp_content += f'Resource mount_{resource_data_name}() {{\n'
    cpp_content += f'    const std::vector<std::uint8_t> {resource_data_name}_resource = {{\n'
    
    hex_lines = [f'{byte}' for byte in binary_data]
    cpp_content += '        ' + ', '.join(hex_lines) + '\n'
    
    cpp_content += f'    }};\n\n'

    if compress:
        resource_data = f'decompress({resource_data_name}_resource)'
    else:
        resource_data = f'{resource_data_name}_resource'
    
    cpp_content += f'    return {{"{resource_file_path}", {resource_data}, "{MIME_TYPE_MAP.get(file_extension, "application/octet-stream")}"}};\n'
    
    cpp_content += f'}}\n'
    return cpp_content

def generate_resource_cpp_file(output_dir, pack_name, resource_file, compress):
    cpp_content = generate_resource_cpp_content(resource_file, compress)

    resource_file_name, _ = os.path.splitext(os.path.basename(resource_file))
    cpp_file_name = f'{pack_name}_{resource_file_name}.cpp'
    cpp_file_path = os.path.join(output_dir, cpp_file_name)
    
    with open(cpp_file_path, 'w') as cpp_file:
        cpp_file.write(cpp_content)
    return f'mount_{resource_file_name.replace(".", "_").replace("-", "_")}'

# Common API creation
def generate_compression_code():
    existing_content = f'\n#include "zlib.h"\n'
    existing_content += '''
const std::vector<std::uint8_t> deskgui::decompress(const std::vector<std::uint8_t>& data) {
    uLongf uncompressed_size = static_cast<uLongf>(data.size()) * 2; // Initial guess for uncompressed size
    std::vector<std::uint8_t> binary_data(uncompressed_size);

    int result = Z_BUF_ERROR;
    while (result == Z_BUF_ERROR) {
        result = uncompress(&binary_data[0], &uncompressed_size, &data[0], static_cast<uLongf>(data.size()));
        if (result == Z_BUF_ERROR) {
            uncompressed_size *= 2; // Double the buffer size
            binary_data.resize(uncompressed_size);
        }
    }

    if (result != Z_OK) {
        // Handle decompression error here
    }

    // Resize the vector to match the actual uncompressed size
    binary_data.resize(uncompressed_size);
    return binary_data;
}
    '''
    return existing_content

def find_compression_code(content):
    zlib_include = f'#include "zlib.h"'
    return content.find(zlib_include) != -1

def generate_extern_resources_code(pack_name, mount_methods):
    content = f'// Start extern {pack_name} resources\n'
    content += '\n'.join([f'extern Resource {method}();' for method in mount_methods]) + '\n'
    content += f'// End extern {pack_name} resources\n'
    return content

def find_extern_resources_code(content, pack_name):
    return content.find(f"// Start extern {pack_name} resources\n"), content.find(f'// End extern {pack_name} resources\n')

def generate_add_resources_code(pack_name, mount_methods):
    content = f'    // Start add {pack_name} resources\n'
    content += f'    if(name == "{pack_name}") {{\n'
    for method in mount_methods:
        content += f'        resources.emplace_back({method}());\n'
    content += f'    }}\n'
    content += f'    // End add {pack_name} resources\n' 
    return content

def find_add_resources_code(content, pack_name):
    return content.find(f'    // Start add {pack_name} resources\n'), content.find(f'    // End add {pack_name} resources\n' )

def generate_api_cpp_content(pack_name, mount_methods, compress):
    cpp_content = f'#include "deskgui/resource_compiler.h"\n\n'
    cpp_content += f'using namespace deskgui;\n\n'

    cpp_content += generate_extern_resources_code(pack_name, mount_methods)

    cpp_content += f'Resources deskgui::getCompiledResources(const std::string& name) {{\n'
    cpp_content += f'    Resources resources;\n'
    cpp_content += generate_add_resources_code(pack_name, mount_methods)
    cpp_content += f'    return resources;\n'
    cpp_content += f'}}\n'

    if compress:
        cpp_content += generate_compression_code()

    return cpp_content


def extend_api_cpp_content(existing_content, pack_name, mount_methods, compress):
    extern_start, extern_end = find_extern_resources_code(existing_content, pack_name)
    
    if extern_start != -1 and extern_end != -1:
        new_content = existing_content[:extern_start]
        new_content += generate_extern_resources_code(pack_name, mount_methods)
        new_content += existing_content[extern_end + len(f'// End extern {pack_name} resources\n'):]
        
        resources_start, resources_end = find_add_resources_code(new_content, pack_name)
        if  resources_start != -1 and resources_end != -1:
            content = new_content
            new_content = content[:resources_start]
            new_content += generate_add_resources_code(pack_name, mount_methods)
            new_content += content[resources_end + len(f'    // End add {pack_name} resources\n' ):]
    else:
        start_marker = f"using namespace deskgui;\n\n"
        end_marker = "    return resources;"
        start_position = existing_content.find(start_marker)
        end_position = existing_content.find(end_marker)

        if start_position != -1 and end_position != -1:
            new_content = existing_content[:start_position + len(start_marker)]
            new_content += generate_extern_resources_code(pack_name, mount_methods)
            new_content += existing_content[start_position + len(start_marker):end_position]
            new_content += generate_add_resources_code(pack_name, mount_methods)
            new_content += existing_content[end_position:]

    if compress and not find_compression_code(existing_content):
        new_content += generate_compression_code()

    # If no modification is made, return the original content
    return new_content

def generate_api_cpp(pack_name, resource_compiler_cpp, mount_methods, compress):
    if os.path.exists(resource_compiler_cpp):
        with open(resource_compiler_cpp, 'r') as cpp_file:
            existing_content = cpp_file.read()

        cpp_content = extend_api_cpp_content(existing_content, pack_name, mount_methods, compress)
    else:
        cpp_content = generate_api_cpp_content(pack_name, mount_methods, compress)

    with open(resource_compiler_cpp, 'w') as cpp_file:
        cpp_file.write(cpp_content)

def main():
    parser = argparse.ArgumentParser(description="Generate C++ files for packed resource")
    parser.add_argument("output_dir", help="Output directory for generated files")
    parser.add_argument("pack_name", help="Name for the packed resource library")
    parser.add_argument("resource_compiler_cpp", help="Name for the packed resource library")
    parser.add_argument("resource_files", nargs="+", help="List of resource files to be packed")
    parser.add_argument("compress", default=False, help="List of resource files to be packed")
    args = parser.parse_args()

    compress = True if args.compress == "TRUE" else False
    
    mount_methods = [generate_resource_cpp_file(args.output_dir, args.pack_name, file, compress) for file in args.resource_files]
    generate_api_cpp(args.pack_name, args.resource_compiler_cpp, mount_methods, compress)

if __name__ == "__main__":
    main()
