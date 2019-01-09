# Building the *DIPlib* project on Linux

Compiling *DIPlib* requires a few programs that often are not present on a bare Linux
installation. It depends on your distribution how you can obtain these. Below we give
instructions for Ubuntu (TODO: add instructions for other distros).

Everything here requires a shell (such as Bash), which typically runs inside a terminal
window. If you're using Linux we'll assume you know where to find this.

## Installing build tools and dependencies on Ubuntu

Type the following command in your shell:
```bash
    sudo apt install build-essential cmake git
```
This will install all the required tools. Depending on your version of Ubuntu, you'll
have a compiler that is too old to correctly build *DIPlib*. To see which version
of GCC you have, type:
```bash
    g++ --version
```
You want to see at least version 5.4, but later versions would be better. If your
version of GCC is older, you will need to manually install a newer version.

Since you're here, why not install some of the optional dependencies as well?
```bash
    sudo apt install zlib1g-dev libtiff-dev freeglut3-dev
```
`zlib` is needed for DIPlib to be able to read and write compresssed ICS files,
`libtiff` is needed for DIPlib to be able to read and write TIFF files, and
`freeglut` is needed to compile *DIPviewer*.

To compile the documentation yourself (not really necessary, you can read the
documentation directly in the header files), you need *Doxygen*:
```bash
    sudo apt install doxygen
```

And to compile the DIPimage User Manual you'll need *Pandoc* and the *crossref*
filter. The `pandoc` package satisfies the first requirement, but unfortunately Ubuntu
doesn't have pre-compiled binaries for the latter. I have not been able to easily
compile it from sources on Ubuntu (TODO: If anyone wants to help fill out this part
of the instructions, I'd be most grateful!)

This latter document also requires *LaTeX*:
```bash
    sudo apt install texlive-latex-base
```

Finally, even if the `python3` program is already installed, you might need to
install the *Python 3* header files and static library:
```bash
    sudo apt install python3-dev
```

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

This will install to `/usr/local`. If you prefer to install elsewhere, change the
`cmake` line with the following:
```bash
    cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/dip -DPYDIP_INSTALL_PATH=$(python3 -m site --user-site)
```
This will install DIPlib, DIPimage and the documentation under the `dip` directory
in your home directory, and PyDIP in the user site packages directory.

We recommend you additionally specify the `-DCMAKE_CXX_FLAGS="-march=native"`
option to `cmake`. This will enable additional optimizations that are specific
to your computer. Note that the resuling binaries will likely be slower on another
computer, and possibly not work at all.

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
