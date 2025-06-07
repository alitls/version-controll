#!/bin/sh

set -e # Exit early if any commands fail

(
  cd "$(dirname "$0")"
  cmake -B build -S .
  cmake --build ./build
)

exec $(dirname $0)/build/app "$@"
