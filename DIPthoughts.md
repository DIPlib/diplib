---
title: 'Implementation notes and thoughts'
author: 'Cris Luengo'
copyright: (c)2014-2017, Cris Luengo.
...

# Implementation notes and thoughts

## Fourier Transform

If the *FFTW* library is installed on the user's system, *DIPlib* will link against it
and use it to compute the FT. The CMakeLists script will have a setting that disables
this behaviour, as *FFTW* uses a GNU license and we want *DIPlib* to be totally
unencumbered by licenses. When not using *FFTW*, the built-in FFT algorithm is used.


## Processing images -- how to access and modify pixels

The old *DIPlib* does most processing through the [frameworks][Frameworks].
These are great because they handle a lot of things -- creating the output
image(s), dealing with different data types and dimensionalities, apply
filters using multithreading, etc. But the frameworks can use some
refactoring, as described below. Also, the documentation is out of date
(and so does not correctly describe the functionality as implemented),
there are a slew of aliases that complicate the landscape, and there are
options that are never used, some are not even implemented.

The [second sub-section][Image iterators] describes an alternative that
leaves more to the user, but might be easier to use under certain circumstances.

We also discuss how to [describe a neighborhood][Neighborhood lists], which
is done in different ways in the old code.


### Frameworks

This is already described in the code.


### Image iterators

This is already described in the code.


### Neighborhood lists

Current code uses two different ways to describe a neighborhood:

- A list (actually two, one containing coordinates for each neighbor, one containing their
  corresponding offsets).
  It describes the immediate neighborhood of a pixel (given by the dimensionality and the
  connectivity), and is used in algorithms that process images using priority queues
  (watershed, region growing, skeleton, labelling, etc.). Most of these algorithms use their
  own version of these lists, not all need the coordinates, for example.

- A pixel table (a list of pixel runs).
  This is already described in the code.

We should keep both methods, and standardize the first one.

The `NeighborhoodList` will be an array with a `Coordinates`, an `Offset`, and a `Weight`
element for each neighbor pixel. An option of the constructor will leave the coordinates
empty if they are not needed. It would be nice to have an iterator for this list, but if
it is build on a `std::vector` that will come automatically. The `Weight` element would
be filled with the distance of the neighbor (different options should be available there).

    NeighborhoodList neighbors ( image, connectivity );
    for( auto img_it = ImageIterator<dip::sfloat>( image ); img_it.IsAtEnd(); ++img_it ) {
       for( auto n_it = neighbors.begin(); n_it != neighbors.end(); ++n_it ) {
          img_it.atOffset( *n_it );
       }
    }


## Python interface

We will define a Python class as a thin layer over the `dip::Image` C++
object. Allocating, indexing, etc. etc. through calls to C++. We'd
create a method to convert to and from *NumPy* arrays, simply passing
the pointer to the data. We'd have to make sure that deallocation
occurs in the right place (data ownership). A *DIPlib* function to
extract a 2D slice ready for display would be needed also, make display
super-fast!

[pybind11](https://github.com/pybind/pybind11), a C++11 Python binding
library, seems to me to be the way to go.


## *MATLAB* interface

The *MATLAB* toolbox will be significantly simplified:

-   No need for the `libdml.so` / `libdml.dll` library, as there are no
    globals to store, and not much code to compile.

-   Basically, all the code from that library sits in a single
    header file to be included in each of the MEX-files:

    -   Casting an `mxArray` input to: `Image`, `IntegerArray`,
        `FloatArray`, `String`, `sint`, `uint`, or `double`.
        (**DONE**)

    -   Casting those types to an `mxArray` output. (**DONE**)

-   We won't need to convert strings to `enum`s, as [that will be done
    in the *DIPlib* library][Passing options to a function].

-   Some MEX-files will have more elaborate data conversion, as is the
    case now (e.g. `dip_measure`).

