#!/usr/bin/env bash
# Re-sign a merged universal .app bundle after lipo, inside-out, matching how
# https://github.com/dismine/macdeployqt's codesignBundle() signs it during a
# normal build: individual binaries first, then each enclosing .framework
# bundle, then the outer .app — never `codesign --deep` on the whole bundle,
# which mis-detects this project's unversioned framework layout as an
# "ambiguous bundle format".
#
# Usage: macos_universal_codesign.sh <identity> <app-bundle-path> [<app-bundle-path> ...]

set -euo pipefail

identity="$1"
shift

sign() {
    codesign --force --preserve-metadata=identifier,entitlements -o runtime --timestamp -s "$identity" "$1"
}

for app in "$@"; do
    echo "Codesigning $app"

    find "$app" -type f -print0 | while IFS= read -r -d '' f; do
        file "$f" | grep -q "Mach-O" || continue

        # Frameworks here are laid out with real duplicate files where a
        # proper framework would use symlinks (top-level copy, Versions/1,
        # Versions/Current all contain the same binary as independent
        # files). Only Versions/<N>/<binary> is what anything actually
        # links against (confirmed via each binary's own LC_ID_DYLIB and
        # its dependents' load commands) — explicitly codesigning the
        # unreferenced flat top-level or Versions/Current copies as
        # separate code makes codesign report "bundle format is ambiguous
        # (could be app or framework)" on the framework directory.
        case "$f" in
            */Versions/Current/*) continue ;;
        esac
        case "$(dirname "$f")" in
            *.framework) continue ;;
        esac

        sign "$f"
    done

    while IFS= read -r -d '' framework; do
        sign "$framework"
    done < <(find "$app" -type d -name "*.framework" -print0)

    sign "$app"

    codesign --deep -v "$app"
done
