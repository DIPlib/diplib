\comment (c)2017-2022, Cris Luengo.

\comment Licensed under the Apache License, Version 2.0 [the "License"];
\comment you may not use this file except in compliance with the License.
\comment You may obtain a copy of the License at
\comment
\comment    http://www.apache.org/licenses/LICENSE-2.0
\comment
\comment Unless required by applicable law or agreed to in writing, software
\comment distributed under the License is distributed on an "AS IS" BASIS,
\comment WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\comment See the License for the specific language governing permissions and
\comment limitations under the License.


\page building_diplib Building


\section building_summary Building the *DIPlib* project

To build *DIPlib* you will need a C++14 compliant compiler and *CMake*.
See \subpage build_dependencies for optional dependencies that you can install to
improve your *DIPlib* experience.

A single *CMake* project builds the *DIPlib* library, *DIPviewer*, *DIPjavaio*, the *DIPimage* toolbox,
and the *PyDIP* package, as well as these documentation pages. In short,
```bash
cmake <path_to_diplib_repository>
make -j check         # will build DIPlib and PyDIP, and run their unit tests
make -j install       # will install DIPlib and DIPimage
make -j pip_install   # will install PyDIP
```
For a full list of targets and *CMake* configuration options, see \subpage building_cmake.

It is also possible to build the *PyDIP* project separately, using a previously installed *DIPlib* library,
```bash
cmake <path_to_diplib_repository>/pydip/
make -j check         # will build PyDIP, and run its unit tests
make -j pip_install   # will install PyDIP
```

The following are step-by-step build instructions:

1. \subpage building_linux
2. \subpage building_macos
3. \subpage building_windows
4. \subpage building_documentation


\section linking_diplib Linking against the *DIPlib* library

When writing a program that depends on *DIPlib* (or *DIPviewer* and/or *DIPjavaio*), the simplest solution
usually is to include its repository as a subproject, and in your *CMake* file do `add_subdirectory(diplib)`.
You then have the `DIP`, `DIPviewer` and `DIPjavaio` targets available (if configured correctly).

