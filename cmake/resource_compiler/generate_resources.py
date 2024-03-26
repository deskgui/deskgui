import argparse
import os

from resource_content import generate_resource_cpp_file
from resource_library import generate_library_cpp

def main():
    parser = argparse.ArgumentParser(
        description="Generate C++ files for resource compiler"
    )
    parser.add_argument("-o", "--output_dir", required=True, help="Output directory for generated files")
    parser.add_argument("-p", "--pack_name", required=True, help="Name for the packed resource library")
    parser.add_argument(
        "-r", "--resource_compiler_cpp", required=True, help="Path of the packed resource library cpp"
    )
    parser.add_argument(
        "-f", "--resource_files", nargs="+", required=True, help="List of resource files to be packed"
    )

    args = parser.parse_args()

    # Check if pack_name contains blank spaces
    if ' ' in args.pack_name:
        print("Error: pack_name should not contain blank spaces.")
        return

    mount_methods = [
        generate_resource_cpp_file(args.output_dir, args.pack_name, file)
        for file in args.resource_files
    ]
    generate_library_cpp(args.pack_name, args.resource_compiler_cpp, mount_methods)


if __name__ == "__main__":
    main()
