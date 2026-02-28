#!/usr/bin/env python3
"""
Upload debug symbols to BugSplat using the symbol-upload tool.

Per-platform targets and debug artifact types:

  Linux   (.debug)  : puzzle, tape, valentina, libQMUParserLib, libVPropertyExplorerLib
  Windows (.pdb)    : puzzle, tape, valentina, QMUParserLib, VPropertyExplorerLib
  macOS             : Puzzle.app, Tape.app, Valentina.app          (search inside .app for .dSYM)
                      QMUParserLib.framework, VPropertyExplorerLib.framework

Excluded targets (never uploaded): parserTest, collectionTest, translationsTest

Version string format:
    <app_version>-<git_hash>-<qt_version>-<platform>[‑multibundle]

    Examples:
        1_1_0-gf4373acf9-Qt_6_2-linux
        1_1_0-gf4373acf9-Qt_6_10-macos-multibundle
        1_1_0-g12e950caa-Qt_6_10-windows
        1_1_0-g12e950caa-Qt_6_10-macos

Authentication — set ONE of the two pairs as environment variables:
    SYMBOL_UPLOAD_USER          BugSplat account email
    SYMBOL_UPLOAD_PASSWORD      BugSplat account password
  OR
    SYMBOL_UPLOAD_CLIENT_ID     OAuth2 Client Credentials Client ID
    SYMBOL_UPLOAD_CLIENT_SECRET OAuth2 Client Credentials Client Secret

Required environment variable:
    BUGSPLAT_DATABASE           Your BugSplat database name

Usage:
    python upload_symbols.py \\
        --build-dir   <path>    \\
        --app-version <ver>     \\
        --git-hash    <hash>    \\
        --qt-version  <qtver>   \\
        [--platform   <plat>]   \\
        [--multibundle]

Examples:
    python upload_symbols.py \\
        --build-dir ./build --app-version 1_1_0 --git-hash gf4373acf9 --qt-version Qt_6_2

    python upload_symbols.py \\
        --build-dir ./build --app-version 1_1_0 --git-hash gf4373acf9 --qt-version Qt_6_10 \\
        --platform macos --multibundle
"""

import argparse
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

# ── Target definitions ─────────────────────────────────────────────────────────

@dataclass(frozen=True)
class Target:
    """
    Describes one uploadable artifact.

    application     : the name passed to symbol-upload --application
    filename        : exact filename (or bundle directory name) to locate on disk.
                      Ignored when filename_prefix is set.
    is_dir          : True when the artifact is a directory (bundle)
    files_glob      : the --files glob passed to symbol-upload.
                      For macOS .app targets this searches recursively for the
                      exact .dSYM bundle name.
    filename_prefix : when set, the artifact is found by matching a file whose
                      name starts with this prefix and ends with '.debug'.
                      Used for versioned Linux shared libraries where the version
                      number is embedded in the filename, e.g.:
                        libQMUParserLib.so.2.7.0.debug
    """
    application: str
    filename: str
    is_dir: bool
    files_glob: str
    filename_prefix: str = ""


# Linux apps: exact filename.
# Linux libraries: versioned filenames (libQMUParserLib.so.2.7.0.debug) matched
# by prefix so the version number does not need to be known in advance.
TARGETS_LINUX: list[Target] = [
    Target("puzzle",               "puzzle.debug",    False, "puzzle.debug"),
    Target("tape",                 "tape.debug",      False, "tape.debug"),
    Target("valentina",            "valentina.debug", False, "valentina.debug"),
    Target("qmuparserlib",         "",                False, "",               "libQMUParserLib"),
    Target("vpropertyexplorerlib", "",                False, "",               "libVPropertyExplorerLib"),
]

# Windows: plain files, no lib prefix
TARGETS_WINDOWS: list[Target] = [
    Target("puzzle",              "puzzle.pdb",              False, "puzzle.pdb"),
    Target("tape",                "tape.pdb",                False, "tape.pdb"),
    Target("valentina",           "valentina.pdb",           False, "valentina.pdb"),
    Target("qmuparserlib",        "QMUParserLib.pdb",        False, "QMUParserLib.pdb"),
    Target("vpropertyexplorerlib","VPropertyExplorerLib.pdb",False, "VPropertyExplorerLib.pdb"),
]

