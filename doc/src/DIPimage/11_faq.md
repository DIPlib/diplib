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


\page sec_dum_faq Frequently asked questions


\section sec_dum_faq_bugs Bugs


\subsection sec_dum_faq_bugs_issues I have found a bug. What do I do?

First make sure you have the latest version of *DIPimage*. Compare the version number shown in the GUI (or by `help DIPimage`)
to that shown for the latest release in the [releases page on GitHub](https://github.com/DIPlib/diplib/releases).
Upgrade to the latest version if necessary and make sure the bug still exists. If the bug persists in this latest version,
do a cursory search in the project's [issues tracker](https://github.com/DIPlib/diplib/issues). If you do find it there,
add a comment if you can provide additional information that will help us fix the bug. If you do not find it there,
[submit a bug report](https://github.com/DIPlib/diplib/issues/new?template=bug_report.md).
As described in the file [`CONTRIBUTING.md`](https://github.com/DIPlib/diplib/blob/master/CONTRIBUTING.md), please include
all the information you can that might help us reproduce the bug, especially the code that causes the bug. 


\section sec_dum_faq_usage Usage


\subsection sec_dum_faq_usage_components In order to make full use of *DIPimage*, what components of *MATLAB* are necessary?

Just *MATLAB* itself. No other toolboxes are required, but of course can be used in conjunction.


\subsection sec_dum_faq_usage_version What versions of *MATLAB* will *DIPimage* run on?

This toolbox requires *MATLAB* R2008a (version 7.6) or later. This is the version that introduced the
`classdef`-style objects, used in the core types of the toolbox. However, we have tested the toolbox only
under much newer versions of *MATLAB*, so your mileage may vary. Feel free to open a ticket to report issues
with your version of *MATLAB*.


\subsection sec_dum_faq_usage_ipt How do I use a *DIPimage* image object with an Image Processing Toolbox function?

The function `dip_array(im)` will extract the *MATLAB* array from within the `dip_image` object `im`. No copies will
initially be made, unless the arrays are modified (read up on
[*MATLAB*'s lazy copying](https://www.mathworks.com/help/matlab/matlab_prog/avoid-unnecessary-copies-of-data.html#mw_7a918b58-da37-494b-8af7-d638fb16217e)).

You can directly use *MATLAB* arrays in the *DIPimage* toolbox functions.


\subsection sec_dum_faq_usage_sources Can I have the source code?

Yes! Since *DIPimage 3*, the toolbox and the library it is built on are open source. You can find it
[on GitHub](https://github.com/DIPlib/diplib/).


\subsection sec_dum_faq_usage_integrate How do I integrate my C or C++ code into *DIPimage*?

See the section \ref sec_dum_mex_files.


\subsection sec_dum_faq_usage_select How do I select which watershed function to run?

Several functions of *DIPimage* also exist in other *MATLAB* toolboxes (and presumably other 3rd party toolboxes).
*MATLAB* selects which version to run according to several rules. If you have added the *DIPimage* path at the top
of the *MATLAB* search path (with `addpath`), then the *DIPimage* `watershed` is the default.
If you like to use the Image Processing Toolbox function `watershed`, you can do the following:

- Change the order of paths in the search path. However, this means none of the *DIPimage* functions take
  precedence anymore. You do this by adding `'-end'` to the `addpath` command (check with `help addpath`).

- Find out which `watershed` functions are available (with `which watershed -all`), and change to the right
directory before calling `watershed`. The function in the current directory is always selected over functions
on the path.
