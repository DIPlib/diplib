/*
 * (c)2017-2020, Cris Luengo.
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
         if( !colorspace_.empty() && ( colorspace_ != "sRGB" )) {
            tmp.SetColorSpace( colorspace_ );
            colorSpaceManager_->Convert( tmp, tmp, "sRGB" );
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
               slice_ = image_.At( std::move( rangeArray ));
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
      if( colorspace_.empty() ) {
         if( slice_.IsScalar() || ( colorspace_ == "sRGB" )) {
            rgbSlice_ = slice_.QuickCopy();
         } else {
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
         }
      } else {
         slice_.SetColorSpace( colorspace_ );
         colorSpaceManager_->Convert( slice_, rgbSlice_, "sRGB" );
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
   return usePhase ? std::arg( v ) : std::abs( v );
}

constexpr uint8 maxUint8 = std::numeric_limits< uint8 >::max();
constexpr dfloat logRange = 1e3;
const dfloat logScale = maxUint8 / std::log( logRange );

struct ScalingParams {
   dfloat offset;
   dfloat scale;
   bool logarithmic;
   bool useModulo;

   ScalingParams( ImageDisplay::MappingMode mappingMode, ImageDisplay::Limits range ) {
      logarithmic = mappingMode == ImageDisplay::MappingMode::LOGARITHMIC;
      useModulo = mappingMode == ImageDisplay::MappingMode::MODULO;
      if( logarithmic ) {
         // For logarithmic scaling, we linearly map the input data to the range [1,1e3], then take the logarithm, and finally scale to [0,255].
         scale = ( logRange - 1.0 ) / ( range.upper - range.lower );
         offset = 1.0 - range.lower * scale;
         //logScale = maxUint8 / std::log( range.upper * scale + offset );
      } else {
         scale = maxUint8 / ( range.upper - range.lower );
         offset = -range.lower * scale;
      }
   }

   uint8 ScaleLinear( dfloat value ) {
      return clamp_cast< uint8 >(value * scale + offset );
   }

   uint8 ScaleLogarithmic( dfloat value ) {
      return clamp_cast< uint8 >( std::log( value * scale + offset ) * logScale );
   }

   uint8 ScaleModulo( dfloat value ) {
      //dip::uint scaled = static_cast< dip::uint >( value * scale + offset );
      dip::uint scaled = static_cast< dip::uint >( value ); // Note that the modulo mode cannot be selected without range being set to [0,255].
      scaled = ( scaled == 0 ) ? ( 0 ) : (( scaled - 1 ) % maxUint8 + 1 );
      return static_cast< uint8 >( scaled );
   }

   uint8 Scale( dfloat value ) {
      if( logarithmic ) {
         return ScaleLogarithmic( value );
      }
      if( useModulo ) {
         return ScaleModulo( value );
      }
      return ScaleLinear( value );
   }
};

template< typename TPI >
void CastToUint8(
      Image const& slice,
      Image& out,
      bool usePhase,
      ScalingParams params
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
         if( params.logarithmic ) {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               *oPtr = params.ScaleLogarithmic( convert( *iPtr, usePhase ));
               iPtr += sliceStride0;
               oPtr += outStride0;
            }
         } else if( params.useModulo ) {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               *oPtr = params.ScaleModulo( convert( *iPtr, usePhase ));
               iPtr += sliceStride0;
               oPtr += outStride0;
            }
         } else {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               *oPtr = params.ScaleLinear( convert( *iPtr, usePhase ));
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
      ScalingParams /*params*/
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
      ScalingParams scalingParams( mappingMode_, range_ );
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
      DIP_OVL_CALL_ALL( CastToUint8, ( slice, output_, usePhase, scalingParams ), slice.DataType() );
      outputIsDirty_ = false;
   }
}

namespace {

void MapPixelValues(
      Image::Pixel const& input,
      Image::Pixel const& output,
      ScalingParams params,
      bool usePhase
) {
   // Input and output both always have 3 tensor elements
   for( dip::uint ii = 0; ii < 3; ++ii ) {
      double value = usePhase
                     ? std::arg( input[ ii ].As< dcomplex >() )
                     : input[ ii ].As< dfloat >();
      output[ ii ] = params.Scale( value );
   }
}

} // namespace

Image::Pixel ImageDisplay::MapSinglePixel( Image::Pixel const& input ) {
   DIP_THROW_IF( input.TensorElements() != image_.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   UpdateOutput(); // needed to have `range_` updated, etc.
   Image::Pixel rgb( input.DataType(), 3 );
   if( slice_.IsScalar() || ( colorspace_ == "sRGB" ) ) {
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
      tmp = colorSpaceManager_->Convert( tmp, "sRGB" );
      rgb = tmp.At( 0 );
   }
   // Mapping function
   ScalingParams scalingParams( mappingMode_, range_ );
   Image::Pixel output( DT_UINT8, 3 );
   if( rgb.DataType().IsComplex() ) {
      switch( complexMode_ ) {
         //case ComplexMode::MAGNITUDE:
         default:
            MapPixelValues( rgb, output, scalingParams, false );
            break;
         case ComplexMode::PHASE:
            MapPixelValues( rgb, output, scalingParams, true );
            break;
         case ComplexMode::REAL:
            MapPixelValues( rgb.Real(), output, scalingParams, false );
            break;
         case ComplexMode::IMAG:
            MapPixelValues( rgb.Imaginary(), output, scalingParams, false );
            break;
      }
   }
   if( image_.IsScalar() ) {
      return output[ 0 ];
   }
   return output;
}

} // namespace dip
