\comment DIPlib 3.0

\comment (c)2017-2020, Cris Luengo.

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


\page why_tensors Why tensors?

Images in *DIPlib 3* have pixel values generalized to tensors. Currently only
tensors of order up to 2 are supported. Thus, pixel values can be scalar (0<sup>th</sup>-order
tensor), vector (1<sup>st</sup>-order) or matrix (2<sup>nd</sup> order). This covers all current
applications of multi-valued pixels, as far as we're aware. On this page we show
some examples of these applications that show the notational simplicity that comes
with the tensor representation. In each of the examples below, please think about
how an equivalent program would look if one did not have access to a tensor
representation.


\comment --------------------------------------------------------------

\section stain_unmixing Stain unmixing

In brightfield microscopy it is well-known that multiple stains on a slide generate
a linear mixture of those stains' color vectors (see Ruifrok & Johnston,
Anal Quant Cytol Histol 23(4):291-9, 2001, who unfortunately called it "color
deconvolution", a name that has stuck even though this has nothing to do
with deconvolution). Given, for example, two stains imaged in an RGB image,
then the absorption $a$ in each of the R, G and B channels is:

$$ \begin{bmatrix} a_R \\ a_G \\ a_B \end{bmatrix} =
   \begin{bmatrix} s_1^R & s_2^R \\
                   s_1^G & s_2^G \\
                   s_1^B & s_2^B \end{bmatrix}
   \begin{bmatrix} d_1 \\ d_2 \end{bmatrix} = S \, d $$

where $d_i$ are the optical densities of the stains at a particular point,
$d_i^R$ is the absorption of stain $i$ in the red channel, etc. The actual
intensity measured by the camera is the transmittance $t$, given by

$$ t = -10^a $$

