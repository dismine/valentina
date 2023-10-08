import argparse
import datetime
import os
import pathlib
import re
import shutil
import sys

import dropbox
from dropbox import DropboxOAuth2FlowNoRedirect
from dropbox.exceptions import ApiError, AuthError
from dropbox.files import WriteMode

APP_KEY = "v33m5tjz020h7uy"


def run_auth():
    """
    Use to generate a refresh token
    """
    auth_flow = DropboxOAuth2FlowNoRedirect(APP_KEY, use_pkce=True, token_access_type='offline')

    authorize_url = auth_flow.start()
    print(f"1. Go to: {authorize_url}")
    print("2. Click \"Allow\" (you might have to log in first).")
    print("3. Copy the authorization code.")
    auth_code = input("Enter the authorization code here: ").strip()

    try:
        oauth_result = auth_flow.finish(auth_code)
    except Exception as e:
        print(f'Error: {e}')
        exit(1)

    print(f"Refresh token: {oauth_result.refresh_token}")


def run_pack(source, destination):
    """
    Pack folder. Automatically fills arguments for shutil.make_archive.
    :param source: path to source root directory. Example: '/path/to/folder/'
    :param destination: path to resulting zip archive. The path must include a format suffix.
    Example: '/path/to/folder.zip'
    """
    base = os.path.basename(destination)
    name = base.split('.')[0]

    formats = {
        ".zip": "zip",
        ".tar.xz": "xztar"
    }
    suffix = ''.join(pathlib.Path(base).suffixes)
    format = formats.get(suffix)
    archive_from = pathlib.Path(source).parent
    archive_to = os.path.basename(source.strip(os.sep))
    print(source, destination, archive_from)
    shutil.make_archive(name, format, archive_from, archive_to)
    shutil.move(f'{name}{suffix}', destination)


def run_upload(refresh_token, file, path):
    with dropbox.Dropbox(oauth2_refresh_token=refresh_token, app_key=APP_KEY) as dbx:
        # Check that the access token is valid
        try:
            dbx.users_get_current_account()
        except AuthError:
            sys.exit("ERROR: Invalid access token; try re-generating an "
                     "access token from the app console on the web.")

        print(f"Uploading {file} to Dropbox as {path}...")
        try:
            with open(file, "rb") as f:
                dbx.files_upload(f.read(), path, mode=WriteMode('overwrite'))
        except ApiError as err:
            # This checks for the specific error where a user doesn't have
            # enough Dropbox space quota to upload this file
            if (err.error.is_path() and
                    err.error.get_path().reason.is_insufficient_space()):
                sys.exit("ERROR: Cannot deploy; insufficient space.")
            elif err.user_message_text:
                print(err.user_message_text)
                sys.exit()
            else:
                print(err)
                sys.exit()
        print("Successfully uploaded")

def folder_mod_time(dbx, folder):
    folder_mod_times = []
    entries = dbx.files_list_folder(folder.path_display)
    # Loop through each item in the folder list
    for result in entries.entries:
        # Check if the item is a file
        if isinstance(result, dropbox.files.FileMetadata):
            # Get the parent folder path of the file
            parent_folder_path = result.path_display.rsplit('/', 1)[0]
            # Get the modification time of the file
            file_mod_time = result.client_modified
            # Add the file modification time to the dictionary for the parent folder
            folder_mod_times.append(file_mod_time)

    folder_mod_times.append(datetime.datetime(1900, 1, 1, 0, 0, 0))
    # Compute the maximum modification time across all files in each folder
    max_mod_time = max(folder_mod_times)
    return max_mod_time

# Define a function to delete a file or folder recursively
def delete_file_or_folder(dbx, item):
    try:
        # Check if the path is a file
        if isinstance(item, dropbox.files.FileMetadata):
            dbx.files_delete_v2(item.path_display)
            print(f"Deleted file: {item.path_display}")
        # Check if the path is a folder
        elif isinstance(item, dropbox.files.FolderMetadata):
            # Recursively delete all files and subfolders inside the folder
            for entry in dbx.files_list_folder(item.path_display).entries:
                delete_file_or_folder(dbx, entry)
            # Delete the folder itself
            dbx.files_delete_v2(item.path_display)
            print(f"Deleted folder: {item.path_display}")
    except dropbox.exceptions.ApiError as e:
        print(f"Error deleting {item.path_display}: {e}")

