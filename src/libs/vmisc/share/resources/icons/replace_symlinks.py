import os
import shutil

def replace_symlinks_with_real_files(folder_path):
    # Stage 1: Replace symbolic links with real files for files
    for root, _, files in os.walk(folder_path):
        for filename in files:
            file_path = os.path.join(root, filename)
            if os.path.islink(file_path):
                real_path = os.path.realpath(file_path)
                os.unlink(file_path)  # Remove the symbolic link
                shutil.copy(real_path, file_path)  # Replace with the real file

    # Stage 2: Replace symbolic links with real files for folders
    for root, dirs, _ in os.walk(folder_path, topdown=False):
        for dirname in dirs:
            dir_path = os.path.join(root, dirname)
            if os.path.islink(dir_path):
                real_path = os.path.realpath(dir_path)
                os.unlink(dir_path)  # Remove the symbolic link (empty directory)
                shutil.copytree(real_path, dir_path)  # Replace with the real directory

if __name__ == "__main__":
    folder_path = "/home/dismine/CAD/Valentina_0.7.x/valentina/src/libs/vmisc/share/resources/icons/La-Sierra-Light"  # Replace this with the folder you want to scan
    replace_symlinks_with_real_files(folder_path)


