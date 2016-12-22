---
title: 'Implementation notes and thoughts'
author: 'Cris Luengo'
...

# Implementation notes and thoughts


## Arithmetic and comparisons

Several more complex arithmetic operations are rather common, and could
be implemented as special functions. For example, instead of writing

    out = a*img1 + b*img2 + c*img3 + d;

one would write

    WeightedAddition( ImageRefArray{img1,img2,img3}, out, IntegerRefArray{a,b,c}, d );

For efficiency. There is template meta-programming that can convert an
expression like the first one into a call like the second one, but that
is too much templates for my taste. Is that really worth it? It doesn't
translate to other languages (Python, MATLAB, etc.).

We could try to complete the function that Mike started writing eons
ago(?), in which a string expression is evaluated and applied pixel by
pixel to a set of images:

    Evaluate( ImageArray{img1,img2,img3}, out, "3*a + 2*(b+c) + 6 > 30*(a+c)" );

The expression is converted to a representation that can be used to
efficiently apply the requested computation on each pixel. Which
operators and functions to allow in there is open for discussion. In
this example, letter `a` would refer to the first image in the input
array, letter `b` to the second, etc.

Would it not be easier to allow the user to write a callback to
`dip::Framework::Scan`?


## Colour space conversion

    dip::ColourConverter colourConverter;
    colourConverter.Define( "newspace", 3 );
                                    // "newspace" has 3 channels.
    colourConverter.Register( rgb2new, "rgb", "newspace", cost );
                                    // cost is optional, default = 1
    colourConverter.Register( new2rgb, "newspace", "rgb" );
    colourConverter.Register( new2grey, "newspace", "grey" );
                                    // cost is optional, default = 1e9
    colourConverter.Register( grey2new, "grey", "newspace" );
    colourConverter.Convert( img, img, "newspace" );
                                    // convert img to 'newspace', write output in img

The object constructor registers the standard colour conversion
functions, the user can allocate new functions. These functions compute
the conversion for a single pixel (or a whole buffer of them?), and have
the form

    rgb2new( FloatArray in, FloatArray out )

...or something more similar to what `dip::Framework::Scan` uses.