-   The few functions in the current *DIPlib* that use an `ImageArray` as
    input or output will use tensor images instead.

-   It seems that, at least in debug mode, the MEX-files with statically
    linked DIPlib are quite large. If the $rpath feature works well, we
    could keep DIPlib as a shared object, but if we need to set any
    runtime LD_LIBRARY_PATH again, we'll do static linking.

The M-file code will need to be adapted:

-   The `dip_image` object will not also double as `dip_image_array`
    (that was a mistake). Instead, tensor images will be a single data
    block, as they will be in *DIPlib*. Thus, some of the class methods
    will have to be rewritten, and some functions that use image arrays
    will have to be adapted. For an array of images, use a cell array.
    On the other hand, more functionality could be deferred to *DIPlib*.
    (**DONE**)

-   Much of the code for the `dip_measurement` object will change also,
    as its representation will likely change with the changes in
    *DIPlib*.

-   The colour conversion code will be replaced with a single call to
    *DIPlib*, as will some other functionality that will be translated
    from MATALB to C++. Much of the tensor arithmetic should be done
    through *DIPlib* also.

-   `dipshow` will be simplified, as simple calls to a *DIPlib* display
    function will generate the 2D array for display. (**DONE**)

### Issues with the current *MATLAB* interface

In *DIPimage* version 2.x, complex images need to be copied over in the
interface, as *DIPlib* and *MATLAB* represent them differently. There are
two solutions: *DIPlib* represents them as *MATLAB* does (which seems like
a really bad idea to me), or we cast them to a non-complex array that
*MATLAB* can be happy with even if it doesn't interpret it correctly.
Only when converting the `dip_image` object to a *MATLAB* array will the
copy be necessary. It could be simple to include a 'complex' dimension
as the first dimension of the `mxArray` (for real images, this would have
a size of 1). The new tensor dimension would then be the second dimension
(for scalar images, again of size 1). Thus, the mxArray would always be:
[complex, tensor, x, y, z, ...]. A reshape would get rid of the first
dimension to translate to a *MATLAB* array, in case of a real image.
A complex image would need a simple copy. (**DONE**)

If we go this route, we might as well go all the way, and always have
a one-dimensional uint8 array inside the `dip_image` object. Each of the
`dip::Image` member elements would be copied into the `dip_image` object,
to make the conversion back and forth in the interface as trivial as possible.
All indexing and basic operations that are computed in *MATLAB* in the
old implementation would go through *DIPlib* in the new implementation.
The `dml` interface would also copy the image data into an `mxArray`
(or two for complex images). This step would then not be as efficient
as it is in the old *DIPimage*, but should be avoided anyway. The only
place where this step needs to be efficient is in the generation of a
display image, which could be arranged by creating that image as an
encapsulated `mxArray` with the right dimensions and strides for *MATLAB*
to show it. (**WE WON'T DO THIS**)


## Image file formats

We will keep the TIFF and ICS readers/writers we currently have, using
[*libtiff*](http://www.remotesensing.org/libtiff/) and
[*libics*](http://libics.sourceforge.net).

For other file types, we could use [*Bio-Formats*](http://www.openmicroscopy.org/site/products/bio-formats).
It is a Java library, we'll probably use
[JNI](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html)
to interface between C++ and Java, see for example
[this tutorial](https://www.codeproject.com/Articles/993067/Calling-Java-from-Cplusplus-with-JNI).
See also [this ITK module](https://github.com/scifio/scifio-imageio) for 
interfacing to *Bio-Formats*, which uses [SCIFIO](https://github.com/scifio/scifio).

We currently also have JPEG and GIF readers/writers, which are not very
useful and can be done away with. We have simplistic code to write Postscript
and CSV, which also aren't useful enough. The BioRad PIC reader is out of date,
nobody uses that format any more. Finally, we have an LSM reader based on the
TIFF reader, but if we use *Bio-Formats* it won't be necessary to maintain that
one either.

