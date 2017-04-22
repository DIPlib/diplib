/*
 * DIPlib 3.0
 * This file contains the definition of the ImageDisplay class
 *
 * (c)2017, Cris Luengo.
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

#include <sstream>

#include "diplib.h"
#include "diplib/display.h"
#include "diplib/math.h"
#include "diplib/overload.h"
#include "diplib/color.h"

namespace dip {

static ColorSpaceManager colorSpaceManager;

// Don't call this function if mappingMode_ == MappingMode::MANUAL!
void ImageDisplay::ComputeLimits() {
   Limits* lims;
   Image tmp;
   if( globalStretch_ ) {
      if( mappingMode_ == MappingMode::PERCENTILE ) {
         lims = &( globalLimits_[ static_cast< unsigned >( complexMode_ ) ].percentile );
      } else {
         lims = &( globalLimits_[ static_cast< unsigned >( complexMode_ ) ].maxMin );
      }
      if( std::isnan( lims->lower )) {
         // Compute from image_
         tmp = image_.QuickCopy();
         if( colorspace_ != "RGB" ) {
            tmp.SetColorSpace( colorspace_ );
            colorSpaceManager.Convert( tmp, tmp, "RGB" );
         };
      }
   } else {
      if( mappingMode_ == MappingMode::PERCENTILE ) {
         lims = &( sliceLimits_[ static_cast< unsigned >( complexMode_ ) ].percentile );
      } else {
         lims = &( sliceLimits_[ static_cast< unsigned >( complexMode_ ) ].maxMin );
      }
      if( std::isnan( lims->lower )) {
         // Compute from rgbSlice_
         tmp = rgbSlice_.QuickCopy();
         // It's already converted to RGB...
      }
   }
   if( tmp.IsForged() ) {
      // Compute!
      if( tmp.DataType().IsComplex()) {
         switch( complexMode_ ) {
            //case ComplexMode::MAGNITUDE:
            default:
               tmp = Abs( tmp );
               break;
            case ComplexMode::PHASE:
               tmp = Phase( tmp );
               break;
            case ComplexMode::REAL:
               tmp = tmp.Real();
               break;
            case ComplexMode::IMAG:
               tmp = tmp.Imaginary();
               break;
         }
      }
      if( mappingMode_ == MappingMode::PERCENTILE ) {
         lims->lower = static_cast< dfloat >( Percentile( tmp, {}, 5.0 ));
         lims->upper = static_cast< dfloat >( Percentile( tmp, {}, 95.0 ));
      } else {
         MinMaxAccumulator res = GetMaximumAndMinimum( tmp );
         lims->lower = res.Minimum();
         lims->upper = res.Maximum();
      }
   }
   range_ = *lims;
}

void ImageDisplay::InvalidateSliceLimits() {
   for( auto& lim : sliceLimits_ ) {
      lim.maxMin = { NaN, NaN };
      lim.percentile = { NaN, NaN };
   }
}

// Compute projection
void ImageDisplay::UpdateSlice() {
   if( sliceIsDirty_ ) {
      dip::uint nDims = image_.Dimensionality();
      if( nDims > 2 ) {
         switch( projectionMode_ ) {
            //case ProjectionMode::SLICE:
            default: {
               RangeArray rangeArray( nDims ); // By default, covers all image pixels
               for( dip::uint ii = 0; ii < nDims; ++ii ) {
                  if(( ii != dim1_ ) && ( ii != dim2_ )) {
                     rangeArray[ ii ] = Range( static_cast< dip::sint >( coordinates_[ ii ] ));
                  }
               }
               slice_ = image_.At( rangeArray );
               break;
            }
            case ProjectionMode::MAX: {
               BooleanArray process( nDims, true );
               process[ dim1_ ] = false;
               process[ dim2_ ] = false;
               if( image_.DataType().IsComplex() ) {
                  MaximumAbs( image_, {}, slice_, process );
               } else {
                  Maximum( image_, {}, slice_, process );
               }
               break;
            }
            case ProjectionMode::MEAN: {
               BooleanArray process( nDims, true );
               process[ dim1_ ] = false;
               process[ dim2_ ] = false;
               Mean( image_, {}, slice_, "", process );
               break;
            }
         }
         slice_.PermuteDimensions( { dim1_, dim2_ } );
      } else {
         slice_ = image_.QuickCopy();
      }
      if( colorspace_.empty() ) {
         rgbSlice_ = slice_.QuickCopy();
      } else if( colorspace_ == "RGB" ) {
         rgbSlice_ = slice_.QuickCopy();
         if( rgbSlice_.TensorElements() < 3 ) {
            Image tmp( rgbSlice_.Sizes(), 3, rgbSlice_.DataType() );
            tmp.Fill( 0 );
            tmp[ 0 ].Copy( rgbSlice_[ 0 ] );
            if( rgbSlice_.TensorElements() > 1 ) {
               tmp[ 1 ].Copy( rgbSlice_[ 1 ] );
            }
            rgbSlice_ = std::move( tmp );
         }
      } else {
         slice_.SetColorSpace( colorspace_ );
         colorSpaceManager.Convert( slice_, rgbSlice_, "RGB" );
      }
      sliceIsDirty_ = false;
      outputIsDirty_ = true;
      InvalidateSliceLimits();
   }
}

namespace {

template< typename T >
dfloat convert( T v, bool /*usePhase*/ ) {
   return v;
}

