# Building *DIPlib*, *DIPimage* and *PyDIP*

## Linux, MacOS, Cygwin and other Unix-like systems

To build the library you will need a C++14 compliant compiler and *CMake*.
See below under "Dependencies" for optional dependencies that you can install to
improve your *DIPlib* experience.

Use the following commands to build:
```bash
mkdir target
cd target
cmake /path/to/dip/root/directory
make -j install
```

The `-j` option to `make` enables a multi-threaded build. Limit the number of
concurrent jobs to, for example, 4 with `-j4`. On system with limited memory,
reduce the number of concurrent jobs if compilation stalls and the system thrashes.

See below under "*CMake* configuration" for other `make` targets and *CMake* configuration
options.

For step-by-step instructions for Linux, see [`INSTALL_Linux.md`](INSTALL_Linux.md).

For step-by-step instructions for MacOS, see [`INSTALL_MacOS.md`](INSTALL_MacOS.md).

## Windows

Unless you want to use *Cygwin* or *MinGW* (see above), we recommend *Microsoft Visual Studio 2017*
(version 15). You'll also need *CMake*.

Using *CMake-gui*, choose where the source directory is and where to build the binaries. Then
press "Configure" and select *Visual Studio*. Finally, press "Generate". You should now have
a *Visual Studio* solution file that you can open in *Visual Studio* and build as usual.

See below under "*CMake* configuration" for generated targets, and *CMake* configuration
options.

For step-by-step instructions, see [`INSTALL_Windows.md`](INSTALL_Windows.md).

See below for optional dependencies that you can install to improve your *DIPlib* experience.

## *CMake* configuration

Available `make` targets:

    all (default) # builds DIPlib, DIPimage and PyDIP, if configured
    install       # builds and installs target 'all'
    check         # builds the unit_tests program and runs it
    check_memory  # ...and runs it under valgrind
    apidoc        # builds the HTML documentation for the library API
    examples      # builds the examples
    package       # creates a distributable package

The following `make` targets are part of the `all` target:

    DIP           # builds the DIPlib library
    DIPviewer     # builds the DIPviewer module (plus the DIPlib library)
    DIPjavaio     # builds the DIPjavaio module (plus the DIPlib library)
    PyDIP         # builds the PyDIP Python module (includes DIP, DIPviewer and
                  #    DIPjavaio targets)
    dum           # builds the DIPimage User Manual PDF

The `apidoc` target requires that *Doxygen* be installed, the target will not be available
if it is not. The `dum` target requires that *Pandoc* be installed, the target will not be
available if it is not; this target will fail to build if additional tools are not installed
(see below under "Dependencies").

Important `cmake` command-line arguments controlling the build of *DIPlib*:

    -DCMAKE_INSTALL_PREFIX=$HOME/dip   # choose an instal location for DIPlib, DIPimage and the docs
    -DCMAKE_BUILD_TYPE=Debug           # by default it is release
    -DDIP_SHARED_LIBRARY=Off           # build a static DIPlib library
    -DCMAKE_C_COMPILER=gcc-6           # specify a C compiler (for libics, LibTIFF, libjpeg and zlib)
    -DCMAKE_CXX_COMPILER=g++-6         # specify a C++ compiler (for everything else)
    -DCMAKE_CXX_FLAGS="-march=native"  # specify additional C++ compiler flags

    -DDIP_ENABLE_STACK_TRACE=Off       # disable stack trace generation on exception
    -DDIP_ENABLE_ASSERT=On             # enable asserts
    -DDIP_ENABLE_DOCTEST=Off           # disable doctest within DIPlib
    -DDIP_ENABLE_MULTITHREADING=Off    # disable OpenMP multithreading
    -DDIP_ENABLE_ICS=Off               # disable ICS file format support
    -DDIP_ENABLE_TIFF=Off              # disable TIFF file format support
    -DDIP_ENABLE_JPEG=Off              # disable JPEG file format support, also affects TIFF
    -DDIP_ENABLE_ZLIB=Off              # disable zlib compression support in ICS and TIFF
    -DDIP_ENABLE_FFTW=On               # enable the use of FFTW3
    -DDIP_ENABLE_UNICODE=Off           # disable UFT-8 strings within DIPlib
    -DDIP_ALWAYS_128_PRNG=On           # use the 128-bit PRNG code where 128-bit
                                       #    integers are not natively supported

