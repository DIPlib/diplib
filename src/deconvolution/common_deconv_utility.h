/*
 * DIPlib 3.0
 * This file contains functions common to all deconvolution methods
 *
 * (c)2018-2022, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/statistics.h"
#include "diplib/transform.h"

namespace dip {

namespace {

inline Image GetOTF( Image const& psf, UnsignedArray const& sizes, bool isOtf ) {
   Image H;
   if( isOtf ) {
      H = psf.QuickCopy();
      DIP_THROW_IF( H.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_THROW_IF( H.Sizes() != sizes, E::SIZES_DONT_MATCH );
   } else {
      DIP_THROW_IF( !psf.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_STACK_TRACE_THIS( H = psf.Pad( sizes ));
      DIP_STACK_TRACE_THIS( FourierTransform( H, H ));
   }
   return H;
}

inline void FourierTransformImageAndKernel(
      Image const& in,
      Image const& psf,
      Image& G, // == FT(in)
      Image& H, // == FT(psf)
      bool isOtf,
      bool pad,
      dip::uint powersOfTwo = 0 // pad to a size that is a multiple of 2 this number of times, even if pad is false.
) {
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( psf.Dimensionality() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
   DIP_THROW_IF( pad && isOtf, E::ILLEGAL_FLAG_COMBINATION );
   if( pad || ( powersOfTwo > 0 )) {
      dip::uint multiple = static_cast< dip::uint >( std::pow( 2, powersOfTwo ));
      dip::String purpose = in.DataType().IsComplex() ? S::COMPLEX : S::REAL;
      dip::UnsignedArray sizes = in.Sizes();
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( pad ) {
            sizes[ ii ] += 2 * psf.Size( ii );
         }
         sizes[ ii ] = OptimalFourierTransformSize( div_ceil( sizes[ ii ], multiple ), S::LARGER, purpose ) * multiple;
      }
      Image tmp = ExtendImageToSize( in, sizes, S::CENTER );
      DIP_STACK_TRACE_THIS( FourierTransform( tmp, G ));
   } else {
      DIP_STACK_TRACE_THIS( FourierTransform( in, G ));
   }
   DIP_STACK_TRACE_THIS( H = GetOTF( psf, G.Sizes(), isOtf ));
}

} // namespace

} // namespace dip
