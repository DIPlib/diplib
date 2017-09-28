# Why tensors? {#why_tensors}

[//]: # (DIPlib 3.0)

[//]: # ([c]2017, Cris Luengo.)

[//]: # (Licensed under the Apache License, Version 2.0 [the "License"];)
[//]: # (you may not use this file except in compliance with the License.)
[//]: # (You may obtain a copy of the License at)
[//]: # ()
[//]: # (   http://www.apache.org/licenses/LICENSE-2.0)
[//]: # ()
[//]: # (Unless required by applicable law or agreed to in writing, software)
[//]: # (distributed under the License is distributed on an "AS IS" BASIS,)
[//]: # (WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.)
[//]: # (See the License for the specific language governing permissions and)
[//]: # (limitations under the License.)

Images in *DIPlib* 3.0 have pixel values generalized to tensors. Currently only
tensors of order up to 2 are supported. Thus, pixel values can be scalar (0th-order
tensor), vector (1st-order) or matrix (2nd order). This covers all current
applications of multi-valued pixels, as far as we're aware. On this page we show
some examples of these applications that show the notational symplicity that comes
with the tensor representation. In each of the examples below, please think about
how an equivalent program would look if one did not have access to a tensor
representation.

\tableofcontents

[//]: # (--------------------------------------------------------------)

\section stain_unmixing Stain unmixing

In brightfield microscopy it is well-known that multiple stains on a slide generate
a linear mixture of those stains' color vectors (see Ruifrok & Johnston, 
Anal Quant Cytol Histol 23(4):291-9, 2001, who unfortunately called it "color
deconvolution", a name that has stuck even though this has nothing to do
with deconvolution). Given, for example, two stains imaged in an RGB image,
then the absorption *a* in each of the R, G and B channels is:

\f$\begin{bmatrix} a_R \\ a_G \\ a_B \end{bmatrix} = \begin{bmatrix}
   s_1^R & s_2^R \\
   s_1^G & s_2^G \\
   s_1^B & s_2^B
\end{bmatrix} \begin{bmatrix} d_1 \\ d_2 \end{bmatrix} = S \, d\f$

where \f$d_i\f$ are the optical densities of the stains at a particular point,
\f$d_i^R\f$ is the absorption of stain *i* in the red channel, etc. The actual
intensity measured by the camera is the transmittance *t*, given by

\f$t = -10^a\f$

(as a fraction of the incident light intensity);
see [Beer-Lambert law](https://en.wikipedia.org/wiki/Beer–Lambert_law).

Thus, given `img`, an RGB image recorded in a brightfield microscope, the intensity
of the background *I*, and the mixing matrix *S*, you would follow the steps above
in reverse:

```cpp
    dip::Image I{ I_R, I_G, I_B };
    dip::Image t = dip::Log10( -img / I );
    dip::Image S{ S_1R, S_1G, S_1B, S_2R, S_2G, S_2B };
    S.ReshapeTensor( 3, 2 );
    dip::Image U = dip::PseudoInverse( S );
    dip::Image d = U * t;
```

Note that the most verbose part of the code above is to create the 0D (single-pixel)
images `S` and `I`, corresponding to the mixing matrix *S* and the background intensity 
vector *I*. These are a single pixel because the same properties are valid across
the image. In the arithmetic operations, singleton-expansion causes these 0D images
to be expanded to the size of the image `img`, such that each pixel is muliplied
by the same matrix (though this happens inplicitly, there's not actually such an
image created in memory). Next, note that the operation `img / I` is a per-element
division, such that `img[0]` (red channel) is divided by `I[0]`, `img[1]` by `I[1]`,
and `img[2]` by `I[2]`. In contrast, `U * t` is a matrix multiplication. 

[//]: # (--------------------------------------------------------------)

\section structure_tensor The structure tensor

The structure tensor (see [Wikipedia](https://en.wikipedia.org/wiki/Structure_tensor))
is defined by the gradient of the image:

\f$S = \overline{(\nabla I)(\nabla I)^T} = \begin{bmatrix}
   \overline{I_x^2} & \overline{I_x I_y} \\
   \overline{I_x I_y} & \overline{I_y^2}
\end{bmatrix}\f$

where \f$I_x\f$ is the partial derivative of the image *I* in the *x* direction, and the 
overline indicates local averaging. The equation above shows the
structure tensor for a 2D image, but the expression on the left is valid for
any number of dimensions. The eigenvalues of *S* describe the local neighborhood
of a pixel. If we sort the eigenvalues such that \f$\lambda_1 > \lambda_2\f$,
then it is possible to define a measure of anisotropy as
\f$\frac{\lambda_1 - \lambda_2}{\lambda_1 + \lambda_2}\f$
or simply as \f$1 - \frac{\lambda_2}{\lambda_1}\f$, and an energy
measure as \f$\lambda_1 + \lambda_2\f$. Anisotropy is high on lines and edges,
and small on uniform areas. Furthermore, the direction of the eigenvector
corresponding to the largest eigenvalue gives the orientation perpendicular to
the line or edge.

The anisotropy measure generalizes to higher-dimensional images. For example,
in a 3D image it is possible to distinguish lines (two large eigenvalues),
planes (one large eigenvalue), and isotropic areas (all eigenvalues approximately
equal). 

To compute the structure tensor using *DIPlib*, we start with a scalar image `img`:

```cpp
    dip::Image g = dip::Gradient( img );
    dip::Image S = dip::Gauss( g * dip::Transpose( g ), 5 );
```

The image `S` is a 2-by-2 tensor image if `img` is a 2D image, or a 5-by-5 tensor
image if `img` is a 5D image. Note that, since `g * Transpose(g)` yields a
symmetric matrix, `S` is stored in such a way that no duplicate matrix elements
are stored (nor computed). That is, for the 2D case, `S` has 3 tensor elements,
corresponding to the two diagonal elements and the one unique off-diagonal element.
The constant 5 in the computation of `S` is the sigma of the Gaussian smoothing.
This value needs to be adjusted depending on the scale of the structures we are
interested in.

```cpp
   dip::Image e = dip::Eigenvalues( S );
   dip::Image anisoptropy = ( e[0] - e[1] ) / ( e[0] + e[1] );
```

Note that it is possible to compute the `anisotropy` image more efficiently
(running through the image `e` once instead of three times, and avoiding
the storage of two intermediate images). See \ref iterators.

The above is implemented in the functions `dip::StructureTensor` and
`dip::StructureTensorAnalysis`.

[//]: # (--------------------------------------------------------------)

\section harris The Harris corner detector

The well-known Harris corner detector (see
[Wikipedia](https://en.wikipedia.org/wiki/Corner_detection#The_Harris_.26_Stephens_.2F_Plessey_.2F_Shi.E2.80.93Tomasi_corner_detection_algorithms))
is based on the structure tensor. Pixels where the two eigenvalues are large
are corners. It is common to use the following equivalence to avoid computation
of the eigenvalues:

\f$\lambda_1 \lambda_2 - \kappa \, (\lambda_1 + \lambda_2)^2 = \textrm{det}(S) - \kappa \, \textrm{trace}^2(S)\f$

We can compute this in *DIPlib* as follows, again starting with a scalar
image `img`:

```cpp
    dip::Image g = dip::Gradient( img );
    dip::Image S = dip::Gauss( g * dip::Transpose( g ), 5 );
    dip::Image trace = dip::Trace( S );
    dip::Image corners = dip::Determinant( S ) - k * trace * trace;
```

Note again that is is possible to compute `corners` more efficiently
by \ref iterators. That way, one can run throug the image `S` only once,
and avoid the temporary intermediate images.

[//]: # (--------------------------------------------------------------)

\section optical_flow Lucas-Kanade optical flow

The Lucas-Kanade solution to the optical flow problem (see
[Wikipedia](https://en.wikipedia.org/wiki/Lucas–Kanade_method))
also involves the structure tensor. The problem to solve is \f$Av=b\f$,
where *A* is a matrix with *x* and *y* partial derivatives at each
pixel in a neighborhood, *b* is a vector with *t* derivatives at each
of those pixels, and *v* is the velocity vector for the neighborhood.
This is rewritten as \f$A^{T}Av=A^{T}b\f$, where \f$A^{T}A\f$ is the
structure tensor. Using *DIPlib*, and assuming a 3D image `img` where
the third dimesion is time, we can write:

```cpp
    dip::Image A = dip::Gradient( img, { 1.0 }, "best", {}, { true, true, false } );
    dip::Image b = -dip::Derivative( img, { 0, 0, 1 } );
    dip::Image ATA = dip::Gauss( A * dip::Transpose( A ), 5 );
    dip::Image ATb = dip::Gauss( A * b, 5 );
    dip::Image v = dip::Inverse( ATA ) * ATb;
```

The most complicated function call is on the first line. Up to now we had
been using all the default parameters to `dip::Gradient`, but now we need
to set the `process` parameter: a boolean array indicating along which
dimensions to compute the derivative. Since this parameter is towards the
end of the parameter list, we must fill out the other default values,
which we simply copied from the function declaration.
