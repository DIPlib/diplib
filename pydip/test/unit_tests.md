``pydip`` unit tests
===

First import PyDIP (``diplib``). Usually, we put it in the ``dip`` namespace.

    >>> import diplib as dip

PyDIP is compatible with ``numpy``. Let's get that too.

    >>> import numpy as np

dip.Image Constructors
---

Both the default constructor and a ``None`` argument lead to an unforged image.

    >>> dip.Image()
    <Empty image>
    >>> dip.Image(None)
    <Empty image>

A single number is interpreted as a 0D image, whose data type can be set optionally.

    >>> dip.Image(50)
    <Scalar image, SINT64, 0D>
    >>> dip.Image(50, 'SFLOAT')
    <Scalar image, SFLOAT, 0D>

A tuple or list is interpreted as the image size.

    >>> dip.Image((50,))
    <Scalar image, SFLOAT, sizes {50}>

This means there is no constructor to create a 0D tensor image. Use ``dip.Create0D`` instead.

    >>> dip.Create0D([50])
    <Scalar image, SINT64, 0D>
    >>> dip.Create0D([10., 20., 30.])
    <Tensor image (3x1 column vector, 3 elements), DFLOAT, 0D>
    >>> dip.Create0D(np.asarray([10., 20., 30.], dtype=np.float32))
    <Tensor image (3x1 column vector, 3 elements), SFLOAT, 0D>

We can also set the number of tensor elements, or the data type, or both.

    >>> dip.Image((50,), 2)
    <Tensor image (2x1 column vector, 2 elements), SFLOAT, sizes {50}>
    >>> dip.Image((50,), dt='DFLOAT')
    <Scalar image, DFLOAT, sizes {50}>
    >>> dip.Image((50,), 2, 'DFLOAT')
    <Tensor image (2x1 column vector, 2 elements), DFLOAT, sizes {50}>

**Note that when specifying the number of tensor elements, a single value is interpreted as the image size.**
    >>> dip.Image(50, 2)
    <Tensor image (2x1 column vector, 2 elements), SFLOAT, sizes {50}>

Numpy arrays (buffers in general) can be converted directly.
Note the reverse order of the dimensions!

    >>> dip.Image(np.zeros((10, 20)))
    <Scalar image, DFLOAT, sizes {20, 10}>

The data type is preserved.

    >>> dip.Image(np.zeros((10, 20), dtype=bool))
    <Scalar image, BIN, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.uint8))
    <Scalar image, UINT8, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.uint16))
    <Scalar image, UINT16, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.uint32))
    <Scalar image, UINT32, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.uint64))
    <Scalar image, UINT64, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.int8))
    <Scalar image, SINT8, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.int16))
    <Scalar image, SINT16, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.int32))
    <Scalar image, SINT32, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.int64))
    <Scalar image, SINT64, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.single))
    <Scalar image, SFLOAT, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=float))
    <Scalar image, DFLOAT, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=np.singlecomplex))
    <Scalar image, SCOMPLEX, sizes {20, 10}>
    >>> dip.Image(np.zeros((10, 20), dtype=complex))
    <Scalar image, DCOMPLEX, sizes {20, 10}>

As a convenience, images with the first or last dimension having less than 10 elements are interpreted as tensor images along that dimension.

    >>> dip.Image(np.zeros((10, 20, 3)))
    <Tensor image (3x1 column vector, 3 elements), DFLOAT, sizes {20, 10}>
    >>> dip.Image(np.zeros((3, 10, 20)))
    <Tensor image (3x1 column vector, 3 elements), DFLOAT, sizes {20, 10}>

If that is not desired, set the ``axis`` argument to ``None``.

    >>> dip.Image(np.zeros((3, 10, 20)), None)
    <Scalar image, DFLOAT, sizes {20, 10, 3}>

Alternatively, ``axis`` may be used to manually specify the tensor dimension.

    >>> dip.Image(np.zeros((3, 10, 20)), -1)
    <Tensor image (20x1 column vector, 20 elements), DFLOAT, sizes {10, 3}>

Use as and conversion to NumPy arrays
---

You can use a `dip.Image` object anywhere where a NumPy array is expected.

    >>> img = dip.Image(np.zeros((2, 5)))
    >>> np.sort(img)
    array([[0., 0., 0., 0., 0.],
           [0., 0., 0., 0., 0.]])

