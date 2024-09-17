\comment (c)2017-2024, Cris Luengo.

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

\page building_macos Building the *DIPlib* project on macOS

Compiling *DIPlib* requires a few programs that do not come preinstalled on macOS. Here we offer a simple way to
install these programs.

See \ref building_cmake for additional information on the build targets and *CMake* configuration options.

We mostly use the command line here, which you will find in the terminal window. To open a terminal window,
press **Command**{ .m-label .m-warning }-**Space**{ .m-label .m-warning } to bring up the *Spotlight* search tool,
type `terminal`, and press **Enter**{ .m-label .m-warning }.
Alternatively, in *Finder*, go to the 'Applications' folder, find the 'Utilities' folder in it,
and the 'Terminal' app inside it.


\section macos_silicon Computers with an Apple Silicon chip (M1 and M2 processors)

If you are building *DIPlib* for use in C++ or *Python* on a Apple Silicon computer, you don't need to do anything
special. The instructions below will result in native (arm64) binaries that will work well with the arm64 version
of *Python*, the arm64 version of *MATLAB*, and with your own arm64 programs. If you already have *Homebrew*
installed, make sure it is the Apple Silicon native version (installed in `/opt/homebrew/`, not in `/usr/local/`),
so that all libraries and so forth that you install use the same architecture.
Note that it is possible to run two versions of *Homebrew* side by side.