def run_clean(refresh_token):
    with dropbox.Dropbox(oauth2_refresh_token=refresh_token, app_key=APP_KEY) as dbx:
        # Check that the access token is valid
        try:
            dbx.users_get_current_account()
        except AuthError:
            sys.exit("ERROR: Invalid access token; try re-generating an "
                     "access token from the app console on the web.")

        clean_folders = ["/0.7.x/Mac OS X", "/0.7.x/Windows"]
        arhive_types = [r'^valentina-Windows10\+-mingw-x64-Qt.*-develop-[a-f0-9]{40}\.tar\.xz$',
                        r'^valentina-Windows7\+-mingw-x86-Qt.*-develop-[a-f0-9]{40}\.tar\.xz$',
                        r'^valentina-WindowsXP\+-mingw-x86-Qt.*-develop-[a-f0-9]{40}\.tar\.xz$',
                        r'^valentina-macOS_11\+-Qt.*-x64-develop-[a-f0-9]{40}\.dmg$',
                        r'^valentina-macOS_11\+-Qt.*-x64-develop-multibundle-[a-f0-9]{40}\.dmg$',
                        r'^valentina-macOS_10.13\+-Qt.*-x64-develop-[a-f0-9]{40}\.dmg$',
                        r'^valentina-macOS_10.13\+-Qt.*-x64-develop-multibundle-[a-f0-9]{40}\.dmg$',
                        r'^valentina-macOS.*\+-Qt.*-arm.*-develop-[a-f0-9]{40}\.dmg$',
                        r'^valentina-macOS.*\+-Qt.*-arm.*-develop-multibundle-[a-f0-9]{40}\.dmg$']

        item_types = {}

        for path in clean_folders:
            result = dbx.files_list_folder(path)
            for entry in result.entries:
                for archive_type in arhive_types:
                    if re.search(archive_type, entry.name):
                        if archive_type not in item_types:
                            item_types[archive_type] = []
                        item_types[archive_type].append(entry)
                        break

        # Keep only the first two files of each type
        to_delete = []
        for items in item_types.values():
            # Separate files and folders
            files = [item for item in items if isinstance(item, dropbox.files.FileMetadata)]
            folders = [item for item in items if isinstance(item, dropbox.files.FolderMetadata)]

            # Sort files by modification time
            files = sorted(files, key=lambda f: f.client_modified)

            # Sort folders by last modified time on server
            folders = sorted(folders, key=lambda f: folder_mod_time(dbx, f))

            # Keep only the first two items of each type
            to_delete += files[:-2] + folders[:-2]

        # Delete the remaining items
        for item in to_delete:
            delete_file_or_folder(dbx, item)


def parse_args(args=None):
    parser = argparse.ArgumentParser(prog='app')
    cmds = parser.add_subparsers(help='commands')

    def cmd(name, **kw):
        p = cmds.add_parser(name, **kw)
        p.set_defaults(cmd=name)
        p.arg = lambda *a, **kw: p.add_argument(*a, **kw) and p
        p.exe = lambda f: p.set_defaults(exe=f) and p

        # global options
        # p.arg('-s', '--settings', help='application settings')
        return p

    cmd('auth', help='Authorize application') \
        .exe(lambda _: (
             run_auth()
        ))

    cmd('pack', help='Compress folder') \
        .arg('source', type=str, help='Path to folder or file') \
        .arg('destination', type=str, help='Path to resulting zip archive') \
        .exe(lambda a: (
             run_pack(a.source, a.destination)
        ))

    cmd('upload', help='Upload file with override') \
        .arg('refresh_token', type=str, help='Refresh token') \
        .arg('file', type=str, help='Path to file') \
        .arg('path', type=str, help='Path on disk') \
        .exe(lambda a: (
             run_upload(a.refresh_token, a.file, a.path)
        ))

    cmd('clean', help='Clean stale artifacts') \
        .arg('refresh_token', type=str, help='Refresh token') \
        .exe(lambda a: (
             run_clean(a.refresh_token)
    ))

    args = parser.parse_args(args)
    if not hasattr(args, 'exe'):
        parser.print_usage()
    else:
        args.exe(args)


if __name__ == '__main__':
    parse_args()

