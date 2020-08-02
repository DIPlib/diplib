# Changes DIPimage 3.0.beta

Here we list the changes to *DIPimage* as compared to version 2.9. *DIPimage* is built on *DIPlib*,
which was completely rewritten for this release. Consequently, there are many changes to *DIPimage*
as well. It is possible that we missed some changes here, but hopefully this list will help in
porting your old code that used *DIPimage* to the new version.

- It is no longer necessary to call `dip_initialise` (which doesn't exist any more). The *DIPlib*
  library no longer requires initialization.

- The `dip_image` object has changed completely internally. Pixel data is stored differently:
  tensor images have all samples in the same *MATLAB* array. Complex images are stored as a
  single real matrix, with real and complex samples next to each other (this translates
  much more nicely to *DIPlib*, where complex data is stored in that same way).

  Consequently, the `dip_image_array` pseudo-class no longer exists. Both scalar and tensor
  images report to be of class `dip_image`. If you need to store multiple images in one
  object, create a cell array instead.

  Additionally, the tensor no longer can hold arbitrary number of dimensions, it is limited
  to vectors and matrices, as it is in *DIPlib*. I don't expect this to impact any user's code,
  and I'd be happy to hear if you actually used tensors with more than two dimensions.

  We tried keeping how the `dip_image` object is used as similar as possible to how it was
  in *DIPimage 2*, so that users need not change their code. Nonetheless, some changes
  must occur:

  - The `size` method was discouraged, we asked users to use `imsize` and `imarsize` instead.
    Those two functions work as before, except that they don't return 1 for dimensions that
    don't exist. `size` now always works like `imsize`, except that it always returns at least
    two values, and will return 1 for non-existing dimensions. This makes it similar to the
    default `size`, and plays nice with the `whos` command. `tensorsize` is like `imarsize`,
    but with a more sensical name, since image arrays no longer exist.

  - `end` was discouraged as well, as the meaning changed from `dip_image` to `dip_image_array`.
    Now `end` will always work for spatial dimensions, and cannot be used with tensor indexing.
    That is, `end` will work in `()` indexing, but not in `{}` indexing.

  - Methods that changed behavior for scalar and tensor images (i.e. `sum`, `max`, etc.) now
    always operate only along spatial dimensions. To sum over the tensor dimension, convert
    it to a spatial dimension using `tensortospatial`.

  - Likewise, methods such as `cat` used to concatenate images in an `dip_image_array`, and
    consequently concatenated the various tensor elements as separate images. These functions
    now all work along spatial dimensions only, leaving the tensor dimension unaffected.

  - The `reshape` method now takes pixels column-wise instead of row-wise from the input. This
    is the natural way of doing it, as it doesn't require data copy. It also makes the function
    consistent with linear indexing (reshaping an image doesn't change the linear index of the
    pixels). We presume there are few (if any) programs that depend on the old behavior. See
    [this section of the *DIPimage* User Manual](https://diplib.github.io/diplib-docs/sec_dum_dip_image.html#sec_dum_dip_image_reshape)
    for more details).

  - The `squeeze` method now might reorder dimensions, for the same reasoning as the change to
    `reshape`. The new preference `'CheapSqueeze'` can be used to revert to the old behavior
    if necessary. See
    [this section of the *DIPimage* User Manual](https://diplib.github.io/diplib-docs/sec_dum_dip_image.html#sec_dum_dip_image_reshape)
    for more details).

  - There are slight changes to how array data types are handled when converting to a `dip_image`
    object and when using an array as input to a *DIPimage* function. Most importantly, the
    *DIPlib* interface translates double-precision scalar arrays to 0D images of a single-precision
    type as long as that does not cause overflow or underflow. This typically yields the expected
    results. To avoid this behavior, explicitly cast to `dip_image`. For example: `img + 1` vs
    `img + dip_image(1)`.

  - The `inner` and `outer` methods no longer exist, use `cross` and `dot`.

  - New methods: `bessely`, `clone`, `cosh`, `cumsum`, `erfc`, `flip`, `gammaln`, `iscomplex`, `issigned`, `isunsigned`,
    `numArgumentsFromSubscript`, `numberchannels`, `numpixels`, `numtensorel`, `sinh`, `slice_ex`, `slice_in`,
    `slice_op`, `spatialtotensor`, `swapdim`, `tanh`, `tensorfun`, `tensorsize`, `tensortospatial`.

  - `eig_largest` has been moved to the toolbox directory, and `eig` has gained an option to return
    only the eigenvector corresponding to the largest or the smallest eigenvalue.

  - `length` and `unique` are no longer methods of `dip_image`.

- Related to the tensor image changes above, `newimar` and `imarfun` are now in the `alias`
  subdirectory, and are identical to the new functions `newtensorim` and `tensorfun`.

- The `dip_measurement` object has changed completely internally. The interface is identical
  except:

  - It is not (yet?) possible to add a feature.

  - It is no longer possible to convert to/from a `struct`. However, it is possible to convert
    to a `table`.

  - Fixed a bug: `msr.featureID` now returns an array that is transposed w.r.t. previous versions.
    This was a bug that we never fixed because of backwards compatibility, we took this
    opportunity to fix it. Now we have total consistency: no matter how the measurement data are
    extracted or converted, objects are always rows, and features are always columns.

  - For consistency, `msr.ID` now returns object IDs as a column vector (objects are rows).

  - A new method, `remap`, implements what used to be the independent function `msr_remap`.
    `msr_remap` is in the `alias` subdirectory.

- The interactive image display window from `dipshow` has had some internal changes.
  User-visible changes are:

   - Three new options in the "Mappings" menu for 3D and 4D images, labeled "Slice", "Max projection"
     and "Mean projection". These allow to display max or mean projection instead of the default
     thin slice.

   - Removal of the "Orientation testing" (2D), and "Max projection" and "Sum projection" (3D)
     options in the "Actions" menu. The two functions that implemented this functionality,
     `diporien` and `dipprojection`, have also been removed.

   - Color images are converted to the sRGB color space for display, instead of the linear RGB color
     space. This replaces the previously used `'Gamma'` preference setting, which is no longer
     applied. The `'GammaGrey'` setting is also no longer applied.

   - A new function `viewslice` can be used to display any image (including tensor images and
     higher-dimensional images) in [*DIPviewer*](https://diplib.github.io/diplib-docs/group__viewer.html#viewer_ui).
     This is an alternative way to examine images, but none of the tools to programmatically
     interact with images displayed through `dipshow` will work with this viewer.

- Configuration settings accessed through `dipsetpref` and `dipgetpref` have been changed since
  *DIPimage 2*:

   - Added settings:

       - `'CheapSqueeze'` controls the behavior of the `dip_image/squeeze` method. Set it to `'off'`
         to mimic the behavior of *DIPimage* versions prior to 3.0 (see the
         [*DIPimage* User Manual](https://diplib.github.io/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_cheapsqueeze)
         for more details).

       - `'DisplayFunction'` can be set to either `'dipshow'` (the default), `'viewslice'`, or
         `'view5d'`, and determines which of these functions is invoked by default when the
         `dip_image/display` method is invoked (see the
         [*DIPimage* User Manual](https://diplib.github.io/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_displayfunction)
         for more details).

       - `'FtOption'` adds one option to the `ft` and `ift` functions, and allows to configure
         *DIPimage* be be compatible with versions prior to 3.0 (see the
         [*DIPimage* User Manual](https://diplib.github.io/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_ftoption)
         for more details).

   - Changed settings:

       - `'KeepDataType'`, which changes how the output data type for arithmetic operations is
         chosen, has the same intent but does not always make the same choice. For example
         the result is different when mixing signed and unsigned integers. (see the
         [*DIPimage* User Manual](https://diplib.github.io/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_keepdatatype)
         for details on the current logic).

       - `'Gamma'` and `'GammaGrey'` are no longer active, though they haven't (yet) been removed.

   - Removed settings:

       - `'BoundaryCondition'`, `'Truncation'`, and `'DerivativeFlavour'`, because relevant
         functions take these parameters as optional input arguments.

       - `'MorphologicalFlavour'`, as it didn't seem useful (the associated function
         `dip_morph_flavour` has also been removed).

       - `'ComputationLimit'`, because image arithmetic is now always performed by *DIPlib*.

       - `'ConflictingPixelSize'` and `'InconsistentPixelSize'`, because the *DIPlib* library
         keeps track of pixel sizes and handles these situations in a fixed manner.

       - `'CommandFilePath'`, because the *DIPimage* GUI no longer probes functions to
         put them in the menus.

       - `'DebugMode'`, as *DIPlib* stack traces are shown or not depending on a
         compile-time flag.

       - `'FFTtype'`, because *DIPlib* can optionally be compiled to use *FFTW* for the FFT.

       - `'FastSubscriptedAssignment'`, introduced recently in *DIPimage 2.9*, because the
         related functionality has not been ported over (yet?).

- Many filters now have a boundary condition parameter. In *DIPimage 2*, one would change
  the boundary condition through a global parameter, which no longer exists. If you never
  touched the global parameter, nothing should change for you. If you did change this global
  parameter in a program, you now need to pass the value to the relevant functions instead.
  See `help boundary_condition`.

- Many functions have been added to match new functionality in *DIPlib*, as well as previous
  functionality that was not accessible from *MATLAB*. Some old functions have gained additional
  parameters to expose more functionality; we tried to do so without breaking backwards
  compatibility.

- Some functions have changed in a way that is not backwards compatible, either by having a
  different interface or different behavior:

  - `dilation_se`, `erosion_se`, etc. are now folded into `dilation`, `erosion`, etc. For
    functions with an `_se` appended to the name, remove the `_se`. Alternatively, the `alias`
    sub-directory contains these names and forwards the calls to the correct functions.

  - `rankmax_opening` and `rankmin_closing` now have `rank` as the second parameter, instead
    of `percentile`. This should make them match the literature better, and make them more
    usable.

  - `smooth` is no longer relevant, moved to the `alias` directory. Use `gaussf` instead.

  - `derivative` has the 2<sup>nd</sup> and 3<sup>rd</sup> arguments switched, it makes more
    sense having the order first.

  - `mdhistogram` has fewer options, but should still be able to produce the same results as
    previously. In `hist2image`, the `coords` input is transposed w.r.t. previous versions,
    for consistency: each row is interpreted as a vertex.

  - `ft` now does normalization in the more common way (forward transform not normalized,
    inverse transform normalized by 1/N), but an option (`'symmetric'`) allows to change
    the normalization to be consistent with *DIPimage 2*, which used a symmetric normalization
    scheme (both forward and backward transforms use 1/N<sup>1/2</sup>).

  - `resample` and `shift` shift the image in the opposite direction from what it did in
    *DIPimage 2*, where the shift was unintuitive.

  - `resample` produces, by default, a floating-point output image also for linear interpolation.
    The function now responds to the `'KeepDataType'` setting, making it possible to produce
    an interpolated result of the same data type as the input image.

  - `affine_trans` rotates in the opposite direction it did before, to match the interpretation
    of angles in the rest of the toolbox. The function now also accepts an affine transformation
    matrix to direct the transformation. The function `transform` provided duplicate functionality
    and no longer exists. `find_affine_trans` no longer exists, as `fmmatch` provides a much better
    algorithm to do the same.

  - `label` now returns an unsigned integer image (previously it was a signed integer image).
    All functions that work on labeled images expect an unsigned integer image as input.

  - `readim` and `writeim` work differently now, in part because *DIPlib* natively supports
    fewer file types now. The `file_info` struct output for `readim` has changed somewhat. The last
    two input parameters to the old `writeim` are no longer supported (`compression` and `physDim`):
    to change the compression method, call `writeics` or `writetiff` directly; the pixel size is
    always given by the image, use `dip_image/pixelsize` to set it. `readics` and `readtiff` are
    the interfaces to the corresponding *DIPlib* file reading functions. RGB TIFF and JPEG images
    are now set to the `'sRGB'` color space, rather than the (linear) `'RGB'` color space.

  - The AVI reading and writing functions `readavi`, `writeavi` and `writedisplayavi` depended on
    outdated *MATLAB* functionality and have been removed. You can use *MATLAB*'s video reading and
    writing capabilities, converting between standard *MATLAB* arrays and a  `dip_image` is trivial.

  - `countneighbours` has been renamed to `countneighbors` for consistency in spelling.

  - `mse`, `mae` and several similar functions have been collected in a new function `errormeasure`.
    The old functions are still available in the `alias` sub-directory.

  - `slice_in` and `slice_ex` specify dimensions starting at 1 (not at 0 as previously). This
    change is for consistency with the rest of the toolbox. These functions now reorder dimensions
    in a way consistent with `dip_image/squeeze` (removing dimension 1 causes dimension 3 to become
    dimension 1), which is more efficient.
    `slice_in`, `slice_ex` and `slice_op` have been made into `dip_image` methods, for efficiency.

  - `get_subpixel` no longer supports the `spline` interpolation method, this string is still
    accepted, but selects the `cubic` method.

  - `structuretensor3d` has been moved to the `alias` directory, `structuretensor` now works for any
    number of dimensions (with special support for 2D and 3D images).

  - `curvature_thirion` and `isophote_curvature` have been moved to the `alias` directory. The function
    `curvature` now takes `'thirion'` and `'isophote'` as options. `orientation4d` has been moved to the
    `alias` directory. A new function `orientation` generalizes it to arbitrary dimensionality.

  - `granulometry` has changed, but it is still possible to call it the old way. However, the parameters
    in this old syntax are interpreted to match the new capabilities of this function. `'usegrey'` and
    `'verbose'` options no longer have an effect. Default values have changed a bit.

  - `ht` no longer exists. Use `riesz` instead. The Riesz transform is the multi-dimensional generalization
    of the 1D Hilbert transform. See also the new function `monogenicsignal`.

  - `nconv` no longer exists, the new function `normconv` will compute the normalized convolution with
    a Gaussian, as well as estimate first derivatives using a Gaussian normalized convolution.

  - `dt` has a new algorithm, which is used by default. It gives exact results and works for any number of
    dimensions, and it should be faster than the previous default algorithm. The old default algorithm can
    be executed with `dt(...,'fast')`.

  - `gdt` has a new algorithm, which is used by default. It approximates Euclidean distances better. The old
    default algorithm can be executed with `gdt(...,3)`. The second output image is no longer produced. A
    new, optional argument specifies a mask image to further constrain paths.

  - `testobject` has a changed interface, the input argument order has changed and most arguments are now
    name-value pairs. Its functionality has been greatly extended.

  - `radoncircle` and `orientationspace` have a different implementation with different capabilities and a different interface.

  - `measurehelp` is no longer a separate function. You can now do `measure help` or `measure('help')`.

  - `jacobi` moved to the `alias` directory, the `eig` method has improved to make `jacobi` irrelevant.

  - `split` was shadowing an important new function in MATLAB. It has been renamed to `splitim`. To minimize issues
    with backwards-compatibility there now is a `split` method for `dip_image` objects, and a `split` function in the
    `alias` directory.

- New functions not mentioned above: `abssqr`, `areaclosing`, `areaopening`, `asf`, `cell2im`, `cluster`,
  `compactwaterseed`, `coordinates`,  `cornerdetector`, `curlvector`, `distancedistribution`, `divergencevector`,
  `drawshape`, `extendregion`, `getmaximumandminimum`, `getsamplestatistics`, `im2cell`, `integral_image`, `lee`,
  `linedetector`, `loggabor`, `nonmaximumsuppression`, `pathclosing`, `pathopening`, `perobjecthist`, `psf`,
  `quantize`, `randomseeds`, `rngseed`, `select`, `semivariogram`, `setborder`, `skew`, `smallobjectsremove`,
  `stochasticwatershed`, `superpixels`, `thetatheta`, `traceobjects`, `warp_subpixel`, `window`, `wrap`.
  Use `help <functionname>` in *MATLAB* to learn what these functions provide.

- Old functions not (yet) ported:
  `afm_flatten`, `arcf`, `backgroundoffset`, `change_chroma`, `change_gamma`, `change_xyz`,
  `color_rotation`, `cpf`, `deblock`, `distancebetweenpointsets`, `dpr`, `find_lambda`,
  `findospeaks`, `frc`, `gamut_destretch`, `gamut_mapping`, `gamut_stretch`, `huecorr`, `hybridf`,
  `iso_luminance_lines`, `jpeg_quality_score`, `lfmse`, `luminance_steered_dilation`, `luminance_steered_erosion`,
  `make_gamut`, `mappg`, `mcd`, `morphscales`, `nufft_type1`, `nufft_type2`, `orientationplot`, `out_of_gamut`,
  `percf_adap`, `percf_adap_banana`, `plot_gamut`, `pst`, `quadraturetensor`, `rgb_to_border`, `rotation3d`,
  `splitandmerge`, `structf`, `tikhonovmiller`, `write_add`.
  These functions will be ported as they are needed.

- Old functions that will not be ported:
  `acquireim`, `dipmaxaspect`, `edir`, `fixlsmfile`, `fast_str2double`, `msr2ds`,
  `measure_gamma_monitor`, `mon_rgb2xyz`, `mon_xyz2rgb`, `monitor_icc`, `print_cmy2xyz`, `print_xyz2cmy`,
  `printer_icc`, `read_icc_profile`, `scan_rgb2xyz`, `scan_xyz2rgb`, `scanner_calibration`, `scanner_icc`,
  `spectra2xyz`, `write_icc_profile`.

- Old functions that used to be in the `alias` directory are no longer. We recommend that you correct affected code,
  but if you want, you can always create those aliases again (copy the files from an older version of *DIPimage*).

- If you customized the menus in the *DIPimage* GUI, you will have to update your `localdipmenus.m`
  file. If you wrote your own functions that integrated in the GUI, you'll have to do so through
  your `localdipmenus.m` now. Preference setting `'CommandFilePath'` no longer exists, nor
  the associated function `dipaddpath`, only functions mentioned in `dipmenus` and `localdipmenus`
  are added to the menu system. Functions are no longer probed with `'DIP_GetParamList'`,
  their parameter descriptions are given by `dipmenus` and `localdipmenus`.

- `getparams` no longer exists. It was a nifty idea but incurred a large overhead. Each function now must
  do its own input parameter checking, for example through `inputParser` and/or `validateattributes`.
