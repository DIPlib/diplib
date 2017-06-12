#!/usr/bin/env bash

# Note: run from ./target/ subdirectory of the main repository
# (TODO: fix so this is not a requirement)

wc ../include/*.h ../include/*/*.h ../include/*/*/*.h ../src/*/*.md ../src/*/*.h ../src/*/*.cpp
# Selecting extensions explicitly to exclude non-code files, and the
# *.hpp files from the PCG library that is in ../include/diplib/private/
