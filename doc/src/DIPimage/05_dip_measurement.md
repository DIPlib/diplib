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


\page sec_dum_dip_measurement The `dip_measurement` Object

The function `measure` returns the measurement results in an object of type
`dip_measurement`. It contains all the measurements done on an image in a
manageable way.

\section sec_dum_dip_measurement_extracting Extracting measurement data

The data in the `dip_measurement` object can be accessed in a very
simple way, but only for reading, not writing (i.e. data manipulation is
not allowed).

Indexing with parentheses is used to access the measurements belonging
to one or more objects. The index used must match the label ID of the
object in the image, and the returned value is an object of type
`dip_measurement`.

The dot operator is used to extract the values corresponding to a single
measurement. The array returned is of type `double`.

For example,

```matlab
msr(11:15).Size
```

will return a `double` array with five elements, being the sizes for
objects number 11 through 15. Note that element 11 doesn't need to be
placed 11th in the list of measurements. If only objects starting at 10
were measured, the above example would be equivalent to

```matlab
msr.Size(2:6)
```

since `msr.Size` returns a `double` array, whose second element would be
the size of object number 11.

The `end` method will return the last label ID in the object. `double`
converts the data in the object to a `double` array, loosing the names
of the measurements and the label IDs.

If you want to handle the measurement results as a data table, use the
`table` function. The `table` class was introduced in *MATLAB* in release
R2013b, and provides a convenient way to work with tabular data.

\section sec_dum_dip_measurement_other_info Other information on the `dip_measurement` object

Besides extracting the measured data, you might want to gain more
knowledge on the object you are dealing with (e.g. which measurements
were taken and how many of them are there). This section describes
functions used for this purpose.

`fieldnames` returns the names of the measurements (features) present
in the object. `isfield` tests for a given feature to be present.

`isempty` returns true if there is no data in the object.

`size` returns the number of IDs as the first dimension, and the number
of values per object as the second. Note that the number of values
returned by size does not need to be equal to the number of features
returned by `fieldnames`. Some measurement features return more than one
value per object. `size(msr,'Size')` returns the number of values for the
`Size` feature. `size(double(msr))` returns the same value as `size(msr)`.

`length(msr)` is equivalent to `size(msr,1)`, and returns the number of
objects.

\section sec_dum_dip_measurement_combining Combining measurement data

To join measurements produced by different calls to `measure`, use the
default *MATLAB* syntax. Horizontal and vertical concatenations produce
different effects.

`[A,B]` joins two measurement objects with the same label IDs, but
different measurements. If some measurements are repeated, or if the
label IDs don't match, an error is generated.

`[A;B]` joins two measurement objects with the same measurements, on
different label IDs. If some IDs are repeated, or if the measurements
don't match, an error is generated.

In some cases, objects in different images have the same labels. These
need to be changed before concatenation is possible. This is done by the
following syntax:

```matlab
msr.id = 51:73;
```

The length of the array assigned to the IDs must have the same number of
elements as the measurement object.

Similarly, it is possible to measure the same thing on different images
of the same objects. For example, one might measure the average grey
value on all three channels of an RGB image. To join these measurements
into a single object, it is possible to add a prefix to the names of the
measurements:

```matlab
msr1.prefix = 'red_';
msr2.prefix = 'green_';
msr3.prefix = 'blue_';
msr = [msr1,msr2,msr3];
```

Note that this prefix cannot be changed, only added to. For example,

```matlab
msr.prefix = 'A';
msr.prefix = 'B';
```

causes the measurements in `msr` to have names like `'BASize'`.

`rmfield` removes the values for a given measurement feature from the
`dip_measurement` object.