template< typename T >
dfloat convert( std::complex< T > v, bool usePhase ) {
   if( usePhase ) {
      return std::arg( v );
   } else {
      return std::abs( v );
   }
}

template< typename TPI >
void CastToUint8(
      Image const& slice,
      Image& out,
      bool usePhase,
      bool logarithmic,
      dip::dfloat offset,
      dip::dfloat scale
) {
   dip::uint width = slice.Size( 0 );
   dip::uint height = slice.Size( 1 );
   dip::sint sliceStride0 = slice.Stride( 0 );
   dip::sint sliceStride1 = slice.Stride( 1 );
   dip::sint outStride0 = out.Stride( 0 );
   dip::sint outStride1 = out.Stride( 1 );
   dip::uint telems = slice.TensorElements();
   dip::sint sliceStrideT = slice.TensorStride();
   dip::sint outStrideT = out.TensorStride();
   for( dip::sint kk = 0; kk < static_cast< dip::sint >( telems ); ++kk ) {
      TPI* slicePtr = static_cast< TPI* >( slice.Pointer( sliceStrideT * kk ) );
      uint8* outPtr = static_cast< uint8* >( out.Pointer( outStrideT * kk ) );
      for( dip::uint jj = 0; jj < height; ++jj ) {
         TPI* iPtr = slicePtr;
         uint8* oPtr = outPtr;
         if( logarithmic ) {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               *oPtr = clamp_cast< uint8 >( std::log( convert( *iPtr, usePhase ) + offset ) * scale );
               iPtr += sliceStride0;
               oPtr += outStride0;
            }
         } else {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               *oPtr = clamp_cast< uint8 >(( convert( *iPtr, usePhase ) + offset ) * scale );
               iPtr += sliceStride0;
               oPtr += outStride0;
            }
         }
         slicePtr += sliceStride1;
         outPtr += outStride1;
      }
   }
}

} // namespace

void ImageDisplay::UpdateOutput() {
   if( outputIsDirty_ ) {
      // Input range to map to output
      if( mappingMode_ != MappingMode::MANUAL ) {
         ComputeLimits();
         if( mappingMode_ == MappingMode::BASED ) {
            dfloat bound = std::max( std::abs( range_.lower ), std::abs( range_.upper ));
            range_ = { -bound, bound };
         }
      }
      // Mapping function
      bool logarithmic;
      dfloat offset;
      dfloat scale;
      if( mappingMode_ == MappingMode::LOGARITHMIC ) {
         logarithmic = true;
         offset = 1.0 - range_.lower;
         scale = 255.0 / std::log( range_.upper + offset );
      } else {
         logarithmic = false;
         offset = -range_.lower;
         scale = 255.0 / ( range_.upper - range_.lower );
      }
      // Complex to real
      Image slice = rgbSlice_.QuickCopy();
      bool usePhase = false;
      if( slice.DataType().IsComplex() ) {
         switch( complexMode_ ) {
            //case ComplexMode::MAGNITUDE:
            default:
               // Nothing to do.
               break;
            case ComplexMode::PHASE:
               usePhase = true;
               break;
            case ComplexMode::REAL:
               slice = slice.Real();
               break;
            case ComplexMode::IMAG:
               slice = slice.Imaginary();
               break;
         }
      }
      // Create output
      DIP_ASSERT( slice.Dimensionality() == 2 );
      output_.ReForge( slice.Sizes(), slice.TensorElements(), DT_UINT8 );
      // Stretch and convert the data
      DIP_OVL_CALL_ALL( CastToUint8, ( slice, output_, usePhase, logarithmic, offset, scale ), slice.DataType() );
      outputIsDirty_ = false;
   }
}

Image const& ImageDisplay::Output() {
   UpdateSlice();
   UpdateOutput();
   return output_;
}

namespace{

template< typename TPI >
TPI cast( TPI value ) {
   return value;
}
int cast( uint8 value ) {
   return static_cast< int >( value );
}
int cast( sint8 value ) {
   return static_cast< int >( value );
}
int cast( bin value ) {
   return static_cast< int >( value );
}

template< typename TPI >
String PixelToString( Image const& slice, dip::uint x, dip::uint y ) {
   dip::sint offset = static_cast< dip::sint >( x ) * slice.Stride( 0 ) +
                      static_cast< dip::sint >( y ) * slice.Stride( 1 );
   TPI* ptr = static_cast< TPI* >( slice.Origin() ) + offset;
   std::stringstream out;
   out << cast( *ptr );
   for( dip::uint ii = 1; ii < slice.TensorElements(); ++ii ) {
      ptr += slice.TensorStride();
      out << ", ";
      out << cast( *ptr );
   }
   return out.str();
}

} // namespace

String ImageDisplay::Pixel( dip::uint x, dip::uint y ) {
   UpdateSlice();
   if( x >= slice_.Size( 0 )) {
      x = slice_.Size( 0 ) - 1;
   }
   if( y >= slice_.Size( 1 )) {
      y = slice_.Size( 1 ) - 1;
   }
   String out;
   DIP_OVL_CALL_ASSIGN_ALL( out, PixelToString, ( slice_, x, y ), slice_.DataType() );
   return out;
}


} // namespace dip
