#!/usr/bin/env bash
shopt -s nullglob
for f in $1/*.html
do
   sed  's|\(<p>.*\)\(&lt;\)\(<a .*\)\{0,1\}diplib/library/.*\.h\(</a>\)\{0,1\}\(&gt;\)|\1\2diplib.h\5|'  < $f  > tmp.html
   mv  tmp.html  $f
done
