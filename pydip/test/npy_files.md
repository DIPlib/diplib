Testing reading and writing of NPY files
===

Import relevant modules

    >>> import diplib as dip
    >>> import numpy as np

Write some arrays with NumPy, then read in as DIPlib images
---

A test matrix, 2D (double float), write using NumPy

    >>> fname = 'test2Da'
    >>> data0 = np.random.rand(25, 28)
    >>> np.save(fname, data0)
    >>> img0 = dip.Image(data0)

Load using DIPlib and compare

    >>> dip.ImageIsNPY(fname)
    True
    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])

A test matrix, 3D (16-bit unsigned int), write using NumPy

    >>> fname = 'test3Da'
    >>> data0 = np.reshape(np.arange(0, 10*11*12), (10, 11, 12)).astype(np.uint16)
    >>> np.save(fname, data0)
    >>> img0 = dip.Image(data0)

Load using DIPlib and compare

    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])

A test matrix, 1D (single complex), write using NumPy

    >>> fname = 'test1Da'
    >>> data0 = np.random.rand(25, 2).astype(np.single)
    >>> data0 = data0.view(dtype=np.csingle)
    >>> np.save(fname, data0)
    >>> img0 = dip.Image(data0)

Load using DIPlib and compare

    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])

Write some images with DIPlib, then read in as NumPy arrays and as DIPlib images
---

A test image, 2D (single float), write using DIPlib

    >>> fname = 'test2Db.npy'
    >>> data0 = np.random.rand(25, 28).astype(np.single)
    >>> img0 = dip.Image(data0)
    >>> dip.ImageWriteNPY(img0, fname)

Load using NumPy and compare

    >>> data = np.load(fname)
    >>> assert(data.dtype == data0.dtype)
    >>> assert(data.shape == data0.shape)
    >>> assert(np.all(data == data0))
    >>> assert(not np.isfortran(data))

Load using DIPlib and compare

    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])

Permute the image, so that its strides are reversed and saved in fortran order by DIPlib

    >>> fname = 'test2Dc.npy'
    >>> img0.PermuteDimensions((1, 0))
    <Scalar image, SFLOAT, sizes {25, 28}>
    >>> img0.Strides()
    [28, 1]
    >>> dip.ImageWriteNPY(img0, fname)
    >>> data0 = np.asarray(img0)

Load using NumPy and compare

    >>> data = np.load(fname)
    >>> assert(data.dtype == data0.dtype)
    >>> assert(data.shape == data0.shape)
    >>> assert(np.all(data == data0))
    >>> assert(np.isfortran(data))

Load using DIPlib and compare

    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])
    >>> img.Strides()
    [28, 1]

A test image, 3D (32-bit signed int), write using DIPlib

    >>> fname = 'test3Db.npy'
    >>> data0 = np.reshape(np.arange(0, 10*11*12), (10, 11, 12)).astype(np.int32)
    >>> img0 = dip.Image(data0)
    >>> dip.ImageWriteNPY(img0, fname)

Load using NumPy and compare

    >>> data = np.load(fname)
    >>> assert(data.dtype == data0.dtype)
    >>> assert(data.shape == data0.shape)
    >>> assert(np.all(data == data0))
    >>> assert(not np.isfortran(data))

Load using DIPlib and compare

    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])

3D with arbitrary dimension order, write using DIPlib

    >>> fname = 'test3Dc.npy'
    >>> img0.PermuteDimensions((0, 2, 1))
    <Scalar image, SINT32, sizes {12, 10, 11}>
    >>> img0.Mirror(1)
    <Scalar image, SINT32, sizes {12, 10, 11}>
    >>> dip.ImageWriteNPY(img0, fname)
    >>> data0 = np.asarray(img0)

Load using NumPy and compare

    >>> data = np.load(fname)
    >>> assert(data.dtype == data0.dtype)
    >>> assert(data.shape == data0.shape)
    >>> assert(np.all(data == data0))
    >>> assert(not np.isfortran(data))

Load using DIPlib and compare

    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])

A test matrix, 1D (double complex), write using DIPlib

    >>> fname = 'test1Db.npy'
    >>> data0 = np.random.rand(25, 2)
    >>> data0 = data0.view(dtype=np.cdouble)
    >>> img0 = dip.Image(data0)
    >>> dip.ImageWriteNPY(img0, fname)

Load using NumPy and compare

    >>> data = np.load(fname)
    >>> assert(data.dtype == data0.dtype)
    >>> assert(data.shape == data0.shape)
    >>> assert(np.all(data == data0))

Load using DIPlib and compare

    >>> img = dip.ImageReadNPY(fname)
    >>> assert(img.DataType() == img0.DataType())
    >>> assert(img.Sizes() == img0.Sizes())
    >>> assert(dip.All(img == img0)[0][0])

TODO: how to test the endianness?
