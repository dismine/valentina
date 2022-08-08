#!/bin/sh
# This script helps run cppcheck with the same keys we have on codeship.com (except for key --platform=unix32).
# Please, run this script from folder <root_folder>/scripts.

# Because we use the last available cppcheck version usually we build it manually. 
CPPCHECK="../../../../cppcheck-2.8/cppcheck"
$CPPCHECK \
          -j4 -f -q \
          -UDRW_DBG \
          -U__INTEL_COMPILER_UPDATE \
          --template '{file}:{line}:{message}:{id}' \
          --inline-suppr \
          --platform=unix64 \
          --std=c++17 \
          --enable=all \
          --library=qt \
          --library=std \
          --library=posix \
          --inconclusive \
          --suppress=*:*/vdxf/libdxfrw/intern/make_unique.h \
          ../src
