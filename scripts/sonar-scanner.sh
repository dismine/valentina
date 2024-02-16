#!/bin/sh
# Analyze project with Sonar cloud on Linux.
# Please, run this script from <root_folder>.

set -x

BUILD_FOLDER=build_sonar

cd ..
rm -rf ../${BUILD_FOLDER}
mkdir ../${BUILD_FOLDER}

QMAKE_PATH=${HOME}/Qt6.6/6.6.0/gcc_64/bin/qmake
PROFILE=qt6Sonar
COMPILER=gcc-12
GCOV=gcov-12

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
  config:debug \
  modules.buildconfig.enableCcache:false \
  modules.buildconfig.enableTestCoverage:true

#  modules.cpp.linkerVariant:mold

# Run tests to generate coverage information
qbs -p autotest-runner -d ../${BUILD_FOLDER} profile:qt6 config:release

# Run gcov to translate `.gcda` files into `.gcov` readable by humans and SonarCloud
mkdir ../${BUILD_FOLDER}/coverage-dir

current_dir=$(pwd)
cd ../${BUILD_FOLDER}/coverage-dir

# --preserve-paths helps us avoid name clash for `.gcov` files corresponding to source files
# with the same name but in different directories.
find .. -name '*.o' | xargs ${GCOV} --preserve-paths

cd "$current_dir"

current_branch=$(git rev-parse --abbrev-ref HEAD)
current_revision=$(git rev-parse HEAD)

sonar-scanner \
  -Dsonar.branch.name=$current_branch \
  -Dsonar.scm.revision=$current_revision \
  -Dsonar.cfamily.gcov.reportsPath="../${BUILD_FOLDER}/coverage-dir"
