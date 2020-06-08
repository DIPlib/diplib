/*
 * DIPlib 3.0
 * This file contains definitions for image windowing functions
 *
 * (c)2017-2018, Cris Luengo.
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

#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/border.h"
#include "diplib/overload.h"
#include "draw_support.h"

namespace dip {

namespace {

template< typename TPI >
void SetBorderInternal( Image & out, Image::Pixel const& value, UnsignedArray const& sizes ) {
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   detail::ProcessBorders< TPI >(
         out,
         [ &value_ ]( auto* ptr, dip::sint tStride ) {
            for( auto v : value_ ) {
               *ptr = v;
               ptr += tStride;
            }
         }, sizes );
}

} // namespace

void SetBorder( Image& out, Image::Pixel const& value, UnsignedArray const& sizes ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_OVL_CALL_ALL( SetBorderInternal, ( out, value, sizes ), out.DataType() );
}


namespace {

Image CreateHammingWindow( dip::uint size, dfloat a0 ) {
   dfloat a1 = 1.0 - a0;
   dfloat scale = 2 * pi / static_cast< dfloat >( size - 1 );
   Image out( { size }, 1, DT_DFLOAT );
   dfloat* ptr = static_cast< dfloat* >( out.Origin() );
   for( dip::uint ii = 0; ii < size; ++ii ) {
      ptr[ ii ] = a0 - a1 * std::cos( scale  * static_cast< dfloat >( ii ));
   }
   return out;
}

Image CreateGaussianWindow( dip::uint size, dfloat sigma ) {
   dfloat scale = 2 / sigma / static_cast< dfloat >( size - 1 );
   dfloat offset = -static_cast< dfloat >( size - 1 ) / 2 * scale;
   Image out( { size }, 1, DT_DFLOAT );
   dfloat* ptr = static_cast< dfloat* >( out.Origin() );
   for( dip::uint ii = 0; ii < size; ++ii ) {
      dfloat exponent = static_cast< dfloat >( ii ) * scale + offset;
      ptr[ ii ] = std::exp( -0.5 * exponent * exponent );
   }
   return out;
}

Image CreateTukeyWindow( dip::uint size, dfloat alpha ) {
   dip::uint n1 = static_cast< dip::uint >( std::ceil( alpha / 2 * static_cast< dfloat >( size - 1 )));
   dip::uint n2 = static_cast< dip::uint >( std::floor(( 1 - alpha / 2 ) * static_cast< dfloat >( size - 1 )));
   dfloat scale = 2 * pi / alpha / static_cast< dfloat >( size - 1 );
   dfloat offset1 = -pi;
   dfloat offset2 = pi * ( 1 - 2 / alpha );
   Image out( { size }, 1, DT_DFLOAT );
   dfloat* ptr = static_cast< dfloat* >( out.Origin() );
   for( dip::uint ii = 0; ii < n1; ++ii ) {
      ptr[ ii ] = 0.5 + 0.5 * std::cos( scale  * static_cast< dfloat >( ii ) + offset1 );
   }
   for( dip::uint ii = n1; ii < n2; ++ii ) {
      ptr[ ii ] = 1;
   }
   for( dip::uint ii = n2; ii < size; ++ii ) {
      ptr[ ii ] = 0.5 + 0.5 * std::cos( scale  * static_cast< dfloat >( ii ) + offset2 );
   }
   return out;
}

Image CreateGaussianTukeyWindow( dip::uint size, dfloat sigma ) {
   // We're setting n1 and n2 such that the Gaussian is cut off at 3*sigma.
   // Sigma is defined in pixels. It should be larger than 1 to make sense, preferably 3 or 5.
   dfloat origin1 = 3 * sigma;
   dfloat origin2 = static_cast< dfloat >( size - 1 ) - origin1;
   dip::uint n1 = static_cast< dip::uint >( std::ceil( 2 * origin1 ));
   dip::uint n2 = static_cast< dip::uint >( std::floor( origin2 - origin1 ));
   dfloat norm = -1.0 / ( sigma * std::sqrt( 2.0 ));
   Image out( { size }, 1, DT_DFLOAT );
   dfloat* ptr = static_cast< dfloat* >( out.Origin() );
   for( dip::uint ii = 0; ii < n1; ++ii ) {
      ptr[ ii ] = 0.5 + 0.5 * std::erf(( origin1 - static_cast< dfloat >( ii )) * norm );
   }
   for( dip::uint ii = n1; ii < n2; ++ii ) {
      ptr[ ii ] = 1;
   }
   for( dip::uint ii = n2; ii < size; ++ii ) {
      ptr[ ii ] = 0.5 + 0.5 * std::erf(( static_cast< dfloat >( ii ) - origin2 ) * norm );
   }
   return out;
}

} // namespace

void ApplyWindow( Image const& in, Image& out, String const& type, dfloat parameter ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   if( nDims < 1 ) {
      out = in;
      return;
   }
   // Create 1D windowing functions
   ImageArray windows( nDims );
   if( type == "Hamming" ) {
      parameter = clamp( parameter, 0.0, 1.0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         windows[ ii ] = CreateHammingWindow( in.Size( ii ), parameter );
      }
   } else if( type == "Gaussian" ) {
      parameter = clamp( parameter, 0.0, 0.5 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         windows[ ii ] = CreateGaussianWindow( in.Size( ii ), parameter );
      }
   } else if( type == "Tukey" ) {
      parameter = clamp( parameter, 0.0, 1.0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         windows[ ii ] = CreateTukeyWindow( in.Size( ii ), parameter );
      }
   } else if( type == "GaussianTukey" ) {
      parameter = std::max( 0.0, parameter );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( static_cast< dfloat >( in.Size( ii )) >= 2 * std::ceil( 6 * parameter )) {
            windows[ ii ] = CreateGaussianTukeyWindow( in.Size( ii ), parameter );
         } else {
            windows[ ii ] = CreateGaussianWindow( in.Size( ii ), 1.0 / 3.0 );
         }
      }
   }
   // Apply windowing functions
   DataType dt = DataType::SuggestFlex( in.DataType() );
   MultiplySampleWise( in, windows[ 0 ], out, dt );
   for( dip::uint ii = 1; ii < nDims; ++ii ) {
      windows[ ii ].ExpandDimensionality( nDims );
      windows[ ii ].SwapDimensions( 0, ii );
      MultiplySampleWise( out, windows[ ii ], out, dt );
   }
}

} // namespace dip
