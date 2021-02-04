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


\page sec_dum_mex_files Writing MEX-files that use *DIPlib*

A MEX-file is a compiled C, C++ or Fortran function (C++ if it uses *DIPlib*) that
can be called from *MATLAB* as if it were a normal *MATLAB* function. It must take
*MATLAB* data as input and produce *MATLAB* data as output. To call a *DIPlib* function
from the MEX-file, the *MATLAB* data must be converted to the right C++ types.
This chapter describes the functionality we wrote for this purpose.

It is assumed that the reader is familiar with basic MEX-files (see the
[*MATLAB* MEX-file documentation](https://www.mathworks.com/help/matlab/matlab-api-for-c.html)
if not) and basic *DIPlib* (see the \ref index "documentation" if not).
We exclusively use [the C API](https://www.mathworks.com/help/matlab/cc-mx-matrix-library.html).

\section sec_dum_mex_files_dml The *DIPlib--MATLAB* interface

The header file \ref "dip_matlab_interface.h" contains a series of functions that can
be used to convert *MATLAB* types to *DIPlib* types and vice versa. These functions
make it very easy to write a *MATLAB* interface to *DIPlib* functions (and this is
its main purpose), but could be used for other purposes too.

All its functionality is in the \ref dml namespace.

\subsection sec_dum_mex_files_dml_output_images Output images

The main trick that it can do is prepare a \ref dip::Image object that, when forged
by a *DIPlib* function, will have *MATLAB* allocate the memory for the pixels,
such that no copy needs to be made when this image is returned to MATLAB. For
this, create an object of type \ref dml::MatlabInterface. Its \ref dml::MatlabInterface::NewImage
function creates a raw image that can be forged (or passed as output image to
any *DIPlib* function). The \ref dml::GetArray(dip::Image const&, bool) function
can then be used to obtain a pointer to the `mxArray` containing a `dip_image`
object with the same pixel data. Note that the `dml::MatlabInterface` object
needs to exist throughout this process, as it owns the data until `dml::GetArray`
extracts it.

This is a skeleton MEX-file that outputs such an image:

```cpp
#include <dip_matlab_interface.h> // Always include this, it includes diplib.h and mex.h

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      // Create an output image
      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      // Forge it
      out.ReForge( { 256, 256 }, 1, dip::DT_UINT8 );
      out.Fill( 0 ); // Usually you'd do something more exciting here!

      // Retrieve the MATLAB array for the output image (it's a dip_image object)
      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
```

The documentation to \ref dml::MatlabInterface gives more details about how to use
the images created in this way.

\subsection sec_dum_mex_files_dml_input_images Input images

The function \ref dml::GetImage takes an `mxArray` pointer, checks it for validity,
and returns a \ref dip::Image object that shares data with the `mxArray` (except in
exceptional circumstances: complex-valued numeric arrays must be copied because
their storage is different in *MATLAB* and *DIPlib*; complex-valued `dip_image`
objects are not copied).

For example, one can add the following line to the `mexFunction` above:

```cpp
dip::Image const in = ( nrhs > 0 ) ? dml::GetImage( prhs[ 0 ] ) : dip::Image();
```

This line calls \ref dml::GetImage only if `prhs[0]` actually exists. If not, it
creates a raw image object. You should always check `nrhs` before reading any of
the `prhs` elements. Likewise, you should always check `nlhs` before assigning
to any of the `plhs` elements except the first one (`plhs` always has at least
one element, even if `nlhs == 0`).

\subsection sec_dum_mex_files_dml_input_data Converting other types

There exist similar `Get...` functions for just about every *DIPlib* type, for example
\ref dml::GetFloat, \ref dml::GetFloatArray or \ref dml::GetFloatCoordinateArray. See
the documentation to the \ref dml namespace for a complete list. These
take an `mxArray` pointer as input, validate it, and output a value of the appropriate
type. If the validation fails, an exception is thrown.

There exist also a series of \ref dml::GetArray functions that do the reverse process:
they take a value of any type typically returned by a *DIPlib* function, and
convert it to an `mxArray`, returning its pointer. Note that *MATLAB* always takes
care of freeing any `mxArray` objects created by the MEX-file, there is no need
to do any manual memory management.

These data conversion functions all copy the data (only images are passed without
copy, other data is typically not large enough to matter).

For more complex examples of MEX-files, see
[`examples/external_interfaces/matlab_mex_example.cpp`](https://github.com/DIPlib/diplib/tree/master/examples/external_interfaces/matlab_mex_example.cpp),
as well as the *DIPimage* MEX-files in
[`dipimage/private/`](https://github.com/DIPlib/diplib/tree/master/dipimage/private) and
[`dipimage/@dip_image/private`](https://github.com/DIPlib/diplib/tree/master/dipimage/%40dip_image/private).


\section sec_dum_mex_files_mex Compiling a MEX-file

To compile a MEX-file like the one shown in the previous section, save it for example
as `myfunction.cpp`, and use the `dipmex` function in *MATLAB*:

```matlab
dipmex myfunction.cpp
```

This function simply calls the `mex` function, adding arguments to allow the compiler
to find the *DIPlib* header files and to link in the *DIPlib* library. Because this
is C++, it is important that the same compiler (and often also the same version) be
used to build the MEX-file as was used to build the *DIPlib* library. We recommend
that you build the library yourself. `mex -setup C++` should be used to configure
*MATLAB*'s `mex` command to use the right compiler.

The result of the `dipmex` command above is a MEX-file called `myfunction`. You can
call it like any other *MATLAB* function:

```matlab
img = myfunction;  % it has no input arguments
```

If your MEX-file needs more source files and/or libraries, simply add them to the
`dipmex` command. You can also add [other arguments for `mex`](https://www.mathworks.com/help/matlab/ref/mex.html) here:

```matlab
dipmex myfunction.cpp other_source.cpp /home/me/mylibs/libstuff.a -I/home/me/mylibs/
```

The MEX-file will always have as name the name of the first source file in the list,
unless an `-output mexname` argument is given.
