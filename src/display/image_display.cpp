/*
 * DIPlib 3.0
 * This file contains the definition of the ImageDisplay class
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
#include "diplib/display.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/overload.h"

namespace dip {

// Don't call this function if mappingMode_ == MappingMode::MANUAL or mappingMode_ == MappingMode::MODULO!
void ImageDisplay::ComputeLimits( bool set ) {
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
         if( !colorspace_.empty() && ( colorspace_ != "RGB" )) {
            tmp.SetColorSpace( colorspace_ );
            colorSpaceManager_->Convert( tmp, tmp, "RGB" );
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
      if( tmp.DataType().IsBinary() ) {
         lims->lower = 0.0;
         lims->upper = 1.0;
      } else {
         if( tmp.DataType().IsComplex() ) {
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
            lims->lower = static_cast< dfloat >( Image::Sample( Percentile( tmp, {}, 5.0 )));
            lims->upper = static_cast< dfloat >( Image::Sample( Percentile( tmp, {}, 95.0 )));
         } else {
            MinMaxAccumulator res = MaximumAndMinimum( tmp );
            lims->lower = res.Minimum();
            lims->upper = res.Maximum();
         }
         if( std::isnan( lims->lower )) {
            lims->lower = 0.0;
         }
         if( std::isnan( lims->upper )) {
            lims->upper = 255.0;
         }
      }
   }
   if( set ) {
      range_ = *lims;
   }
}

void ImageDisplay::InvalidateSliceLimits() {
   for( auto& lim : sliceLimits_ ) {
      lim.maxMin = { nan, nan };
      lim.percentile = { nan, nan };
   }
}

ImageDisplay::Limits ImageDisplay::GetLimits( bool compute ) {
   Limits* lims;
   if( globalStretch_ ) {
      lims = &( globalLimits_[ static_cast< unsigned >( complexMode_ ) ].maxMin );
   } else {
      lims = &( sliceLimits_[ static_cast< unsigned >( complexMode_ ) ].maxMin );
   }
   if( compute && std::isnan( lims->lower )) {
      auto tmp = mappingMode_;
      mappingMode_ = MappingMode::MAXMIN;
      ComputeLimits( false ); // updates the value that `lims` points to, but doesn't update `range_`.
      mappingMode_ = tmp;
   }
   return *lims;
}

// Compute projection
void ImageDisplay::UpdateSlice() {
   if( sliceIsDirty_ ) {
      dip::uint nDims = image_.Dimensionality();
      dip::uint outDims = twoDimOut_ ? 2 : 1;
      if( nDims > outDims ) {
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
         if( dim1_ == dim2_ ) {
            slice_.PermuteDimensions( { dim1_ } );
         } else {
            slice_.PermuteDimensions( { dim1_, dim2_ } );
         }
      } else {
         slice_ = image_.QuickCopy();
      }
      sizeIsDirty_ = false;
      sliceIsDirty_ = false;
      rgbSliceIsDirty_ = true;
   }
}

void ImageDisplay::UpdateRgbSlice() {
   UpdateSlice();
   if( rgbSliceIsDirty_ ) {
      if( slice_.IsScalar() || ( colorspace_ == "RGB" )) {
         rgbSlice_ = slice_.QuickCopy();
      } else if( colorspace_.empty() ) {
         if( rgbSlice_.SharesData( slice_ )) {
            rgbSlice_.Strip();
         }
         rgbSlice_.ReForge( slice_.Sizes(), 3, slice_.DataType() );
         if( red_ >= 0 ) {
            rgbSlice_[ 0 ].Copy( slice_[ red_ ] );
         } else {
            rgbSlice_[ 0 ].Fill( 0 );
         }
         if( green_ >= 0 ) {
            rgbSlice_[ 1 ].Copy( slice_[ green_ ] );
         } else {
            rgbSlice_[ 1 ].Fill( 0 );
         }
         if( blue_ >= 0 ) {
            rgbSlice_[ 2 ].Copy( slice_[ blue_ ] );
         } else {
            rgbSlice_[ 2 ].Fill( 0 );
         }
      } else {
         slice_.SetColorSpace( colorspace_ );
         colorSpaceManager_->Convert( slice_, rgbSlice_, "RGB" );
      }
      rgbSliceIsDirty_ = false;
      outputIsDirty_ = true;
      InvalidateSliceLimits();
   }
}

namespace {

template< typename T >
dfloat convert( T v, bool /*usePhase*/ ) {
   return static_cast< dfloat >( v );
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
      bool useModulo,
      dip::dfloat offset,
      dip::dfloat scale
) {
   dip::uint width = slice.Size( 0 );
   dip::uint height = slice.Dimensionality() == 2 ? slice.Size( 1 ) : 1;
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
         } else if( useModulo ) {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               dfloat scaled = ( convert( *iPtr, usePhase ) + offset ) * scale;
               scaled = scaled == 0 ? 0 : ( std::fmod( scaled - 1, 255.0 ) + 1 );
               *oPtr = clamp_cast< uint8 >( scaled );
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
template<>
void CastToUint8< bin >(
      Image const& slice,
      Image& out,
      bool /*usePhase*/,
      bool /*logarithmic*/,
      bool /*useModulo*/,
      dip::dfloat /*offset*/,
      dip::dfloat /*scale*/
) {
   dip::uint width = slice.Size( 0 );
   dip::uint height = slice.Dimensionality() == 2 ? slice.Size( 1 ) : 1;
   dip::sint sliceStride0 = slice.Stride( 0 );
   dip::sint sliceStride1 = slice.Stride( 1 );
   dip::sint outStride0 = out.Stride( 0 );
   dip::sint outStride1 = out.Stride( 1 );
   dip::uint telems = slice.TensorElements();
   dip::sint sliceStrideT = slice.TensorStride();
   dip::sint outStrideT = out.TensorStride();
   for( dip::sint kk = 0; kk < static_cast< dip::sint >( telems ); ++kk ) {
      bin* slicePtr = static_cast< bin* >( slice.Pointer( sliceStrideT * kk ) );
      uint8* outPtr = static_cast< uint8* >( out.Pointer( outStrideT * kk ) );
      for( dip::uint jj = 0; jj < height; ++jj ) {
         bin* iPtr = slicePtr;
         uint8* oPtr = outPtr;
         for( dip::uint ii = 0; ii < width; ++ii ) {
            *oPtr = *iPtr ? uint8( 255 ) : uint8( 0 );
            iPtr += sliceStride0;
            oPtr += outStride0;
         }
         slicePtr += sliceStride1;
         outPtr += outStride1;
      }
   }
}

} // namespace

