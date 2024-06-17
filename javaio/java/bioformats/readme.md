`bio-formats-stripped.jar` is a stripped-down version of `bioformats_package.jar`
We simply removed all the files that weren't necessary to build our `BioFormatsInterface.java`
source file into `DIPjavaio.jar`. This left a small amount of compiled Java that we
could include into our repository, to simplify the build process. Note that this
Java package is only used to resolve references when building `DIPjavaio.jar`, none
of the code actually ands up in there. When using *DIPjavaio*, one must fetch the
original `bioformats_package.jar`.

The files in `bio-formats-stripped.jar` come from these repositories:

 - https://github.com/ome/bioformats
 - https://github.com/ome/ome-common-java
 - https://github.com/ome/ome-model

The original sources for these files are distributed under a 2-clause BSD licence.
Below we reproduce the copyright statement for these files and the associated license
text.

```none
Copyright (C) 2005 - 2017 Open Microscopy Environment:
  - Board of Regents of the University of Wisconsin-Madison
  - Glencoe Software, Inc.
  - University of Dundee

Copyright (C) 2006 - 2016 Open Microscopy Environment:
  - Massachusetts Institute of Technology
  - National Institutes of Health
  - University of Dundee
  - Board of Regents of the University of Wisconsin-Madison
  - Glencoe Software, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
```
