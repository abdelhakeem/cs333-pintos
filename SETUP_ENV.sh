#!/bin/sh

echo "Updating PATH..."
export PATH=$PATH:$(realpath ./src/utils)

echo "Updating GDBMACROS..."
export GDBMACROS=$(realpath ./src/misc/gdb-macros)
