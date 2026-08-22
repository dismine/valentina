#!/usr/bin/env bash
# Merge two per-architecture Valentina macOS install-root trees (as produced by
# the intel_build/arm_build CI jobs) into one universal (fat) tree via lipo.
#
# Usage: macos_universal_merge.sh <intel-root> <arm-root> <output-root>

set -euo pipefail

intel_root="$1"
arm_root="$2"
out_root="$3"

rm -rf "$out_root"
cp -R "$intel_root" "$out_root"

merged=0
already_universal=0
while IFS= read -r -d '' intel_file; do
    rel="${intel_file#"$intel_root"/}"
    arm_file="$arm_root/$rel"
    out_file="$out_root/$rel"

    if [ ! -f "$arm_file" ]; then
        echo "warning: $rel missing on the arm side, keeping the intel-only slice" >&2
        continue
    fi

    file "$intel_file" | grep -q "Mach-O" || continue

    # Files bundled straight from Qt's own SDK (plugins, frameworks) are
    # already universal on both sides, byte-differences and all (signatures,
    # timestamps) — lipo refuses to "merge" two inputs that already share an
    # architecture, so just keep the copy already placed by the base cp -R.
    archs=$(lipo -archs "$intel_file")
    if grep -q "x86_64" <<<"$archs" && grep -q "arm64" <<<"$archs"; then
        already_universal=$((already_universal + 1))
        continue
    fi

    lipo -create "$intel_file" "$arm_file" -output "$out_file"

    info=$(lipo -info "$out_file")
    if ! grep -q "x86_64" <<<"$info" || ! grep -q "arm64" <<<"$info"; then
        echo "error: $rel is not a universal binary after merge ($info)" >&2
        exit 1
    fi

    merged=$((merged + 1))
done < <(find "$intel_root" -type f -print0)

echo "Merged $merged Mach-O files into universal binaries ($already_universal were already universal) under $out_root"
[ "$merged" -gt 0 ] || { echo "error: no Mach-O files were merged" >&2; exit 1; }
