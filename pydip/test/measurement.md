Measurement with PyDIP
===

Import relevant modules

    >>> import diplib as dip
    >>> import numpy as np

Test data
---

    >>> a = dip.ImageRead(examples_dir + "/cermet.ics")
    >>> a.SetPixelSize(1, "um")
    >>> a
    <Scalar image, UINT8, sizes {256, 256}>
    >>> b = a < 120
    >>> b = dip.Label(b, minSize=30)
    >>> b = dip.EdgeObjectsRemove(b)
    >>> m = dip.MeasurementTool.Measure(b, a, ['Size', 'Solidity', 'Statistics'])
    >>> m
    <Measurement with 4 features for 43 objects>

Printing all data
---

    >>> print(m)
       |       Size |   Solidity |                                            Statistics | ConvexArea | 
    -- | ---------- | ---------- | ----------------------------------------------------- | ---------- | 
       |            |            |       Mean |     StdDev |   Skewness | ExcessKurtosis |            | 
       |   ...) |            |            |            |            |                |   ...) | 
    -- | ---------- | ---------- | ---------- | ---------- | ---------- | -------------- | ---------- | 
     6 |      262.0 |     0.9668 |      45.34 |      30.82 |     0.7216 |        -0.6831 |      271.0 | 
     7 |      63.00 |     0.9474 |      86.35 |      13.41 |     0.2313 |        -0.5471 |      66.50 | 
     8 |      243.0 |     0.9293 |      75.09 |      21.16 |     0.1711 |        -0.9723 |      261.5 | 
     9 |      209.0 |     0.9698 |      61.63 |      25.80 |     0.3937 |        -0.7994 |      215.5 | 
    10 |      462.0 |     0.9665 |      62.10 |      20.27 |     0.7329 |         0.1613 |      478.0 | 
    11 |      611.0 |     0.9745 |      81.17 |      17.92 |    -0.3812 |        -0.2219 |      627.0 | 
    12 |      80.00 |     0.9816 |      83.10 |      15.72 |     0.1468 |        -0.7721 |      81.50 | 
    13 |      205.0 |     0.9762 |      52.92 |      32.19 |     0.1556 |         -1.183 |      210.0 | 
    14 |      419.0 |     0.9836 |      41.60 |      30.24 |     0.8653 |        -0.3741 |      426.0 | 
    15 |      363.0 |     0.9041 |      71.56 |      22.25 |    -0.2541 |        -0.5946 |      401.5 | 
    16 |      487.0 |     0.9740 |      57.81 |      25.17 |    0.05945 |        -0.4846 |      500.0 | 
    17 |      383.0 |     0.9746 |      53.10 |      24.60 |     0.6360 |        -0.3009 |      393.0 | 
    18 |      250.0 |     0.9709 |      50.21 |      30.08 |     0.6251 |        -0.8159 |      257.5 | 
    20 |      137.0 |     0.9786 |      64.47 |      22.41 |     0.5215 |        -0.8983 |      140.0 | 
    21 |      378.0 |     0.9668 |      64.85 |      21.35 |     0.3866 |        -0.5561 |      391.0 | 
    22 |      392.0 |     0.9043 |      48.06 |      31.20 |     0.4776 |        -0.8514 |      433.5 | 
    23 |      230.0 |     0.9746 |      70.43 |      23.68 |    -0.2813 |        -0.6269 |      236.0 | 
    25 |      262.0 |     0.9686 |      62.26 |      25.31 |     0.3051 |        -0.7452 |      270.5 | 
    26 |      637.0 |     0.9245 |      52.94 |      23.86 |     0.8441 |       -0.08530 |      689.0 | 
    28 |      341.0 |     0.9757 |      54.94 |      25.06 |     0.8843 |        -0.3705 |      349.5 | 
    29 |      501.0 |     0.9747 |      51.85 |      24.15 |     0.9221 |       -0.05920 |      514.0 | 
    30 |      556.0 |     0.8580 |      60.65 |      22.53 |     0.5287 |        -0.3121 |      648.0 | 
    31 |      592.0 |     0.8889 |      58.28 |      29.00 |     0.1195 |         -1.026 |      666.0 | 
    32 |      172.0 |     0.9718 |      68.47 |      23.14 |     0.3064 |        -0.9392 |      177.0 | 
    33 |      566.0 |     0.9792 |      41.71 |      30.85 |     0.7348 |        -0.5709 |      578.0 | 
    35 |      842.0 |     0.9268 |      53.14 |      26.75 |     0.1291 |        -0.4931 |      908.5 | 
    37 |      209.0 |     0.9676 |      56.00 |      26.01 |     0.5350 |        -0.8241 |      216.0 | 
    38 |      147.0 |     0.9545 |      65.14 |      24.51 |     0.3733 |        -0.9707 |      154.0 | 
    39 |      375.0 |     0.9766 |      71.89 |      21.69 |    0.06353 |        -0.7623 |      384.0 | 
    40 |      385.0 |     0.9637 |      51.05 |      27.73 |     0.6729 |        -0.5471 |      399.5 | 
    41 |      223.0 |     0.9612 |      63.78 |      25.31 |     0.1825 |        -0.4636 |      232.0 | 
    42 |      347.0 |     0.9734 |      55.33 |      26.30 |     0.5900 |        -0.7111 |      356.5 | 
    43 |      604.0 |     0.9527 |      50.44 |      26.84 |     0.6709 |        -0.5829 |      634.0 | 
    44 |      354.0 |     0.9739 |      42.53 |      33.74 |     0.6403 |        -0.9280 |      363.5 | 
    45 |      543.0 |     0.9696 |      50.64 |      24.14 |      1.068 |         0.3071 |      560.0 | 
    47 |      147.0 |     0.9515 |      67.05 |      22.61 |     0.2393 |        -0.5154 |      154.5 | 
    48 |      405.0 |     0.9000 |      83.24 |      23.60 |    -0.9721 |      0.0003058 |      450.0 | 
    49 |      577.0 |     0.9714 |      30.64 |      31.71 |      1.246 |         0.2249 |      594.0 | 
    50 |      497.0 |     0.9717 |      61.73 |      18.86 |      1.101 |         0.3655 |      511.5 | 
    52 |      525.0 |     0.9813 |      34.06 |      31.89 |      1.047 |        -0.1825 |      535.0 | 
    53 |      803.0 |     0.9634 |      54.23 |      25.55 |     0.4471 |        -0.5974 |      833.5 | 
    54 |      253.0 |     0.9750 |      59.83 |      25.32 |     0.4961 |        -0.8077 |      259.5 | 
    55 |      193.0 |     0.9772 |      65.91 |      23.49 |     0.4554 |        -0.8702 |      197.5 | 
    <BLANKLINE>

