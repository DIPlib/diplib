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
#include "diplib/statistics.h"
#include "diplib/lookup_table.h"
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
            rgbSlice_[ 0 ].Copy( slice_[ static_cast< dip::uint >( red_ ) ] );
         } else {
            rgbSlice_[ 0 ].Fill( 0 );
         }
         if( green_ >= 0 ) {
            rgbSlice_[ 1 ].Copy( slice_[ static_cast< dip::uint >( green_ ) ] );
         } else {
            rgbSlice_[ 1 ].Fill( 0 );
         }
         if( blue_ >= 0 ) {
            rgbSlice_[ 2 ].Copy( slice_[ static_cast< dip::uint >( blue_ ) ] );
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

namespace {

// Generated in MATLAB using:
//    g = linspace(0,255,256)';
//    g = [g,g,g]
static constexpr std::array< uint8, 256 * 3 > greyColorMap =
      {{
             0, 0, 0,
             1, 1, 1,
             2, 2, 2,
             3, 3, 3,
             4, 4, 4,
             5, 5, 5,
             6, 6, 6,
             7, 7, 7,
             8, 8, 8,
             9, 9, 9,
             10, 10, 10,
             11, 11, 11,
             12, 12, 12,
             13, 13, 13,
             14, 14, 14,
             15, 15, 15,
             16, 16, 16,
             17, 17, 17,
             18, 18, 18,
             19, 19, 19,
             20, 20, 20,
             21, 21, 21,
             22, 22, 22,
             23, 23, 23,
             24, 24, 24,
             25, 25, 25,
             26, 26, 26,
             27, 27, 27,
             28, 28, 28,
             29, 29, 29,
             30, 30, 30,
             31, 31, 31,
             32, 32, 32,
             33, 33, 33,
             34, 34, 34,
             35, 35, 35,
             36, 36, 36,
             37, 37, 37,
             38, 38, 38,
             39, 39, 39,
             40, 40, 40,
             41, 41, 41,
             42, 42, 42,
             43, 43, 43,
             44, 44, 44,
             45, 45, 45,
             46, 46, 46,
             47, 47, 47,
             48, 48, 48,
             49, 49, 49,
             50, 50, 50,
             51, 51, 51,
             52, 52, 52,
             53, 53, 53,
             54, 54, 54,
             55, 55, 55,
             56, 56, 56,
             57, 57, 57,
             58, 58, 58,
             59, 59, 59,
             60, 60, 60,
             61, 61, 61,
             62, 62, 62,
             63, 63, 63,
             64, 64, 64,
             65, 65, 65,
             66, 66, 66,
             67, 67, 67,
             68, 68, 68,
             69, 69, 69,
             70, 70, 70,
             71, 71, 71,
             72, 72, 72,
             73, 73, 73,
             74, 74, 74,
             75, 75, 75,
             76, 76, 76,
             77, 77, 77,
             78, 78, 78,
             79, 79, 79,
             80, 80, 80,
             81, 81, 81,
             82, 82, 82,
             83, 83, 83,
             84, 84, 84,
             85, 85, 85,
             86, 86, 86,
             87, 87, 87,
             88, 88, 88,
             89, 89, 89,
             90, 90, 90,
             91, 91, 91,
             92, 92, 92,
             93, 93, 93,
             94, 94, 94,
             95, 95, 95,
             96, 96, 96,
             97, 97, 97,
             98, 98, 98,
             99, 99, 99,
             100, 100, 100,
             101, 101, 101,
             102, 102, 102,
             103, 103, 103,
             104, 104, 104,
             105, 105, 105,
             106, 106, 106,
             107, 107, 107,
             108, 108, 108,
             109, 109, 109,
             110, 110, 110,
             111, 111, 111,
             112, 112, 112,
             113, 113, 113,
             114, 114, 114,
             115, 115, 115,
             116, 116, 116,
             117, 117, 117,
             118, 118, 118,
             119, 119, 119,
             120, 120, 120,
             121, 121, 121,
             122, 122, 122,
             123, 123, 123,
             124, 124, 124,
             125, 125, 125,
             126, 126, 126,
             127, 127, 127,
             128, 128, 128,
             129, 129, 129,
             130, 130, 130,
             131, 131, 131,
             132, 132, 132,
             133, 133, 133,
             134, 134, 134,
             135, 135, 135,
             136, 136, 136,
             137, 137, 137,
             138, 138, 138,
             139, 139, 139,
             140, 140, 140,
             141, 141, 141,
             142, 142, 142,
             143, 143, 143,
             144, 144, 144,
             145, 145, 145,
             146, 146, 146,
             147, 147, 147,
             148, 148, 148,
             149, 149, 149,
             150, 150, 150,
             151, 151, 151,
             152, 152, 152,
             153, 153, 153,
             154, 154, 154,
             155, 155, 155,
             156, 156, 156,
             157, 157, 157,
             158, 158, 158,
             159, 159, 159,
             160, 160, 160,
             161, 161, 161,
             162, 162, 162,
             163, 163, 163,
             164, 164, 164,
             165, 165, 165,
             166, 166, 166,
             167, 167, 167,
             168, 168, 168,
             169, 169, 169,
             170, 170, 170,
             171, 171, 171,
             172, 172, 172,
             173, 173, 173,
             174, 174, 174,
             175, 175, 175,
             176, 176, 176,
             177, 177, 177,
             178, 178, 178,
             179, 179, 179,
             180, 180, 180,
             181, 181, 181,
             182, 182, 182,
             183, 183, 183,
             184, 184, 184,
             185, 185, 185,
             186, 186, 186,
             187, 187, 187,
             188, 188, 188,
             189, 189, 189,
             190, 190, 190,
             191, 191, 191,
             192, 192, 192,
             193, 193, 193,
             194, 194, 194,
             195, 195, 195,
             196, 196, 196,
             197, 197, 197,
             198, 198, 198,
             199, 199, 199,
             200, 200, 200,
             201, 201, 201,
             202, 202, 202,
             203, 203, 203,
             204, 204, 204,
             205, 205, 205,
             206, 206, 206,
             207, 207, 207,
             208, 208, 208,
             209, 209, 209,
             210, 210, 210,
             211, 211, 211,
             212, 212, 212,
             213, 213, 213,
             214, 214, 214,
             215, 215, 215,
             216, 216, 216,
             217, 217, 217,
             218, 218, 218,
             219, 219, 219,
             220, 220, 220,
             221, 221, 221,
             222, 222, 222,
             223, 223, 223,
             224, 224, 224,
             225, 225, 225,
             226, 226, 226,
             227, 227, 227,
             228, 228, 228,
             229, 229, 229,
             230, 230, 230,
             231, 231, 231,
             232, 232, 232,
             233, 233, 233,
             234, 234, 234,
             235, 235, 235,
             236, 236, 236,
             237, 237, 237,
             238, 238, 238,
             239, 239, 239,
             240, 240, 240,
             241, 241, 241,
             242, 242, 242,
             243, 243, 243,
             244, 244, 244,
             245, 245, 245,
             246, 246, 246,
             247, 247, 247,
             248, 248, 248,
             249, 249, 249,
             250, 250, 250,
             251, 251, 251,
             252, 252, 252,
             253, 253, 253,
             254, 254, 254,
             255, 255, 255,
       }};

// Generated in MATLAB using:
//    g = linspace(0,255,256)';
//    g = [g,g,g];
//    g(1,:) = [0,0,255];
//    g(256,:) = [255,0,0]
static constexpr std::array< uint8, 256 * 3 > saturationColorMap =
      {{
             0, 0, 255,
             1, 1, 1,
             2, 2, 2,
             3, 3, 3,
             4, 4, 4,
             5, 5, 5,
             6, 6, 6,
             7, 7, 7,
             8, 8, 8,
             9, 9, 9,
             10, 10, 10,
             11, 11, 11,
             12, 12, 12,
             13, 13, 13,
             14, 14, 14,
             15, 15, 15,
             16, 16, 16,
             17, 17, 17,
             18, 18, 18,
             19, 19, 19,
             20, 20, 20,
             21, 21, 21,
             22, 22, 22,
             23, 23, 23,
             24, 24, 24,
             25, 25, 25,
             26, 26, 26,
             27, 27, 27,
             28, 28, 28,
             29, 29, 29,
             30, 30, 30,
             31, 31, 31,
             32, 32, 32,
             33, 33, 33,
             34, 34, 34,
             35, 35, 35,
             36, 36, 36,
             37, 37, 37,
             38, 38, 38,
             39, 39, 39,
             40, 40, 40,
             41, 41, 41,
             42, 42, 42,
             43, 43, 43,
             44, 44, 44,
             45, 45, 45,
             46, 46, 46,
             47, 47, 47,
             48, 48, 48,
             49, 49, 49,
             50, 50, 50,
             51, 51, 51,
             52, 52, 52,
             53, 53, 53,
             54, 54, 54,
             55, 55, 55,
             56, 56, 56,
             57, 57, 57,
             58, 58, 58,
             59, 59, 59,
             60, 60, 60,
             61, 61, 61,
             62, 62, 62,
             63, 63, 63,
             64, 64, 64,
             65, 65, 65,
             66, 66, 66,
             67, 67, 67,
             68, 68, 68,
             69, 69, 69,
             70, 70, 70,
             71, 71, 71,
             72, 72, 72,
             73, 73, 73,
             74, 74, 74,
             75, 75, 75,
             76, 76, 76,
             77, 77, 77,
             78, 78, 78,
             79, 79, 79,
             80, 80, 80,
             81, 81, 81,
             82, 82, 82,
             83, 83, 83,
             84, 84, 84,
             85, 85, 85,
             86, 86, 86,
             87, 87, 87,
             88, 88, 88,
             89, 89, 89,
             90, 90, 90,
             91, 91, 91,
             92, 92, 92,
             93, 93, 93,
             94, 94, 94,
             95, 95, 95,
             96, 96, 96,
             97, 97, 97,
             98, 98, 98,
             99, 99, 99,
             100, 100, 100,
             101, 101, 101,
             102, 102, 102,
             103, 103, 103,
             104, 104, 104,
             105, 105, 105,
             106, 106, 106,
             107, 107, 107,
             108, 108, 108,
             109, 109, 109,
             110, 110, 110,
             111, 111, 111,
             112, 112, 112,
             113, 113, 113,
             114, 114, 114,
             115, 115, 115,
             116, 116, 116,
             117, 117, 117,
             118, 118, 118,
             119, 119, 119,
             120, 120, 120,
             121, 121, 121,
             122, 122, 122,
             123, 123, 123,
             124, 124, 124,
             125, 125, 125,
             126, 126, 126,
             127, 127, 127,
             128, 128, 128,
             129, 129, 129,
             130, 130, 130,
             131, 131, 131,
             132, 132, 132,
             133, 133, 133,
             134, 134, 134,
             135, 135, 135,
             136, 136, 136,
             137, 137, 137,
             138, 138, 138,
             139, 139, 139,
             140, 140, 140,
             141, 141, 141,
             142, 142, 142,
             143, 143, 143,
             144, 144, 144,
             145, 145, 145,
             146, 146, 146,
             147, 147, 147,
             148, 148, 148,
             149, 149, 149,
             150, 150, 150,
             151, 151, 151,
             152, 152, 152,
             153, 153, 153,
             154, 154, 154,
             155, 155, 155,
             156, 156, 156,
             157, 157, 157,
             158, 158, 158,
             159, 159, 159,
             160, 160, 160,
             161, 161, 161,
             162, 162, 162,
             163, 163, 163,
             164, 164, 164,
             165, 165, 165,
             166, 166, 166,
             167, 167, 167,
             168, 168, 168,
             169, 169, 169,
             170, 170, 170,
             171, 171, 171,
             172, 172, 172,
             173, 173, 173,
             174, 174, 174,
             175, 175, 175,
             176, 176, 176,
             177, 177, 177,
             178, 178, 178,
             179, 179, 179,
             180, 180, 180,
             181, 181, 181,
             182, 182, 182,
             183, 183, 183,
             184, 184, 184,
             185, 185, 185,
             186, 186, 186,
             187, 187, 187,
             188, 188, 188,
             189, 189, 189,
             190, 190, 190,
             191, 191, 191,
             192, 192, 192,
             193, 193, 193,
             194, 194, 194,
             195, 195, 195,
             196, 196, 196,
             197, 197, 197,
             198, 198, 198,
             199, 199, 199,
             200, 200, 200,
             201, 201, 201,
             202, 202, 202,
             203, 203, 203,
             204, 204, 204,
             205, 205, 205,
             206, 206, 206,
             207, 207, 207,
             208, 208, 208,
             209, 209, 209,
             210, 210, 210,
             211, 211, 211,
             212, 212, 212,
             213, 213, 213,
             214, 214, 214,
             215, 215, 215,
             216, 216, 216,
             217, 217, 217,
             218, 218, 218,
             219, 219, 219,
             220, 220, 220,
             221, 221, 221,
             222, 222, 222,
             223, 223, 223,
             224, 224, 224,
             225, 225, 225,
             226, 226, 226,
             227, 227, 227,
             228, 228, 228,
             229, 229, 229,
             230, 230, 230,
             231, 231, 231,
             232, 232, 232,
             233, 233, 233,
             234, 234, 234,
             235, 235, 235,
             236, 236, 236,
             237, 237, 237,
             238, 238, 238,
             239, 239, 239,
             240, 240, 240,
             241, 241, 241,
             242, 242, 242,
             243, 243, 243,
             244, 244, 244,
             245, 245, 245,
             246, 246, 246,
             247, 247, 247,
             248, 248, 248,
             249, 249, 249,
             250, 250, 250,
             251, 251, 251,
             252, 252, 252,
             253, 253, 253,
             254, 254, 254,
             255, 0,   0,
       }};

// Generated in MATLAB using:
//    gv = 0.4;
//    a = linspace(gv,1.0,128)'; % blue
//    b = linspace(gv,0.9,128)'; % yellow
//    c = linspace(gv,0.0,128)'; % black
//    g = [flipud([c,c,a]);b,b,c]; % blue-yellow
//    round(g*255)
static constexpr std::array< uint8, 256 * 3 > divergentColorMap =
      {{
             0, 0, 255,
             1, 1, 254,
             2, 2, 253,
             2, 2, 251,
             3, 3, 250,
             4, 4, 249,
             5, 5, 248,
             6, 6, 247,
             6, 6, 245,
             7, 7, 244,
             8, 8, 243,
             9, 9, 242,
             10, 10, 241,
             10, 10, 239,
             11, 11, 238,
             12, 12, 237,
             13, 13, 236,
             14, 14, 235,
             14, 14, 233,
             15, 15, 232,
             16, 16, 231,
             17, 17, 230,
             18, 18, 228,
             18, 18, 227,
             19, 19, 226,
             20, 20, 225,
             21, 21, 224,
             22, 22, 222,
             22, 22, 221,
             23, 23, 220,
             24, 24, 219,
             25, 25, 218,
             26, 26, 216,
             27, 27, 215,
             27, 27, 214,
             28, 28, 213,
             29, 29, 212,
             30, 30, 210,
             31, 31, 209,
             31, 31, 208,
             32, 32, 207,
             33, 33, 206,
             34, 34, 204,
             35, 35, 203,
             35, 35, 202,
             36, 36, 201,
             37, 37, 200,
             38, 38, 198,
             39, 39, 197,
             39, 39, 196,
             40, 40, 195,
             41, 41, 194,
             42, 42, 192,
             43, 43, 191,
             43, 43, 190,
             44, 44, 189,
             45, 45, 188,
             46, 46, 186,
             47, 47, 185,
             47, 47, 184,
             48, 48, 183,
             49, 49, 182,
             50, 50, 180,
             51, 51, 179,
             51, 51, 178,
             52, 52, 177,
             53, 53, 175,
             54, 54, 174,
             55, 55, 173,
             55, 55, 172,
             56, 56, 171,
             57, 57, 169,
             58, 58, 168,
             59, 59, 167,
             59, 59, 166,
             60, 60, 165,
             61, 61, 163,
             62, 62, 162,
             63, 63, 161,
             63, 63, 160,
             64, 64, 159,
             65, 65, 157,
             66, 66, 156,
             67, 67, 155,
             67, 67, 154,
             68, 68, 153,
             69, 69, 151,
             70, 70, 150,
             71, 71, 149,
             71, 71, 148,
             72, 72, 147,
             73, 73, 145,
             74, 74, 144,
             75, 75, 143,
             75, 75, 142,
             76, 76, 141,
             77, 77, 139,
             78, 78, 138,
             79, 79, 137,
             80, 80, 136,
             80, 80, 135,
             81, 81, 133,
             82, 82, 132,
             83, 83, 131,
             84, 84, 130,
             84, 84, 129,
             85, 85, 127,
             86, 86, 126,
             87, 87, 125,
             88, 88, 124,
             88, 88, 122,
             89, 89, 121,
             90, 90, 120,
             91, 91, 119,
             92, 92, 118,
             92, 92, 116,
             93, 93, 115,
             94, 94, 114,
             95, 95, 113,
             96, 96, 112,
             96, 96, 110,
             97, 97, 109,
             98, 98, 108,
             99, 99, 107,
             100, 100, 106,
             100, 100, 104,
             101, 101, 103,
             102, 102, 102,
             102, 102, 102,
             103, 103, 101,
             104, 104, 100,
             105, 105, 100,
             106, 106, 99,
             107, 107, 98,
             108, 108, 97,
             109, 109, 96,
             110, 110, 96,
             111, 111, 95,
             112, 112, 94,
             113, 113, 93,
             114, 114, 92,
             115, 115, 92,
             116, 116, 91,
             117, 117, 90,
             118, 118, 89,
             119, 119, 88,
             120, 120, 88,
             121, 121, 87,
             122, 122, 86,
             123, 123, 85,
             124, 124, 84,
             125, 125, 84,
             126, 126, 83,
             127, 127, 82,
             128, 128, 81,
             129, 129, 80,
             130, 130, 80,
             131, 131, 79,
             132, 132, 78,
             133, 133, 77,
             134, 134, 76,
             135, 135, 75,
             136, 136, 75,
             137, 137, 74,
             138, 138, 73,
             139, 139, 72,
             140, 140, 71,
             141, 141, 71,
             142, 142, 70,
             143, 143, 69,
             144, 144, 68,
             145, 145, 67,
             146, 146, 67,
             147, 147, 66,
             148, 148, 65,
             149, 149, 64,
             150, 150, 63,
             151, 151, 63,
             152, 152, 62,
             153, 153, 61,
             154, 154, 60,
             155, 155, 59,
             156, 156, 59,
             157, 157, 58,
             158, 158, 57,
             159, 159, 56,
             160, 160, 55,
             161, 161, 55,
             162, 162, 54,
             163, 163, 53,
             164, 164, 52,
             165, 165, 51,
             166, 166, 51,
             167, 167, 50,
             168, 168, 49,
             169, 169, 48,
             170, 170, 47,
             171, 171, 47,
             172, 172, 46,
             173, 173, 45,
             174, 174, 44,
             175, 175, 43,
             176, 176, 43,
             177, 177, 42,
             178, 178, 41,
             179, 179, 40,
             180, 180, 39,
             181, 181, 39,
             182, 182, 38,
             183, 183, 37,
             184, 184, 36,
             185, 185, 35,
             186, 186, 35,
             187, 187, 34,
             188, 188, 33,
             189, 189, 32,
             190, 190, 31,
             191, 191, 31,
             192, 192, 30,
             193, 193, 29,
             194, 194, 28,
             195, 195, 27,
             196, 196, 27,
             197, 197, 26,
             198, 198, 25,
             199, 199, 24,
             200, 200, 23,
             201, 201, 22,
             202, 202, 22,
             203, 203, 21,
             204, 204, 20,
             205, 205, 19,
             206, 206, 18,
             207, 207, 18,
             208, 208, 17,
             209, 209, 16,
             210, 210, 15,
             211, 211, 14,
             212, 212, 14,
             213, 213, 13,
             214, 214, 12,
             215, 215, 11,
             216, 216, 10,
             217, 217, 10,
             218, 218, 9,
             219, 219, 8,
             220, 220, 7,
             221, 221, 6,
             222, 222, 6,
             223, 223, 5,
             224, 224, 4,
             225, 225, 3,
             226, 226, 2,
             227, 227, 2,
             228, 228, 1,
             230, 230, 0
       }};

// Black + 16 repetitions of 16 unique colors.
// Greens are closer together perceptually, so we reduce the number of green entries in this map.
static constexpr std::array< uint8, 256 * 3 > labelColorMap =
      {{
             0,   0,   0,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
             255, 0,   85,
             255, 0,   0,
             0,   255, 0,
             0,   0,   255,
             255, 255, 0,
             0,   255, 255,
             255, 0,   255,
             255, 85,  0,
             170, 255, 0,
             0,   170, 255,
             85,  0,   255,
             255, 0,   170,
             255, 170, 0,
             0,   255, 128,
             0,   85,  255,
             170, 0,   255,
       }};

/*
 * The following colormaps are from colorcet:
 * http://peterkovesi.com/projects/colourmaps/index.html
 *
 * Copyright (c) 2017 Peter Kovesi
 * Centre for Exploration Targeting
 * The University of Western Australia
 * peter.kovesi at uwa edu au
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided "as is", without warranty of any kind.
 */

// Blue-Magenta-Yellow highly saturated colour map
static constexpr std::array< uint8, 256 * 3 > linearColorMap =
      {{
             0,15,93,
             1,15,94,
             1,16,96,
             1,16,98,
             1,17,99,
             2,17,101,
             2,18,103,
             3,18,104,
             4,18,106,
             5,19,107,
             6,19,109,
             7,20,111,
             8,20,112,
             10,21,114,
             12,21,115,
             13,21,117,
             15,22,118,
             17,22,119,
             18,22,121,
             20,23,122,
             22,23,124,
             24,23,125,
             25,24,126,
             27,24,128,
             29,24,129,
             31,25,130,
             33,25,131,
             35,25,133,
             37,25,134,
             39,26,135,
             41,26,136,
             43,26,137,
             46,26,138,
             48,26,139,
             50,27,140,
             52,27,141,
             55,27,142,
             57,27,143,
             59,27,143,
             62,27,144,
             64,27,144,
             67,26,145,
             70,26,145,
             72,26,146,
             75,26,146,
             78,25,146,
             81,25,146,
             84,25,146,
             86,24,146,
             89,24,146,
             92,23,146,
             94,23,146,
             97,23,146,
             99,22,146,
             102,22,146,
             104,21,146,
             107,21,146,
             109,20,146,
             111,20,146,
             114,19,146,
             116,19,146,
             118,18,145,
             121,18,145,
             123,17,145,
             125,17,145,
             127,16,145,
             129,16,145,
             131,15,144,
             134,15,144,
             136,14,144,
             138,14,144,
             140,13,143,
             142,13,143,
             144,13,143,
             146,12,142,
             148,12,142,
             150,11,142,
             152,11,141,
             154,11,141,
             156,10,140,
             158,10,140,
             160,10,140,
             162,10,139,
             163,10,139,
             165,9,138,
             167,9,138,
             169,9,138,
             171,9,137,
             173,9,137,
             174,10,136,
             176,10,136,
             178,10,136,
             180,10,135,
             182,10,135,
             183,11,134,
             185,11,134,
             187,12,133,
             189,12,133,
             190,13,133,
             192,13,132,
             194,14,132,
             195,14,131,
             197,15,131,
             199,16,130,
             200,16,130,
             202,17,129,
             204,18,129,
             205,19,128,
             207,20,128,
             208,21,127,
             210,22,127,
             211,23,126,
             213,25,125,
             214,26,125,
             215,28,124,
             217,29,123,
             218,31,123,
             220,32,122,
             221,34,121,
             222,35,120,
             223,37,120,
             224,39,119,
             226,41,118,
             227,42,117,
             228,44,116,
             229,46,115,
             230,48,114,
             231,49,114,
             232,51,113,
             233,53,112,
             234,55,111,
             235,57,110,
             236,59,108,
             237,61,107,
             237,62,106,
             238,64,105,
             239,66,104,
             240,68,103,
             240,70,102,
             241,72,101,
             242,74,100,
             242,76,98,
             243,77,97,
             243,79,96,
             244,81,95,
             244,83,94,
             245,85,93,
             245,87,92,
             246,88,92,
             246,90,91,
             247,92,90,
             247,94,89,
             247,96,88,
             248,98,87,
             248,100,86,
             248,101,85,
             248,103,85,
             248,105,84,
             249,107,83,
             249,109,82,
             249,111,82,
             249,112,81,
             249,114,80,
             249,116,79,
             249,118,79,
             249,120,78,
             249,122,77,
             249,123,77,
             249,125,76,
             249,127,76,
             249,129,75,
             249,130,74,
             249,132,74,
             249,134,73,
             249,135,72,
             249,137,72,
             249,138,71,
             249,140,70,
             249,142,70,
             249,143,69,
             249,145,68,
             249,146,68,
             249,148,67,
             249,149,66,
             249,151,66,
             250,152,65,
             250,154,65,
             250,155,64,
             250,157,63,
             250,158,63,
             250,159,62,
             250,161,61,
             251,162,61,
             251,164,60,
             251,165,59,
             251,166,59,
             251,168,58,
             252,169,57,
             252,171,57,
             252,172,57,
             252,173,56,
             252,175,56,
             252,176,56,
             252,177,56,
             252,179,56,
             252,180,56,
             252,182,56,
             253,183,56,
             253,184,56,
             253,186,56,
             253,187,56,
             253,189,56,
             253,190,56,
             253,191,56,
             253,193,57,
             253,194,57,
             252,195,57,
             252,197,57,
             252,198,58,
             252,200,58,
             252,201,58,
             252,202,59,
             252,204,59,
             252,205,59,
             252,206,60,
             252,208,60,
             252,209,61,
             252,211,61,
             251,212,62,
             251,213,62,
             251,215,63,
             251,216,63,
             251,217,64,
             251,219,64,
             251,220,65,
             250,222,65,
             250,223,66,
             250,224,66,
             250,226,67,
             250,227,67,
             249,228,68,
             249,230,69,
             249,231,69,
             249,232,70,
             248,234,70,
             248,235,71,
             248,237,72,
             248,238,72,
             247,239,73,
             247,241,74,
             247,242,74,
             247,243,75,
             246,245,76,
             246,246,76,
             246,247,77,
             245,249,78
       }};

// Linear-diverging blue - grey - red
static constexpr std::array< uint8, 256 * 3 > divergingLinearColorMap =
      {{
             0,42,215,
             0,43,214,
             0,43,213,
             0,44,212,
             0,44,212,
             0,45,211,
             0,45,210,
             0,46,209,
             7,46,208,
             13,47,207,
             19,47,206,
             23,48,205,
             27,48,204,
             30,49,203,
             33,49,202,
             35,50,201,
             38,50,201,
             40,51,200,
             42,51,199,
             44,52,198,
             46,52,197,
             47,53,196,
             49,53,195,
             51,54,194,
             52,54,193,
             54,55,192,
             55,55,191,
             56,56,191,
             58,56,190,
             59,57,189,
             60,57,188,
             61,57,187,
             62,58,186,
             64,58,185,
             65,59,184,
             66,59,183,
             67,60,182,
             68,60,181,
             69,61,181,
             69,61,180,
             70,62,179,
             71,62,178,
             72,63,177,
             73,63,176,
             74,64,175,
             74,64,174,
             75,64,173,
             76,65,172,
             77,65,172,
             77,66,171,
             78,66,170,
             79,67,169,
             79,67,168,
             80,68,167,
             81,68,166,
             81,69,165,
             82,69,164,
             82,70,163,
             83,70,163,
             84,70,162,
             84,71,161,
             85,71,160,
             85,72,159,
             86,72,158,
             86,73,157,
             87,73,156,
             87,74,155,
             88,74,155,
             88,74,154,
             88,75,153,
             89,75,152,
             89,76,151,
             90,76,150,
             90,77,149,
             91,77,148,
             91,78,147,
             91,78,146,
             92,78,146,
             92,79,145,
             92,79,144,
             93,80,143,
             93,80,142,
             93,81,141,
             94,81,140,
             94,82,139,
             94,82,138,
             94,82,138,
             95,83,137,
             95,83,136,
             95,84,135,
             96,84,134,
             96,85,133,
             96,85,132,
             96,86,131,
             96,86,130,
             97,86,130,
             97,87,129,
             97,87,128,
             97,88,127,
             98,88,126,
             98,89,125,
             98,89,124,
             98,89,123,
             98,90,122,
             98,90,122,
             99,91,121,
             99,91,120,
             99,92,119,
             99,92,118,
             99,92,117,
             99,93,116,
             99,93,115,
             99,94,114,
             99,94,113,
             100,95,113,
             100,95,112,
             100,95,111,
             100,96,110,
             100,96,109,
             100,97,108,
             100,97,107,
             100,98,106,
             100,98,105,
             100,98,104,
             100,99,103,
             100,99,103,
             100,100,102,
             100,100,101,
             101,100,100,
             103,100,99,
             105,100,99,
             106,100,98,
             108,100,97,
             110,100,97,
             111,99,96,
             113,99,96,
             114,99,95,
             116,99,94,
             117,99,94,
             119,99,93,
             120,99,92,
             122,98,92,
             123,98,91,
             125,98,91,
             126,98,90,
             128,98,89,
             129,98,89,
             131,97,88,
             132,97,87,
             134,97,87,
             135,97,86,
             136,97,86,
             138,96,85,
             139,96,84,
             140,96,84,
             142,96,83,
             143,96,82,
             144,95,82,
             146,95,81,
             147,95,81,
             148,95,80,
             150,94,79,
             151,94,79,
             152,94,78,
             154,94,77,
             155,93,77,
             156,93,76,
             158,93,75,
             159,93,75,
             160,92,74,
             161,92,73,
             163,92,73,
             164,91,72,
             165,91,72,
             166,91,71,
             168,91,70,
             169,90,70,
             170,90,69,
             171,90,68,
             173,89,68,
             174,89,67,
             175,89,66,
             176,88,66,
             177,88,65,
             179,88,64,
             180,87,64,
             181,87,63,
             182,86,62,
             183,86,62,
             185,86,61,
             186,85,60,
             187,85,60,
             188,84,59,
             189,84,58,
             191,84,58,
             192,83,57,
             193,83,56,
             194,82,56,
             195,82,55,
             197,81,54,
             198,81,53,
             199,80,53,
             200,80,52,
             201,79,51,
             202,79,51,
             204,78,50,
             205,78,49,
             206,77,48,
             207,77,48,
             208,76,47,
             209,76,46,
             210,75,46,
             212,74,45,
             213,74,44,
             214,73,43,
             215,73,43,
             216,72,42,
             217,71,41,
             218,71,40,
             220,70,39,
             221,69,39,
             222,69,38,
             223,68,37,
             224,67,36,
             225,66,35,
             226,65,35,
             228,65,34,
             229,64,33,
             230,63,32,
             231,62,31,
             232,61,30,
             233,60,29,
             234,59,28,
             235,58,27,
             237,58,27,
             238,56,26,
             239,55,25,
             240,54,24,
             241,53,22,
             242,52,21,
             243,51,20,
             244,50,19,
             246,48,18,
             247,47,17,
             248,46,15,
             249,44,14,
             250,43,13,
             251,41,11,
             252,40,9,
             253,38,8,
             255,36,6,
             255,34,5,
             255,32,3,
             255,30,1,
             255,27,0,
             255,25,0
       }};

// Cyclic: magenta - yellow - green - blue. Alows four orientations/phase angles to be visulaised.  Really good!
static constexpr std::array< uint8, 256 * 3 > cyclicColorMap =
      {{
             239,85,242,
             241,87,240,
             242,90,239,
             243,93,237,
             245,95,235,
             246,98,233,
             246,101,231,
             247,104,229,
             248,107,226,
             249,111,224,
             249,114,222,
             250,117,219,
             250,120,217,
             250,123,214,
             251,126,211,
             251,129,209,
             251,132,206,
             251,135,203,
             251,138,201,
             252,141,198,
             252,144,195,
             252,147,192,
             252,149,190,
             252,152,187,
             252,155,184,
             252,158,181,
             252,160,179,
             252,163,176,
             252,166,173,
             252,168,170,
             252,171,167,
             252,173,165,
             252,176,162,
             251,178,159,
             251,181,156,
             251,183,153,
             251,185,150,
             252,188,147,
             252,190,144,
             252,192,141,
             252,195,138,
             252,197,135,
             252,199,132,
             252,201,129,
             252,204,126,
             252,206,123,
             252,208,120,
             252,210,117,
             252,212,113,
             252,214,110,
             252,216,107,
             252,218,103,
             252,221,100,
             251,223,96,
             251,224,92,
             251,226,88,
             250,228,85,
             250,230,81,
             249,231,77,
             248,233,73,
             247,234,69,
             245,235,65,
             244,236,61,
             242,237,57,
             241,238,53,
             239,238,50,
             237,238,46,
             234,239,43,
             232,238,40,
             230,238,37,
             227,238,35,
             225,237,33,
             222,237,31,
             219,236,29,
             216,235,27,
             214,234,26,
             211,233,25,
             208,232,24,
             205,231,24,
             202,230,23,
             199,229,22,
             196,228,22,
             193,227,21,
             190,226,21,
             187,225,21,
             184,224,20,
             181,223,20,
             178,222,19,
             175,221,19,
             172,220,19,
             169,218,18,
             166,217,18,
             163,216,18,
             160,215,17,
             157,214,17,
             154,213,17,
             151,212,16,
             148,211,16,
             145,210,16,
             141,208,15,
             138,207,15,
             135,206,15,
             132,205,14,
             129,204,14,
             125,203,14,
             122,202,13,
             119,200,13,
             115,199,13,
             112,198,13,
             108,197,12,
             105,196,12,
             101,195,12,
             98,194,12,
             94,192,12,
             90,191,12,
             86,190,12,
             83,189,13,
             79,188,14,
             75,186,15,
             72,185,16,
             68,184,18,
             65,183,20,
             62,181,22,
             59,180,25,
             56,179,27,
             54,177,30,
             52,176,33,
             51,174,37,
             50,173,40,
             50,171,44,
             50,169,47,
             50,168,51,
             51,166,55,
             53,164,59,
             54,163,63,
             56,161,66,
             57,159,70,
             59,157,74,
             60,155,78,
             62,153,82,
             63,152,86,
             64,150,89,
             65,148,93,
             66,146,97,
             67,144,100,
             68,142,104,
             68,140,107,
             69,138,111,
             69,137,114,
             69,135,118,
             69,133,121,
             69,131,125,
             69,129,128,
             68,127,132,
             68,125,135,
             67,123,138,
             66,121,142,
             65,120,145,
             64,118,148,
             63,116,151,
             62,114,155,
             60,112,158,
             59,110,161,
             57,108,164,
             56,106,167,
             54,104,170,
             52,102,173,
             51,100,176,
             49,97,179,
             48,95,182,
             47,93,185,
             45,91,187,
             44,88,190,
             43,86,193,
             42,83,195,
             42,81,198,
             41,78,200,
             40,76,203,
             39,73,205,
             39,70,208,
             38,67,210,
             38,64,213,
             37,62,215,
             37,59,217,
             36,56,219,
             36,53,222,
             37,50,224,
             37,47,226,
             38,44,228,
             40,41,229,
             41,39,231,
             44,36,233,
             46,34,234,
             49,32,236,
             52,30,237,
             55,29,238,
             59,28,239,
             62,27,240,
             66,27,241,
             69,27,242,
             73,27,242,
             77,28,243,
             80,28,243,
             84,29,244,
             87,31,244,
             90,32,245,
             94,33,245,
             97,35,246,
             100,36,246,
             103,38,246,
             106,39,247,
             109,41,247,
             112,42,247,
             115,44,248,
             117,45,248,
             120,46,248,
             123,48,249,
             126,49,249,
             128,50,249,
             131,52,250,
             134,53,250,
             137,54,250,
             140,55,250,
             143,56,251,
             145,57,251,
             148,58,251,
             152,59,251,
             155,60,251,
             158,61,251,
             161,62,251,
             164,62,251,
             167,63,251,
             171,64,251,
             174,64,251,
             177,65,251,
             181,65,251,
             184,66,251,
             187,66,251,
             191,67,251,
             194,67,251,
             197,68,251,
             200,68,250,
             203,69,250,
             207,69,250,
             210,70,250,
             213,70,250,
             216,71,249,
             219,72,249,
             221,72,249,
             224,73,248,
             227,75,248,
             229,76,247,
             231,77,246,
             233,79,245,
             236,81,244,
             237,83,243
       }};


}

void ApplyColorMap(
      Image const& in,
      Image& out,
      String const& colorMap
) {
   uint8 const* values;
   if(( colorMap == "grey" ) || ( colorMap == "gray" )) {
      values = greyColorMap.data();
   } else if ( colorMap == "saturation" ) {
      values = saturationColorMap.data();
   } else if ( colorMap == "divergent" ) {
      values = divergentColorMap.data();
   } else if ( colorMap == "label" ) {
      values = labelColorMap.data();
   } else if ( colorMap == "linear" ) {
      values = linearColorMap.data();
   } else if ( colorMap == "diverging_linear" ) {
      values = divergingLinearColorMap.data();
   } else if ( colorMap == "cyclic" ) {
      values = cyclicColorMap.data();
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
   void* data = const_cast< void* >( static_cast< void const* >( values ));
   Image im( NonOwnedRefToDataSegment( data ), data, DT_UINT8, { 256 }, { 3 }, Tensor( 3 ), 1 );
   LookupTable lut( im );
   DIP_STACK_TRACE_THIS( lut.Apply( in, out ));
   out.SetColorSpace( "RGB" );
}

void Overlay(
      Image const& c_in,
      Image const& overlay,
      Image& out,
      Image::Pixel const& color
) {
   DIP_THROW_IF( !c_in.IsForged() || !overlay.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( overlay.Sizes() != c_in.Sizes(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF( !overlay.IsScalar(), E::MASK_NOT_SCALAR );
   Image in = c_in;
   if( in.DataType().IsBinary() ) {
      in.Convert( DT_UINT8 ); // This changes the data type without copy.
   }
   if( overlay.DataType().IsUInt() || !in.IsScalar() || !color.IsScalar() ) {
      // This code is not run in the case that `overlay` is binary, and `in` and `color` are scalar. In this case,
      //    we create a grey-value output.
      if( in.IsScalar()  ) {
         in.ExpandSingletonTensor( 3 );
         in.SetColorSpace( "RGB" );
      } else {
         DIP_THROW_IF( in.TensorElements() != 3, "Input image must have 1 or 3 tensor elements" );
         DIP_THROW_IF( in.IsColor() && ( in.ColorSpace() != "RGB" ), "Convert input image to RGB color space first" );
         in.SetColorSpace( "RGB" );
      }
   }
   if( out.IsForged() && out.IsSingletonExpanded() ) {
      // This could happen if &out == &c_in.
      out.Strip();
   }
   out.Copy( in );
   if( overlay.DataType().IsBinary() ) {
      // A binary overlay
      DIP_THROW_IF( !color.IsScalar() && ( color.TensorElements() != 3 ), "Color must have 1 or 3 tensor elements" );
      DIP_STACK_TRACE_THIS( out.At( overlay ) = color );
      // out.At( overlay ).Fill( color );
   } else if( overlay.DataType().IsUnsigned() ) {
      // A labeled overlay
      Image mask = overlay > 0;
      Image labels = overlay.At( mask );
      DIP_STACK_TRACE_THIS( ApplyColorMap( labels, labels, "label" ));
      DIP_STACK_TRACE_THIS( out.At( mask ) = labels );
      // out.At( mask ).Copy( labels );
   } else {
      DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}

} // namespace dip
