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


\page building_windows Building the *DIPlib* project on Windows

Compiling *DIPlib* requires a few programs that do not come preinstalled on Windows.
We describe here how to obtain these programs, and how to use them to compile the
*DIPlib* project.

See \ref building_cmake for additional information on the build targets and *CMake* configuration options.


\section windows_msvc *Visual Studio*

You can download the free *MS Visual Studio Community Edition*
[from its website](https://www.visualstudio.com/vs/community/).

We recommend that you use at least the 2019 edition. We were able to build with the
2015 and 2017 editions, but had to turn off some components that would not build because
they used some advanced C++14 constructs that will not compile with older versions of *MSVC*.

Download and install as directed. Select the "Desktop C++ applications" option.
Optionally, you can select to install *Python 3* as well. You can do this if you
don't have *Python* but want to compile the *PyDIP* interface.

Make sure you select the *Git* option as well. If you don't have *Git* installed,
and cannot install it through *Visual Studio*,
[download and install it from here](https://git-scm.com/downloads).


\section windows_cmake *CMake*

You can obtain the latest *CMake* [on its website](https://cmake.org/download/).

Again, download and install as directed.


\section windows_doxpp *dox++*

To compile the documentation yourself (which shouldn't be necessary, but some IDEs
might require), you need *dox++*. See \ref building_documentation for details.


\section windows_git Cloning the repository

Next, get the source repository from *GitHub*. Open the "Git GUI" program on your
start menu, and select "Clone existing repository". The source location is

    https://github.com/DIPlib/diplib.git

Pick any directory on your system as destination. For example `src\DIPlib` in
your user directory. The standard clone type is OK to use. Click "Clone".


\section windows_dependencies Installing dependencies

Windows does not have a simple dependency installation system, so this step
involves a lot of manual labor. Fortunately, all these dependencies are optional,
so feel free to skip this section.

- Download [the *GLFW* binaries](http://www.glfw.org/download.html).
If you intend to build a 64-bit version of *DIPlib* (recommended!) get the 64-bit
version of *GLFW*. Extract the ZIP file and note the location.

- Download [the *Bio-Formats* library](https://www.openmicroscopy.org/bio-formats/downloads).
Put it somewhere sensible and note the location. The same location (as well as the
library installation path) will be used to find it during execution.

!!! todo
    Describe how to get *FFTW3*.


\section windows_configure Creating *Visual Studio* project files

Open the *CMake* program. Enter the name of the directory you cloned the repository
in (in our example, `C:\Users\<name>\src\DIPlib`); you can browse to this directory.
Under "Where to build the binaries" enter a different, new directory. For example
`target\DIPlib` in your user directory. Click on "Configure". A pop-up window will
ask you for which generator to use. You should select your version of *Visual Studio*
here. Make sure you select the **Win64** version. If you select the default version,
you will build 32-bit binaries (really, in 2019 we're still building 32-bit binaries
by default?). Also, the latest *MATLAB* versions no longer support 32-bit binaries,
so unless you select the 64-bit generator, you won't be able to build *DIPimage*.

You will see a series of options, and a configuration report at the bottom. Here you
can change options:

- `CMAKE_INSTALL_PREFIX` should be set to some directory where you want *DIPlib* and
friends installed. By default this is `C:\Program Files\DIPlib`. You can pick any
directory to your liking here. Note that the default directory requires administrator
privileges (I was not able to install there, even though I have the administrator
password). Let's say we select `C:\Users\<name>\DIPlib`.

- If `DIP_BUILD_DIPIMAGE` is not on the list, you don't have *MATLAB* installed, or you are
building 32-bit binaries.

- If `DIP_BUILD_DIPVIEWER` is not on the list, you need to specify the locations for
*GLFW*. Click the "Advanced" check box, this will show additional parameters.
Look for `GLFW_INCLUDE_DIR` and `GLFW_LIBRARY`. For both of these, click the "..." button
at the right and navigate to where you extracted the *GLFW* ZIP file. Select the "include"
directory for the first parameter, and "lib-vc2015\glfw3.lib" for the second.

- If `DIP_BUILD_DIPVIEWER_JAVA` is not on the list but `DIP_BUILD_DIPVIEWER` is, you
don't have *MATLAB* installed, you are building 32-bit binaries or the *Java SDK* could not
be found. This is only necessary if you want to use the `viewslice` command from *DIPimage*.

- If `BIOFORMATS_JAR` is not on the list, the *Java SDK* could not be found. If it is,
point it to the "bioformats_package.jar" you downloaded earlier. This is only necessary if
you want to import image formats that are not directly supported by *DIPlib*.

- If using a version of *MSVC* older than the 2019 edition, set both `DIP_ENABLE_UNICODE`
and `DIP_ENABLE_DOCTEST` to `Off`. You will get compilation errors if you don't do this.

- If you set `DIP_SHARED_LIBRARY` to `Off`, *DIPimage* and *PyDIP* will likely not work
correctly.

Finally, click on "Generate" to create a *Visual Studio* solution file.


\section windows_build Building

Find the file `DIPlib.sln` in the directory you selected as output for *CMake*, and
open it. Also, *CMake* will prompt you to open this file after it has generated it.

Opening the solution file will launch *Visual Studio*. In the "Solution Explorer" you'll
find a list of targets (if configured for *DIPimage*, there will be very many targets!).
The `DIP` target is the *DIPlib* library itself. To build the *DIPimage* toolbox, use
the `INSTALL` target. This target builds everything
and installs it in the destination directory you specified in *CMake*
(`C:\Users\<name>\DIPlib` in our example). This is the target you will want to build.
To additionally install *PyDIP*, use the `pip_install` target (but note that you need
to run the `INSTALL` target first).

In the tool bar, make sure that "Release" and "x64" are selected (or x86 if you want to
build 32-bit binaries). Right-click on `INSTALL` and select "Build".

If everything works correctly, you will have:

- `C:\Users\<name>\DIPlib\bin`: `DIP.dll`, as well as `DIPviewer.dll`,
`DIPjavaio.dll`, `DIPjavaio.jar`, `dipview.exe` and `dipviewjava.exe`.

- `C:\Users\<name>\DIPlib\lib`: `DIP.lib`, as well as `DIPviewer.lib` and `DIPjavaio.lib`.

- `C:\Users\<name>\DIPlib\include`: The *DIPlib* include files, which you'll need when
building your own C++ programs using *DIPlib*.

- `C:\Users\<name>\DIPlib\share\DIPimage`: The *DIPimage* toolbox for *MATLAB*.

- In your selected *Python* package path: `diplib` (the *Python* module).


\section windows_dipimage Using *DIPimage*

Once the `INSTALL` target has finished building and installing the toolbox, start
*MATLAB*. Type the following commands:
```matlab
addpath('C:\Users\<name>\DIPlib\share\DIPimage')
setenv('PATH',['C:\Users\<name>\DIPlib\bin',';',getenv('PATH')]);
```
This will make the toolbox available (replace `C:\Users\<name>\DIPlib` with the
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


\section windows_pydip Using *PyDIP*

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
