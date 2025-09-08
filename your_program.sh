#!/bin/sh
#
# Use this script to run your program LOCALLY.
#
# Note: Changing this script WILL NOT affect how CodeCrafters runs your program.
#
# Learn more: https://codecrafters.io/program-interface

set -e # Exit early if any commands fail

# Copied from .script/compile.sh
#
# - Edit this to change how your program compiles locally
# - Edit .script/compile.sh to change how your program compiles remotely
(
  cd "$(dirname "$0")" # Ensure compile steps are run within the repository directory
  cmake -B build -S .
  cmake --build ./build
)

# Copied from .script/run.sh
#
# - Edit this to change how your program runs locally
# - Edit .script/run.sh to change how your program runs remotely
exec $(dirname $0)/build/git "$@"
