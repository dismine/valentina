import argparse
import os
import subprocess
import sys
import glob
import zipfile
import requests
import shutil
import re

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

def generate_sym_files(install_root):
    sym_files = []

    platform = sys.platform
    debug_ext = debug_extension()

    debug_files = glob.glob(os.path.join(install_root, "**", "*" + debug_ext), recursive=True)

    if not debug_files:
        print("No debug files found.")

    for debug_file in debug_files:
        corrected_debug_name = os.path.basename(debug_file)
        debug_dir = os.path.dirname(debug_file)

        if sys.platform == "darwin":
            # Remove ".app" if present
            corrected_debug_name = corrected_debug_name.replace(".app", "")
            # Remove ".framework" if present
            corrected_debug_name = corrected_debug_name.replace(".framework", "")
        elif platform == "linux":
            # Remove .so + version numbers if present (e.g., .so.1.0.0 or .so.2.7.0)
            corrected_debug_name = re.sub(r'\.so\.\d+(\.\d+)*', '', corrected_debug_name)

        # Convert to lowercase
        corrected_debug_name = corrected_debug_name.lower()

        print(f"Generating symbols for: {corrected_debug_name}")

        # Copy debug file with corrected name
        corrected_debug_path = os.path.join(debug_dir, corrected_debug_name)
        if debug_file != corrected_debug_path:
          if sys.platform == "darwin":
            shutil.copytree(debug_file, corrected_debug_path)
          else:
            shutil.copy(debug_file, corrected_debug_path)

        sym_file = os.path.splitext(corrected_debug_name)[0] + ".sym"
        dump_syms_cmd = ["dump_syms", '-o', sym_file, '--inlines', corrected_debug_path]
        subprocess.run(dump_syms_cmd, check=True)

        if debug_file != corrected_debug_path:
          # Remove temporary debug file
          if sys.platform == "darwin":
            shutil.rmtree(corrected_debug_path)
          else:
            os.remove(corrected_debug_path)

        sym_files.append((debug_file, sym_file))

    return sym_files

def generate_version_string(val_version, commit_hash, qt_version, multibundle):
    # Determine the platform
    platform = sys.platform
    multibundle_str = ""

    if platform == "win32":
        platform_str = "windows"
    elif platform == "darwin":
        platform_str = "macos"

        if multibundle:
          multibundle_str = "-multibundle"
    elif platform == "linux":
        platform_str = "linux"
    else:
        platform_str = "unknown"

    # Generate the version string
    version_string = f"{val_version}-{commit_hash}-Qt_{qt_version}-{platform_str}{multibundle_str}"
    return version_string

def get_app_name(sym_file):
    # Get the base name of the symbol file without extension
    base_name = os.path.basename(sym_file).split(".sym")[0].lower()

    # Determine the platform
    platform = sys.platform
    if platform == "linux":
        if base_name.startswith("lib"):
            base_name = base_name[3:]
            return base_name.split(".so")[0]
    elif platform == "darwin":
        if base_name.endswith(".framework"):
            return base_name.split(".framework")[0]
        elif base_name.endswith(".app"):
            return base_name.split(".app")[0]

    return base_name

def upload_symbols(install_root, val_version, commit_hash, qt_version, clean=False, multibundle=False):
    # Platform-specific commands for generating and uploading symbol files
    platform = sys.platform
    sym_files = generate_sym_files(install_root)

    if not sym_files:
        print("No symbol files found. Exiting upload process.")
        return

    app_version = generate_version_string(val_version, commit_hash, qt_version, multibundle)
    print(f"Uploading symbols for version {app_version}")

    for _, sym_file in sym_files:
        app_name = get_app_name(sym_file)
        print(f"Uploading symbols for application {app_name}")

        sym_file_name = os.path.basename(sym_file)
        url = f"https://{database}.bugsplat.com/post/bp/symbol/breakpadsymbols.php?appName={app_name}&appVer={app_version}&code_file={sym_file_name}"

        with open(sym_file, 'rb') as symbol_file:
            files = {'symbol_file': (f'{sym_file_name}', symbol_file)}
            response = requests.post(url, files=files)

        if response.status_code == 200:
            print(f"Symbol file '{sym_file}' uploaded successfully.")
        else:
            print("Request failed with status code:", response.status_code)
            print("Server response:")
            print(response.text)

    # Cleanup if requested
    if clean:
        debug_ext = debug_extension()
        for debug_file, sym_file in sym_files:
            try:
                os.remove(sym_file)
                print(f"Symbol file '{sym_file}' removed.")
            except PermissionError as e:
                print(f"PermissionError removing '{sym_file}': {e}")

            try:
                if sys.platform == "darwin":
                    shutil.rmtree(debug_file)
                else:
                    os.remove(debug_file)
                print(f"Debug file '{debug_file}' removed.")
            except PermissionError as e:
                print(f"PermissionError removing '{debug_file}': {e}")

if __name__ == "__main__":
    # Command-line usage: python symupload.py /path/to/install_root/folder 0_7_52 abcdef123456 6_6 --clean
    #   - First argument: Path to the install root folder
    #   - Second argument: Valentina version
    #   - Third argument: Commit git hash
    #   - Fourth argument: Qt version
    #   - Optional argument: --clean (Clean up after upload)
    #   - Optional argument: --multibundle (Mark multibundle version. Has effect only on macos)

    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Upload symbols to BugSplat.")
    parser.add_argument("install_root", type=str, help="Path to the installation folder")
    parser.add_argument("val_version", type=str, help="Valentina version")
    parser.add_argument("hash", type=str, help="Commit git hash")
    parser.add_argument("qt_version", type=str, help="Qt version")
    parser.add_argument("--clean", action="store_true", help="Clean up after upload")
    parser.add_argument("--multibundle", type=str, default="false", choices=["true", "false"], help="Mark multibundle version. Has effect only on Macos")

    args = parser.parse_args()

    multibundle = (args.multibundle == "true")

    # Call install_package function with provided arguments
    upload_symbols(args.install_root, args.val_version, args.hash, args.qt_version, args.clean, multibundle)
