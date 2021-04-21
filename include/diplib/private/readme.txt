This directory contains header files that are included by other
header files, and thus necessary to compile code that uses DIPlib.
However, these files should not be included directly by client
code.

pcg_*.hpp is the PCG Random Number Generation library.
For more information, see http://www.pcg-random.org/
The git repository also has testing code: https://github.com/imneme/pcg-cpp

robin_*.h is a fast implementation of the hash map and hash set.
See https://github.com/Tessil/robin-map.
