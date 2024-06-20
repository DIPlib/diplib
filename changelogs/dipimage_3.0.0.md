---
layout: post
title: "Changes DIPimage 3.0.0"
date: 2021-02-22
---

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
      [this section of the *DIPimage* User Manual](https://diplib.org/diplib-docs/sec_dum_dip_image.html#sec_dum_dip_image_reshape)
      for more details).

    - The `squeeze` method now might reorder dimensions, for the same reasoning as the change to
      `reshape`. The new preference `'CheapSqueeze'` can be used to revert to the old behavior
      if necessary. See
      [this section of the *DIPimage* User Manual](https://diplib.org/diplib-docs/sec_dum_dip_image.html#sec_dum_dip_image_reshape)
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
      higher-dimensional images) in [*DIPviewer*](https://diplib.org/diplib-docs/dipviewer.html#viewer_ui).
      This is an alternative way to examine images, but none of the tools to programmatically
      interact with images displayed through `dipshow` will work with this viewer.

- Configuration settings accessed through `dipsetpref` and `dipgetpref` have been changed since
  *DIPimage 2*:

    - Added settings:

        - `'CheapSqueeze'` controls the behavior of the `dip_image/squeeze` method. Set it to `'off'`
          to mimic the behavior of *DIPimage* versions prior to 3.0 (see the
          [*DIPimage* User Manual](https://diplib.org/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_cheapsqueeze)
          for more details).

        - `'DisplayFunction'` can be set to either `'dipshow'` (the default), `'viewslice'`, or
          `'view5d'`, and determines which of these functions is invoked by default when the
          `dip_image/display` method is invoked (see the
          [*DIPimage* User Manual](https://diplib.org/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_displayfunction)
          for more details).

        - `'FtOption'` adds one option to the `ft` and `ift` functions, and allows to configure
          *DIPimage* be be compatible with versions prior to 3.0 (see the
          [*DIPimage* User Manual](https://diplib.org/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_ftoption)
          for more details).

    - Changed settings:

        - `'KeepDataType'`, which changes how the output data type for arithmetic operations is
          chosen, has the same intent but does not always make the same choice. For example
          the result is different when mixing signed and unsigned integers. (see the
          [*DIPimage* User Manual](https://diplib.org/diplib-docs/sec_dum_customizing.html#sec_dum_customizing_dippref_keepdatatype)
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

    - `rotation3d` now produces consistent output with the `'direct'` and `'9 shears'` methods.

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
  `drawshape`, `extendregion`, `getmaximumandminimum`, `getsamplestatistics`, `growregions`, `growregionsweighted`,
  `im2cell`, `integral_image`, `lee`,
  `linedetector`, `loggabor`, `nonmaximumsuppression`, `pathclosing`, `pathopening`, `perobjecthist`, `psf`,
  `quantize`, `randomseeds`, `rngseed`, `select`, `semivariogram`, `setborder`, `skew`, `smallobjectsremove`,
  `stochasticwatershed`, `superpixels`, `thetatheta`, `traceobjects`, `warp_subpixel`,
  `watershedmaxima`, `watershedminima`, `window`, `wrap`.
  Use `help <functionname>` in *MATLAB* to learn what these functions provide.

- Old functions not (yet) ported:
  `afm_flatten`, `arcf`, `backgroundoffset`, `change_chroma`, `change_gamma`, `change_xyz`,
  `color_rotation`, `cpf`, `deblock`, `distancebetweenpointsets`, `dpr`, `find_lambda`,
  `findospeaks`, `frc`, `gamut_destretch`, `gamut_mapping`, `gamut_stretch`, `huecorr`, `hybridf`,
  `iso_luminance_lines`, `jpeg_quality_score`, `lfmse`, `luminance_steered_dilation`, `luminance_steered_erosion`,
  `make_gamut`, `mappg`, `mcd`, `morphscales`, `nufft_type1`, `nufft_type2`, `orientationplot`, `out_of_gamut`,
  `percf_adap`, `percf_adap_banana`, `plot_gamut`, `pst`, `quadraturetensor`, `rgb_to_border`,
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

- The low-level interface has been removed. This interface was composed of functions starting with `dip_` or `dipio_`,
  and directly mapped the *DIPlib* functions. Removing this interface simplifies the toolbox code,
  as the "normal" toolbox functions did a bunch of parameter parsing, and then called the relevant *DIPlib* function
  through this low-level interface. Now, the "normal" toolbox functions directly call the relevant *DIPlib* function,
  which does the parameter parsing. This is a list of old low-level interface functions, with the corresponding function
  to replace it with:
    - `dip_adaptivebanana` → `gaussf_adap_banana`
    - `dip_adaptivegauss` → `gaussf_adap`
    - `dip_adaptivepercentile` → `percf_adap`, but not yet ported
    - `dip_adaptivepercentilebanana` → `percf_adap_banana`, but not yet ported
    - `dip_and` → `&`
    - `dip_arcfilter` → `arcf`, but not yet ported
    - `dip_areaopening` → `areaopening` or `areaclosing`
    - `dip_arith` → `+`, `-`, `*`, `/`.
    - `dip_armdhistogram` → `mdhistogram`
    - `dip_armdhistogrammap` → `mdhistogrammap`
    - `dip_attenuationcorrection` → not available (but exists in *DIPlib*)
    - `dip_biasedsigma` → not available
    - `dip_bilateral` → `bilateralf`
    - `dip_bilateralfilter` → `bilateralf`
    - `dip_bilateralfilter2` → `bilateralf`
    - `dip_binaryanchorskeleton2d` → not available
    - `dip_binaryclosing` → `bclosing`
    - `dip_binarydilation` → `bdilation`
    - `dip_binaryerosion` → `berosion`
    - `dip_binarynoise` → `noise`
    - `dip_binaryopening` → `bopening`
    - `dip_binarypropagation` → `bpropagation`
    - `dip_binaryrandomvariable` → not available, *MATLAB* has built-in support for random number generation
    - `dip_binaryskeleton3d` → not available, but `bskeleton` produces a 3D skeleton, if not always correct
    - `dip_canny` → `canny`
    - `dip_changebyteorder` → not available
    - `dip_chordlength` → `chordlength`
    - `dip_cityblockdistancetopoint` → not available (but exists in *DIPlib*)
    - `dip_clip` → `clip`
    - `dip_closing` → `closing`
    - `dip_compare` → `<`, `<=`, `~=`, `==`, `>=`, `>`
    - `dip_contraststretch` → `stretch`
    - `dip_convolve1d` → `convolve`
    - `dip_convolveft` → `convolve`
    - `dip_crop` → `cut`
    - `dip_croptobetterfouriersize` → not available, but see the `fast` mode of the `ft` function
    - `dip_crosscorrelationft` → `crosscorrelation`
    - `dip_cumulativesum` → `integral_image`
    - `dip_danielsonlinedetector` → `linedetector`
    - `dip_derivative` → `derivative`
    - `dip_dgg` → `dgg`
    - `dip_dilation` → `dilation`
    - `dip_directedpathopening` → `pathopening`, `pathclosing`
    - `dip_distancebetweenpointsets` → not available
    - `dip_dmllibfile` → not available, no longer meaningful
    - `dip_drawline` → `drawline`
    - `dip_drawlines` → `drawline`
    - `dip_edgeobjectsremove` → `brmedgeobjs`
    - `dip_edt` → `dt`
    - `dip_ellipticdistancetopoint` → not available (but exists in *DIPlib*)
    - `dip_erfclip` → `erfclip`
    - `dip_erosion` → `erosion`
    - `dip_euclideandistancetopoint` → not available (but exists in *DIPlib*)
    - `dip_euclideanskeleton` → `bskeleton`
    - `dip_exit` → not available, no longer necessary
    - `dip_exponentialfitcorrection` → not available (but exists in *DIPlib*)
    - `dip_extendregion` → `extendregion`
    - `dip_findshift` → `findshift`
    - `dip_finitedifference` → `derivative`
    - `dip_finitedifferenceex` → `derivative`
    - `dip_fm` → not available, but `gdt` provides a different implementation of the Fast Marching algorithm
    - `dip_fouriertransform` → `ft`, `ift`
    - `dip_ftbox` → `testobject`
    - `dip_ftcross` → `testobject`
    - `dip_ftcube` → `testobject`
    - `dip_ftellipsoid` → `testobject`
    - `dip_ftgaussian` → `testobject`
    - `dip_ftsphere` → `testobject`
    - `dip_gaboriir` → `gabor`
    - `dip_gauss` → `gaussf`
    - `dip_gaussft` → `gaussf`
    - `dip_gaussiannoise` → `noise`
    - `dip_gaussianrandomvariable` → not available, *MATLAB* has built-in support for random number generation
    - `dip_gaussiansigma` → not available
    - `dip_gaussiir` → `gaussf`
    - `dip_gdt` → `gdt`
    - `dip_generalconvolution` → `convolve`
    - `dip_generalisedkuwahara` → `selectionf`
    - `dip_generalisedkuwaharaimproved` → `selectionf`
    - `dip_generateramp` → `ramp`, `xx`, `yy`, `zz`
    - `dip_getboundary` → not available (the boundary condition is now a parameter to functions that use it)
    - `dip_getlibraryinformation` → not available, information is available in the `Contents.m` file
    - `dip_getmaximumandminimum` → `getmaximumandminimum`
    - `dip_getmeasurefeatures` → `measure('features')`
    - `dip_getnumberofthreads` → `dipgetpref('numberofthreads')`
    - `dip_getobjectlabels` → `unique(dip_array(img))`
    - `dip_gettruncation` → not available (the truncation is now a parameter to functions that use it)
    - `dip_gradientdirection2d` → `angle(gradientvector(img))`
    - `dip_gradientmagnitude` → `gradmag`
    - `dip_growregions` → `growregions`
    - `dip_growregionsweighted` → `growregionsweighted`
    - `dip_hartleytransform` → not available
    - `dip_hysteresisthreshold` → `threshold` with the `'hysteresis'` method
    - `dip_idivergence` → `errormeasure`
    - `dip_imagechaincode` → `traceobjects`
    - `dip_imagelut` → `lut`
    - `dip_imarinvlut` → not available
    - `dip_imarlut` → not available
    - `dip_incoherentotf` → `psf`
    - `dip_incoherentpsf` → `psf`
    - `dip_initialise_libs` → not available, no longer necessary
    - `dip_isodatathreshold` → `threshold` with the `'isodata'` method
    - `dip_kuwahara` → `kuwahara`
    - `dip_kuwaharaimproved` → `kuwahara`
    - `dip_label` → `label`
    - `dip_laplace` → `laplace`
    - `dip_laplacemindgg` → `laplace_min_dgg`
    - `dip_laplaceplusdgg` → `laplace_plus_dgg`
    - `dip_lee` → `lee`
    - `dip_linefit` → not available
    - `dip_localminima` → `watershedminima`, `watershedmaxima`
    - `dip_map` → `mirror`, `dip_image/permute`
    - `dip_maxima` → `maxima`
    - `dip_maximum` → `dip_image/max`
    - `dip_maximumpixel` → `dip_image/max`
    - `dip_mdhistogram` → `mdhistogram`
    - `dip_mdhistogrammap` → `mdhistogrammap`
    - `dip_mean` → `dip_image/mean`
    - `dip_measure` → `measure`
    - `dip_medianfilter` → `medif`
    - `dip_minima` → `minima`
    - `dip_minimum` → `dip_image/min`
    - `dip_minimumpixel` → `dip_image/min`
    - `dip_mirror` → `mirror`
    - `dip_modulofloatperiodic` → `dip_image/mod`
    - `dip_morphologicalgradmag` → not available (but exist in *DIPlib*)
    - `dip_morphologicalrange` → not available (but exist in *DIPlib*)
    - `dip_morphologicalreconstruction` → `reconstruction`
    - `dip_morphologicalsmoothing` → not available (but exist in *DIPlib*)
    - `dip_morphologicalthreshold` → not available (but exist in *DIPlib*)
    - `dip_multiscalemorphgrad` → not available (but exist in *DIPlib*)
    - `dip_nonmaximumsuppression` → `nonmaximumsuppression`
    - `dip_objecttomeasurement` → `msr2obj`
    - `dip_opening` → `opening`
    - `dip_or` → `|`
    - `dip_orientationspace` → `orientationspace`
    - `dip_orientedgauss` → not available, but can be emulated with `gaussf_adap`
    - `dip_paircorrelation` → `paircorrelation`
    - `dip_pathopening` → `pathopening`, `pathclosing`
    - `dip_percentile` → `dip_image/percentile`
    - `dip_percentilefilter` → `percf`
    - `dip_pgst3dline` → not available
    - `dip_pgst3dsurface` → not available
    - `dip_poissonnoise` → `noise`
    - `dip_poissonrandomvariable` → not available, *MATLAB* has built-in support for random number generation
    - `dip_positionmaximum` → `dip_image/max`
    - `dip_positionminimum` → `dip_image/min`
    - `dip_positionpercentile` → `dip_image/percentile`
    - `dip_probabilisticcorrelation` → `paircorrelation`
    - `dip_prod` → `dip_image/prod`
    - `dip_pseudoinverse` → not available
    - `dip_quantizedbilateralfilter` → `bilateralf`
    - `dip_radialdistribution` → not available
    - `dip_radialmaximum` → `radialmax`
    - `dip_radialmean` → `radialmean`
    - `dip_radialminimum` → `radialmin`
    - `dip_radialsum` → `radialsum`
    - `dip_randomseed` → `rngseed`
    - `dip_randomvariable` → not available, *MATLAB* has built-in support for random number generation
    - `dip_rangethreshold` → `threshold` with the `'double'` method.
    - `dip_rankcontrastfilter` → not available
    - `dip_resampleat` → `get_subpixel`
    - `dip_resampling` → `resample`
    - `dip_resamplingft` → `resample`
    - `dip_rotation` → `rotation`
    - `dip_rotation_with_bgval` → not available, but can be implemented by first extending the image with the given background value, and then selecting the appropriate boundary condition in `rotation`
    - `dip_rotation3d` → `rotation3d`
    - `dip_rotation3d_axis` → `rotation`
    - `dip_rotation3daxis` → `rotation`
    - `dip_seededwatershed` → `waterseed`
    - `dip_separableconvolution` → `convolve`
    - `dip_setboundary` → not available (the boundary condition is now a parameter to functions that use it)
    - `dip_setnumberofthreads` → `dipsetpref('numberofthreads')`
    - `dip_settruncation` → not available (the truncation is now a parameter to functions that use it)
    - `dip_sharpen` → not available (but exist in *DIPlib*), can be implemented with `img - weight*laplace(img)`
    - `dip_shift` → `shift`
    - `dip_sigma` → not available
    - `dip_simplegaussfitimage` → not available (but exist in *DIPlib*)
    - `dip_simulatedattenuation` → not available (but exist in *DIPlib*)
    - `dip_skewing` → `skew`
    - `dip_smallobjectsremove` → `smallobjectsremove`
    - `dip_sobelgradient` → `sobelf`
    - `dip_sortindices` → not available
    - `dip_standarddeviation` → `dip_image/std`
    - `dip_structureadaptivegauss` → `gaussf_adap`
    - `dip_structureanalysis` → not available (but exist in *DIPlib*)
    - `dip_structuretensor2d` → `structuretensor`
    - `dip_structuretensor3d` → `structuretensor`
    - `dip_subpixellocation` → `subpixlocation`
    - `dip_subpixelmaxima` → `findmaxima`
    - `dip_subpixelminima` → `findminima`
    - `dip_subsampling` → `subsample`
    - `dip_sum` → `dip_image/sum`
    - `dip_svd` → `dip_image/svd`
    - `dip_symmetriceigensystem2` → `dip_image/eig`
    - `dip_symmetriceigensystem3` → `dip_image/eig`
    - `dip_systemdoctor` → not available
    - `dip_tensorimageinverse` → `dip_image/inv`
    - `dip_testobjectaddnoise` → `testobject`
    - `dip_testobjectblur` → `testobject`
    - `dip_testobjectcreate` → `testobject`
    - `dip_testobjectmodulate` → `testobject`
    - `dip_threshold` → `threshold` with the `'fixed'` method, or just `>=`
    - `dip_tikhonovmiller` → not available
    - `dip_tikhonovregparam` → not available
    - `dip_tophat` → `tophat`
    - `dip_uniform` → `unif`
    - `dip_uniformnoise` → `noise`
    - `dip_uniformrandomvariable` → not available, *MATLAB* has built-in support for random number generation
    - `dip_upperenvelope` → not available
    - `dip_upperskeleton2d` →  not available (but exist in *DIPlib*)
    - `dip_variancefilter` → `varif`
    - `dip_vdt` → `vdt`
    - `dip_watershed` → `watershed`
    - `dip_wiener` → `wiener`
    - `dip_wrap` → `wrap`
    - `dip_xor` → `dip_image/xor`
    - `dipio_appendrawdata` → not available
    - `dipio_colour2gray` → `dip_image/colorspace`
    - `dipio_getimagereadformats` → not available
    - `dipio_getimagewriteformats` → not available
    - `dipio_getlibraryinformation` → not available
    - `dipio_imagefilegetinfo` → not available (`readim` returns file info as as 2nd output arguments)
    - `dipio_imageread` → `readim`, `readrawim`, `readics`, `readtiff`
    - `dipio_imagereadcolour` → `readim`
    - `dipio_imagereadcolourseries` → `readtimeseries`
    - `dipio_imagereadroi` → `readroiim`
    - `dipio_imagereadtiff` → `readtiff`
    - `dipio_imagewrite` → `writeim`, `writeics`, `writetiff`
    - `dipio_imagewritecolour` → `writeim`
    - `dipio_imagewriteics` → `writeics`