The color converion functions could be objects derived from a base class.
Making them into objects allows them to carry information, and lets the
user interact with them in ways that otherwise must be made explicit in
the `dip::ColourConverter` class. For example, the RGB converter could
carry a whitepoint value, that the user can set by interacting directly
with the RGB object. (It's not really clear yet how...) This seems better
than my original idea of having the whitepoint attached to the image
(which is also how we do it in DIPimage).

With the conversion path as a list of function/object pointers, we call the
point-scanning framework and for each pixel we call each of the
functions in the path. For efficiency, these functions could iterate
over a line, like all the other FrameWork callback functions do. We'd
generate a temporary buffer that is large enough to contain the largest
vector representation along the conversion path. For example: RGB->CMYK->QWT
requires the intermediate CMYK pixel data to be stored somewhere. It does
not fit into the buffer for QWT (ficticious), which has only space for 3
values per pixel, so an intermediate buffer with 4 values per pixel must
be made.

## Processing images -- how to access and modify pixels

The old DIPlib does most processing through the [frameworks][Frameworks].
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

An alternative would be to combine the function pointer and the
structure with parameters to pass to this function into a functor
object. This could add a lot of flexibility and of course avoids the
use of pointers altogether.


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
  It describes a filter support of arbitrary shape by giving the offset to the first pixel
  in each run, and the number of pixels in the run. Subsequent pixels are then accessed by
  adding the correct stride to the offset of the first pixel.
  The pixel table framework use this system, I don't think it's used outside it.

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

The `PixelTable` will be an array with a `Offset` and a `Length` element for each run.
Again, if it's build on a `std::vector` we'll have an iterator automatically.
But it would be nice to also have an iterator that iterates over each of the pixels in the
filter's support. It's a little awkward having a container with two different iterators,
but they'd both be useful. To add a `Weight` element, we'd have an additional array directly
listing weights for each of the pixels represented. These weights would be optionally added
to the object. The iterator that visits each of the pixels would make it easy to access
these weights.

The constructor takes a binary image (weight is distance to center pixel) or grey-value
image (weight is grey value) representing the neighborhood support or filtering kernel.
A parameter in the constructor causes no weights being added to the object. Another parameter
dictates the direction of the pixel runs. This direction should match with the processing
dimension for some algorithms, and it might be beneficial to use the smallest stride dimension
for some others, or the dimension that yields the longest runs.


## Alias handler

Many of the current *DIPlib* functions (the ones that cannot work
in-place) use a function `ImagesSeparate()` to create temporary images
when output images are also input images. The resource handler takes
care of moving the data blocks from the temporary images to the output
images when the function ends. With the current design of shared pointers
to the data, this is no longer necessary. Say a function is called with

    dip::Image A;
    dip::Gauss(A, A, 4);

Then the function `dip::Gauss()` does this:

    void dip::Gauss(const dip::Image &in_, dip::Image &out, double size) {
       Image in = in_;
       out.Strip();
       // do more processing ...
    }

What happens here is that the new image `in` is a copy of the input image, `A`,
pointing at the same data segment. The image `out` is a refernce to image `A`.
When we strip `A`, the new image `in` still points at the original data segment,
which will not be freed until `in` goes out of scope. Thus, the copy `in`
preserves the original image data, leaving the output image, actually the
image `A` in the caller's space, available for modifying.

However, if `out` is not stripped, and data is written into it, then `in` is
changed during processing. So if the function cannot work in plance, it should
always test for aliasing of image data, and strip/forge the output image if
necessary:

    void dip::Gauss(const dip::Image &in_, dip::Image &out, double size) {
       Image in = in_;                 // preserve original input data
       bool needReForge = false;
       if (out does not have the required properties) {
          needReForge = true;
       } else if (in and out share data) {
          needReForge = true;
       }
       if (needReForge) {
          out.Strip();                 // create new data segment for output
          out.SetDimensions(...);
          ...
          out.Forge();
       }
       // do more processing ...
    }


## Functionality currently not in *DIPlib* that would be important to include

- An overlay function that adds a binary or labelled image on top of a
  grey-value or colour image.

- Stain unmixing for bright-field microscopy.

- Some form of image display for development and debugging. We can
  have the users resort to third-party libraries or saving
  intermediate images to file, or we can try to copy *OpenCV*'s image
  display into *dipIO*.

- Some filters that are trivial to add:

    - Scharr (slightly better than Sobel)

    - h-minima & h-maxima

    - opening by reconstruction

    - alternating sequential open-close filter (3 versions: with
      structural opening, opening by reconstruction, and area opening)

- Dilation/erosion by a rotated line is currently implemented by first
  skewing the image, applying filter along rows or columns, then
  skewing back. We can add a 2D-specific version that operates
  directly over rotated lines. The diamond structuring element can
  then be decomposed into two of these operations. We can also add
  approximations of the circle with such lines.

- We're also lacking some other morphological filters:

    - hit'n'miss, where the interval is rotated over 180, 90 or 45 degrees.

    - thinning & thickening, to be implemented as iterated hit'n'miss.

    - levelling

- Most of the functionality that is now implemented in *DIPimage*
  only:

    - automatic threshold determination (Otsu, triangle,
      background, etc.).

    - [Colour space conversion].

    - 2D snakes.

    - look-up tables (LUT, both for grey-scale and colour LUTs, using
      interpolation when input image is float) -- some of this is implemented
      already in DIPlib.

    - general 2D affine transformation, 3D rotation.

    - xx, yy, zz, rr, phiphi, ramp; extend this to `Coordinates()`, which
      makes a tensor image.

- Radon transform for lines and circles, Hough transform for lines.

- Level-set segmentation, graph-cut segmentation.

- The `Label()` function should return the number of labels. It could
  optionally also return the sizes of the objects, since these are
  counted anyway. The labelling algorithm by Mike is quite efficient,
  but we should compare with the more common union-find algorithm, which
  is likely to be optimal for this application (Mike's code uses a priority
  queue, union-find doesn't need it).

- We need to figure out if it is worth it to use loop unrolling for
  some basic operations.


## Python interface

We will define a Python class as a thin layer over the `dip::Image` C++
object. Allocating, indexing, etc. etc. through calls to C++. We'd
create a method to convert to and from *NumPy* arrays, simply passing
the pointer to the data. We'd have to make sure that deallocation
occurs in the right place (data ownership). A *DIPlib* function to
extract a 2D slice ready for display would be needed also, make display
super-fast!


## MATLAB interface

The MATLAB toolbox will be significantly simplified:

-   No need for the `libdml.so` / `libdml.dll` library, as there are no
    globals to store, and not much code to compile.

-   Basically, all the code from that library will sit in a single
    header file to be included in each of the MEX-files:

    -   Casting an `mxArray` input to: `Image`, `IntegerArray`,
        `FloatArray`, `String`, `sint`, `uint`, or `double`.

    -   Casting those types to an `mxArray` output.

-   We won't need to convert strings to `enum`s, as [that will be done
    in the *DIPlib* library][Passing options to a function].

-   Some MEX-files will have more elaborate data conversion, as is the
    case now (e.g. `dip_measure`).

-   The few functions in the current *DIPlib* that use an `ImageArray` as
    input or output will use tensor images instead.

The M-file code will need to be adapted:

-   The `dip_image` object will not also double as `dip_image_array`
    (that was a mistake). Instead, tensor images will be a single data
    block, as they will be in *DIPlib*. Thus, some of the class methods
    will have to be rewritten, and some functions that use image arrays
    will have to be adapted. For an array of images, use a cell array.
    On the other hand, more functionality could be deferred to *DIPlib*.

-   Much of the code for the `dip_measurement` object will change also,
    as its representation will likely change with the changes in
    *DIPlib*.

-   The colour conversion code will be replaced with a single call to
    *DIPlib*, as will some other functionality that will be translated
    from MATALB to C++. Much of the tensor arithmetic should be done
    through *DIPlib* also.

-   `dipshow` will be simplified, as simple calls to a *DIPlib* display
    function will generate the 2D array for display.

### Issues with the current MATLAB interface

Currently (version 2), complex images need to be copied over in the
interface, as DIPlib and MATLAB represent them differently. There are
two solutions: DIPlib represents them as MATLAB does (which seems like
a really bad idea to me), or we cast them to a non-complex array that
MATLAB can be happy with even if it doesn't interpret it correctly.
Only when converting the `dip_image` object to a MATLAB array will the
copy be necessary. It could be simple to include a 'complex' dimension
as the first dimension of the `mxArray` (for real images, this would have
a size of 1). The new tensor dimension would then be the second dimension
(for scalar images, again of size 1). Thus, the mxArray would always be:
[complex, tensor, x, y, z, ...]. A reshape would get rid of the first
dimension to translate to a MATLAB array, in case of a real image.
A complex image would need a simple copy.

If we go this route, we might as well go all the way, and always have
a one-dimensional uint8 array inside the `dip_image` object. Each of the
`dip::Image` member elements would be copied into the `dip_image` object,
to make the conversion back and forth in the interface as trivial as possible.
All indexing and basic operations that are computed in MATLAB in the
old implementation would go through DIPlib in the new implementation.
The `dml` interface would also copy the image data into an `mxArray`
(or two for complex images). This step would then not be as efficient
as it is in the old DIPimage, but should be avoided anyway. The only
place where this step needs to be efficient is in the generation of a
display image, which could be arranged by creating that image as an
encapsulated `mxArray` with the right dimensions and strides for MATLAB
to show it.


## 3^rd^ party libraries

We should use [*FFTW*](http://www.fftw.org) for the Fourier transform.
But it's GPL licensed, meaning that we won't be able to include it
without making the library GPL also? We might be able to include it as
an "optional" element, let the user download *FFTW* for him/herself. It
is not clear whether that breaks the GPL license terms. And what does
the poor soul do that wants to create non-GPL software with this? An
alternative could be [*djbfft*](http://cr.yp.to/djbfft.html), which
looks like it hasn't been updated in a while (since 1999!), but is
claimed by the author to be faster than *FFTW2* and any other library
at that time. I'm sure this is good enough for us. There are no
indications of a license at all... Another alternative is
[*Kiss FFT*](https://sourceforge.net/projects/kissfft/), a simple FFT
written in a single C++ header file and BSD licensed, and presumably
quite a bit faster than the current FFT implementation in DIPlib.
However, the implementation on OpenCV is 4 to 10 times faster than
*Kiss FFT* in my quick test with various array sizes (even if less precise).

[*Tina's Random Number Generator Library*](http://numbercrunch.de/trng/)
for RNG? (does parallel RNG also). Update: apparently the C++11
standard library includes the concepts and ideas from this library.

We will keep the TIFF and ICS readers/writers we currently have, using
[*libtiff*](http://www.remotesensing.org/libtiff/) and
[*libics*](http://libics.sourceforge.net). For other file types, we
could use *Bio-Formats*. How to link to Java from C++?

[*Eigen*](http://eigen.tuxfamily.org) is a pretty sweet linear algebra
library. It's stable, very efficient, and templated like the standard
library. We will be able to wrap a pixel (with its strides) as an
*Eigen* matrix and do computations.