# macOS: dSYM bundles live in dedicated *_dSYM sibling directories, not inside
# the .app. Actual names on disk:
#   Tape_dSYM/Tape.app.dSYM
#   Puzzle_dSYM/Puzzle.app.dSYM
#   Valentina_dSYM/Valentina.app.dSYM
#   Valentina_dSYM/QMUParserLib.framework.dSYM
#   Valentina_dSYM/VPropertyExplorerLib.framework.dSYM
#
# We search directly for the .dSYM bundle by exact name and point --directory
# at its parent. No .app/.framework discovery needed.
TARGETS_MACOS: list[Target] = [
    Target("puzzle",               "Puzzle.app.dSYM",                     True, "Puzzle.app.dSYM"),
    Target("tape",                 "Tape.app.dSYM",                       True, "Tape.app.dSYM"),
    Target("valentina",            "Valentina.app.dSYM",                  True, "Valentina.app.dSYM"),
    Target("qmuparserlib",         "QMUParserLib.framework.dSYM",         True, "QMUParserLib.framework.dSYM"),
    Target("vpropertyexplorerlib", "VPropertyExplorerLib.framework.dSYM", True, "VPropertyExplorerLib.framework.dSYM"),
]

TARGETS_BY_PLATFORM: dict[str, list[Target]] = {
    "linux":   TARGETS_LINUX,
    "windows": TARGETS_WINDOWS,
    "macos":   TARGETS_MACOS,
}


# ── Version builder ────────────────────────────────────────────────────────────

def build_version(app_version: str, git_hash: str, qt_version: str,
                  platform: str, multibundle: bool) -> str:
    """
    Assemble the BugSplat version string, e.g.:
        1_1_0-gf4373acf9-Qt_6_10-macos-multibundle
    """
    parts = [app_version, git_hash, qt_version, platform]
    if multibundle:
        parts.append("multibundle")
    return "-".join(parts)


# ── Helpers ────────────────────────────────────────────────────────────────────

def detect_platform() -> str:
    import platform
    system = platform.system().lower()
    mapping = {"linux": "linux", "windows": "windows", "darwin": "macos"}
    if system not in mapping:
        raise SystemExit(f"[ERROR] Unsupported platform: {system!r}")
    return mapping[system]


def get_env(name: str) -> str:
    value = os.environ.get(name, "").strip()
    if not value:
        raise SystemExit(
            f"[ERROR] Required environment variable '{name}' is not set or empty."
        )
    return value


def resolve_auth() -> list[str]:
    """
    Return explicit auth CLI flags for symbol-upload.
    Prefers OAuth2 client credentials; falls back to user/password.
    """
    client_id     = os.environ.get("SYMBOL_UPLOAD_CLIENT_ID", "").strip()
    client_secret = os.environ.get("SYMBOL_UPLOAD_CLIENT_SECRET", "").strip()
    user          = os.environ.get("SYMBOL_UPLOAD_USER", "").strip()
    password      = os.environ.get("SYMBOL_UPLOAD_PASSWORD", "").strip()

    if client_id and client_secret:
        return ["--clientId", client_id, "--clientSecret", client_secret]
    if user and password:
        return ["--user", user, "--password", password]

    raise SystemExit(
        "[ERROR] No valid authentication found.\n"
        "        Set SYMBOL_UPLOAD_CLIENT_ID + SYMBOL_UPLOAD_CLIENT_SECRET\n"
        "        or SYMBOL_UPLOAD_USER + SYMBOL_UPLOAD_PASSWORD."
    )


def find_artifact(build_dir: Path, target: Target) -> tuple[Path, str] | tuple[None, None]:
    """
    Recursively search build_dir for the artifact described by target.

    Two matching strategies:

    Prefix-based (target.filename_prefix set) — for versioned Linux shared
    libraries where the version number is embedded in the filename, e.g.:
        libQMUParserLib.so.2.7.0.debug
    Matches the first file whose name starts with filename_prefix and ends
    with '.debug'. The files_glob returned is the exact filename found, so
    symbol-upload receives a precise match with no wildcards.

    Exact (target.filename set) — for all other targets. Matches by exact
    filename, validating file vs. directory as appropriate. Returns the
    target's pre-defined files_glob unchanged.

    Returns (artifact_path, files_glob) on success, or (None, None) if not found.
    """
    if target.filename_prefix:
        for candidate in build_dir.rglob(f"{target.filename_prefix}*.debug"):
            if candidate.is_file():
                return candidate, candidate.name
        return None, None

    for candidate in build_dir.rglob(target.filename):
        if target.is_dir and candidate.is_dir():
            return candidate, target.files_glob
        if not target.is_dir and candidate.is_file():
            return candidate, target.files_glob
    return None, None


