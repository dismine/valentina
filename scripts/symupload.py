import argparse
import os
import subprocess
import sys
import glob
import zipfile

database = "valentina"

def debug_extension():
    platform = sys.platform
    if platform == "darwin":
        debug_ext = ".dSYM"
    elif platform == "win32":
        debug_ext = ".pdb"
    elif platform == "linux":
        debug_ext = ".debug"
    else:
        print(f"Unsupported platform {platform}")
        return "uknown"

    return debug_ext

def check_binary(binary):
    # Check if binary file exists
    if not os.path.exists(binary):
        # If binary file doesn't exist, it may be part of a framework
        framework_binary = os.path.basename(binary.replace(".framework", ""))
        for root, dirs, files in os.walk(binary):
            for file in files:
                if file == framework_binary:
                    return os.path.join(root, file)
    return binary

def zip_sym(sym_file):
  zip_sym_file = sym_file + ".zip"
  with zipfile.ZipFile(zip_sym_file, 'w', zipfile.ZIP_DEFLATED) as zipf:
      zipf.write(sym_file, os.path.basename(sym_file))
  return zip_sym_file

def generate_sym_files(install_root):
    sym_files = []

    platform = sys.platform
    debug_ext = debug_extension()

    debug_files = glob.glob(os.path.join(install_root, "**", "*" + debug_ext), recursive=True)

    for debug_file in debug_files:
        print(f"Generating symbols for: {os.path.basename(debug_file)}")
        if platform == "win32":
            # For Windows, return the executable file
            sym_file = os.path.splitext(debug_file)[0] + ".exe"
        else:
            sym_file = os.path.splitext(debug_file)[0] + ".sym"
            if platform == "darwin":
                binary = check_binary(os.path.splitext(debug_file)[0])
                dump_syms_cmd = ["dump_syms", "-g", debug_file, binary]
            elif platform == "linux":
                dump_syms_cmd = ["dump_syms", debug_file]

            with open(sym_file, "w") as f:
                subprocess.run(dump_syms_cmd, check=True, stdout=f)

            if platform == "linux":
                # When symbols are dumped from a debug file Crashpad creates an incorrect module name.
                sed_cmd = ["sed", "-i", "1s/.debug//", sym_file]
                subprocess.run(sed_cmd, check=True)

        sym_files.append((debug_file, zip_sym(sym_file)))

    return sym_files

def generate_version_string(val_version, commit_hash, qt_version):
    # Determine the platform
    platform = sys.platform
    if platform == "win32":
        platform_str = "windows"
    elif platform == "darwin":
        platform_str = "macos"
    elif platform == "linux":
        platform_str = "linux"
    else:
        platform_str = "unknown"

    # Generate the version string
    version_string = f"{val_version}-{commit_hash}-Qt_{qt_version}-{platform_str}"
    return version_string

def get_app_name(sym_file):
    # Get the base name of the symbol file without extension
    base_name = os.path.splitext(os.path.basename(sym_file))[0].lower()

    # Determine the platform
    platform = sys.platform
    if platform == "linux":
        if base_name.startswith("lib"):
            base_name = base_name[3:]
            return base_name.split(".so")[0]
    elif platform == "darwin":
        if base_name.endswith(".framework"):
            return base_name.split(".framework")[0]

    return base_name

def upload_symbols(install_root, val_version, commit_hash, qt_version, clean=False):
    # Platform-specific commands for generating and uploading symbol files
    platform = sys.platform
    sym_files = generate_sym_files(install_root)
    app_version = generate_version_string(val_version, commit_hash, qt_version)
    print(f"Uploading symbols for version {app_version}")

    for _, sym_file in sym_files:
        app_name = get_app_name(sym_file)
        print(f"Uploading symbols for application {app_name}")

        if platform == "linux":
            upload_cmd = ["sym_upload", sym_file, f"https://{database}.bugsplat.com/post/bp/symbol/breakpadsymbols.php?appName={app_name}&appVer={app_version}"]
        elif platform == "darwin":
            upload_cmd = ["symupload", sym_file, f"https://{database}.bugsplat.com/post/bp/symbol/breakpadsymbols.php?appName={app_name}&appVer={app_version}"]
        elif platform == "win32":
            upload_cmd = ["symupload.exe", "--product", app_name, sym_file, f"https://{database}.bugsplat.com/post/bp/symbol/breakpadsymbols.php?appName={app_name}&appVer={app_version}"]

        try:
            subprocess.run(upload_cmd, check=True)
            print(f"Symbol file '{sym_file}' uploaded successfully.")
        except subprocess.CalledProcessError as e:
            print(f"Error: {e}")

    # Cleanup if requested
    if clean:
        debug_ext = debug_extension()
        for debug_file, sym_file in sym_files:
            os.remove(sym_file)
            print(f"Symbol file '{sym_file}' removed.")
            os.remove(debug_file)
            print(f"Debug file '{debug_file}' removed.")

if __name__ == "__main__":
    # Command-line usage: python symupload.py /path/to/install_root/folder 0_7_52 abcdef123456 6_6 --clean
    #   - First argument: Path to the install root folder
    #   - Second argument: Valentina version
    #   - Third argument: Commit git hash
    #   - Fourth argument: Qt version
    #   - Optional argument: --clean (Clean up after upload)

    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Upload symbols to BugSplat.")
    parser.add_argument("install_root", type=str, help="Path to the installation folder")
    parser.add_argument("val_version", type=str, help="Valentina version")
    parser.add_argument("hash", type=str, help="Commit git hash")
    parser.add_argument("qt_version", type=str, help="Qt version")
    parser.add_argument("--clean", action="store_true", help="Clean up after upload")

    args = parser.parse_args()

    # Call install_package function with provided arguments
    upload_symbols(args.install_root, args.val_version, args.hash, args.qt_version, args.clean)