Getting properties
---

The number of objects is the number of rows above

    >>> m.NumberOfObjects()
    43

The number of features is the number of column groups, and the number of values is the number of columns.
For example, `'Statistics'` is one feature that has 4 values.

    >>> m.NumberOfFeatures()
    4
    >>> m.NumberOfValues()
    7
    >>> m.NumberOfValues('Statistics')
    4

We can also test if a feature is present and if an object is present:

    >>> m.FeatureExists('Size')
    True
    >>> m.FeatureExists('Foo')
    False
    >>> m.ObjectExists(15)
    True
    >>> m.ObjectExists(5)
    False

Finally, we can retrieve all object IDs and information about all features:

    >>> m.Objects()
    [6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23, 25, 26, 28, 29, 30, 31, 32, 33, 35, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 49, 50, 52, 53, 54, 55]
    >>> m.Features()
    [FeatureInformation(name='Size', startColumn=0, numberValues=1), FeatureInformation(name='Solidity', startColumn=1, numberValues=1), FeatureInformation(name='Statistics', startColumn=2, numberValues=4), FeatureInformation(name='ConvexArea', startColumn=6, numberValues=1)]
    >>> m.Values()
    [ValueInformation(name='', units=...), ValueInformation(name='', units=), ValueInformation(name='Mean', units=), ValueInformation(name='StdDev', units=), ValueInformation(name='Skewness', units=), ValueInformation(name='ExcessKurtosis', units=), ValueInformation(name='', units=...)]

Note how `m.Features()` returns information about the column groups, whereas `m.Values()`
returns information about the columns. Combining both might be necessary to fully understand
how to interpret the data in the measurement object.

Casting to NumPy
---

The whole table can be cast to a NumPy array without copying the data:

    >>> n = np.asarray(m)
    >>> n.shape
    (43, 7)

Note that modifying the NumPy array will also modify the Measurement object:

    >>> n[0,0] = 555
    >>> m[6]['Size']
    [555.0]

We can also cast to a Pandas DataFrame with `m.ToDataFrame()`, but we don't test that here
to avoid the CI needing to install Pandas also.

Indexing
---

We can index both with a objectID and with a feature name, in either order:

    >>> m['Size'][15]
    [363.0]
    >>> m[15]['Size']
    [363.0]

The return value is always a list with the values for that feature and for that object, even if
there's only one value.

When using only one of the two indexing operations, we get an object representing a column group
or a row of the measurement table:

    >>> m['Size']
    <IteratorFeature for feature Size and 43 objects>
    >>> m[15]
    <IteratorObject with 4 features for object 15>

