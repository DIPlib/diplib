#!/usr/bin/env python3

# This Python script tests all code that reads grey-value data
# from TIFF files (that is, it doesn't exercise that code that
# reads binary and color-mapped data, which is less complex).
# It does not test reading multi-page TIFFs or the like, all
# input images are single-page RGB images with ZIP encoding.
# Other aspects of the TIFF reading capability are tested elsewhere.

# To run this test, you need to fist run `generate_tiff_tests.sh`
# in ../tools.

import diplib as dip


print("Step 1: Read in full images, make sure they are the same")

first = dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif')

if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif') / 257)[0]):
    print("Error: stripes01-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif'))[0]):
    print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif') / 257)[0]):
    print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif'))[0]):
    print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif') / 257)[0]):
    print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif'))[0]):
    print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif') / 257)[0]):
    print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif'))[0]):
    print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif') / 257)[0]):
    print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif'))[0]):
    print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif') / 257)[0]):
    print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif'))[0]):
    print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif') / 257)[0]):
    print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif'))[0]):
    print("Error: tifftest-tiles-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif') / 257)[0]):
    print("Error: tifftest-tiles-separated-16")


print("Step 2: Read in 2nd tensor element only")

compare = first(1)

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif',  slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-separated-16")


print("Step 3: Read in 1st and 3rd tensor elements only")

compare = first(slice(0, -1, 2))

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)))[0]):
    print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0, 0, 1), [slice(0, -1, 1), slice(0, -1, 1)], slice(0, -1, 2)) / 257)[0]):
    print("Error: tifftest-tiles-separated-16")


print("Step 4: Read in all tensor elements for a continuous ROI")

compare = first[slice(2, 601, 1), slice(11, 401, 1)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-separated-16")


print("Step 5: Read in all tensor elements for a subsampled ROI")

compare = first[slice(2, 601, 3), slice(11, 401, 5)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)))[0]):
    print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(0, -1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-separated-16")


print("Step 6: Read in 2nd tensor element for a continuous ROI")

compare = first(1)[slice(2, 601, 1), slice(11, 401, 1)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 1), slice(11, 401, 1)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-separated-16")


print("Step 7: Read in 2nd tensor element for a subsampled ROI")

compare = first(1)[slice(2, 601, 3), slice(11, 401, 5)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)))[0]):
    print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0, 0, 1), [slice(2, 601, 3), slice(11, 401, 5)], slice(1, 1, 1)) / 257)[0]):
    print("Error: tifftest-tiles-separated-16")
