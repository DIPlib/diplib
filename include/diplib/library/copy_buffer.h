/*
 * DIPlib 3.0
 * This file contains functionality to copy a pixel buffer with cast.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


//
// NOTE!
// Unlike other files in diplib/library, this file is NOT included through diplib.h.
// However, it contains no publicly documented functionality.
//


#ifndef DIP_COPY_BUFFER_H
#define DIP_COPY_BUFFER_H

#include "diplib/library/datatype.h"


namespace dip {

enum class DIP_NO_EXPORT BoundaryCondition; // forward declaration, defined in diplib/boundary.h

namespace detail {


// Copies pixels from one 1D buffer to another, converting data type using `clamp_cast`.
//
// If `inStride` and/or `inTensorStride` are 0, The function is similar to `FillBuffer` along that dimension.
// If `outStride` and/or `outTensorStride` are 0, then only one sample can be written in that dimension;
// we choose here to write only the first sample from inBuffer. This is different than if all values
// would have been written in order to that same location, where only the last write (the last
// sample) would remain. However, neither option makes sense.
//
// This is an internal function not meant to be used by the library user.
void CopyBuffer(
      void const* inBuffer,
      DataType inType,
      dip::sint inStride,
      dip::sint inTensorStride,
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,          // number of pixels in the two buffers
      dip::uint tensorElements,  // number of samples per pixel
      std::vector< dip::sint > const& lookUpTable = {} // it this is null, simply copy over the tensor as is; otherwise use this to determine which tensor values to copy where
);

// Expands the boundary of a 1D buffer, which extends `left` pixels on to the left, and `right` pixels to the right.
// That is, the total number of pixels in the buffer is `pixels + left + right`, but the `buffer` pointer
// points at the middle `pixels` elements, which are filled in. This function fills out the other `left + right`
// pixels according to `bc`.
//
// This is an internal function not meant to be used by the library user.
void ExpandBuffer(
      void* buffer,
      DataType type,
      dip::sint stride,
      dip::sint tensorStride,
      dip::uint pixels,          // number of pixels in the buffer
      dip::uint tensorElements,  // number of samples per pixel
      dip::uint left,            // number of pixels before buffer to fill
      dip::uint right,           // number of pixels after buffer to fill
      BoundaryCondition bc
);

// Fills one 1D buffer with a constant value `value`.
//
// This is an internal function not meant to be used by the library user.
template< typename outT >
static inline void FillBufferFromTo(
      outT* outBuffer,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      outT value
) {
   if( tensorElements == 1 ) {
      for( dip::uint pp = 0; pp < pixels; ++pp ) {
         *outBuffer = value;
         outBuffer += outStride;
      }
   } else {
      for( dip::uint pp = 0; pp < pixels; ++pp ) {
         outT* out = outBuffer;
         for( dip::uint tt = 0; tt < tensorElements; ++tt ) {
            *out = value;
            out += outTensorStride;
         }
         outBuffer += outStride;
      }
   }
}


} // namespace detail
} // namespace dip


#endif // DIP_COPY_BUFFER_H
