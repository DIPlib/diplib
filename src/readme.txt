Source files are spread out over subdirectories just to keep some order.

analysis/         Analysis (diplib/analysis.h)
binary/           Binary image processing (diplib/binary.h)
color/            Color spaces and transforms (diplib/color.h)
deconvolution/    Deconvolution (diplib/deconvolution.h)
detection/        Feature detection (diplib/detection.h)
display/          Preparing images for display (diplib/display.h)
distance/         Distance transforms (diplib/distance.h)
file_io/          Image file I/O (diplib/file_io.h)
generation/       Creating image data (diplib/generation.h)
geometry/         Interpolation and geometric transformations (diplib/geometry.h)
histogram/        Histograms (diplib/histogram.h, diplib/distribution.h)
library/          Core library functionality (diplib/library/*.h, diplib/boundary.h, diplib/framework.h,
                                              diplib/kernel.h, diplib/neighborlist.h, diplib/pixel_table.h)
linear/           Linear filters (diplib/linear.h)
mapping/          Grey-value mapping (diplib/lookup_table.h, diplib/mapping.h)
math/             Pixel math (diplib/math.h)
measurement/      Measurement infrastructure and functions (diplib/measurement.h, diplib/chain_code.h)
microscopy/       Stain unmixing, colocalization, attenuation correction, etc. (diplib/microscopy.h)
morphology/       Mathematical morphology (diplib/morphology.h)
nonlinear/        Non-linear filters (diplib/nonlinear.h)
regions/          Labeling and labeled image processing (diplib/regions.h)
segmentation/     Segmentation (diplib/segmentation.h)
statistics/       Image statistics (diplib/statistics.h)
support/          Functions that don't work on images (diplib/library/numeric.h)
transform/        Fourier and related transforms (diplib/transform.h, diplib/dft.h)
