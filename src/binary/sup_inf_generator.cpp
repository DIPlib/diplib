/*
 * DIPlib 3.0
 * This file contains binary propagation.
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/binary.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/morphology.h"
#include "diplib/boundary.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/pixel_table.h"

namespace dip {

Interval::Interval( Image const& image ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !image.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( image.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   for( auto& s : image.Sizes() ) {
      DIP_THROW_IF( !(s & 1), "The interval is not odd in size" );
   }
   hit_ = image == 1;
   DIP_THROW_IF( !Any( hit_ ).As< bool >(), "The interval needs at least one foreground pixel" );
   miss_ = image == 0;
}

Interval::Interval( Image hit, Image miss ) : hit_( std::move( hit )), miss_( std::move( miss )) {
   DIP_THROW_IF( !hit_.IsForged() || !miss_.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !hit_.IsScalar() || !miss_.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !hit_.DataType().IsBinary() || !miss_.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( hit_.Sizes() != miss_.Sizes(), E::SIZES_DONT_MATCH );
   for( auto& s : hit_.Sizes() ) {
      DIP_THROW_IF( !(s & 1), "The interval is not odd in size" );
   }
   DIP_THROW_IF( !Any( hit_ ).As< bool >(), "The interval needs at least one foreground pixel" );
   DIP_THROW_IF( dip::Any( dip::Infimum( hit_, miss_ )).As< bool >(), "The hit and miss images are not disjoint" );
}

namespace {

// Rotates 2D binary image clockwise
Image RotateBy45Degrees( Image const& input ) {
   DIP_ASSERT( input.Dimensionality() == 2 );
   Image output;
   dip::uint len = input.Size( 0 );
   if( input.Size( 1 ) == len ) {
      output = input.Copy();
   } else {
      // It's not square, let's create a square image
      len = std::max( len, input.Size( 1 ));
      UnsignedArray sizes( 2, len );
      output = input.Pad( sizes );
   }
   DIP_ASSERT( output.DataType().IsBinary() );
   dip::sint stepX = output.Stride( 0 );
   dip::sint stepY = output.Stride( 1 );
   // Rotate by shells. Each shell (pixels with the same L_inf distance) rotates independently.
   dip::bin* ptr = static_cast< dip::bin* >( output.Origin() );
   for( dip::uint shell = 0; shell < len / 2; ++shell ) {
      dip::sint n = static_cast< dip::sint >( len / 2 - shell ); // number of pixels in half an edge
      dip::sint nStepX = n * stepX;
      dip::sint nStepY = n * stepY;
      dip::sint nEndX = 2 * nStepX;
      dip::sint nEndY = 2 * nStepY;
      for( dip::sint ii = 0; ii < n; ++ ii ) {
         dip::sint iX = ii * stepX;
         dip::sint iY = ii * stepY;
         dip::bin* ptr1 = ptr + iX;
         dip::bin* ptr2 = ptr + nStepY - iY;
         dip::bin value = *ptr1;
         *ptr1 = *ptr2;
         ptr1 = ptr2; ptr2 += nStepY; *ptr1 = *ptr2;
         ptr1 = ptr2; ptr2 = ptr + nStepX - iX + nEndY; *ptr1 = *ptr2;
         ptr1 = ptr2; ptr2 += nStepX; *ptr1 = *ptr2;
         ptr1 = ptr2; ptr2 = ptr + nEndX + iY + nStepY; *ptr1 = *ptr2;
         ptr1 = ptr2; ptr2 -= nStepY; *ptr1 = *ptr2;
         ptr1 = ptr2; ptr2 = ptr + nStepX + iX; *ptr1 = *ptr2;
         *ptr2 = value;
      }
      ptr += stepX + stepY;
   }
   return output;
}

}

IntervalArray Interval::GenerateRotatedVersions(
      dip::uint rotationAngle,
      String rotationDirection
) const {
   DIP_THROW_IF( hit_.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint step = 1;
   if( rotationAngle == 45 ) {
   } else if( rotationAngle == 90 ) {
      step = 2;
   } else if( rotationAngle == 180 ) {
      step = 4;
   } else {
      DIP_THROW_INVALID_FLAG( std::to_string( rotationAngle ));
   }
   bool interleaved = true;
   bool clockwise = true;
   if( rotationDirection == "interleaved clockwise" ) {
   } else if( rotationDirection == "interleaved counter-clockwise" ) {
      clockwise = false;
   } else if( rotationDirection == "clockwise" ) {
      interleaved = false;
   } else if( rotationDirection == "counter-clockwise" ) {
      interleaved = false;
      clockwise = false;
   } else {
      DIP_THROW_INVALID_FLAG( rotationDirection );
   }
   dip::uint N = 8 / step;
   IntervalArray output( N, Interval{} );
   output[ 0 ] = *this;
   if( step == 1 ) {
      // 45 degrees + ( 0, 90, 180, 270 )
      dip::uint cur = clockwise ? 1 : 7;
      output[ cur ].hit_ = RotateBy45Degrees( hit_ );
      output[ cur ].miss_ = RotateBy45Degrees( miss_ );
      for( dip::uint ii = 0; ii < 3; ++ii ) {
         dip::uint next = clockwise ? cur + 2 : cur - 2;
         output[ next ] = output[ cur ];
         output[ next ].hit_.Rotation90( 1 );
         output[ next ].miss_.Rotation90( 1 );
         cur = next;
      }
   }
   if( step != 4 ) {
      // 90, 180, 270
      // We skip with strides of `3 - step` through the array to reach these elements
      //   with step==1: 3-1=2, every other element
      //   with step==2: 3-2=1, every element
      dip::uint cur = 0;
      for( dip::uint ii = 0; ii < 3; ++ii ) {
         dip::uint next = clockwise ? cur + ( 3 - step ) : ( cur == 0 ? N : cur ) - ( 3 - step );
         output[ next ] = output[ cur ];
         output[ next ].hit_.Rotation90( 1 );
         output[ next ].miss_.Rotation90( 1 );
         cur = next;
      }
   } else {
      // 180
      output[ 1 ] = output[ 0 ];
      output[ 1 ].hit_.Rotation90( 2 );
      output[ 1 ].miss_.Rotation90( 2 );
   }
   // not interleaved: 0, 45, 90, 135, 180, 225, 270, 315.
   // interleaved:     0, 180, 45, 225, 90, 270, 135, 315.
   if( interleaved ) {
      if( step == 1 ) {
         // not interleaved: 0, 45, 90, 135, 180, 225, 270, 315.
         // swap 1, 2 & 4:   0, 180, 45, 135, 90, 225, 270, 315.
         // swap 3, 5 & 6:   0, 180, 45, 225, 90, 270, 135, 315.
         Interval tmp = output[ 1 ];
         output[ 1 ] = output[ 4 ];
         output[ 4 ] = output[ 2 ];
         output[ 2 ] = tmp;
         tmp = output[ 3 ];
         output[ 3 ] = output[ 5 ];
         output[ 5 ] = output[ 6 ];
         output[ 6 ] = tmp;
      } else if( step == 2 ) {
         // not interleaved: 0, 90, 180, 270.
         // swap 1 & 2:      0, 180, 90, 270.
         std::swap( output[ 1 ], output[ 2 ] );
      }
   }
   return output;
}

namespace {

enum class Mode { SUP, INF };

class SupInfGeneratingLineFilter : public Framework::FullLineFilter {
   public:
      SupInfGeneratingLineFilter( Mode mode ) : supGenerating_( mode == Mode::SUP ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint nKernelPixels, dip::uint ) override {
         return lineLength * nKernelPixels;
      }
      virtual void SetNumberOfThreads( dip::uint, PixelTableOffsets const& pixelTable ) override {
         // Fill in `offsets_` and `hitmiss_` using `pixelTable`.
         // The pixel table has been prepared such that pixels with a positive weight are hit, and a negative
         // weight are miss. "Don't care" pixels are not in the table.
         offsets_ = pixelTable.Offsets();
         hitmiss_.resize( offsets_.size() );
         auto const& weights = pixelTable.Weights();
         for( dip::uint ii = 0; ii < hitmiss_.size(); ++ii ) {
            hitmiss_[ ii ] = weights[ ii ] != 0;
            // hitmiss_ is 1 or 0, for hit and miss respectively.
         }
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         dip::bin* in = static_cast< dip::bin* >( params.inBuffer.buffer ); // yes, it's dip::bin, but this is simpler.
         dip::sint inStride = params.inBuffer.stride;
         dip::bin* out = static_cast< dip::bin* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         if( supGenerating_ ) {
            for( dip::uint ii = 0; ii < length; ++ii ) {
               dip::bin result = true;
               for( dip::uint jj = 0; jj < offsets_.size(); ++jj ) {
                  if( in[ offsets_[ jj ]] ^ hitmiss_[ jj ] ) { // Note that this requires our dip::bin image to be correct (i.e. 0 or 1, not other numbers).
                     result = false;
                     break;
                  }
               }
               *out = result;
               out += outStride;
               in += inStride;
            }
         } else {
            // For inf-generating operator we do the same as above, but we invert the input and invert the output
            for( dip::uint ii = 0; ii < length; ++ii ) {
               dip::bin result = false;
               for( dip::uint jj = 0; jj < offsets_.size(); ++jj ) {
                  if( !in[ offsets_[ jj ]] ^ hitmiss_[ jj ] ) { // Note that this requires our dip::bin image to be correct (i.e. 0 or 1, not other numbers).
                     result = true;
                     break;
                  }
               }
               *out = result;
               out += outStride;
               in += inStride;
            }
         }
      }
   private:
      bool supGenerating_;
      std::vector< dip::sint > offsets_;
      std::vector< dip::bin > hitmiss_;
};

void SupInfGenerating(
      Image const& in,
      Image& out,
      Interval const& interval,
      BoundaryCondition boundaryCondition,
      Mode supGenerating
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      // Create a kernel image with 0, 1, and NaN values.
      Image kernel = interval.HitImage().Similar( DT_SFLOAT );
      kernel.Fill( dip::nan );
      kernel.At( interval.HitImage() ) = 1;
      kernel.At( interval.MissImage() ) = 0; // Silly, we're recreating an image the user likely originally made when creating the interval...
      SupInfGeneratingLineFilter lineFilter( supGenerating );
      Framework::Full( in, out, DT_BIN, DT_BIN, DT_BIN, 1, { boundaryCondition }, kernel, lineFilter );
   DIP_END_STACK_TRACE
}

}

void SupGenerating(
      Image const& in,
      Image& out,
      Interval const& interval,
      String const& boundaryCondition
) {
   DIP_STACK_TRACE_THIS( SupInfGenerating( in, out, interval, StringToBoundaryCondition( boundaryCondition ), Mode::SUP ));
}

void InfGenerating(
      Image const& in,
      Image& out,
      Interval const& interval,
      String const& boundaryCondition
) {
   DIP_STACK_TRACE_THIS( SupInfGenerating( in, out, interval, StringToBoundaryCondition( boundaryCondition ), Mode::INF ));
}

void UnionSupGenerating(
      Image const& c_in,
      Image& out,
      IntervalArray const& intervals,
      String const& boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( intervals.empty(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   Image in = c_in;
   if( out.Aliases( in )) {
      DIP_STACK_TRACE_THIS( out.Strip() );   // prevent in-place operation
   }
   DIP_START_STACK_TRACE
      // TODO: expand input image as in ThickeningThinning below.
      SupGenerating( in, out, intervals[ 0 ], boundaryCondition );
      Image tmp;
      for( dip::uint ii = 1; ii < intervals.size(); ++ii ) {
         SupGenerating( in, tmp, intervals[ ii ], boundaryCondition );
         Supremum( out, tmp, out );
      }
   DIP_END_STACK_TRACE
}

void IntersectionInfGenerating(
      Image const& c_in,
      Image& out,
      IntervalArray const& intervals,
      String const& boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( intervals.empty(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   Image in = c_in;
   if( out.Aliases( in )) {
      DIP_STACK_TRACE_THIS( out.Strip() );   // prevent in-place operation
   }
   DIP_START_STACK_TRACE
      // TODO: expand input image as in ThickeningThinning below.
      InfGenerating( in, out, intervals[ 0 ], boundaryCondition );
      Image tmp;
      for( dip::uint ii = 1; ii < intervals.size(); ++ii ) {
         InfGenerating( in, tmp, intervals[ ii ], boundaryCondition );
         Infimum( out, tmp, out );
      }
   DIP_END_STACK_TRACE
}

namespace {

void ThickeningThinning(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations,
      String const& boundaryCondition,
      bool thickening
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( intervals.empty(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   // Find out what size border we need
   dip::uint nDims = in.Dimensionality();
   UnsignedArray border = intervals[ 0 ].Sizes();
   DIP_THROW_IF( border.size() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
   for( dip::uint ii = 1; ii < intervals.size(); ++ii ) {
      UnsignedArray const& b = intervals[ ii ].Sizes();
      DIP_THROW_IF( b.size() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
      for( dip::uint jj = 0; jj < b.size(); ++jj ) {
         border[ jj ] = std::max( border[ jj ], b[ jj ] );
      }
   }
   for( auto& b : border ) {
      b /= 2;
   }
   // Prepare the output image, adding a border if it's not already there
   if( boundaryCondition != S::ALREADY_EXPANDED ) {
      StringArray bc;
      if( !boundaryCondition.empty() ) {
         bc.push_back( boundaryCondition );
      }
      DIP_STACK_TRACE_THIS( out = ExtendImage( in, border, bc, { "masked" } ));
   } else {
      if( &out != &in ) {
         // Get a view of input image plus its border
         Image largerIn = in;
         IntegerArray origin( nDims );
         UnsignedArray sizes = in.Sizes();
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            origin[ ii ] = -static_cast< dip::sint >( border[ ii ] );
            sizes[ ii ] += 2 * border[ ii ];
         }
         largerIn.dip__ShiftOrigin( largerIn.Offset( origin ));
         largerIn.dip__SetSizes( sizes );
         // Copy
         if( out.IsForged() && !out.CompareProperties( largerIn, Option::CmpProp::Samples, Option::ThrowException::DONT_THROW )) {
            out.Strip(); // Make sure we don't get a data type conversion here
         }
         out.Copy( largerIn );
         // Out must be a view on this
         out.dip__ShiftOrigin( out.Offset( border ));
         out.dip__SetSizes( in.Sizes() );
      }
   }
   DIP_START_STACK_TRACE
      Image tmp;
      bool untilConvergence = iterations == 0;
      while( true ) {
         bool change = false;
         for( auto const& inter : intervals ) {
            SupInfGenerating( out, tmp, inter, BoundaryCondition::ALREADY_EXPANDED, Mode::SUP );
            if( mask.IsForged() ) {
               tmp &= mask;
            }
            if( thickening ) {
               out += tmp;
            } else {
               out -= tmp;
            }
            if( untilConvergence && Any( tmp ).As< bool >() ) {
               change = true;
            }
         }
         if( untilConvergence ) {
            if( !change ) {
               break;
            }
         } else {
            --iterations;
            if( iterations == 0 ) {
               break;
            }
         }
      }
   DIP_END_STACK_TRACE
}

}

void Thickening(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations,
      String const& boundaryCondition
) {
   DIP_STACK_TRACE_THIS( ThickeningThinning( in, mask, out, intervals, iterations, boundaryCondition, true ));
}

void Thinning(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations,
      String const& boundaryCondition
) {
   DIP_STACK_TRACE_THIS( ThickeningThinning( in, mask, out, intervals, iterations, boundaryCondition, false ));
}

constexpr uint8 X = 2; // The "don't care" pixels.

IntervalArray HomotopicThinningInterval2D( dip::uint connectivity ) {
   constexpr uint8 data1[] = { 0, 0, 0,
                               X, 1, X,
                               1, 1, 1 };
   constexpr uint8 data2[] = { X, 0, 0,
                               1, 1, 0,
                               X, 1, X };
   constexpr uint8 data3[] = { 0, 0, 0,
                               1, 1, 1,
                               X, 1, X };
   IntervalArray out;
   if( connectivity == 1 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in ).GenerateRotatedVersions( 45 );
   } else if( connectivity == 2 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in ).GenerateRotatedVersions( 90 );
      in = Image( data2, { 3, 3 } );
      IntervalArray more = Interval( in ).GenerateRotatedVersions( 90 );
      for( auto& ii : more ) {
         out.push_back( std::move( ii ));
      }
      in = Image( data3, { 3, 3 } );
      more = Interval( in ).GenerateRotatedVersions( 90 );
      for( auto& ii : more ) {
         out.push_back( std::move( ii ));
      }
   } else {
      DIP_THROW( E::CONNECTIVITY_NOT_SUPPORTED );
   }
   return out;
}

IntervalArray EndPixelInterval2D( dip::uint connectivity ) {
   constexpr uint8 data1[] = { X, 0, X,
                               0, 1, 0,
                               X, X, X };
   constexpr uint8 data2[] = { 0, 0, 0,
                               0, 1, 0,
                               0, X, X };
   IntervalArray out;
   if( connectivity == 1 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in ).GenerateRotatedVersions( 90 );
   } else if( connectivity == 2 ){
      Image in( data2, { 3, 3 } );
      out = Interval( in ).GenerateRotatedVersions( 45 );
   } else {
      DIP_THROW( E::CONNECTIVITY_NOT_SUPPORTED );
   }
   return out;
}

IntervalArray HomotopicEndPixelInterval2D( dip::uint connectivity ) {
   constexpr uint8 data1[] = { X, 0, X,
                               0, 1, 0,
                               X, 1, X };
   constexpr uint8 data2[] = { 0, 0, 0,
                               0, 1, 0,
                               X, 1, X };
   IntervalArray out;
   if( connectivity == 1 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in ).GenerateRotatedVersions( 90 );
   } else if( connectivity == 2 ){
      Image in( data2, { 3, 3 } );
      out = Interval( in ).GenerateRotatedVersions( 45 );
   } else {
      DIP_THROW( E::CONNECTIVITY_NOT_SUPPORTED );
   }
   return out;
}

Interval SinglePixelInterval( dip::uint nDims ) {
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   UnsignedArray sizes( nDims, 3 );
   Image in( sizes, 1, DT_UINT8 );
   in.Fill( 0 );
   sizes.fill( 1 ); // reuse `sizes` for the index to the center pixel
   in.At( sizes ) = 1;
   return Interval( in );
}

IntervalArray BranchPixelInterval2D() {
   constexpr uint8 data1[] = { 1, X, X,
                               X, 1, 1,
                               1, X, X };
   constexpr uint8 data2[] = { 1, X, X,
                               X, 1, X,
                               1, X, 1 };
   Image in( data1, { 3, 3 } );
   IntervalArray out = Interval( in ).GenerateRotatedVersions( 45 );
   in = Image( data2, { 3, 3 } );
   IntervalArray more = Interval( in ).GenerateRotatedVersions( 45 );
   for( auto& ii : more ) {
      out.push_back( std::move( ii ));
   }
   return out;
}

Interval BoundaryPixelInterval2D() {
   constexpr uint8 data[] = { X, X, X,
                              X, 1, 0,
                              X, X, X };
   Image in( data, { 3, 3 } );
   return Interval( in );
}

IntervalArray ConvexHullInterval2D() {
   constexpr uint8 data[] = { 1, 1, X,
                              1, 0, X,
                              1, X, X };
   Image in( data, { 3, 3 } );
   return Interval( in ).GenerateRotatedVersions(45);
}

}

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE("[DIPlib] testing private function RotateBy45Degrees") {
   dip::Image in( { 7, 7 }, 1, dip::DT_BIN );
   in = 0;
   in.At( 0, 1 ) = 1;
   in.At( 2, 1 ) = 1;
   in.At( 2, 2 ) = 1;
   in.At( 3, 3 ) = 1;
   dip::Image out = in;
   for( dip::uint ii = 0; ii < 8; ++ii ) {
      out = dip::RotateBy45Degrees( out ); // is in anonymous namespace, can be accessed like this in this file only.
   }
   DOCTEST_CHECK( dip::testing::CompareImages( in, out ));
}

#endif // DIP__ENABLE_DOCTEST