def upload_target(
    target: Target,
    artifact_path: Path,
    files_glob: str,
    version: str,
    database: str,
    auth_flags: list[str],
) -> None:
    """
    Invoke symbol-upload for a single target.

    --directory  points to the parent directory of the located artifact.
                 For macOS this is the *_dSYM folder containing the .dSYM bundle.
    --files      is always the exact filename of the artifact, so symbol-upload
                 matches precisely one item with no ambiguity.
    --dumpSyms   instructs symbol-upload to run dump_syms and upload the
                 resulting .sym rather than the raw debug artifact.
    """
    base_cmd = [
        "symbol-upload",
        "--database",    database,
        "--application", target.application,
        "--version",     version,
        "--directory",   str(artifact_path.parent),
        "--files",       files_glob,
        "--dumpSyms",    "true",
    ]
    cmd = base_cmd + auth_flags

    # Build a redacted copy for logging — mask values that follow credential flags.
    CREDENTIAL_FLAGS = {"--user", "--password", "--clientId", "--clientSecret"}
    redacted = []
    skip_next = False
    for token in cmd:
        if skip_next:
            redacted.append("***")
            skip_next = False
        elif token in CREDENTIAL_FLAGS:
            redacted.append(token)
            skip_next = True
        else:
            redacted.append(token)

    print(f"[INFO] Uploading '{target.application}'")
    print(f"       Artifact : {artifact_path}")
    print(f"       Command  : {' '.join(redacted)}")

    result = subprocess.run(cmd, text=True, capture_output=True)

    if result.stdout:
        print(result.stdout.rstrip())
    if result.stderr:
        print(result.stderr.rstrip(), file=sys.stderr)

    if result.returncode != 0:
        raise SystemExit(
            f"[ERROR] symbol-upload failed for '{target.application}' "
            f"(exit code {result.returncode})."
        )

    print(f"[OK]   '{target.application}' uploaded successfully.\n")


# ── Argument parsing ───────────────────────────────────────────────────────────

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Upload BugSplat debug symbols for the Valentina project.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "--build-dir", "-b",
        required=True, type=Path, metavar="DIR",
        help="Root of the build output directory to search for debug artifacts.",
    )
    parser.add_argument(
        "--app-version",
        required=True, metavar="VERSION",
        help="Application version with underscores, e.g. 1_1_0.",
    )
    parser.add_argument(
        "--git-hash",
        required=True, metavar="HASH",
        help="Short git commit hash, e.g. gf4373acf9.",
    )
    parser.add_argument(
        "--qt-version",
        required=True, metavar="QT_VERSION",
        help="Qt version with underscores, e.g. Qt_6_10.",
    )
    parser.add_argument(
        "--platform",
        choices=["linux", "windows", "macos"],
        default=None, metavar="PLATFORM",
        help="Target platform. Auto-detected from the host OS when omitted.",
    )
    parser.add_argument(
        "--multibundle",
        action="store_true", default=False,
        help="Append '-multibundle' suffix to the version string (macOS only).",
    )
    return parser.parse_args()


# ── Main ───────────────────────────────────────────────────────────────────────

def main() -> None:
    args = parse_args()

    database   = get_env("BUGSPLAT_DATABASE")
    auth_flags = resolve_auth()

    platform = args.platform or detect_platform()

    if args.multibundle and platform != "macos":
        print(f"[WARN] --multibundle is set but platform is '{platform}'. "
              "The suffix will still be appended as requested.")

    version = build_version(
        app_version = args.app_version,
        git_hash    = args.git_hash,
        qt_version  = args.qt_version,
        platform    = platform,
        multibundle = args.multibundle,
    )

    build_dir: Path = args.build_dir.resolve()
    if not build_dir.is_dir():
        raise SystemExit(f"[ERROR] Build directory does not exist: {build_dir}")

    targets = TARGETS_BY_PLATFORM[platform]

    print(f"[INFO] Platform  : {platform}")
    print(f"[INFO] Build dir : {build_dir}")
    print(f"[INFO] Version   : {version}")
    print(f"[INFO] Database  : {database}")
    print(f"[INFO] Targets   : {len(targets)}")
    print()

    uploaded = 0
    missing  = []

    for target in targets:
        artifact_path, files_glob = find_artifact(build_dir, target)

        if artifact_path is None:
            label = target.filename or f"{target.filename_prefix}*.debug"
            print(f"[WARN] Artifact '{label}' not found — skipping '{target.application}'.")
            missing.append(target.application)
            continue

        upload_target(
            target        = target,
            artifact_path = artifact_path,
            files_glob    = files_glob,
            version       = version,
            database      = database,
            auth_flags    = auth_flags,
        )
        uploaded += 1

    print(f"[INFO] Uploaded : {uploaded} / {len(targets)} target(s).")
    if missing:
        print(f"[WARN] Not found: {', '.join(missing)}")


if __name__ == "__main__":
    main()
