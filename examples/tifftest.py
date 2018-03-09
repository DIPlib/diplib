#!/usr/bin/env python3

# This Python script tests all code that reads grey-value data
# from TIFF files (that is, it doesn't exercise that code that
# reads binary and color-mapped data, which is less complex).
# It does not test reading multi-page TIFFs or the like, all
# input images are single-page RGB images with ZIP encoding.
# Other aspects of the TIFF reading capability are tested elsewhere.

# To run this test, you need to fist run `generate_tiff_tests.sh`
# in ../tools.

import sys, os

# Modify the path here to point to where you installed PyDIP:
# TODO: we need to install PyDIP to a place where Python will find it.
dipdir = os.getcwd() + '/../target/dip/lib'
sys.path.append( dipdir )

import PyDIP as dip

# Step 1: Read in full images, make sure they are the same

print("Step 1: full image")

first = dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif')

if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif')/257)[0]):
   print("Error: stripes01-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif'))[0]):
   print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif')/257)[0]):
   print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif'))[0]):
   print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif')/257)[0]):
   print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif'))[0]):
   print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif')/257)[0]):
   print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif'))[0]):
   print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif')/257)[0]):
   print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif'))[0]):
   print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif')/257)[0]):
   print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif'))[0]):
   print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif')/257)[0]):
   print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif'))[0]):
   print("Error: tifftest-tiles-separated-08")
if any(dip.Any(first - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif')/257)[0]):
   print("Error: tifftest-tiles-separated-16")

# Step 2: Read in 2nd tensor element only

print("Step 2: TensorElement(1)")

compare = first.TensorElement(1)

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-tiles-separated-16")

# Step 3: Read in 1st and 3rd tensor elements only

print("Step 3: TensorElement(slice(0,-1,2))")

compare = first.TensorElement(slice(0,-1,2))

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2)))[0]):
   print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0,0,1), [slice(0,-1,1), slice(0,-1,1)], slice(0,-1,2))/257)[0]):
   print("Error: tifftest-tiles-separated-16")

# Step 4: Read in all tensor elements for a continuous ROI

print("Step 4: [slice(2,601,1),slice(11,401,1)]")

compare = first[slice(2,601,1),slice(11,401,1)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1)))[0]):
   print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-tiles-separated-16")

# Step 5: Read in all tensor elements for a subsampled ROI

print("Step 5: [slice(2,601,3),slice(11,401,5)]")

compare = first[slice(2,601,3),slice(11,401,5)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1)))[0]):
   print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(0,-1,1))/257)[0]):
   print("Error: tifftest-tiles-separated-16")

# Step 4: Read in 2nd tensor element for a continuous ROI

print("Step 6: TensorElement(1)[slice(2,601,1),slice(11,401,1)]")

compare = first.TensorElement(1)[slice(2,601,1),slice(11,401,1)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1)))[0]):
   print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0,0,1), [slice(2,601,1), slice(11,401,1)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-tiles-separated-16")

# Step 5: Read in 2nd tensor element for a subsampled ROI

print("Step 7: TensorElement(1)[slice(2,601,3),slice(11,401,5)]")

compare = first.TensorElement(1)[slice(2,601,3),slice(11,401,5)]

if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes01-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes01-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes01-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes01-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes01-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes04-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes04-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes04-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes04-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes04-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes32-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes32-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-stripes32-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-stripes32-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-stripes32-separated-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-tiles-contiguous-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-contiguous-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-tiles-contiguous-16")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-08.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1)))[0]):
   print("Error: tifftest-tiles-separated-08")
if any(dip.Any(compare - dip.ImageReadTIFF('tifftest-tiles-separated-16.tif', slice(0,0,1), [slice(2,601,3), slice(11,401,5)], slice(1,1,1))/257)[0]):
   print("Error: tifftest-tiles-separated-16")
