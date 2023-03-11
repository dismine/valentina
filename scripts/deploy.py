import argparse
import os
import pathlib
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

    args = parser.parse_args(args)
    if not hasattr(args, 'exe'):
        parser.print_usage()
    else:
        args.exe(args)


if __name__ == '__main__':
    parse_args()