void ImageDisplay::UpdateOutput() {
   UpdateRgbSlice();
   if( outputIsDirty_ ) {
      // Input range to map to output
      if(( mappingMode_ != MappingMode::MANUAL ) && ( mappingMode_ != MappingMode::MODULO )) {
         ComputeLimits();
         if( mappingMode_ == MappingMode::BASED ) {
            dfloat bound = std::max( std::abs( range_.lower ), std::abs( range_.upper ));
            range_ = { -bound, bound };
         }
      }
      // Mapping function
      bool logarithmic = mappingMode_ == MappingMode::LOGARITHMIC;
      bool useModulo = mappingMode_ == MappingMode::MODULO;
      dfloat offset;
      dfloat scale;
      if( logarithmic ) {
         offset = 1.0 - range_.lower;
         scale = 255.0 / std::log( range_.upper + offset );
      } else {
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
      DIP_ASSERT(( !twoDimOut_ && ( slice.Dimensionality() == 1 )) || ( twoDimOut_ && ( slice.Dimensionality() == 2 )));
      output_.ReForge( slice.Sizes(), slice.TensorElements(), DT_UINT8 );
      // Stretch and convert the data
      DIP_OVL_CALL_ALL( CastToUint8, ( slice, output_, usePhase, logarithmic, useModulo, offset, scale ), slice.DataType() );
      outputIsDirty_ = false;
   }
}

namespace {

void MapPixelValues(
      Image::Pixel const& input,
      Image::Pixel const& output,
      dfloat offset, dfloat scale,
      bool usePhase, bool logarithmic, bool useModulo
) {
   // Input and output both always have 3 tensor elements
   for( dip::uint ii = 0; ii < 3; ++ii ) {
      double value = usePhase
                     ? std::arg( input[ ii ].As< dcomplex >() )
                     : input[ ii ].As< dfloat >();
      value += offset;
      if( logarithmic ) {
         output[ ii ] = clamp_cast< uint8 >( std::log( value ) * scale );
      } else if( useModulo ) {
         value = value == 0 ? 0 : ( std::fmod( value * scale - 1, 255.0 ) + 1 );
         output[ ii ] = clamp_cast< uint8 >( value );
      } else {
         output[ ii ] = clamp_cast< uint8 >( value * scale );
      }
   }
}

} // namespace

Image::Pixel ImageDisplay::MapSinglePixel( Image::Pixel const& input ) {
   DIP_THROW_IF( input.TensorElements() != image_.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   UpdateOutput(); // needed to have `range_` updated, etc.
   Image::Pixel rgb( input.DataType(), 3 );
   if( slice_.IsScalar() || ( colorspace_ == "RGB" ) ) {
      rgb = input;
   } else if( colorspace_.empty() ) {
      if( red_ >= 0 ) {
         rgb[ 0 ] = input[ static_cast< dip::uint >( red_ ) ];
      } else {
         rgb[ 0 ] = 0;
      }
      if( green_ >= 0 ) {
         rgb[ 1 ] = input[ static_cast< dip::uint >( green_ ) ];
      } else {
         rgb[ 1 ] = 0;
      }
      if( blue_ >= 0 ) {
         rgb[ 2 ] = input[ static_cast< dip::uint >( blue_ ) ];
      } else {
         rgb[ 2 ] = 0;
      }
   } else {
      Image tmp( input );
      tmp.SetColorSpace( colorspace_ );
      tmp = colorSpaceManager_->Convert( tmp, "RGB" );
      rgb = tmp.At( 0 );
   }
   // Mapping function
   bool logarithmic = mappingMode_ == MappingMode::LOGARITHMIC;
   bool useModulo = mappingMode_ == MappingMode::MODULO;
   dfloat offset;
   dfloat scale;
   if( logarithmic ) {
      offset = 1.0 - range_.lower;
      scale = 255.0 / std::log( range_.upper + offset );
   } else {
      offset = -range_.lower;
      scale = 255.0 / ( range_.upper - range_.lower );
   }
   Image::Pixel output( DT_UINT8, 3 );
   if( rgb.DataType().IsComplex() ) {
      switch( complexMode_ ) {
         //case ComplexMode::MAGNITUDE:
         default:
            MapPixelValues( rgb, output, offset, scale, false, logarithmic, useModulo );
            break;
         case ComplexMode::PHASE:
            MapPixelValues( rgb, output, offset, scale, true, logarithmic, useModulo );
            break;
         case ComplexMode::REAL:
            MapPixelValues( rgb.Real(), output, offset, scale, false, logarithmic, useModulo );
            break;
         case ComplexMode::IMAG:
            MapPixelValues( rgb.Imaginary(), output, offset, scale, false, logarithmic, useModulo );
            break;
      }
   }
   if( image_.IsScalar() ) {
      return output[ 0 ];
   } else {
      return output;
   }
}

} // namespace dip
