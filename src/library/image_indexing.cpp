/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"

using namespace dip;


void dip::DefineROI(
   const Image& src,
   Image& dest,
   const UnsignedArray& origin,
   const UnsignedArray& dims,
   const IntegerArray& spacing
){
   // ...
}

// Constructor: Creates a new image pointing to data of 'src'.
Image::Image(
   const Image& src,
   const UnsignedArray& origin,
   const UnsignedArray& dims,
   const IntegerArray& spacing
){
   DefineROI( src, *this, origin, dims, spacing );
}
