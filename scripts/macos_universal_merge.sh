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
while IFS= read -r -d '' intel_file; do
    rel="${intel_file#"$intel_root"/}"
    arm_file="$arm_root/$rel"
    out_file="$out_root/$rel"

    if [ ! -f "$arm_file" ]; then
        echo "warning: $rel missing on the arm side, keeping the intel-only slice" >&2
        continue
    fi

    file "$intel_file" | grep -q "Mach-O" || continue

    lipo -create "$intel_file" "$arm_file" -output "$out_file"

    info=$(lipo -info "$out_file")
    if ! grep -q "x86_64" <<<"$info" || ! grep -q "arm64" <<<"$info"; then
        echo "error: $rel is not a universal binary after merge ($info)" >&2
        exit 1
    fi

    merged=$((merged + 1))
done < <(find "$intel_root" -type f -print0)

echo "Merged $merged Mach-O files into universal binaries under $out_root"
[ "$merged" -gt 0 ] || { echo "error: no Mach-O files were merged" >&2; exit 1; }
