This directory contains header files that are included by other
header files, and thus necessary to compile code that uses DIPlib.
However, these files should not be included directly by client
code.

pcg_*.hpp is the PCG Random Number Generation library (master branch
as of 2022-04-08). From https://github.com/imneme/pcg-cpp
For more information, see http://www.pcg-random.org

robin_*.h is robin-map v1.3.0, a fast implementation of the hash map and
hash set. From https://github.com/Tessil/robin-map