*MATLAB R2023b* and up can run natively on Apple Silicon (and we strongly recommend you install the Apple Silicon
version of *MATLAB*, not the Intel version, if you can). No special instructions for building *DIPimage* are needed.
However, the interface to *DIPviewer* (the `viewslice` function) depends on the *Java Native Access library*, and the version
that comes with *MATLAB* (as of R2023b) does not work on Apple Silicon machines. If you find that `viewslice` does
not work, see [this issue](https://github.com/DIPlib/diplib/issues/151).

If you are running a version of *MATLAB* for the x86_64 architecture (Intel), then you need to build *DIPlib*
and *DIPimage* for that same x86_64 architecture, see below.

If you are not sure what architecture your *MATLAB* installation is for, then find the *MATLAB* binary and
examine it with the `file` command (on the Terminal command line):
```bash
file /Applications/MATLAB_R2023b.app/Contents/MacOS/MATLAB
```
This will print out either "Mach-O 64-bit executable **x86_64**" or "Mach-O 64-bit executable **arm64**".

\subsection macos_silicon_cross_compile Cross-compiling for the x86_64 architecture on an Apple Silicon Mac.

There are two ways to cross-compile *DIPlib* for the x86_64 architecture. We recommend running all the instructions
below in x86_64 compatibility mode (through Rosetta 2, which is an emulation layer).
Rosetta 2 can be installed by launching any program that is built for the x86_64 architecture,
if you have an Intel-based *MATLAB* running, you already have Rosetta 2 installed. You can install Rosetta 2 manually
by typing the following in a terminal window:
```bash
softwareupdate --install-rosetta
```

Simply open a terminal in x86_64 emulation mode, and run all instructions below in that terminal.
In particular, installing *Homebrew* in such a terminal results in a x86_64 version of *Homebrew*,
which installs in `/usr/local/`.
All tools and libraries installed by this version of *Homebrew* will be for the x86_64 architecture.
You can open a terminal in x86_64 emulation mode in various ways, the simplest is to just type
```bash
arch -x86_64 zsh
```
This starts a new shell in the current terminal window. `exit` will exit this shell, returning you to the previous,
native shell in that same terminal window.

A second way to cross-compile *DIPlib* for the x86_64 architecture is by adding
`-DCMAKE_OSX_ARCHITECTURES=x86_64` to your `cmake` command (see ["Building"](#macos_building) below).
This will have all tools run in native mode, but cross-compile to produce x86_64 binaries.
This works, but I had trouble getting *CMake* to identify the right version of all the libraries,
and attempting to link to arm64 libraries, which of course doesn't work.
I was able to build *DIPimage* this way, by disabling all optional components that depend on external libraries.


\section macos_xcode *Xcode*

You can install *Xcode* from the App Store if you don't already have it installed.
However, you will not need all of *Xcode*, it is the command line tools that we're after.
You can install them by typing in a terminal window:
```bash
xcode-select --install
```
This will bring up a dialog box asking if you want to install the developer command line tools.

The developer command line tools include `git`, `make`, compilers (`clang`) and linker.

However, `cmake` is not included in this package.


\section macos_homebrew *Homebrew*

To install *CMake*, we recommend you use *Homebrew*.
If you don't have *Homebrew* installed yet, type the following in a terminal window (or rather, copy-paste it):

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

See [the *Homebrew* web site](https://brew.sh) for up-to-date instructions.

Once *Homebrew* is installed, the following will install *CMake* (`cmake`):
```bash
brew install cmake
```

If you want to compile *DIPviewer*, you need to install `glfw` as well:
```bash
brew install glfw
```

To compile the documentation yourself (note that the compiled documentation can be found online),
you need *dox++*. See \ref building_documentation for details.

Finally, if you have a version of macOS that is older than 12.0, then you have *Python 2* by default.
You need to install *Python 3* if you want to build and use *PyDIP*:
```bash
brew install python3
```

Other useful tools available through *Homebrew* are *Valgrind*, *QCacheGrind*,
and tools included in the `binutils` package, though we won't use any of them in this guide.


\section macos_openmp *OpenMP*

*Clang*, as provided with *Xcode*, does support *OpenMP*, but does not provide the *OpenMP* library.
If you want to enable parallel processing within *DIPlib*, you have two options:

1. Install the *OpenMP* library for use with *Xcode*'s *Clang*:

    ```bash
    brew install libomp
    ```

    You will need at least *CMake* version 3.12 for this to work (*Homebrew*'s version is suitable).

2. Install *GCC*:

    ```bash
    brew install gcc
    ```

In our experience, *Clang* is faster at compiling, but *GCC* usually produces slightly faster code.


\section macos_git Cloning the repository

Next, get the source repository from *GitHub*:
```bash
mkdir ~/src
cd ~/src
git clone https://github.com/DIPlib/diplib.git
```

This creates a directory `src/diplib` in your home directory. Feel free to use any other
directory if you prefer.


\section macos_build Building

To build, run `cmake` and `make` from a build directory:
```bash
mkdir ~/src/diplib/target
cd ~/src/diplib/target
cmake ..
make -j check
make -j install
```
This will install to `/usr/local`. This is also where *Homebrew* puts its stuff.
If you prefer to install elsewhere, change the `cmake` line with the following:
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/dip
```
This will install *DIPlib*, *DIPviewer*, *DIPjavaio*, *DIPimage* and the documentation under the `dip` directory
in your home directory. You can pick whatever installation directory makes sense to you, it doesn't matter where
things get installed.

Before running `make`, examine the output of `cmake` to verify all the features you need are enabled,
and that your chosen dependencies were found. This is also a good moment to disable the features that
you don't need. By default, everything will be build and installed, if the required external dependencies
are found: you need *GLFW* for *DIPviewer*, a *Java JDK* for *DIPjavaio*, *Python* for *PyDIP*, and 
*MATLAB* for *DIPimage*. Add `-DDIP_BUILD_DIPVIEWER=Off`, `-DDIP_BUILD_JAVAIO=Off`, `-DDIP_BUILD_DIPIMAGE=Off`
or `-DDIP_BUILD_PYDIP=Off` to your *CMake* command to turn the corresponding component off, if you don't
need that component.

See \ref cmake_variables for a summary of all the *CMake* options to manually specify paths and configure your build.

*PyDIP* is installed separately through `pip`. Once the `install` target has finished building and installing, run
```bash
make pip_install
```

Note that it is necessary to install the *DIPlib* libraries before running the `pip_install` target.
If this target fails with `pip` complaining about a non-existent file, one possible cause is building on a system
with a newer version of macOS and an older version of *Python*. The wheel will be created for the newer macOS, but
*Python* will expect a wheel matching the macOS version that *Python* was built on. Adding `-DCMAKE_OSX_DEPLOYMENT_TARGET=12`
to the `cmake` command and rebuilding the whole project would fix this issue (the 12 there being the version of
macOS used to build you *Python* binaries, the error message will show what version you need to build for).
\[Also, binaries built for an older macOS will work on a newer macOS, but not the other way around\].

We recommend you additionally specify the `-DCMAKE_CXX_FLAGS="-march=native"` option to `cmake`.
This will enable additional optimizations that are specific to your computer.
Note that the resulting binaries will likely be slower on a different computer, and possibly not work at all.

If you build a static version of the *DIPlib* library, *DIPimage* and *PyDIP* will not work correctly.

Finally, if you installed the `gcc` package because you want to use *OpenMP*,
add `-DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11` to the `cmake` command line.
By default, `cmake` will find the compiler that came with *Xcode*.
These two options specify that you want to use the *GCC* compilers instead.
(**Note**: at the time of this writing, `gcc-11` and `g++-11` were the executables installed by the `gcc` package.
This will change over time, as new versions of *GCC* are adopted by *HomeBrew*. Adjust as necessary.)


\section macos_bioformats Enabling *Bio-Formats*

If building only *DIPimage* (the *MATLAB* toolbox), skip this section and instead follow the directions
you can read when you do `help readim` in *MATLAB* after installation.

First, make sure you have the *Java 8 SDK* (*JDK 8*) or later installed,
you can obtain it from the [Oracle website](http://www.oracle.com/technetwork/java/javase/downloads/index.html)
for commercial purposes, or from [jdk.java.net](https://jdk.java.net) for an open-source build.

When running *CMake* with the proper *JDK* installed, the *DIPjavaio* module becomes available.

Check the *CMake* output to see which *JNI* was found. It should match the version of Java found.
These two should be listed together, but the *JNI* output is only produced on first run.
Delete the `CMakeCache.txt` file to run `cmake` fresh and see all its output.

Sometimes the version of *JNI* found is not the one in the *JDK*.
For example, on my Mac it might find the *JNI* that belongs to *Java 6*.
In this case, add `-DJAVA_HOME=<path>` to the `cmake` command line:
```bash
cmake .. -DJAVA_HOME=/Library/Java/JavaVirtualMachines/jdk1.8.0_121.jdk/Contents/Home/
```

Note that these arguments to `cmake` must be combined with the arguments mentioned earlier,
into a single, long command line argument.

If an application that links to *DIPjavaio* pops up a message box saying that you need to install the legacy *Java 6*,
then your *Java 8* is not configured properly.
[See here](https://oliverdowling.com.au/2014/03/28/java-se-8-on-mac-os-x/) for instructions on how to set it up.


\section macos_dipimage Using *DIPimage*

Once the `install` target has finished building and installing the toolbox, start *MATLAB*.
Type the following command:
```matlab
addpath('/Users/<name>/dip/share/DIPimage')
```
This will make the toolbox available (replace `/Users/<name>/dip` with the actual path you installed to).

To get started using *DIPimage*, read the \ref dipimage_user_manual, and look through the help, starting at
```matlab
help DIPimage
```
Or start the GUI:
```matlab
dipimage
```


\section macos_pydip Using *PyDIP*

Once the `pip_install` target has finished installing, start *Python*.
The following command will import the *PyDIP* package as `dip`, which is shorter to type and mimics the namespace
used in the C++ library:
```python
import diplib as dip
```

To get started using *PyDIP*, look through the help, starting at
```python
help(dip)
```
The \ref pydip_user_manual is still quite short, but does contain some important information to get you started.
