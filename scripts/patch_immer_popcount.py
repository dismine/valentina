#!/usr/bin/env python3

# MSVC's __popcnt/__popcnt64 intrinsics (used by immer's hamts popcount() on _MSC_VER)
# compile directly to the x86 POPCNT instruction with no CPUID runtime check. POPCNT
# only exists on Intel Nehalem/2008+ (and AMD equivalents), so any older CPU raises
# EXCEPTION_ILLEGAL_INSTRUCTION the first time a pattern object is inserted into a
# VContainer immer::map. GCC/Clang's __builtin_popcount does not have this problem: it
# falls back to a portable implementation unless the target explicitly guarantees
# POPCNT, which Valentina's builds never request.
#
# immer hardcodes "#define IMMER_HAS_BUILTIN_POPCOUNT 1" unconditionally (not
# #ifndef-guarded), so it cannot be overridden with a compiler -D flag. Patch the
# vendored header in the Conan cache after "conan install" so Windows/MSVC builds fall
# back to the portable implementation, keeping Valentina within its Qt 5.15 CPU baseline.
#
# Usage: patch_immer_popcount.py [CONAN_HOME]  (defaults to $CONAN_HOME or ~/.conan2)

import os
import sys
from pathlib import Path

ORIGINAL = "#define IMMER_HAS_BUILTIN_POPCOUNT 1\n"
PATCHED = (
    "#if defined(_MSC_VER)\n"
    "#define IMMER_HAS_BUILTIN_POPCOUNT 0\n"
    "#else\n"
    "#define IMMER_HAS_BUILTIN_POPCOUNT 1\n"
    "#endif\n"
)
MARKER = "IMMER_HAS_BUILTIN_POPCOUNT 0"


def main() -> int:
    conan_home = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(
        os.environ.get("CONAN_HOME", Path.home() / ".conan2")
    )

    targets = list(conan_home.glob("p/**/include/immer/detail/hamts/bits.hpp"))
    if not targets:
        print(f"error: no immer bits.hpp found under {conan_home}", file=sys.stderr)
        return 1

    patched = 0
    for path in targets:
        text = path.read_text(encoding="utf-8")
        if MARKER in text:
            print(f"already patched: {path}")
            continue
        if ORIGINAL not in text:
            print(f"error: expected line not found in {path}", file=sys.stderr)
            return 1
        path.write_text(text.replace(ORIGINAL, PATCHED), encoding="utf-8")
        print(f"patched: {path}")
        patched += 1

    print(f"done: {patched} file(s) patched, {len(targets) - patched} already patched")
    return 0


if __name__ == "__main__":
    sys.exit(main())
