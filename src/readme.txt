Source files are spread out over subdirectories just to keep some order.

analysis/         Analysis (diplib/analysis.h)
binary/           Binary image processing (diplib/binary.h)
color/            Color spaces and transforms (diplib/color.h)
display/          Preparing images for display (diplib/display.h)
distance/         Distance transforms (diplib/distance.h)
file_io/          Image file I/O (diplib/file_io.h)
linear/           Linear filters (diplib/linear.h)
geometry/         Interpolation and geometric transformations (diplib/geometry.h)
generation/       Creating image data (diplib/generation.h)
histogram/        Histograms (diplib/histogram.h)
library/          Core library functionality (diplib/library/*.h, diplib/boundary.h, diplib/framework.h,
                                              diplib/neighborhood.h, diplib/pixel_table.h)
mapping/          Grey-value mapping (diplib/lookup_table.h, diplib/mapping.h)
math/             Pixel math (diplib/math.h, diplib/statistics.h)
measurement/      Measurement infrastructure and functions (diplib/measurement.h, diplib/chain_code.h)
microscopy/       Deconvolution, attenuation correction, stain unmixing, etc. (diplib/microscopy.h)
morphology/       Mathematical morphology (diplib/morphology.h)
nonlinear/        Non-linear filters (diplib/nonlinear.h)
regions/          Labeling and labeled image processing (diplib/regions.h)
segmentation/     Segmentation (diplib/segmentation.h)
support/          Functions that don't work on images (diplib/library/numeric.h)
transform/        Fourier and other transforms (diplib/transform.h)

documentation/    Markdown files

../dependencies/* External code needed for compilation of DIPlib and/or components,
                  but not necessary to compile code that uses DIPlib.

---

The old DIPlib sources have these additional directories, I thought
we could condense the directory structure a little bit:

adaptfilters/  -> nonlinear
bilateral/     -> nonlinear
derivatives/   -> linear
detection/     -> analysis
distribution/  -> histogram
findshift/     -> analysis
framework/     -> library
infra/         -> library
manipulation/  -> interpolation
noise/         -> generation
numerical/     -> support
paint/         -> generation
pgst/          -> nonlinear
pixel_table/   -> library
restoration/   -> microscopy
sort/          -> support
structure/     -> analysis
tpscalar/      -> library
