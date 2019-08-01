#!/usr/bin/env bash

# Note: run from any subdirectory of the main repository (for example ./tools/ or ./target/)
# (TODO: fix so this is not a requirement)

wc ../include/*.h ../include/*/*.h ../include/*/*/*.h ../doc/src/*.md ../src/*/*.h ../src/*/*.cpp \
   ../viewer/src/*.cpp ../viewer/src/*/*.cpp ../viewer/java/*.java \
   ../javaio/src/* ../javaio/java/*.java ../javaio/java/*/*.java
# Selecting extensions explicitly to exclude non-code files,
# and the *.hpp files from the PCG library that is in ../include/diplib/private/
# and the font file that is in ../viewer/src/fg_font_data.h.