In C++ these are iterator objects, but not in the Python bindings (as of yet). However, these
objects can be iterated over:

    >>> t = 0
    >>> for s in m['Size']:
    ...     t += s[0]
    >>> t
    16523.0

    >>> for s in m[15]:
    ...     print(s)
    [363.0]
    [0.9041095890410958]
    [71.55922865013774, 22.252784082073312, -0.25412609396208..., -0.594586255044733]
    [401.5]

They can also be cast to a NumPy array without copy:

    >>> s = np.asarray(m['Size'])
    >>> s.shape
    (43, 1)

Note again that modifying the NumPy array will also modify the Measurement object: 

    >>> s[0,0] = 777
    >>> m[6]['Size']
    [777.0]

Since `m['Statistics']` is a column group with four columns, we need a way to specify
one specific column. Use the `Subset()` method for this. It modifies the object that
it is applied to.

    >>> s = m['Statistics']
    >>> s[15]
    [71.55922865013774, 22.252784082073312, -0.25412609396208..., -0.594586255044733]
    >>> s.Subset(1)
    <IteratorFeature for feature Statistics and 43 objects>
    >>> s[15]
    [22.252784082073312]

Dictionary behaviour
---

We have overloaded `len()` and added methods `keys()`, `values()` and `items()` for the
partially-indexed measurement object, so that they behave similarly to a dict.

`keys()` returns the values you can use to index, `values()` returns all the values
you'd obtain with those keys, and `items()` returns a list of all the (key, value) tuples.

    >>> r = m[15]
    >>> len(r)
    7
    >>> r.keys()
    ['Size', 'Solidity', 'Statistics', 'ConvexArea']
    >>> r.values()
    [[363.0], [0.9041095890410958], [71.55922865013774, 22.252784082073312, -0.25412609396208..., -0.594586255044733], [401.5]]
    >>> r.items()
    [('Size', [363.0]), ('Solidity', [0.9041095890410958]), ('Statistics', [71.55922865013774, 22.252784082073312, -0.25412609396208..., -0.594586255044733]), ('ConvexArea', [401.5])]

Note that `len()` returns the total number of values, not the number of features. So `len(r)` is not
the same as `len(r.keys())`. This might be problematic. TODO: Do we fix this? Do we want to?

    >>> c = m['Size']
    >>> len(c)
    43
    >>> c.keys()
    [6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23, 25, 26, 28, 29, 30, 31, 32, 33, 35, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 49, 50, 52, 53, 54, 55]
    >>> c.values()
    [[777.0], [63.0], [243.0], [209.0], [462.0], [611.0], [80.0], [205.0], [419.0], [363.0], [487.0], [383.0], [250.0], [137.0], [378.0], [392.0], [230.0], [262.0], [637.0], [341.0], [501.0], [556.0], [592.0], [172.0], [566.0], [842.0], [209.0], [147.0], [375.0], [385.0], [223.0], [347.0], [604.0], [354.0], [543.0], [147.0], [405.0], [577.0], [497.0], [525.0], [803.0], [253.0], [193.0]]
    >>> c.items()
    [(6, [777.0]), (7, [63.0]), (8, [243.0]), (9, [209.0]), (10, [462.0]), (11, [611.0]), (12, [80.0]), (13, [205.0]), (14, [419.0]), (15, [363.0]), (16, [487.0]), (17, [383.0]), (18, [250.0]), (20, [137.0]), (21, [378.0]), (22, [392.0]), (23, [230.0]), (25, [262.0]), (26, [637.0]), (28, [341.0]), (29, [501.0]), (30, [556.0]), (31, [592.0]), (32, [172.0]), (33, [566.0]), (35, [842.0]), (37, [209.0]), (38, [147.0]), (39, [375.0]), (40, [385.0]), (41, [223.0]), (42, [347.0]), (43, [604.0]), (44, [354.0]), (45, [543.0]), (47, [147.0]), (48, [405.0]), (49, [577.0]), (50, [497.0]), (52, [525.0]), (53, [803.0]), (54, [253.0]), (55, [193.0])]

These methods don't make sense for the full measurement object, because it can be
indexed in two ways. But `len(m)` does work, and produces the number of rows:

    >>> len(m)
    43
    >>> len(np.asarray(m))
    43

Operators
---

Comparison operators applied to a `FeatureIterator` return a `LabelMap` object, which can be used
to select rows from a `Measurement` object. Multiple `LabelMap` objects can be combined with
Boolean operators:

    >>> p = m['Size'] > 500
    >>> p.Count()  # the number of selected objects
    13
    >>> s = m['Statistics'].Subset(1) < 22
    >>> s.Count()
    8
    >>> ms = m[p & s]
    >>> ms.Objects()
    [11]

The `Relabel()` method of the `LabelMap` can be used to relabel the objects in the measurement:

    >>> p.Relabel()
    >>> m[p].Objects()
    [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13]

As well as relabel the image that the measurement was taken from:

    >>> bs = p.Apply(b)