You can explicitly convert to a NumPy array.

    >>> img = dip.Image((3,2), dt='UINT8')
    >>> img.Fill(0)
    >>> np.array(dip.Convert(img, 'BIN'))
    array([[False, False, False],
           [False, False, False]])
    >>> np.array(dip.Convert(img, 'UINT8'))
    array([[0, 0, 0],
           [0, 0, 0]], dtype=uint8)
    >>> np.array(dip.Convert(img, 'UINT16'))
    array([[0, 0, 0],
           [0, 0, 0]], dtype=uint16)
    >>> np.array(dip.Convert(img, 'UINT32'))
    array([[0, 0, 0],
           [0, 0, 0]], dtype=uint32)
    >>> np.array(dip.Convert(img, 'UINT64'))
    array([[0, 0, 0],
           [0, 0, 0]], dtype=uint64)
    >>> np.array(dip.Convert(img, 'SFLOAT'))
    array([[0., 0., 0.],
           [0., 0., 0.]], dtype=float32)
    >>> np.array(dip.Convert(img, 'DFLOAT'))
    array([[0., 0., 0.],
           [0., 0., 0.]])
    >>> np.array(dip.Convert(img, 'SCOMPLEX'))
    array([[0.+0.j, 0.+0.j, 0.+0.j],
           [0.+0.j, 0.+0.j, 0.+0.j]], dtype=complex64)
    >>> np.array(dip.Convert(img, 'DCOMPLEX'))
    array([[0.+0.j, 0.+0.j, 0.+0.j],
           [0.+0.j, 0.+0.j, 0.+0.j]])

Indexing
---

First, indexing into an empty image is not allowed.

    >>> a = dip.Image()
    >>> a[0]
    Traceback (most recent call last):
        ...
    RuntimeError: Image is not forged

Indexing an image returns a ``Pixel``. As scalar images are treated as single-element tensors, this is a list.

    >>> a = dip.Image(50)
    >>> a[0] = 50
    >>> a[0]
    [50]
    >>> a = dip.Image((50,))
    >>> a.Fill(0)
    >>> a[0] = 50
    >>> a[0]
    [50.0]
    >>> a = dip.Image((10, 20))
    >>> a.Fill(0)
    >>> a[0, 0] = 50
    >>> a[0, 0]
    [50.0]

When indexing using only a single index, the *linear index* is used. This is different from ``numpy``, where an array is returned.

    >>> a[0]
    [50.0]

If we want to extract a row or a column, we need to use *slicing*.

``Pixel``s of tensor images can be set all at once.

    >>> a = dip.Image((10, 20), 3)
    >>> a.Fill([30, 40, 50])
    >>> a[5, 4] = [60, 70, 80]
    >>> a[5, 4]
    [60.0, 70.0, 80.0]

When a single value is given, it is copied to all ``Sample``s in the ``Pixel``.

    >>> a[5, 4] = 90
    >>> a[5, 4]
    [90.0, 90.0, 90.0]

It is also possible to index into an image using a binary mask image of the same size.

    >>> b = dip.Image(a.Sizes(), dt='BIN')
    >>> b.Fill(0)
    >>> b[0:-1:5, 0:-1:10] = 1
    >>> a[b] = [100, 110, 120]
    >>> a[b]
    <Tensor image (3x1 column vector, 3 elements), SFLOAT, sizes {4}>
    >>> a(1)[5, 10]
    [110.0]

Slicing
---

Slicing works the same as for ``numpy`` arrays, but again, note the reversed dimensions.

    >>> a = dip.Image((10, 20))
    >>> a[:, 0]
    <Scalar image, SFLOAT, sizes {10, 1}>
    >>> a[0, :]
    <Scalar image, SFLOAT, sizes {1, 20}>
    >>> a[:, 0] = 50
    >>> a[5, 0]
    [50.0]

**But advanced indexing is not allowed.**

    >>> a[[2, 3], 5]
    Traceback (most recent call last):
        ...
    TypeError: __getitem__(): incompatible function arguments. The following argument types are supported:

