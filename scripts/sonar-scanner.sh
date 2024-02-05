#!/bin/sh
# Analyze project with Sonar cloud on Linux.
# Please, run this script from folder <root_folder>/scripts.

set -x

BUILD_FOLDER=build_sonar

cd ..
rm -rf ../${BUILD_FOLDER}
mkdir ../${BUILD_FOLDER}

QMAKE_PATH=${HOME}/Qt6.6/6.6.0/gcc_64/bin/qmake
PROFILE=qt6Sonar
COMPILER=clang

qbs setup-toolchains /usr/bin/${COMPILER} ${COMPILER}
qbs setup-qt ${QMAKE_PATH} ${PROFILE}
qbs config profiles.${PROFILE}.baseProfile ${COMPILER}
qbs config defaultProfile ${PROFILE}
qbs config --list profiles

build-wrapper-linux-x86-64 \
  --out-dir bw-output \
  qbs build \
  --no-install \
  -d ../${BUILD_FOLDER} \
  -f valentina.qbs \
  --jobs $(nproc) \
  profile:${PROFILE} \
  config:release \
  modules.buildconfig.enableCcache:false

#  modules.cpp.linkerVariant:mold

current_branch=$(git rev-parse --abbrev-ref HEAD)
current_revision=$(git rev-parse HEAD)

sonar-scanner \
  -Dsonar.cfamily.threads=$(nproc) \
  -Dsonar.branch.name=$current_branch \
  -Dsonar.scm.revision=$current_revision
