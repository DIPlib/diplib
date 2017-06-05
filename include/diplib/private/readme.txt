This directory contains header files that are included by other
header files, and thus necessary to compile code that uses DIPlib.
However, these files should not be included directly by client
code.

pcg_*.h is the PCG Random Number Generation library, we just
changed the file names to end in .h instead of .hpp.
For more information, see http://www.pcg-random.org/
The git repository also has testing code: https://github.com/imneme/pcg-cpp
