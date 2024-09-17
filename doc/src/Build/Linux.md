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


\page building_linux Building the *DIPlib* project on Linux

Compiling *DIPlib* requires a few programs that often are not present on a bare Linux
installation. It depends on your distribution how you can obtain these. Below we give
instructions for Ubuntu.

!!! todo
    Add instructions for other distros.

See \ref building_cmake for additional information on the build targets and *CMake* configuration options.

Everything here requires a shell (such as Bash), which typically runs inside a terminal
window. If you're using Linux we'll assume you know where to find this.


\section linux_init Installing build tools and dependencies on Ubuntu

Type the following command in your shell:
```bash
sudo apt install build-essential cmake git
```
This will install all the required tools. Depending on your version of Ubuntu, you might
have a compiler that is too old to correctly build *DIPlib*. To see which version
of *GCC* you have, type:
```bash
g++ --version
```
You want to see at least version 5.4, but later versions would be better. If your
version of *GCC* is older, you will need to manually install a newer version.

If you want to compile *DIPviewer*, you need to install `freeglut` as well:
```bash
sudo apt install freeglut3-dev
```

To compile the documentation yourself (note that the compiled documentation can be found
online), you need *dox++*. See \ref building_documentation for details.

Finally, even if the `python3` program is already installed, you might need to
install the *Python 3* header files and static library:
```bash
sudo apt install python3-dev
```


\section linux_git Cloning the repository

Next, get the source repository from *GitHub*:
```bash
mkdir ~/src
cd ~/src
git clone https://github.com/DIPlib/diplib.git
```
This creates a directory `src/diplib` in your home directory. Feel free to use any other
directory if you prefer.


\section linux_build Building

To build, run `cmake` and `make` from a build directory:
```bash
mkdir ~/src/diplib/target
cd ~/src/diplib/target
cmake ..
make -j check
make -j install
```

This will install to `/usr/local`. If you prefer to install elsewhere, change the
`CMAKE_INSTALL_PREFIX`. For example:
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME
```
This will install *DIPlib*, *DIPviewer*, *DIPjavaio*, *DIPimage* and the documentation
under the `lib`, `include` and `share` directories in your home directory.

Before running `make`, examine the output of `cmake` to verify all the features you need are enabled,
and that your chosen dependencies were found. This is also a good moment to disable the features that
you don't need. By default, everything will be build and installed, if the required external dependencies
are found: you need *FreeGLUT* or *GLFW* for *DIPviewer*, a *Java JDK* for *DIPjavaio*, *Python* for *PyDIP*, and
*MATLAB* for *DIPimage*. Add `-DDIP_BUILD_DIPVIEWER=Off`, `-DDIP_BUILD_JAVAIO=Off`, `-DDIP_BUILD_DIPIMAGE=Off`
or `-DDIP_BUILD_PYDIP=Off` to your *CMake* command to turn the corresponding component off, if you don't
need that component.

See \ref cmake_variables for a summary of all the *CMake* options to manually specify paths and configure your build.

*PyDIP* is installed separately through `pip`. Once the `install` target has finished building
and installing, run
```bash
make pip_install
```
Note that it is necessary to install the *DIPlib* libraries before running the `pip_install` target.

We recommend you additionally specify the `-DCMAKE_CXX_FLAGS="-march=native"`
option to `cmake`. This will enable additional optimizations that are specific
to your computer. Note that the resulting binaries will likely be slower on a different
computer, and possibly not work at all.

If you build a static version of the *DIPlib* library, *DIPimage* and *PyDIP* will not work
correctly.


\section linux_bioformats Enabling *Bio-Formats*

If building only *DIPimage* (the *MATLAB* toolbox), skip this section and instead follow the directions
you can read when you do `help readim` in *MATLAB* after installation.

First, make sure you have the *Java 8 SDK* (*JDK 8*) or later installed,
you can obtain it from the [Oracle website](http://www.oracle.com/technetwork/java/javase/downloads/index.html)
for commercial purposes, or from [jdk.java.net](https://jdk.java.net) for an open-source build.

When running *CMake* with the proper *JDK* installed, the *DIPjavaio* module becomes available.

Check the *CMake* output to see which *JNI* was found. It should match the version of Java found.
These two should be listed together, but the *JNI* output is only produced on first run.
Delete the `CMakeCache.txt` file to run `cmake` fresh and see all its output.

If the version of *JNI* found is not the one in the *JDK*, or if it is not found at all, add `-DJAVA_HOME=<path>`
to the `cmake` command line:
```bash
cmake .. -DJAVA_HOME=/opt/jvm/java-8-oracle
```
Note that these arguments to `cmake` must be combined with the arguments mentioned earlier, into a single,
long command line argument.


\section linux_dipimage Using *DIPimage*

Once the `install` target has finished building and installing the toolbox, start
*MATLAB*. Type the following command:
```matlab
addpath('/Users/<name>/dip/share/DIPimage')
```
This will make the toolbox available (replace `/Users/<name>/dip` with the
actual path you installed to).

To get started using *DIPimage*, read the \ref dipimage_user_manual,
and look through the help, starting at
```matlab
help DIPimage
```
Or start the GUI:
```matlab
dipimage
```

If you see error messages such as
```text
Invalid MEX-file '/Users/\<name>/dip/share/DIPimage/measure.mexa64':
/<path>/bin/glnxa64/../../sys/os/glnxa64/libstdc++.so.6: version `GLIBCXX_3.4.26' not found
(required by /Users/\<name>/dip/share/DIPimage/../../lib/libDIP.so)
```
then please read this next section.

\subsection linux_matlab_gcc  *MATLAB* and the *GCC* libraries

Skip this section if you do not have any problems running *DIPimage* functions.

*MATLAB* is built on Linux with a version of *GCC* that is typically various years behind the latest, and it ships
with copies of the *GCC* libraries so that it can run on older systems that have older versions of those libraries.
When building a MEX-file with a newer version of *GCC*, the MEX-file will depend on a newer
version of the *GCC* libraries than what *MATLAB* links against, and will therefore fail to load within *MATLAB*.

In [the documentation](https://www.mathworks.com/support/requirements/supported-compilers.html) they specify which
compiler versions are compatible with each version of *MATLAB*. This list includes the version of *GCC* used to build
that version of *MATLAB*, and a few older releases. That is, their solution to this issue is to tell customers to use
only those specific versions of *GCC* to build MEX-files. If you do want to follow *MATLAB*'s recommendation, then
install one of the supported compilers, and add tell CMake about them using the following command:
```bash
cmake . -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9
```
(replacing `gcc-9` and `g++-9` with the actual names of the compiler installed). Then rebuild the whole project.

However, there's a simpler solution. Given that you have a newer version of *GCC* installed than that used by *MATLAB*,
and given that the *GCC* libraries are perfectly backwards-compatible, one can simply delete the libraries distributed
with *MATLAB*, and have it use the ones in your system. In the directory `<matlabroot>/sys/os/glnxa64`, delete
the file `libstdc++.*`, and possibly also the files `libg2c.*` and `libgcc_s*` (or rather, rename them, so you can put
them back if things go sour). Note that you need to restart *MATLAB* after making this change. You can discover
what `<matlabroot>` is by typing `matlabroot` at the *MATLAB* command prompt.


\section linux_pydip Using *PyDIP*

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
The \ref pydip_user_manual is still quite short, but does contain some important
information to get you started.
