#!/usr/bin/env bash

# This program generates a series of TIFF images to test the TIFF image reader in DIPlib.
# Images generated:
#    tifftest-stripes01-contiguous-08
#    tifftest-stripes01-contiguous-16
#    tifftest-stripes04-contiguous-08
#    tifftest-stripes04-contiguous-16
#    tifftest-stripes32-contiguous-08
#    tifftest-stripes32-contiguous-16
#    tifftest-tiles-contiguous-08
#    tifftest-tiles-contiguous-16
#    tifftest-stripes01-separated-08
#    tifftest-stripes01-separated-16
#    tifftest-stripes04-separated-08
#    tifftest-stripes04-separated-16
#    tifftest-stripes32-separated-08
#    tifftest-stripes32-separated-16
#    tifftest-tiles-separated-08
#    tifftest-tiles-separated-16
#
# Based on a public-domain program by Bob Friesenhahn
# Written by Cris Luengo, hereby placed in the public domain where applicable, MIT-license otherwise.
# The source stall_number.jpg image was taken by Cris Luengo in a Denver public parking space.

CONVERT="gm convert"
GM_STD_OPT="-verbose -compress Zip"
ORIGINAL='stall_number.jpg'

# GraphicsMagick command line options to specify how TIFF files are written:
# 
# striped: -define tiff:rows-per-strip=1
# striped: -define tiff:rows-per-strip=4
# striped: -define tiff:rows-per-strip=32
# tiled:   -define tiff:tile tiff:tile-geometry=128x128
#
# contiguous: (default)
# separated: -interlace plane

# Bit depths
for depth in 8 16
do

   for rows in 1 4 32
   do

      # Contiguous
      outname=`printf "tifftest-stripes%02d-contiguous-%02d.tif" $rows $depth`
      $CONVERT $ORIGINAL $GM_STD_OPT -define tiff:bits-per-sample=$depth -define tiff:rows-per-strip=$rows $outname

      # Separated
      outname=`printf "tifftest-stripes%02d-separated-%02d.tif" $rows $depth`
      $CONVERT $ORIGINAL $GM_STD_OPT -interlace plane -define tiff:bits-per-sample=$depth -define tiff:rows-per-strip=$rows $outname

   done

   # Contiguous
   outname=`printf "tifftest-tiles-contiguous-%02d.tif" $depth`
   $CONVERT $ORIGINAL $GM_STD_OPT -define tiff:bits-per-sample=$depth -define tiff:tile -define tiff:tile-geometry=128x128 $outname

   # Separated
   outname=`printf "tifftest-tiles-separated-%02d.tif" $depth`
   $CONVERT $ORIGINAL $GM_STD_OPT -interlace plane -define tiff:bits-per-sample=$depth -define tiff:tile -define tiff:tile-geometry=128x128 $outname

done