Controlling the build of *DIPviewer*:

    -DDIP_BUILD_DIPVIEWER=Off          # don't build/install the DIPviewer module

Controlling the build of *DIPjavaio*:

    -DIP_BUILD_JAVAIO=Off              # don't build/install the DIPjavaio module
    -DJAVA_HOME=<path>                 # use the JDK in <path>
    -DBIOFORMATS_JAR=<path>/bioformats_package.jar
                                       # specify location of the Bio-Formats JAR file

Controlling the build of *DIPimage*:

    -DDIP_BUILD_DIPIMAGE=Off           # don't build/install the DIPimage MATLAB toolbox
    -DMatlab_ROOT_DIR=<path>           # compile DIPimage against MATLAB in <path>

Controlling the build of *PyDIP*:

    -DDIP_BUILD_PYDIP=Off              # don't build/install the PyDIP Python module
    -DPYDIP_INSTALL_PATH=$HOME/...     # choose an instal location for PyDIP (see below)
    -DPYBIND11_PYTHON_VERSION=3.6      # compile PyDIP against Python 3.6

Some of these options might not be available on your system. For example, if you don't have
MATLAB installed, the `DIP_BUILD_DIPIMAGE` option will not be defined. In this case, setting
it to `Off` will yield a warning message when running CMake.

Note that on some platforms, the Python module requires the *DIPlib* library to build as
a dynamic load library (`-DDIP_SHARED_LIBRARY=On`, which is the default).

The `PYDIP_INSTALL_PATH` option defaults to the system-wide site packages directory for the
selected version of Python. To obtain the user-specific site packages directory, use the
following shell command: `python3 -m site --user-site`. The output can be used for the PyDIP
installation path for users that cannot or do not want to install in the system-wide directory.
For example:
```bash
cmake /path/to/dip/root/directory -DCMAKE_INSTALL_PREFIX=$HOME/dip -DPYDIP_INSTALL_PATH=$(python3 -m site --user-site)
```

## Dependencies

Here we list all external dependencies needed to compile the various parts of the project. *DIPlib*
also depends on a few other external projects, whose sources are included in this repository (see
[`README.md`](README.md) under "License" for more information). Note that, with the exception of
dynamic linking to a few external libraries, none of these dependencies are required when using the
*DIPlib* library (that is, *DIPlib*'s public header files do not import headers from other projects).

If you have [*FFTW3*](http://www.fftw.org) installed, you can set the `DIP_ENABLE_FFTW`
*CMake* variable to have *DIPlib* use *FFTW3* instead of the built-in FFT algorithm.
*FFTW3* is more efficient, especially for image sizes that do not factor into small
numbers, but it has a copyleft license.

*DIPviewer* requires that *OpenGL* be available on your system (should come with the OS),
as well as one of [*FreeGLUT*](http://freeglut.sourceforge.net) or [*GLFW*](http://www.glfw.org).
On Windows, [*GLEW*](http://glew.sourceforge.net) is also required.

*DIPjavaio* requires that the Java 8 SDK (JDK 8) be installed. This module is intended as a
bridge to [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/), which you will need
to download separately. *Bio-Formats* has a copyleft license.

*DIPimage* requires that [*MATLAB*](https://www.mathworks.com/products/matlab.html) be installed
for compilation and execution (of course).
Optionally, you can install [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/) to
enable *DIPimage* to read many microscopy image file formats (type `help readim` in *MATLAB*,
after installing *DIPimage*, to learn more).

*PyDIP* requires that [*Python*](https://www.python.org) (preferably *Python3*) be installed.

To build the *DIPlib* documentation (HTML), [*Doxygen*](http://www.doxygen.org) is needed.
There is a chance it will only work on Unix-like systems (not yet tested under Windows).

Compiling the *DIPimage* User Manual (PDF) requires [*Pandoc*](https://pandoc.org),
[*pandoc-crossref*](https://hackage.haskell.org/package/pandoc-crossref), and
[*LaTeX*](http://www.tug.org/texlive/). Note that you'll need certain *LaTeX* packages,
such as `upquote`, that are not in the most basic set of packages. You can install these
through the *TeX Live* package manager.
