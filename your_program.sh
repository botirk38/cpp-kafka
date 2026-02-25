#!/bin/sh
#
# Build and run the Kafka server locally.
#

set -e

cd "$(dirname "$0")"

if [ -n "${VCPKG_ROOT}" ]; then
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
else
  cmake -B build -S .
fi

cmake --build ./build

exec ./build/kafka "$@"
