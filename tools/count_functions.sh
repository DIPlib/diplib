#!/usr/bin/env bash

# Note: run from ./target/ subdirectory of the main repository
# (TODO: fix so this is not a requirement)

sed  -e 's|@DOCUMENTATION_OUTPUT@|.|g' \
     -e 's|@CMAKE_INSTALL_PREFIX@|doc|g' \
     -e 's|@PROJECT_SOURCE_DIR@|..|g' \
     -e 's|@CMAKE_CURRENT_LIST_DIR@|../doc|g' \
     -e 's|GENERATE_HTML *= YES|GENERATE_HTML = NO|' \
     -e 's|GENERATE_XML *= NO|GENERATE_XML = YES|' \
      < ../doc/Doxyfile.in > doc/doxy-coverage.conf

doxygen doc/doxy-coverage.conf
../tools/doxy-coverage.py doc/xml
