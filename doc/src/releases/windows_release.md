\comment Windows Release

\comment (c)2022, Ronald Ligteringen.

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

# Instructions for Windows release

*DIPlib* can be used as a C++ library linked to C++ code, via a Matlab interface and via a Python interface. The Python interface is build separately and is made available via PyPi. The Matlab interface (a.k.a. *DIPimage*) is build for macOS and Windows and is made available via GitHub Releases <https://github.com/DIPlib/diplib/releases>. Users are encouraged to build their own release of DIPlib and select which interface should be build for which platform. To aid with the build on Windows this document describes the steps taken for creating the release of the *DIPImage* for Windows.

## Setting up the file-structure

The following file-structure is used for building the Windows release (not all directories and files are specified):

*Note: round corners indicate directories, sharp corners indicate files, second line is description*

```mermaid
graph
    root(C:) --> 1(diplib<br>root)
    1 --> 11(diplib<br>final build)
    1 --> 12(download)
    1 --> 13(images)
    1 --> 14(source)
    1 --> 15(fftw)
    1 --> 16(...)
    1 --> 18[bioformats_package.jar]
    11 --> 111(bin)
    11 --> 112(include)
    11 --> 113(lib)
    11 --> 114(share)
    12 --> 121[fftw-3.3.10.tar.gz]
    14 --> 141(diplib<br>git repo)
    14 --> 142(fftw-3.3.10)
    141 --> 1411(build)
    142 --> 1421(build)
    15 --> 151(include)
    15 --> 152(lib)
```

### Explanation

- all software related to the build of the release is stored under `C:\diplib`
- all downloads are stored in `download`
- *DIPlib* images are stored in `images`
- all unpacked software sources are stored in `source`
- the CMake build directory `build` is created inside the unpacked source
- the compiled dependencies are stored with their package-name (no version)
- the *DIPimage* build is stored in `diplib`

## Prerequisites

These instructions are written January 2022 and the following environment and programs have been used:
-. Windows 10 Enterprise, version 21H2
- Git, version 2.34.1 <https://git-scm.com/download/win>. Set global name and e-mail address in `Git Bash`:

   ```bash
   git config --global user.name "<Full name>"
   git config --global user.email "<e-mail address>"
   ```

- CMake, version 3.22.1 <https://cmake.org/download/#latest>
- Microsoft Visual Studio Community 2022 (64-bit), version 17.0.5 <https://visualstudio.microsoft.com/downloads/>. Select the C++ environment during installation
- OpenJDK, version 17.0.2 <https://jdk.java.net/17/>. It's sufficient to just unpack the `zip` file
- Matlab 2021b, version 9.11.0.1837725 <https://nl.mathworks.com/downloads>

Also the following dependencies have been downloaded:

- FFTW, version 3.3.10 <https://fftw.org/download.html>. Download the source as we will build our own static library (see [FFTW static library](#fftw-static-library))
- GLFW, version 3.3.6 <https://www.glfw.org/download.html>. Download the 64-bit Windows binaries as they contain a prebuild static library
- Freetype, version 2.11.1 <https://github.com/ubawurinna/freetype-windows-binaries/releases>. Download the `Source code (zip)` as is it contains a prebuild static library
- Bio-Formats package, version 6.8.1 <https://www.openmicroscopy.org/bio-formats/downloads/>. Download `bioformats_package.jar`

For testing the *DIPlib* images are used:

- *DIPlib* images <ftp://qiftp.tudelft.nl/software/DIPimage/images.zip>

### FFTW static library

To build the FFTW static libraries `Visual Studio` and `CMake` must be available.

For the FFTW library two versions need to be build: single precision `fftw3f.lib` and double precision `fftw3.lib` (for more information: <https://fftw.org/fftw3_doc/Precision.html>)

1. unpack the download source
1. start CMake and select the directory of the unpacked source which contains `CMakeLists.txt`
  ![CMake paths](cmake_paths.PNG)
1. press the <kbd>Configure</kbd> button and confirm the creation of the build directory. In the next window the generator will show the just install `Visual Studio` version which can be confirmed with the `Finish` button.
1. for **double precision** set and unset the variables in the main window as follows:
   - `BUILD_SHARED_LIBS` unset
   - `CMAKE_INSTALL_PREFIX` `c:\diplib\fftw`
   - `ENABLE_AVX` set
   - `ENABLE_AVX2` set
   - `ENABLE_SSE` set
   - `ENABLE_SSE2` set
   - `ENABLE_THREADS` set
   - `WITH_COMBINED_THREADS` set
1. again press the <kbd>Configure</kbd> button to update the new values
  ![CMake variables](cmake_variables.PNG)
1. press the <kbd>Generate</kbd> button to generate the build files for Visual Studio
1. close all running versions of Visual Studio and then press the <kbd>Open Project</kbd> button to start a new version of Visual Studio with the created build files
1. switch to Visual Studio and set the Solution Configuration to `Release`, see screenshot:
  ![vs release](vs_release.PNG)
1. right-click on the `INSTALL` solution in the Solution Explorer and select `Build`, see screenshot:
  ![vs install](vs_install.PNG)
  ![vs_build](vs_build.PNG)
1. return to CMake
1. for **single precision** only set the following variable:
   - `ENABLE_FLOAT` set
1. press the <kbd>Configure</kbd>
1. press the <kbd>Generate</kbd>
1. switch to Visual Studio and select the Reload option
1. right-click on the `INSTALL` solution

## Building *DIPimage*

The *DIPlib* source used for building *DIPimage* is retrieved from the GitHub-repository: <https://github.com/DIPlib/diplib>. [This document](windows_setup_git_ssh.md) describes briefly the steps to authenticate from Windows to GitHub with an ssh-keypair.

*to be continued*