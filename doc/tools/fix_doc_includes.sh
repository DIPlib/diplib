#!/usr/bin/env bash
shopt -s nullglob
for f in $1/*.html
do
   # First SED replacement: replaces all "#include <diplib/library/*.h>" for "#include <diplib.h>" (fixing HTML links also)
   # Second SED replacement: replaces all "dml::anonymous_namespace{dip_matlab_interface.h}::NAME" with "dml::NAME"
   sed  -e 's|\(<p>.*\)\(&lt;\)\(<a .*\)\{0,1\}diplib/library/.*\.h\(</a>\)\{0,1\}\(&gt;\)|\1\2diplib.h\5|' \
        -e 's/dml::anonymous_namespace{dip_matlab_interface\.h}::/dml::/' \
      < $f  > tmp.html
   mv  tmp.html  $f
done
