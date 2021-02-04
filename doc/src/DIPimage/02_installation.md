\comment DIPlib 3.0

\comment (c)2017-2020, Cris Luengo.
\comment Based on original DIPimage user manual: (c)1999-2014, Delft University of Technology.

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


\page sec_dum_installing Installing *DIPimage*

This toolbox requires *MATLAB* R2008a (version 7.6) or later, though some functionality
might require newer versions. The official distributions are compiled with much more
recent versions of *MATLAB* and do not run on earlier versions.
The [download page](https://github.com/DIPlib/diplib/releases) on GitHub
should specify the versions of *MATLAB* a specific distribution is compatible
with.

You can choose to install the toolbox from source, or download the binary.

On all platforms, it is possible to have multiple versions of *DIPimage* installed
(this is not true for versions of *DIPimage* before 3.0).
But you can only add one of them to your *MATLAB* path.

\section sec_dum_installing_windows Windows installation

\subsection sec_dum_installing_windows_bindist Installation from binary distribution

To install *DIPimage*, simply run the installation program and follow the
directions in it. The tool will tell you to start *MATLAB* and type the
command

```matlab
run('C:\Program Files\dip\dipstart.m')
```

where `C:\Program Files\dip\` is the directory to which you installed
*DIPimage*. The script `dipstart.m`, executed this way, contains three
commands needed to initialise the toolbox (two if you didn't install the
images). These must be executed every time you start *MATLAB*. You can
modify (or create) a file `startup.m` in the directory to which *MATLAB*
starts up, to contain the `run` command above. The script `startup.m` is
executed automatically every time *MATLAB* starts.

\subsection sec_dum_installing_windows_source Installation from source

Clone the git repository (or download a ZIP file) from
[GitHub](https://github.com/DIPlib/diplib). The file `README_Windows.md`
in the root of the repository contains step-by-step instructions to
build *DIPimage*, including how to obtain the necessary tools and
dependencies. You can also
[read the instructions online](https://github.com/DIPlib/diplib/blob/master/README_Windows.md).

Once all the binaries are compiled, start *MATLAB* and type:

```matlab
addpath('C:\dip\share\DIPimage')
setenv('PATH',['C:\dip\bin',';',getenv('PATH')]);
```

assuming that `C:\dip\` was the root directory where the binaries were installed
to.

\section sec_dum_installing_linux UNIX/Linux installation

\subsection sec_dum_installing_linux_bindist Installation from binary distribution

!!! par "Not yet available"
    This will probably be a tar ball.

\subsection sec_dum_installing_linux_source Installation from source

Clone the git repository (or download a ZIP file) from
[GitHub](https://github.com/DIPlib/diplib). You will require a C++14
compiler (GCC and Clang are available on all platforms), and CMake.

The root directory of the git repository has a
[README.md](https://github.com/DIPlib/diplib/blob/master/README.md) file
that gives some directions on compiling. In principle this should suffice:

```bash
mkdir target
cd target
cmake /path/to/git/repository/root
make -j install
```

If you wish to install *DIPimage* in your home directory instead of `/usr`,
the CMake command could be:
```bash
cmake /path/to/git/repository/root -DCMAKE_INSTALL_PREFIX=/home/uname
```

Note that you need to have *MATLAB* installed on the build machine to build *DIPimage*.

Once all the binaries are compiled, start *MATLAB* and type:

```matlab
addpath('/usr/share/DIPimage')
```

or:

```matlab
addpath('/home/uname/share/DIPimage')
```

You can add this line to your `startup.m` file (preferably in `$HOME/matlab/`).

\section sec_dum_installing_macos MacOS installation

\subsection sec_dum_installing_macos_bindist Installation from binary distribution

!!! par "Not yet available"
    This might be a bundle, or a drag-drop disk image. A different option could be a Homebrew formula.

\subsection sec_dum_installing_macos_source Installation from source

Building from source under MacOS is identical to Linux, see the directions
in \ref sec_dum_installing_linux_source. The file `README_MacOS.md` in the root of the repository
contains step-by-step instructions to build *DIPimage*, including how to obtain
the necessary tools and dependencies. You can also
[read the instructions online](https://github.com/DIPlib/diplib/blob/master/README_MacOS.md).