(as a fraction of the incident light intensity);
see [Beer-Lambert law](https://en.wikipedia.org/wiki/Beer–Lambert_law).

Thus, given `img`, an RGB image recorded in a brightfield microscope, the intensity
of the background $I$, and the mixing matrix $S$, you would follow the steps above
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
images `S` and `I`, corresponding to the mixing matrix $S$ and the background intensity
vector $I$. These are a single pixel because the same properties are valid across
the image. In the arithmetic operations, singleton-expansion causes these 0D images
to be expanded to the size of the image `img`, such that each pixel is multiplied
by the same matrix (though this expansion happens implicitly, there's not actually such an
image created in memory). Next, note that the operation `img / I` is a per-element
division, such that `img[0]` (red channel) is divided by `I[0]`, `img[1]` by `I[1]`,
and `img[2]` by `I[2]`. In contrast, `U * t` is a matrix multiplication.

The above is implemented in the functions \ref dip::BeerLambertMapping and \ref dip::UnmixStains.


\comment --------------------------------------------------------------

\section structure_tensor The structure tensor

The structure tensor (see [Wikipedia](https://en.wikipedia.org/wiki/Structure_tensor))
is defined by the gradient of the image:

$$ S = \overline{(\nabla I)(\nabla I)^T} = \begin{bmatrix}
       \overline{I_x^2} & \overline{I_x I_y} \\
       \overline{I_x I_y} & \overline{I_y^2} \end{bmatrix} $$

where $I_x$ is the partial derivative of the image $I$ in the $x$ direction, and the
overline indicates local averaging. The equation above shows the
structure tensor for a 2D image, but the expression on the left is valid for
any number of dimensions. The eigenvalues of $S$ describe the local neighborhood
of a pixel. If we sort the eigenvalues such that $\lambda_1 > \lambda_2$,
then it is possible to define a measure of anisotropy as
$\frac{\lambda_1 - \lambda_2}{\lambda_1 + \lambda_2}$
or simply as $1 - \frac{\lambda_2}{\lambda_1}$, and an energy
measure as $\lambda_1 + \lambda_2$. Anisotropy is high on lines and edges,
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

The above is implemented in the functions \ref dip::StructureTensor and
\ref dip::StructureTensorAnalysis.


\comment --------------------------------------------------------------

\section harris The Harris corner detector

The well-known Harris corner detector (see
[Wikipedia](https://en.wikipedia.org/wiki/Corner_detection#The_Harris_.26_Stephens_.2F_Plessey_.2F_Shi.E2.80.93Tomasi_corner_detection_algorithms))
is based on the structure tensor. Pixels where the two eigenvalues are large
are corners. It is common to use the following equivalence to avoid computation
of the eigenvalues:

$$ \lambda_1 \lambda_2 - \kappa \, (\lambda_1 + \lambda_2)^2 = \mathrm{det}\,S - \kappa \, (\mathrm{trace}\,S)^2 $$

We can compute this in *DIPlib* as follows, again starting with a scalar
image `img`:

```cpp
dip::Image g = dip::Gradient( img );
dip::Image S = dip::Gauss( g * dip::Transpose( g ), 5 );
dip::Image trace = dip::Trace( S );
dip::Image corners = dip::Determinant( S ) - k * trace * trace;
```

Note again that is is possible to compute `corners` more efficiently
by \ref iterators. That way, one can run through the image `S` only once,
and avoid the temporary intermediate images.

The above is implemented in the function \ref dip::HarrisCornerDetector.


\comment --------------------------------------------------------------

\section optical_flow Lucas-Kanade optical flow

The Lucas-Kanade solution to the optical flow problem (see
[Wikipedia](https://en.wikipedia.org/wiki/Lucas–Kanade_method))
also involves the structure tensor. The problem to solve is $Av=b$,
where $A$ is a matrix with $x$ and $y$ partial derivatives at each
pixel in a neighborhood, $b$ is a vector with $t$ derivatives at each
of those pixels, and $v$ is the velocity vector for the neighborhood.
This is rewritten as $A^T A v = A^T b$, where $A^T A$ is the
structure tensor. Using *DIPlib*, and assuming a 3D image `img` where
the third dimension is time, we can write:

```cpp
dip::Image A = dip::Gradient( img, { 1.0 }, "best", {}, { true, true, false } );
dip::Image b = -dip::Derivative( img, { 0, 0, 1 } );
dip::Image ATA = dip::Gauss( A * dip::Transpose( A ), 5 );
dip::Image ATb = dip::Gauss( A * b, 5 );
dip::Image v = dip::Inverse( ATA ) * ATb;
```

The most complicated function call is on the first line. Up to now we had
been using all the default parameters to \ref dip::Gradient, but now we need
to set the `process` parameter: a boolean array indicating along which
dimensions to compute the derivative. Since this parameter is towards the
end of the parameter list, we must fill out the other default values,
which we simply copied from the function declaration.


\comment --------------------------------------------------------------

\section diffusion Anisotropic diffusion

There are many more examples where per-pixel matrix algebra is useful and
*DIPlib* allows simple, efficient implementation. The last example we'll
give here is from the function \ref dip::CoherenceEnhancingDiffusion.

The diffusion equation can be discretized along the time axis to yield
an iterative update process described by

$$ I^{t+1} = I^t + \lambda \, \mathrm{div} \left( D \, \nabla I^t \right) \; . $$

If $D$ is constant (spatially and in time), the above iterative process
leads to Gaussian smoothing. By adjusting $D$ to be small at edges,
an anisotropic, edge-enhancing diffusion is obtained. Coherence enhancing
diffusion uses a tensor for $D$. The above-mentioned function allows
to construct this tensor in two ways, starting from (yet again!) the
structure tensor $S$. Here we apply an eigen decomposition, leading
to a full matrix image $V$ (the eigenvectors), and a diagonal matrix
image $E$ (the eigenvalues):

$$ S = V \, E \, V^T \; . $$

Next we compute $t = \mathrm{trace}\,E^{-1}$ and $E' = \frac{1}{t} E^{-1}$,
and re-compose a tensor using these new eigenvalues:

$$ D = V \, E' \, V^T \; . $$

Using *DIPlib* we can write:

```cpp
dip::Image S = dip::StructureTensor( img ); // see above for how this is computed
dip::Image E, V;
dip::EigenDecomposition( S, E, V );
E = 1 / E;
E /= dip::Trace( E );
dip::Image D = V * E * dip::Transpose( V );
img += lambda * dip::Divergence( D * dip::Gradient( img ));
```

Iterating this bit of code leads to a coherence enhancing diffusion simulation
on the image `img`.
