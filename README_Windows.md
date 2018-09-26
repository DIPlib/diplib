# Building the *DIPlib* project on Windows

Compiling *DIPlib* requires a few programs that do not come preinstalled on Windows.
We describe here how to obtain these programs, and how to use them to compile the
*DIPlib* project.

## *Visual Studio*

You can download the free MS Visual Studio Community Edition here:
https://www.visualstudio.com/vs/community/.

You need at least the 2015 edition. We used the 2017 edition. Download and install
as directed. Select the "Desktop C++ applications" option. Optionally, you can
select to install *Python 3* as well. You can do this if you don't have *Python* but
want to compile the *PyDIP* interface.

Make sure you select the *Git* option as well. If you don't have *Git* installed,
and cannot install it through *Visual Studio*, download and install it from here:
https://git-scm.com/downloads.

## *CMake*

You can obtain the latest *CMake* here: https://cmake.org/download/.

Again, download and install as directed.

## Cloning the repository

Next, get the source repository from *GitHub*. Open the "Git GUI" program on your
start menu, and select "Clone existing repository". The source location is

    https://github.com/DIPlib/diplib.git

Pick any directory on your system as destination. For example `src\DIPlib` in
your user directory. The standard clone type is OK to use. Click "Clone".

## Installing dependencies

Windows does not have a simple dependency installation system, so this step
involves a lot of manual labor. Fortunately, all these dependencies are optional,
so feel free to skip this section.

- Download the *ZLib* sources from here: https://zlib.net/. Build the static library as
per instructions, and note where it ends up.
If you intend to build a 64-bit version of *DIPlib* (recommended!) build a 64-bit
version of *ZLib*.

- Download the *LibTIFF* sources from here: http://www.simplesystems.org/libtiff/.
Build the static library as per instructions, and note where it ends up.
If you intend to build a 64-bit version of *DIPlib* (recommended!) build a 64-bit
version of *LibTIFF*.

- Download the *GLFW* binaries from here: http://www.glfw.org/download.html.
If you intend to build a 64-bit version of *DIPlib* (recommended!) get the 64-bit
version of *GLFW*. Extract the ZIP file and note the location.

- Download the *GLEW* binaries from here: https://sourceforge.net/projects/glew/
The ZIP file says "win32", but it includes the 64-bit binaries also. Extract the
ZIP file and note the location.

- TODO: *FFTW3*.

## Creating *Visual Studio* project files

Open the *CMake* program. Enter the name of the directory you cloned the repository
in (in your example, `C:\Users\<name>\src\DIPlib`); you can browse to this directory.
Under "Where to build the binaries" enter a different, new directory. For example
`target\DIPlib` in your user directory. Click on "Configure". A pop-up window will
ask you for which generator to use. You should select your version of *Visual Studio*
here. Make sure you select the **Win64** version. If you select the default version,
you will build 32-bit binaries (really, in 2017 we're still building 32-bit binaries
by default?). Also, the latest *MATLAB* versions no longer support 32-bit binaries,
so unless you select the 64-bit generator, you won't be able to build *DIPimage*.

You will see a series of options, and a configuration report at the bottom. Here you
can change options:

- `CMAKE_INSTALL_PREFIX` should be set to some directory where you want *DIPlib* and
friends installed. By default this is `C:\Program Files\DIPlib`. You can pick any
directory to your liking here. Note that the default directory requires administrator
privileges (I was not able to install there, even though I have the administrator
password). Let's say we select `C:\Users\<name>\DIPlib`.

- If `DIP_BUILD_DIPIMAGE` is not on the list, you don't have MATLAB installed, or you are
building 32-bit binaries.

- If `DIP_BUILD_DIPVIEWER` is not on the list, you need to specify the locations for
*GLFW* and *GLEW*. Click the "Advanced" check box, this will show additional parameters.
Look for `GLFW_INCLUDE_DIR` and `GLFW_LIBRARY`. For both of these, click the "..." button
at the right and navigate to where you extracted the *GLFW* ZIP file. Select the "include"
directory for the first parameter, and "lib-vc2015\glfw3.lib" for the second. Next, do
the same for `GLEW_INCLUDE_DIR` and `GLEW_LIBRARY_RELEASE`: select "include" for the
first parameter and "lib\Release\x64\glew32.lib" for the second. If the `GLEW_` parameters
are not on the list, click on "Configure" again after entering the *GLFW* library
location. This should populate the *GLEW* parameters.

- If you have *ZLib* compiled, look for `ZLIB_INCLUDE_DIR` and `ZLIB_LIBRARY_RELEASE`
(again, select the "Advanced" check box to see these). Set the first to the directory
that contains the *Zlib* sources, and the second to the static library that you compiled.
Check the box next to `LIBICS_USE_ZLIB`.

- If you have *LibTIFF* compiled, look for `TIFF_INCLUDE_DIR` and `TIFF_LIBRARY_RELEASE`
(again, select the "Advanced" check box to see these). Set them to the appropriate values.

Finally, click on "Generate" to create a *Visual Studio* solution file.

## Building

Find the file `DIPlib.sln` in the directory you selected as output for *CMake*, and
open it. Also, *CMake* will prompt you to open this file after it has generated it.

Opening the solution file will launch *Visual Studio*. In the "Solution Explorer" you'll
find a list of targets (if configured for *DIPimage*, there will be very many targets!).
The `DIP` target is the *DIPlib* library itself. To build the *DIPimage* toolbox and
*PyDIP* (the *Python* interface), use the `INSTALL` target. This target builds everything
and installs it in the destination directory you specified in *CMake*
(`C:\Users\<name>\DIPlib` in our example). This is the target you will want to build.

In the tool bar, make sure that "Release" and "x64" are selected (or x86 if you want to
build 32-bit binaries). Right-click on `INSTALL` and select "Build".

If everything works correctly, you will have:

- `C:\Users\<name>\DIPlib\lib`: `DIP.dll`, as well as `DIPviewer.dll` and `PyDIP` (the
*Python* module) if built.

- `C:\Users\<name>\DIPlib\include`: The *DIPlib* include files, which you'll need when
building your own C++ progrmas using *DIPlib*.

- `C:\Users\<name>\DIPlib\share\DIPimage`: The *DIPimage* toolbox for MATLAB.

## Using *DIPimage*

Once the `INSTALL` target has finished building and installing the toolbox, start
*MATLAB*. Type the following commands:
```matlab
    addpath('C:\Users\<name>\DIPlib\share\DIPimage')
    setenv('PATH',['C:\Users\<name>\DIPlib\lib',';',getenv('PATH')]);
```
This will make the toolbox available (replace `C:\Users\<name>\DIPlib` with the
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

Once the `INSTALL` target has finished building and installing, start *Python*.
Type the following commands:
```python
    import sys
    sys.path.append(r'C:\Program Files\DIPlib\lib')
    os.environ["PATH"] = os.environ["PATH"] + r'C:\Program Files\DIPlib\lib'
    import PyDIP as dip
    import PyDIP.PyDIPviewer as dv
```
(Replace `C:\Program Files\DIPlib\lib` with the actual path you installed to.)
The `os.chdir` command is necessary because we haven't installed *PyDIP* to a
directory that *Python* knows about. This will be fixed at some point.

To get started using *PyDIP*, look through the help, starting at
```python
    help(dip)
```
