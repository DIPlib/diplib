# Building the *DIPlib* project on macOS

Compiling *DIPlib* requires a few programs that do not come preinstalled on macOS.
Here we offer a simple way to install these programs.

See [`INSTALL.md`](INSTALL.md) for general concepts and additional information
on the compilation options.

We mostly use the command line here, which you will find in the terminal window. To
open a terminal window, press \<Command>-\<Space> to bring up the *Spotlight* search tool,
type `terminal`, and press \<Enter>. Alternatively, in *Finder*, go to the 'Applications'
folder, find the 'Utilities' folder in it, and the 'Terminal' app inside it.

## *XCode*

Since you're interested in compiling from source, we presume that you already have
*XCode* installed. If not, you can find it on the App Store. However, you will not
need all of *XCode*, the command line tools are what we're after. You can install them
by typing in a terminal window:
```bash
xcode-select --install
```
This will bring up a dialog box asking if you want to install the developer command
line tools.

The developer command line tools include `git`, `make`, compilers (`clang`) and linker.

However, `cmake` is not included in this package.

## *Homebrew*

To install *CMake*, we recommend you use *Homebrew*. If you don't have *Homebrew*
installed yet, type the following in a terminal window (or rather, copy-paste it):
```bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
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

To compile the documentation yourself (note that the compiled documentation can be found
online), you need *dox++*. See [`INSTALL_documentation.md`](INSTALL_documentation.md) for details.

Finally, macOS comes with *Python 2*. We recommend *Python 3*:
```bash
brew install python3
```

Other useful tools available through *Homebrew* are *Valgrind*, *QCacheGrind*, and
tools included in the `binutils` package.

## *OpenMP*

*Clang*, as provided with *XCode*, does support *OpenMP*, but does not provide the OpenMP library.
If you want to enable parallel processing within *DIPlib*, you have two options:

1. Install the OpenMP library for use with *XCode*'s *Clang*:
   ```bash
   brew install libomp
   ```

   You will need at least *CMake* version 3.12 for this to work (*Homebrew*'s version is suitable).

2. Install *GCC*:
   ```bash
   brew install gcc
   ```

In our experience, *Clang* is faster at compiling, but *GCC* usually produces slightly faster code.

## Cloning the repository

Next, get the source repository from *GitHub*:
```bash
mkdir ~/src
cd ~/src
git clone https://github.com/DIPlib/diplib.git
```
This creates a directory `src/diplib` in your home directory.

## Building

As explained in the [`INSTALL.md`](INSTALL.md) file, type
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
This will install *DIPlib*, *DIPviewer*, *DIPjavaio*, *DIPimage* and the documentation
under the `dip` directory in your home directory.

Examine the output of `cmake` to verify all the features you need are enabled, and your
chosen dependencies were found. The [`INSTALL.md`](INSTALL.md) file summarizes all the
CMake options to manually specify paths.

*PyDIP* is installed separately through `pip`. Once the `install` target has finished building
and installing, run
```bash
make pip_install
```

We recommend you additionally specify the `-DCMAKE_CXX_FLAGS="-march=native"`
option to `cmake`. This will enable additional optimizations that are specific
to your computer. Note that the resulting binaries will likely be slower on another
computer, and possibly not work at all.

If you build a static version of the *DIPlib* library, *DIPimage* and *PyDIP* will not work
correctly.

Finally, if you installed the `gcc` package because you want to use *OpenMP*,
add `-DCMAKE_C_COMPILER=gcc-8 -DCMAKE_CXX_COMPILER=g++-8` to the `cmake` command
line. By default, `cmake` will find the compiler that came with `XCode`. These
two options specify that you want to use the *GCC* compilers instead.
(**Note**: at the time of this writing, `gcc-8` was the executable installed by
the `gcc` package. This will change over time, as new versions of *GCC* are adopted
by HomeBrew. Adjust as necessary.)

## Enabling *Bio-Formats*

First, make sure you have the *Java 8 SDK* (*JDK 8*) installed, you can obtain it from the
[Oracle website](http://www.oracle.com/technetwork/java/javase/downloads/index.html) for commercial
purposes, or from [jdk.java.net](https://jdk.java.net) for an open-source build. Next, download
`bioformats_package.jar` from the [*Bio-Formats* website](https://www.openmicroscopy.org/bio-formats/).
You need to add the location of this file to the `cmake` command line using the `-DBIOFORMATS_JAR=<path>`
flag.

When running *CMake* with the proper *JDK* installed, the *DIPjavaio* module becomes available.

Check the *CMake* output to see which *JNI* was found. It should match the version of Java found.
These two should be listed together, but the *JNI* output is only produced on first run. Delete the
`CMakeCache.txt` file to run `cmake` fresh and see all its output.

Sometimes the version of *JNI* found is not the one in the *JDK*. For example, on my Mac it might find the *JNI*
that belongs to *Java 6*. In this case, add `-DJAVA_HOME=<path>` to the `cmake` command line:
```bash
cmake .. -DBIOFORMATS_JAR=$HOME/java/bioformats_package.jar -DJAVA_HOME=/Library/Java/JavaVirtualMachines/jdk1.8.0_121.jdk/Contents/Home/
```
Note that these arguments to `cmake` must be combined with the arguments mentioned earlier, into a single,
long command line argument.

If an application that links to *DIPjavaio* pops up a message box saying that you need to install the
legacy *Java 6*, then your *Java 8* is not configured properly.
[See here](https://oliverdowling.com.au/2014/03/28/java-se-8-on-mac-os-x/) for instructions
on how to set it up.

## Using *DIPimage*

Once the `install` target has finished building and installing the toolbox, start
*MATLAB*. Type the following command:
```matlab
addpath('/Users/<name>/dip/share/DIPimage')
```
This will make the toolbox available (replace `/Users/<name>/dip` with the
actual path you installed to).

To get started using *DIPimage*, read the
[*DIPimage User Manual*](https://diplib.org/diplib-docs/dipimage_user_manual.html),
and look through the help, starting at
```matlab
help DIPimage
```
Or start the GUI:
```matlab
dipimage
```

## Using *PyDIP*

Once the `pip_install` target has finished installing, start *Python*.
The following command will import the *PyDIP* package as `dip`, which is shorter to
type and mimics the namespace used in the C++ library:
```python
import diplib as dip
```

To get started using *PyDIP*, look through the help, starting at
```python
help(dip)
```
