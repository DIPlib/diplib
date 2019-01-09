# Building the *DIPlib* project on MacOS

Compiling *DIPlib* requires a few programs that do not come preinstalled on the Mac.
Here we offer a simple way to install these programs.

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

Since you're here, why not install some of the optional dependencies as well?
```bash
    brew install libtiff glfw
```
`libtiff` is needed for DIPlib to be able to read and write TIFF files, and
`glfw` is needed to compile *DIPviewer*. Note that ZLib is also a dependency,
but it comes with MacOS.

To compile the documentation yourself (not really necessary, you can read the
documentation directly in the header files), you need *Doxygen*:
```bash
    brew install doxygen
```

And to compile the DIPimage User Manual you'll need *Pandoc*:
```bash
    brew install pandoc pandoc-crossref
```

This latter document also requires *LaTeX*, which ideally you should get through
[*MacTeX*](http://www.tug.org/mactex/), but you can also choose to directly get the
[*TeX Live*](http://www.tug.org/texlive/) distribution, which is identical but misses
a few Mac-specific applications.

Finally, MacOS comes with *Python 2*. We recommend *Python 3*:
```bash
    brew install python3
```

Other useful tools available through *Homebrew* are *Valgrind*, *QCacheGrind*, and
tools included in the `binutils` package.

## *OpenMP*

*Clang*, as provided with *XCode*, does not support *OpenMP*. If you want to enable
parallel processing within *DIPlib*, you'll need a compiler that does. *Homebrew*
can help with that as well. We recommend that you install the `gcc` package.
There is also an `llvm` package, which contains *CLang* with *OpenMP*, but using
it is non-trivial as command names clash with those provided by the system.
In our experience, *Clang* is faster at compiling, but *GCC* usually produces
slightly faster code.

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
    cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/dip -DPYDIP_INSTALL_PATH=$(python3 -m site --user-site)
```
This will install DIPlib, DIPimage and the documentation under the `dip` directory
in your home directory, and PyDIP in the user site packages directory.

We recommend you additionally specify the `-DCMAKE_CXX_FLAGS="-march=native"`
option to `cmake`. This will enable additional optimizations that are specific
to your computer. Note that the resuling binaries will likely be slower on another
computer, and possibly not work at all.

Finally, if you installed the `gcc` package because you want to use *OpenMP*,
add `-DCMAKE_C_COMPILER=gcc-8 -DCMAKE_CXX_COMPILER=g++-8` to the `cmake` command
line. By default, `cmake` will find the compiler that came with `XCode`. These
two options specify that you want to use the *GCC* compilers instead.
(**Note**: at the time of this writing, `gcc-8` was the executable installed by
the `gcc` package. This will change over time, as new versions of GCC are adopted
by HomeBrew. Adjust as necessary.)

You can also do
```bash
    make apidoc
```

if you want to compile the DIPlib documentation.

## Using *DIPimage*

Once the `install` target has finished building and installing the toolbox, start
*MATLAB*. Type the following command:
```matlab
    addpath('/Users/<name>/dip/share/DIPimage')
```
This will make the toolbox available (replace `/Users/<name>/dip` with the
actual path you installed to).

To get started using *DIPimage*, look through the help, starting at
```matlab
    help DIPimage
```
Or start the GUI:
```matlab
    dipimage
```

## Using *PyDIP*

Once the `install` target has finished building and installing, start *Python*.
The following command will import the PyDIP package as `dip`, which is shorter to
type and mimics the namespace used in the C++ library:
```python
    import PyDIP as dip
```

To get started using *PyDIP*, look through the help, starting at
```python
    help(dip)
```
