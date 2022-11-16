# Building *DIPlib*, *DIPimage* and *PyDIP*

To build the library you will need a C++14 compliant compiler and *CMake*.
See [Dependencies](https://diplib.org/diplib-docs/build_dependencies.html) for optional dependencies
that you can install to improve your *DIPlib* experience.

For a full list of targets and *CMake* configuration options, see [*CMake* configuration](https://diplib.org/diplib-docs/building_cmake.html).

## Linux, MacOS, Cygwin and other Unix-like systems

Use the following commands (from an empty directory) to build:
```bash
cmake <path_to_diplib_repository>
make -j check         # will build DIPlib and PyDIP, and run their unit tests
make -j install       # will install DIPlib and DIPimage
make -j pip_install   # will install PyDIP
```

It is also possible to build the *PyDIP* project separately, using a previously installed *DIPlib* library,
```bash
cmake <path_to_diplib_repository>/pydip/
make -j check         # will build PyDIP, and run its unit tests
make -j pip_install   # will install PyDIP
```

For step-by-step instructions for Ubuntu Linux, see
[Building the *DIPlib* project on Linux](https://diplib.org/diplib-docs/building_linux.html).

For step-by-step instructions for MacOS, see
[Building the *DIPlib* project on macOS](https://diplib.org/diplib-docs/building_macos.html).


## Windows

Unless you want to use *Cygwin* or *MinGW* (see above), we recommend *Microsoft Visual Studio 2019*
(version 16) or newer. You'll also need *CMake*.

Using *CMake-gui*, choose where the source directory is and where to build the binaries. Then
press "Configure" and select *Visual Studio*. Finally, press "Generate". You should now have
a *Visual Studio* solution file that you can open in *Visual Studio* and build as usual.

For step-by-step instructions, see [Building the *DIPlib* project on Windows](https://diplib.org/diplib-docs/building_windows.html).


## Linking against the library

If you are not using *CMake* to build your project, then you need to manually define some preprocessor macros
when linking against *DIPlib. See [Linking against the *DIPlib* library](https://diplib.org/diplib-docs/building_diplib.html#linking_diplib)
for details.