Alternatively you can use the installed *DIPlib* libraries. When using *CMake*, simply do
`find_package(DIPlib)`, then link against the imported `DIPlib::DIP`, `DIPlib::DIPviewer` and/or `DIPlib::DIPjavaio`
targets. See the [example *CMake* script](https://github.com/DIPlib/diplib/blob/master/examples/independent_project/CMakeLists.txt).

When using *CMake* in either of the ways above, it will automatically add the relevant include directories,
set the required compilation flags, and define the required macros by just linking to the relevant targets.

If you do not use *CMake*, there are several macros that you should define when building any program
that links against *DIPlib*:

- If *DIPlib* was build with the `DIP_SHARED_LIBRARY` flag not set, then you need to define the `DIP_CONFIG_DIP_IS_STATIC`
macro when compiling the code that links against it. Likewise, if the `DIP_ALWAYS_128_PRNG` flag was set,
then you must define a `DIP_CONFIG_ALWAYS_128_PRNG` macro when compiling your program. Mismatching this flag
could cause your program to not link, or worse, crash at runtime.

- The following flags do not need to be matched, but they should be if you want the inline functions to behave
the same as the pre-compiled ones:
    - If the flag `DIP_ENABLE_STACK_TRACE` is set, define the macro `DIP_CONFIG_ENABLE_STACK_TRACE`.
    - If the flag `DIP_ENABLE_ASSERT` is set, define the macro `DIP_CONFIG_ENABLE_ASSERT`.

- If your compiler supports `__PRETTY_FUNCTION__`, set the macro `DIP_CONFIG_HAS_PRETTY_FUNCTION` to
get better stack traces.

- For *DIPviewer*, if `DIP_SHARED_LIBRARY` was not set, define the `DIP_CONFIG_DIPVIEWER_IS_STATIC` macro.
Also define `DIP_CONFIG_HAS_FREEGLUT` or `DIP_CONFIG_HAS_GLFW` depending on which back-end is used.

- For *DIPjavaio*, if `DIP_SHARED_LIBRARY` was not set, define the `DIP_CONFIG_DIPJAVAIO_IS_STATIC` macro.
Also define `DIP_CONFIG_HAS_DIPJAVAIO` for the function `dip::ImageRead` to be able to make use of it.


\comment -------------------------------------------------------


\page building_cmake *CMake* configuration

\section cmake_targets Available targets

- `DIP`:           builds the *DIPlib* library
- `DIPviewer`:     builds the *DIPviewer* module (plus the *DIPlib* library)
- `DIPjavaio`:     builds the *DIPjavaio* module (plus the *DIPlib* library)
- `PyDIP`:         builds the *PyDIP* *Python* module (includes the `DIP`, `DIPviewer` and `DIPjavaio` targets)
- `dipview`:       builds the standalone `dipview` image viewer program
- `dipviewjava`:   builds the *DIPjavaio*-enabled standalone `dipviewjava` image viewer program
- `all`: (default) builds all of the targets above that were configured
- `install`:       builds `all` and installs everything except *PyDIP*
- `check`:         builds the `unit_tests` program and runs it
- `check_memory`:  builds the `unit_tests` program and runs it under `valgrind`
- `doc`:           builds all the HTML documentation
- `examples`:      builds the example programs
- `package`:       creates a distributable package for `install` [note: not fully functional]
- `bdist_wheel`:   builds a *Python* wheel for *PyDIP*
- `pip_install`:   builds and installs a *Python* wheel for *PyDIP*
- `pip_uninstall`: uninstalls *PyDIP*

The `doc` target requires that [*dox++*](https://crisluengo.github.io/doxpp/) is available,
the target will not be available if it is not; this target will fail to build if additional tools are not installed.
See \ref building_documentation for details.

The `pip_install` target requires the *DIPlib* libraries to already be installed, so always run the `install` target
first.

\section cmake_variables *CMake* variables

These variables can be set on the `cmake` command line with `-D<variable>=<value>`.
Alternatively, set them in the *CMake* GUI.

Important *CMake* variables controlling the build of *DIPlib*:

- `CMAKE_INSTALL_PREFIX`: Choose an install location for all the components except *PyDIP*. A directory
    structure will be created, including the POSIX-standard `lib`, `include` and `share` directores.
- `CMAKE_BUILD_TYPE`: Set to `Release` (default), `Debug` or `Sanitize` (GCC or Clang only). On Windows this is
    ignored, choose the build type in *MSVC*.
- `DIP_SHARED_LIBRARY`: `On` (default) or `Off`. Leave `On` to build a shared *DIPlib* library, set to `Off`
    to build a static *DIPlib* library. Note that *DIPimage* and *PyDIP* require a shared library.
- `CMAKE_C_COMPILER`: Specify a C compiler for the dependencies *libics*, *LibTIFF*, *libjpeg* and *zlib*.
- `CMAKE_CXX_COMPILER`: Specify a C++ compiler. The system's default C and C++ compilers are used by default.
- `CMAKE_CXX_FLAGS`: Specify additional C++ compiler flags. For example, with GCC or Clang you could set this
    to `"-march=native"` to optimize the build for the current machine.
- `DIP_ENABLE_STACK_TRACE`: `On` (default) or `Off`. In *DIPlib*, exceptions generate a stack trace,
    which you might want to disable in some circumstances.
- `DIP_ENABLE_ASSERT`: `On` or `Off` (default). Typically one would enable asserts in debug builds only.
- `DIP_ENABLE_DOCTEST`: `On` (default) or `Off`. The unit tests are embedded in *DIPlib* code through *doctest*,
    and will end up in the library unless disabled.
- `DIP_ENABLE_MULTITHREADING`: `On` (default) or `Off`. *DIPlib* uses *OpenMP* for multithreading if available
    on the system.
- `DIP_ENABLE_ICS`: `On` (default) or `Off`. Enable support for the ICS file format using the included *libics* library.
- `DIP_ENABLE_TIFF`: `On` (default) or `Off`. Enable support for the TIFF file format using the included *LibTIFF* library.
- `DIP_ENABLE_JPEG`: `On` (default) or `Off`. Enable support for the JPEG file format using the included *libjpeg* library.
- `DIP_ENABLE_PNG`: `On` (default) or `Off`. Enable support for the PNG file format using the included *libspng* library.
- `DIP_ENABLE_ZLIB`: `On` (default) or `Off`. Enable support for ZIP (deflate) compression in the ICS and TIFF file
    formats using the included *zlib* library.
- `DIP_ENABLE_FFTW`: `On` or `Off` (default). Enable the use of the *FFTW3* library, if available.
- `DIP_ENABLE_FREETYPE`: `On` or `Off` (default). Enable the use of the *FreeType2* library, if available.
- `DIP_ENABLE_UNICODE`: `On` (default) or `Off`. Enable support for UTF-8 strings within *DIPlib*.
- `DIP_ALWAYS_128_PRNG`: `On` or `Off` (default). If `On`, use the 128-bit PRNG code even if 128-bit integers
    are not natively supported.

Controlling the build of *DIPviewer*:

- `DIP_BUILD_DIPVIEWER`: `On` (default) or `Off`. Choose whether to build and install the *DIPviewer* module, if
    its dependencies are available.

Controlling the build of *DIPjavaio*:

- `DIP_BUILD_JAVAIO`: `On` (default) or `Off`. Choose whether to build and install the *DIPjavaio* module,
    if its dependencies are available.
- `JAVA_HOME`: Set the path to the JDK to use.
- `DIP_JAVA_VERSION`: Set the version of Java to target. Defaults to "1.8".
- Note that for the *MATLAB* toolbox, *DIPjavaio* is not used; build this only if you work in C++ or *Python*.

Controlling the build of *DIPimage*:

- `DIP_BUILD_DIPIMAGE`: `On` (default) or `Off`. Choose whether to build and install the *DIPimage* toolbox,
    if *MATLAB* is available.
- `Matlab_ROOT_DIR`: Set the path of the *MATLAB* installation to compile *DIPimage* against.
- `JAVA_HOME`: Set the path to the JDK to use. Java is used by *DIPimage* to interface with *DIPviewer*.
- `DIP_JAVA_VERSION`: Set the version of Java to target. Defaults to "1.8". For *MATLAB* versions prior to R2017b
    use "1.7" (and set `JAVA_HOME` to a release of the JDK that supports 1.7). You should not target a Java version
    later than the one used by your *MATLAB*. Check the version of Java used by *MATLAB* by running `version -java`
    on the *MATLAB* command prompt.

Controlling the build of *PyDIP*:

- `DIP_BUILD_PYDIP`: `On` (default) or `Off`. Choose whether to build the *PyDIP* package, if *Python* is available.
- `Python_EXECUTABLE`: Set the path to the *Python* binary, in case *CMake* doesn't find the right one. It is also
    possible to instead set `Python_ROOT_DIR`, see [*FindPython*](https://cmake.org/cmake/help/latest/module/FindPython.html).
- `DIP_PYDIP_WHEEL_INCLUDE_LIBS`: `On` or `Off` (default). Include the *DIPlib* libraries in the *PyDIP* wheel.
    Turn on for binary distribution, keep off for personal builds.

Some of these options might not be available on your system. For example, if you don't have
*MATLAB* installed, the `DIP_BUILD_DIPIMAGE` option will not be defined. In this case, setting
it to `Off` will yield a warning message when running *CMake*.


\comment -------------------------------------------------------


\page build_dependencies Dependencies

Here we list all external dependencies needed to compile the various parts of the project.
*DIPlib* also depends on a few other external projects, whose sources are included in this repository (see
[`README.md`](https://github.com/DIPlib/diplib/blob/master/README.md) under "License" for more information).
Note that, with the exception of dynamic linking to a few external libraries, none of these dependencies
are required when using the *DIPlib* library (that is, *DIPlib*'s public header files do not import headers
from other projects).

If you have [*FFTW3*](http://www.fftw.org) installed, you can set the `DIP_ENABLE_FFTW`
*CMake* variable to have *DIPlib* use *FFTW3* instead of the default *PocketFFT* library.
*FFTW3* is usually a bit faster, but it has a copyleft license.

If you have [*FreeType 2*](https://www.freetype.org) installed, you can set the `DIP_ENABLE_FREETYPE`
*CMake* variable to have *DIPlib* use *FreeType* for rendering text, the
[`dip::FreeTypeTool`](https://diplib.org/diplib-docs/dip-FreeTypeTool.html)
class (introduced in *DIPlib* 3.1.1) cannot be instantiated without it.

*DIPviewer* requires that *OpenGL* be available on your system (should come with the OS),
as well as one of [*FreeGLUT*](http://freeglut.sourceforge.net) or [*GLFW*](http://www.glfw.org).

*DIPjavaio* requires that the *Java 8 SDK* (*JDK 8*) or later be installed. This module is intended as a
bridge to [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/), which you will need
to download separately. *Bio-Formats* has a copyleft license. Note that the *MATLAB* toolbox *DIPimage*
does not use this component, it uses *Bio-Formats* directly.

*DIPimage* requires that [*MATLAB*](https://www.mathworks.com/products/matlab.html) be installed
for compilation and (of course) execution.
Optionally, you can install [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/) to
enable *DIPimage* to read many microscopy image file formats (type `help readim` in *MATLAB*,
after installing *DIPimage*, to learn more).

*PyDIP* requires that [*Python 3*](https://www.python.org) be installed.
