#!/usr/bin/env bash

# Note: run from ./target/ subdirectory of the main repository
# (TODO: fix so this is not a requirement)

sed  -e 's|@DOCUMENTATION_OUTPUT@|.|g' \
     -e 's|@CMAKE_INSTALL_PREFIX@|.|g' \
     -e 's|@CMAKE_CURRENT_SOURCE_DIR@|..|g' \
     -e 's|GENERATE_HTML *= YES|GENERATE_HTML = NO|' \
     -e 's|GENERATE_XML *= NO|GENERATE_XML = YES|' \
      < ../src/documentation/Doxyfile.in > doxy-coverage.conf

doxygen doxy-coverage.conf
../tools/doxy-coverage.py xml
