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


\page building_documentation Building the *DIPlib* documentation

The documentation is built using [*dox++*](https://crisluengo.github.io/doxpp/), which
requires *Python 3* and a set of packages. It also requires a *LaTeX* installation to render
the equations. Here we show how to install dependencies on Ubuntu Linux and macOS. For
other Linux distributions the instructions are similar. We have not yet tried building
the documentation on Windows, if you manage to get that done, please write down some
instructions below.


\section apidoc_dependencies Installing the dependencies

*Python 3* can be installed on Ubuntu with
```bash
sudo apt install python3
```
and on macOS with
```bash
brew install python3 python3-pip
```
We have built the documentation with *Python* 3.6, 3.9, and 3.12; it is unknown if versions older
than 3.6 will work.

Once *Python* is installed, install the required packages:
```bash
pip3 install clang jinja2 markdown markdown-headdown mdx_math_svg Pygments
```

Finally, install *LaTeX*. Ubuntu has a *LaTeX* package, but we recommend [*TeX Live*](https://tug.org/texlive/).
There is a version of *TeX Live* specifically packaged for macOS, but that just adds some macOS-specific
GUI tools; it is perfectly fine to use the Unix installation instructions on macOS.

First, download [`install-tl-unx.tar.gz`](http://mirror.ctan.org/systems/texlive/tlnet/install-tl-unx.tar.gz),
for example to `~/tmp/`. Extract its contents (`tar xzf install-tl-unx.tar.gz`), then navigate into the
newly created directory (`cd install-tl-*`) and run the installer program (`./install-tl`) (under Linux
you might have to run this as `sudo` if you want to install to the default location). Here you can
set up destination directories, etc. (there's a graphical version of the installation program, and a
text version, but they both should allow the same options). To speed up installation, turn off the
options to "install font/macro doc tree" and "install font/macro source tree". Neither are needed,
and together take up nearly half the downloaded bytes. Do turn on the option "create symlinks in standard
directories", which is off by default. If you don't want to install the "full scheme" (which installs
all packages), you might have to separately install a few of the packages needed by the tool that
renders equations using *LaTeX*:
```bash
tlmgr install standalone preview ucs xkeyval currfile filehook newtx fontaxes xstring
```


\section apidoc_doxpp Installing *dox++*

This tool can be installed anywhere. For the purposes of these instructions, we will put it into `/opt/`,
but you can put it in your home directory as well. All that is needed is cloning the repository:
```bash
cd /opt
git clone https://github.com/crisluengo/doxpp.git
```
This will create the directory `/opt/doxpp/`, containing the two executables `dox++parse` and `dox++html`.
We will need to tell *CMake* where to find these tools.


\section apidoc_cmake Configuring *CMake*

To set up *CMake* to build the documentation, we need to tell it where the *dox++* programs are. From
your *DIPlib* build directory,
```bash
cmake . -DDOXPP_PARSE_EXECUTABLE=/opt/doxpp/dox++parse -DDOXPP_HTML_EXECUTABLE=/opt/doxpp/dox++html
```
(obviously substitute your *dox++* path where it says `/opt/doxpp`).

*CMake* should now show, in its configuration report, the line:
```text
Documentation installed to /usr/local/share/doc/DIPlib
```
(or something similar, depending on the chosen installation path.)


\section apidoc_building Building the documentation

Once everything is set up, the documentation is build with
```bash
make apidoc
```
from the *DIPlib* build directory.

The documentation will appear in a `html/` subdirectory of the directory specified in the configuration report.

The first time you build the documentation, it will take quite some time to render all the equations. These
will be cached, such that subsequent builds of the documentation are faster.

If there are errors produced during the rendering of the equations, a long *LaTeX* output will be shown. This
output should point out which *LaTeX* package is missing. If you used *TeX Live* to install *LaTeX*, use its
package manager to install the missing package and try building again:
```bash
tlmgr install <package name>
```

Other errors are not fatal. Some errors even are irrelevant because they relate to non-documented
members. If Clang produces an error while parsing a header file, the contents of that header file will
not make it into the documentation.


\section apidoc_official Building the official documentation

The official documentation at [`diplib.org/diplib-docs/`](https://diplib.org/diplib-docs/) is uploaded
to [the `diplib-docs` GitHub repository](https://github.com/DIPlib/diplib-docs).
The `/usr/local/share/doc/DIPlib/html` output directory of the *dox++* tool is the *git* repository.
Simply push a new build to update the website.

It is important that the official documentation is complete. This means that all modules and extras
must be included. For these to be included, their headers must be parseable by Clang. This again
requires that their dependencies be present. Therefore, it is necessary to have
[*MATLAB*](https://www.mathworks.com/products/matlab.html), [*OpenCV*](https://opencv.org) and
[*Vigra*](http://ukoethe.github.io/vigra/) installed, and the "include directories" option in the
`doc/dox++config.in` file adjusted so that Clang runs through all header files without error.

!!! todo
    Have *CMake* write all configured include directories into the `dox++config` file.
