#!/usr/bin/env python3
"""Pack and upload Valentina development builds to Cloudflare R2."""

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
from datetime import datetime, timezone

BRANCH_ROOTS = {"master": "stable", "develop": "edge"}
PLATFORMS = ("windows", "macos", "linux")
SHA_WIDTH = 12
R2_ENV_VARS = ("R2_ACCOUNT_ID", "R2_ACCESS_KEY_ID", "R2_SECRET_ACCESS_KEY", "R2_DEV_BUCKET")


def branch_root(branch):
    """Map a git ref name onto its bucket root. Unknown branches are fatal."""
    if branch not in BRANCH_ROOTS:
        raise SystemExit(
            f"Refusing to deploy from branch {branch!r}; expected one of {sorted(BRANCH_ROOTS)}."
        )
    return BRANCH_ROOTS[branch]


def format_build_folder(commit_timestamp, commit_sha):
    """Build the per-commit folder name: UTC minute plus a fixed-width sha.

    The sha is sliced here rather than taken from `git --format=%h` because
    core.abbrev=auto varies with object count -- 9 in a full local clone of this
    repo, fewer under actions/checkout's default fetch-depth of 1. A disagreement
    would split one commit's artifacts across two folders with no error anywhere.
    """
    stamp = datetime.fromtimestamp(int(commit_timestamp), timezone.utc)  # not datetime.UTC: 3.10
    return f"{stamp:%Y%m%dT%H%M}-{commit_sha[:SHA_WIDTH]}"


def read_commit(cwd=None):
    """Return (committer_timestamp, full_sha) for HEAD."""
    result = subprocess.run(
        ["git", "show", "-s", "--format=%ct%n%H", "HEAD"],
        capture_output=True,
        text=True,
        check=True,
        cwd=cwd,
    )
    timestamp, sha = result.stdout.split()
    return timestamp, sha


def object_key(branch, platform, name, commit_timestamp, commit_sha):
    if platform not in PLATFORMS:
        raise SystemExit(f"Unknown platform {platform!r}; expected one of {list(PLATFORMS)}.")
    folder = format_build_folder(commit_timestamp, commit_sha)
    return f"{branch_root(branch)}/{folder}/{platform}/{name}"


def r2_config(env=None):
    """Read R2 credentials from the environment. Never echoes values."""
    env = os.environ if env is None else env
    missing = [name for name in R2_ENV_VARS if not env.get(name)]
    if missing:
        raise SystemExit(f"Missing required environment variable(s): {', '.join(missing)}")
    return {name: env[name] for name in R2_ENV_VARS}


def run_pack(source, destination):
    """
    Pack folder. Automatically fills arguments for shutil.make_archive.
    :param source: path to source root directory. Example: '/path/to/folder/'
    :param destination: path to resulting zip archive. The path must include a format suffix.
    Example: '/path/to/folder.zip'
    """
    import py7zr  # deferred: only CI installs it, developer checkouts do not have it

    base = os.path.basename(destination)
    name = base.split('.')[0]

    formats = {
        ".zip": "zip",
        ".tar.xz": "xztar",
        ".7z": "7zip"
    }
    suffix = ''.join(pathlib.Path(base).suffixes)
    archive_from = pathlib.Path(source).parent
    archive_to = os.path.basename(source.strip(os.sep))
    print(source, destination, archive_from)
    format = formats.get(suffix)
    if format:
        if format == "7zip":
            with py7zr.SevenZipFile(f"{name}{suffix}", 'w') as archive:
                archive.writeall(source, arcname=os.path.basename(source))
        else:
            shutil.make_archive(name, format, archive_from, archive_to)
        shutil.move(f'{name}{suffix}', destination)
    else:
        print("Unsupported archive format.")


def run_upload(files, branch, platform, name=None):
    branch_root(branch)  # fail on an unusable branch before anything else is read
    if name is not None and len(files) > 1:
        raise SystemExit("--name cannot be combined with multiple files.")

    config = r2_config()

    import boto3  # deferred: only CI installs it, developer checkouts do not have it

    client = boto3.client(
        "s3",
        endpoint_url=f"https://{config['R2_ACCOUNT_ID']}.r2.cloudflarestorage.com",
        aws_access_key_id=config["R2_ACCESS_KEY_ID"],
        aws_secret_access_key=config["R2_SECRET_ACCESS_KEY"],
        region_name="auto",
    )
    bucket = config["R2_DEV_BUCKET"]
    timestamp, sha = read_commit()

    for path in files:
        if not os.path.isfile(path):
            raise SystemExit(f"Not a file: {path}")
        leaf = name or os.path.basename(path)
        key = object_key(branch, platform, leaf, timestamp, sha)
        print(f"Uploading {path} -> s3://{bucket}/{key}")
        client.upload_file(path, bucket, key)  # handles multipart for 150 MB+ artifacts

    print("Successfully uploaded")


def parse_args(argv=None):
    parser = argparse.ArgumentParser(prog="deploy")
    commands = parser.add_subparsers(dest="command", required=True)

    pack = commands.add_parser("pack", help="Compress a folder")
    pack.add_argument("source", help="Path to folder or file")
    pack.add_argument("destination", help="Path to the resulting archive")

    upload = commands.add_parser("upload", help="Upload build artifacts to R2")
    upload.add_argument("--branch", required=True, help="Git ref name, e.g. master")
    upload.add_argument("--platform", required=True, choices=PLATFORMS)
    upload.add_argument("--name", default=None, help="Override the object's leaf filename")
    upload.add_argument("files", nargs="+", help="Files to upload")

    args = parser.parse_args(argv)

    if args.command == "pack":
        run_pack(args.source, args.destination)
    else:
        run_upload(args.files, args.branch, args.platform, args.name)


if __name__ == "__main__":
    parse_args()