We may slice ranges as well. **Note that unlike normal Python syntax, the end of the range is included.**

    >>> a[0:2, 0]
    <Scalar image, SFLOAT, sizes {3, 1}>
    >>> a[:, 0:15:2]
    <Scalar image, SFLOAT, sizes {10, 8}>
    >>> a[2:9:3, 2] = 60
    >>> a[5, 2]
    [60.0]

Of course, ranges work for ``Pixel``s as well.

    >>> a = dip.Image((10, 20), 3)
    >>> a.Fill([30, 40, 50])
    >>> a[2:9:3, 2] = [60, 70, 80]
    >>> a[8, 2]
    [60.0, 70.0, 80.0]

Indexing tensor elements
---

The tensor elements can be indexed using function call syntax.

    >>> a = dip.Image((10, 20), 3)
    >>> a(0)
    <Scalar image, SFLOAT, sizes {10, 20}>

They may be operated upon as normal scalar images. Note the list return value, as a scalar image is still a 1D tensor image.

    >>> a(0).Fill(1)
    >>> a(0)[0, 0]
    [1.0]

As with the spatial dimensions, we can extract ranges as well, although we need to use the ``slice`` keyword. **Again, the end of the range is included.**

    >>> a(1).Fill(2)
    >>> a(slice(0, 1))[0, 0]
    [1.0, 2.0]

Of course, indexing an empty image is not allowed.

    >>> a = dip.Image()
    >>> a(0)
    Traceback (most recent call last):
        ...
    RuntimeError: Image is not forged

Implicit conversions
---

``None`` can be implicitly converted to an image.

    >>> a = dip.Image((10, 20))
    >>> dip.Mean(a, None)
    <Scalar image, SFLOAT, sizes {1, 1}>

Images can be implicitly converted to and from Python buffers (e.g. ``numpy`` arrays)

    >>> dip.Gauss(np.zeros((10, 20)))
    <Scalar image, DFLOAT, sizes {20, 10}>
    >>> a = dip.CreateRamp((10, 20), 0)
    >>> np.amax(a)
    4.0

Scalars can be implicitly converted to ``DimensionArray``s

    >>> dip.Gauss(a, 1)
    <Scalar image, SFLOAT, sizes {10, 20}>

``DimensionArray``s can be implicitly converted to and from Python lists

    >>> a = dip.CreateCoordinates((10, 20))
    >>> dip.Gauss(a, [1, 2])
    <Tensor image (2x1 column vector, 2 elements), SFLOAT, sizes {10, 20}>
    >>> dip.MaximumPixel(a(0))
    [9, 0]

Scalars can also be implicitly converted to ``Sample``s or ``Pixel``s

    >>> a.Fill(0)
    >>> dip.DrawBandlimitedBall(a, 30, (4, 6), 255, "filled", 5)

Operators
---

All standard operators are defined and behave the same as in the C++ library

    >>> b = a + 1
    >>> b -= 1
    >>> a == b
    <Tensor image (2x1 column vector, 2 elements), BIN, sizes {10, 20}>
    >>> dip.All(a == b)[0]
    [True, True]

In all operators, a list of values implicitly converts to a pixel

    >>> b += [5,2]
    >>> [5,2] + b
    <Tensor image (2x1 column vector, 2 elements), DFLOAT, sizes {10, 20}>

The multiplication operator (``*``) applies matrix multiplication to each tensor. We
need to make sure that the tensor sizes are compatible, tensors are always column
vectors by default, and we cannot multiply two column vectors together

    >>> b * [1,-1]
    Traceback (most recent call last):
    ...
    RuntimeError: Inner tensor dimensions must match in multiplication

This does not work to create a row vector

    >>> b * [[1,-1]]
    Traceback (most recent call last):
    ...
    TypeError: can't multiply sequence by non-int of type 'diplib.PyDIP_bin.Image'

To create a row vector, we'd need to use the ``dip.Create0D()`` function

    >>> b * dip.Create0D([1,-1]).Transpose()
    <Tensor image (2x2 column-major matrix, 4 elements), DFLOAT, sizes {10, 20}>
    >>> dip.Create0D([1,-1]).Transpose() * b
    <Scalar image, DFLOAT, sizes {10, 20}>

To do element-wise multiplication of tensors, use the ``dip.MultiplySampleWise()``
function

    >>> dip.MultiplySampleWise(b, [1,-1])
    <Tensor image (2x1 column vector, 2 elements), DFLOAT, sizes {10, 20}>
