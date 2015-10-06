Source files are spread out over subdirectories just to keep some order.

library/          Core library functionality
support/          Support functionality used by some image analysis routines

analysis/         Analysis (shift estimation, pair correlations, etc.)
binary/           Binary image processing
color/            Color spaces and transforms
distance/         Distance transforms
filtering/        Filters (linear & nonlinear smoothing, derivatives)
framework/        Frameworks
generation/       Creating image data
histogram/        Histograms
interpolation/    Interpolation and geometric transformations
math/             Pixel math
measurement/      Measurement infrastructure and functions
microscopy/       Deconvolution, attenuation correction, dye unmixing, etc.
morphology/       Mathematical morphology
regions/          Labeling and labeled image processing
segmentation/     Segmentation
transform/        Fourier and other transform

documentation/    Markdown files

---

The old DIPlib sources have these additional directories, I thought
we could condense the directory structure a little bit:

adaptfilters/  -> filtering
bilateral/     -> filtering
derivatives/   -> filtering
detection/     -> analysis
display/       -> support
distribution/  -> library
findshift/     -> analysis
infra/         -> library
linear/        -> filtering
manipulation/  -> interpolation
noise/         -> generation? math?
numerical/     -> support
paint/         -> generation
pgst/          -> filtering
pixel_table/   -> support or library
point/         -> math (threshold to segmentation)
restoration/   -> microscopy
sort/          -> support
structure/     -> analysis
tpscalar/      -> library
