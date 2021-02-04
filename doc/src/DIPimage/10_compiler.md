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


\page sec_dum_matlab_compiler *DIPimage* and the *MATLAB Compiler*

!!! warning
    This section hasn't been updated yet for *DIPimage 3*.
    Most of the information here is no longer correct.

\section sec_dum_matlab_compiler_intro The *MATLAB Compiler*

Since *MATLAB* version 7.0 (Release 14), the *MATLAB Compiler* no longer
generates C or C++ code from M-files. Instead, it packages all M-files
and MEX-files into a *Component Technology File* (CTF) archive,
generates a small stub executable, and requires the end user to install
the *MATLAB* Component Runtime (MCR). This MCR is the *MATLAB* interpreter
(but without licensing restrictions). The upside of this is that there
are no longer limitations as to what M-files can be compiled, meaning it
is now possible to create standalone applications that use *DIPimage*. The
downside is that, since code is not really compiled, there is no
performance benefit to compiling.

The *MATLAB* compiler can generate shared objects (dynamically linked
libraries) as well as executables. This means that it is still possible
to compile M-file code so that it can be called from your own C or C++
code, even though this compiled M-file code can no longer be statically
linked into your executable.

M-files in the CTF archive are encrypted so that it is not possible to
obtain source code from the compiled application. MEX-files are also
protected in some way so they cannot be run outside of the deployed
application. Therefore, even though the code is not truly compiled, your
code is reasonably well protected against reverse-engineering.

The explanations below are for Linux/UNIX systems. If you use Windows,
similar issues will have to be taken into account. The [*MATLAB Compiler*
User's Guide](https://www.mathworks.com/help/pdf_doc/compiler/compiler.pdf)
contains all the information needed to compile an M-file that uses
*DIPimage*.

\section sec_dum_matlab_compiler_compiling Compiling an M-file that uses *DIPimage*

Please first read the *MATLAB Compiler* User's Guide, and make sure you
are able to generate the `magicsquare.m` stand-alone example application
using the `mcc` command (not through the `deploytool` GUI, since the
explanations below assume you are familiar with `mcc`).

There are a few things that have to be taken into account when your
M-file uses *DIPimage*. First, like with other toolboxes, the *DIPimage*
directory must not be added to the *MATLAB* path through the `startup.m`
file (as suggested in the *DIPimage* installation instructions), but
through the `mcc` command line. Second, instead of calling
`dip_initialise`, call `dip_initialise_libs`.

`dip_initialise` searches for the correct version of the *DIPimage*
toolbox to use, depending on the *MATLAB* version you are running. It then
adds the necessary paths and calls `dip_initialise_libs`. Since this
process doesn't work with the *MATLAB Compiler*, you will need to do these
two steps separately.

Hence, you need to create a special version of your `startup.m` file in
the directory where your application M-file lives. Remove all the
`addpath` instructions, and change the line `dip_initialise` into
`dip_initialise_libs` (if you do not want the *DIPlib* version information
to be displayed on startup, you can use the `'silent'` argument to
`dip_initialise_libs`). Alternatively, you can call the
`dip_initialise_libs` function in your application M-file. In this case,
make and empty `startup.m` file to avoid your default one to be used.

To find out which directories you need to add to the Compiler search
path, type `path` on the *MATLAB* command line. It should return a long
list of directories, three of which look like this:

```bash
/something/dip/common/mlv7_4/diplib
/something/dip/common/mlv7_4/dipimage_mex
/something/dip/common/dipimage
```

These three paths can be added the the `mcc` command line using the
'`-I`' argument:

```bash
mcc -m myapplication.m ...
    -I /something/dip/common/dipimage ...
    -I /something/dip/common/mlv7_4/dipimage_mex ...
    -I /something/dip/common/mlv7_4/diplib
```

Under some circumstances, `mcc` might give a warning telling you that
the `dip_initialise_libs` command is unknown. However, when running the
resulting executable, *DIPlib* gets initialized just fine. This must be
due to the order in which paths get added and commands are executed.

When running the stand-alone application you just created, the three
*DIPlib* shared libraries must be on the `LD_LIBRARY_PATH` environment
variable, as discussed in \ref sec_dum_installing_linux. It is possible to
edit the shell script that is created by `mcc` (`run_myapplication.sh`)
to properly set the `LD_LIBRARY_PATH` environment variable.

\section sec_dum_matlab_compiler_deploying Deploying your compiled program

<del>First of all, note that you need a special license of *DIPimage* and
*DIPlib* to be able to distribute a program that uses this toolbox and
associated libraries. Please read [our web page](https://diplib.org/)
for information on how to obtain such a license.</del>

The CTF file created by `mcc` needs either the exact same version of
*MATLAB*, or the MCR created with that version, to run. It will also need
the three *DIPlib* shared libraries `libdip.so`, `libdipio.so` and
`libdml_mlvX_X.so` (the name of this last SO file should match the
directory name given as path to the *MATLAB Compiler*). The end-user needs
to install these three libraries and adjust the `LD_LIBRARY_PATH`
environment variable prior to starting the executable.

There is a very simple way of including the *DIPlib* libraries in the CTF
file:

```bash
mcc -m myapplication.m ...
    -I /something/dip/common/dipimage ...
    -I /something/dip/common/mlv7_4/dipimage_mex ...
    -I /something/dip/common/mlv7_4/diplib ...
    -a /something/dip/Linux/libdip.so ...
    -a /something/dip/Linux/libdipio.so ...
    -a /something/dip/Linux/libdml_mlv7_4.so
```

The CTF archive will be called `myapplication.ctf`, and, once extracted,
the *DIPlib* libraries will be in the directory
`myapplication_mcr/something/dip/Linux/` (assuming 32-bit Linux OS).

Thus, assuming your user puts the files `myapplication.ctf` and
`myapplication` into the directory `/home/user/myapp/`, and installed
the MCR into `/usr/local/mcr/v76/`, your user will have to do the
following to start the application:

```bash
MCRROOT=/usr/local/mcr/v76
LD_LIBRARY_PATH=/home/user/myapp/myapplication_mcr/something/dip/Linux/
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MCRROOT}/runtime/glnx86
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MCRROOT}/bin/glnx86
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MCRROOT}/sys/os/glnx86
MCRJREVER=`cat ${MCRROOT}/sys/java/jre/glnx86/jre.cfg`
MCRJRE=${MCRROOT}/sys/java/jre/glnx86/jre${MCRJREVER}/lib/i386
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MCRJRE}/native_threads
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MCRJRE}/server
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MCRJRE}/client
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MCRJRE}
XAPPLRESDIR=${MCRROOT}/X11/app-defaults
export LD_LIBRARY_PATH
export XAPPLRESDIR /home/user/myapp/myapplication <arguments>
```

You would do good creating a little shell script that collects these
commands, and provide it with your executable (instead of
`run_myapplication.sh`).

Note that, in the shell commands above, the ones that contain `MCRJRE`
are needed only if Java is enabled. You can add `-R -nojvm` to the `mcc`
command to disable Java if your application does not use it. In the same
way, add `-R -nodisplay` if your application does not use the graphic
display.
