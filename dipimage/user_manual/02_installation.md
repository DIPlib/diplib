Installing *DIPimage*
=====================

This toolbox requires (probably) *MATLAB* 7.0 or later, though some functionality
requires newer versions. The official distributions are compiled with much more
recent versions of *MATLAB* and do not run on earlier versions.
The [download page](http://www.diplib.org/download) on the *DIPimage* web site
should specify the versions of *MATLAB* a specific distribution is compatible
with.

You can choose to install the toolbox from source, or download the binary.

On all platforms, it is possible to have multiple versions of *DIPimage* installed
(this is not true for the older versions of *DIPimage*, before 3.0).
But you can only add one of them to your *MATLAB* path.

Windows Installation
--------------------

### Installation from binary distribution

To install *DIPimage*, simply run the installation program and follow the
directions in it. The tool will tell you to start *MATLAB* and type the
command
```
    run('C:\Program Files\dip\dipstart.m')
```
where `C:\Program Files\dip\` is the directory to which you installed
*DIPimage*. The script `dipstart.m`, executed this way, contains three
commands needed to initialise the toolbox (two if you didn't install the
images). These must be executed every time you start *MATLAB*. You can
modify (or create) a file `startup.m` in the directory to which *MATLAB*
starts up, to contain the `run` command above. The script `startup.m` is
executed automatically every time *MATLAB* starts.

### Installation from source

Clone the git repository (or download a ZIP file) from
[GitHub](https://github.com/DIPlib/diplib). You will require a very
recent version of Visual Studio (we used the 2017 Community edition, which
is free), and CMake (also free).

Using CMake-gui, choose where the source directory is and where to build the
binaries. Then press "Configure" and select Visual Studio. Finally, press
"Generate". You should now have a Visual Studio solution file that you can
open in Visual Studio and build as usual. Note that you need to have *MATLAB*
installed on the build machine to build *DIPimage*.

Once all the binaries are compiled, start *MATLAB* and type:
```
    addpath('C:\dip\share\dipimage')
    setenv('PATH',['C:\dip\lib',';',getenv('PATH')]);
```
assuming that `C:\dip\` was the root directory where the binaries were installed
to.

UNIX/Linux Installation
-----------------------

### Installation from binary distribution

*This will probably be a DEB package and an RPM package*.

### Installation from source {#sec:unix_compile}

Clone the git repository (or download a ZIP file) from
[GitHub](https://github.com/DIPlib/diplib). You will require a C++14
compiler (GCC and Clang are available on all platforms), and CMake.

The root directory of the git repository has a
[README.md](https://github.com/DIPlib/diplib/blob/master/README.md) file
that gives some directions on compiling. In principle this should suffice:
```
    mkdir target
    cd target
    cmake /path/to/git/repository/root
    make -j install
```

If you wish to install *DIPimage* in your home directory instead of `/usr`,
the CMake command could be:
```
    cmake /path/to/git/repository/root -DCMAKE_INSTALL_PREFIX=/home/uname
```

Note that you need to have *MATLAB* installed on the build machine to build *DIPimage*.

Once all the binaries are compiled, start *MATLAB* and type:
```
    addpath('/usr/share/dipimage')
```
or:
```
    addpath('/home/uname/share/dipimage')
```

You can add this line to your `startup.m` file (preferably in `$HOME/matlab/`).

MacOS X Installation
--------------------

### Installation from binary distribution

If you already have a version of *DIPimage* installed, rename the
directory it is in, so that you will still have the old version if the
installation of the new version fails. Extract the distribution file. It
will create a directory `dip/` with a number of subdirectories. We
suggest you place this directory in `/Applications/`, so that you now
have a directory `/Applications/dip/`[^21].

Next, start *MATLAB* and type:
```
    addpath('/Applications/dip/share/dipimage')
```

You can add this line to your `startup.m` file (preferably in `$HOME/matlab/`).

### Installation from source

See the directions in [@sec:unix_compile].


[^21]: if you choose to install *DIPimage* in a different location change
    this in the following instuctions accordingly
