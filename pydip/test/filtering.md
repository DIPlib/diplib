Filtering with PyDIp
===

Import relevant modules

    >>> import diplib as dip
    >>> import numpy as np

Test image
---

A small array we can use as a test image

    >>> array = np.random.randint(0, 255, size=(91, 120), dtype=np.uint8)
    >>> img = dip.Image(array)
    >>> img
    <Scalar image, UINT8, sizes {120, 91}>

Applying a filter
---

The simplest way to apply a filter allows the function to create a new image for the output. Depending on the
filter used, the output's data type can be the input type, or can be promoted to a floating-point type.

    >>> out = dip.Gauss(img, 2)
    >>> out
    <Scalar image, SFLOAT, sizes {120, 91}>

The function does exactly the same when applied to a NumPy array.

    >>> out = dip.Gauss(array, 2)
    >>> out
    <Scalar image, SFLOAT, sizes {120, 91}>

One can of course also use keyword arguments.

    >>> out = dip.Gauss(img, sigmas=2)
    >>> out
    <Scalar image, SFLOAT, sizes {120, 91}>

You can find out the name and default value of the arguments using ``help``:

    >>> help(dip.Gauss)
    Help on built-in function Gauss in module staging.diplib.PyDIP_bin:
    <BLANKLINE>
    Gauss(...) method of builtins.PyCapsule instance
        Gauss(*args, **kwargs)
        Overloaded function.
    <BLANKLINE>
        1. Gauss(in: staging.diplib.PyDIP_bin.Image, sigmas: list[float] = [1.0], derivativeOrder: list[int] = [0], method: str = 'best', boundaryCondition: list[str] = [], truncation: float = 3.0) -> staging.diplib.PyDIP_bin.Image
    <BLANKLINE>
        Convolution with a Gaussian kernel and its derivatives
    <BLANKLINE>
        2. Gauss(in: staging.diplib.PyDIP_bin.Image, *, out: staging.diplib.PyDIP_bin.Image, sigmas: list[float] = [1.0], derivativeOrder: list[int] = [0], method: str = 'best', boundaryCondition: list[str] = [], truncation: float = 3.0) -> None
    <BLANKLINE>
        Convolution with a Gaussian kernel and its derivatives
    <BLANKLINE>

The on-line documentation has more details about the meaning of each argument, and implementation details. For
``dip.Gauss``, the documentation is here: <https://diplib.org/diplib-docs/linear.html#dip-Gauss-dip-Image-CL-dip-Image-L-dip-FloatArray--dip-UnsignedArray--dip-String-CL-dip-StringArray-CL-dip-dfloat->

The second syntax allows one to pass an image object to be used as output. If the output image is of the correct
size and type, its data segment will be reused. Otherwise it will be reallocated

    >>> out = dip.Image()
    >>> dip.Gauss(img, out=out, sigmas=2)
    >>> out
    <Scalar image, SFLOAT, sizes {120, 91}>
    >>> dip.Gauss(img, out=out, sigmas=2)  # no reallocation here!
    >>> out
    <Scalar image, SFLOAT, sizes {120, 91}>

To force the output to be of a different type, protect the output image.

    >>> out = dip.Image((120, 91), 1, "UINT8")
    >>> out.Protect()
    False
    >>> dip.Gauss(img, out=out, sigmas=2)
    >>> out
    <Scalar image, UINT8, sizes {120, 91}>

If the image is protected but of the wrong type, an error will occur.

    >>> out = dip.Image((40, 91), 1, "UINT8")
    >>> out.Protect()
    False
    >>> dip.Gauss(img, out=out, sigmas=2)
    Traceback (most recent call last):
    ...
    staging.diplib.PyDIP_bin.ParameterError: Image is protected
    ...

When passing a NumPy array as output, it must have the right sizes, since it cannot be reallocated. It will behave
like a protected image.

    >>> outarr = np.zeros((91, 120), dtype=np.uint16)
    >>> dip.Gauss(img, out=outarr, sigmas=2)
    >>> outarr = np.zeros((91, 40), dtype=np.uint16)
    >>> dip.Gauss(img, out=outarr, sigmas=2)
    Traceback (most recent call last):
    ...
    staging.diplib.PyDIP_bin.ParameterError: Image is protected
    ...

To have the filter work in-place, simply pass the input image as the ``out`` argument. Do protect the image to
ensure it is not reallocated.

    >>> img.Protect()
    True
    >>> dip.Gauss(img, out=img, sigmas=2)
    >>> out
    <Scalar image, UINT8, sizes {40, 91}>

Notice that in this case ``img`` was already protected because it was constructed from, and shared data with,
a NumPy array. The NumPy array ``array`` has now been modified as well by the filter.
