#!/usr/bin/env python3
"""
Ruby bytecode compiler for PRC-152 firmware

This script compiles .rb files in the ruby/ directory to C header files
containing bytecode arrays that can be included in the firmware.

Requirements:
- mruby must be installed (provides mrbc compiler)
- Install via: brew install mruby (macOS) or apt install mruby (Linux)

Usage:
- Run manually: python compile_ruby.py
- Or automatically via PlatformIO pre-build script
"""

import os
import subprocess
import sys
from pathlib import Path


def find_mrbc():
    """Find the mrbc compiler"""
    # Try common locations
    paths = [
        "mrbc",  # In PATH
        "/usr/bin/mrbc",
        "/usr/local/bin/mrbc",
        "/opt/homebrew/bin/mrbc",  # macOS ARM
        os.path.expanduser("~/.mruby/bin/mrbc"),
    ]

    for path in paths:
        try:
            result = subprocess.run([path, "--version"],
                                    capture_output=True, text=True)
            if result.returncode == 0:
                print(f"Found mrbc: {path}")
                return path
        except FileNotFoundError:
            continue

    return None


def compile_ruby_to_bytecode(mrbc_path, rb_file, mrb_file):
    """Compile a .rb file to .mrb bytecode"""
    cmd = [mrbc_path, "-o", str(mrb_file), str(rb_file)]
    print(f"Compiling: {rb_file.name}")
    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error compiling {rb_file}:")
        print(result.stderr)
        return False
    return True


def bytecode_to_header(mrb_file, header_file, var_name):
    """Convert .mrb bytecode to C header file"""
    with open(mrb_file, "rb") as f:
        bytecode = f.read()

    with open(header_file, "w") as f:
        f.write(f"// Auto-generated from {mrb_file.name}\n")
        f.write(f"// DO NOT EDIT - regenerate with compile_ruby.py\n\n")
        f.write("#ifndef __RUBY_BYTECODE_{}_H__\n".format(var_name.upper()))
        f.write("#define __RUBY_BYTECODE_{}_H__\n\n".format(var_name.upper()))
        f.write("#include <stdint.h>\n\n")
        f.write(f"const uint8_t {var_name}_bytecode[] = {{\n")

        # Write bytes in rows of 16
        for i, byte in enumerate(bytecode):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{byte:02x},")
            if i % 16 == 15:
                f.write("\n")
            else:
                f.write(" ")

        if len(bytecode) % 16 != 0:
            f.write("\n")

        f.write("};\n\n")
        f.write(f"const size_t {var_name}_bytecode_len = sizeof({var_name}_bytecode);\n\n")
        f.write("#endif\n")

    print(f"Generated: {header_file.name} ({len(bytecode)} bytes)")
    return True


def main():
    script_dir = Path(__file__).parent
    ruby_dir = script_dir / "ruby"
    include_dir = script_dir / "include"
    build_dir = script_dir / ".ruby_build"

    # Create build directory
    build_dir.mkdir(exist_ok=True)

    # Find mrbc compiler
    mrbc = find_mrbc()
    if not mrbc:
        print("=" * 60)
        print("WARNING: mrbc compiler not found!")
        print("Ruby files will NOT be compiled.")
        print("")
        print("To install mruby:")
        print("  macOS:  brew install mruby")
        print("  Ubuntu: sudo apt install mruby")
        print("  Manual: https://github.com/mruby/mruby")
        print("=" * 60)

        # Create stub header so build doesn't fail
        stub_header = include_dir / "ruby_bytecode_demo.h"
        with open(stub_header, "w") as f:
            f.write("// Stub - mrbc not available\n")
            f.write("#ifndef __RUBY_BYTECODE_DEMO_H__\n")
            f.write("#define __RUBY_BYTECODE_DEMO_H__\n")
            f.write("#include <stdint.h>\n")
            f.write("const uint8_t demo_bytecode[] = {};\n")
            f.write("const size_t demo_bytecode_len = 0;\n")
            f.write("#endif\n")
        return 1

    # Compile each .rb file
    rb_files = list(ruby_dir.glob("*.rb"))
    if not rb_files:
        print("No .rb files found in ruby/ directory")
        return 0

    success = True
    for rb_file in rb_files:
        var_name = rb_file.stem  # filename without extension
        mrb_file = build_dir / f"{var_name}.mrb"
        header_file = include_dir / f"ruby_bytecode_{var_name}.h"

        if not compile_ruby_to_bytecode(mrbc, rb_file, mrb_file):
            success = False
            continue

        if not bytecode_to_header(mrb_file, header_file, var_name):
            success = False
            continue

    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
