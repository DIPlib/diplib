/*
 * (c)2018-2022, Cris Luengo.
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

#include "diplib/binary.h"

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/math.h"
#include "diplib/morphology.h"
#include "diplib/pixel_table.h"
#include "diplib/statistics.h"

namespace dip {

namespace {
// The "don't care" pixels.
constexpr sfloat X = std::numeric_limits< sfloat >::quiet_NaN(); // dip::nan is a dfloat...

constexpr char const* INTERVAL_NOT_ODD = "The interval is not odd in size";
constexpr char const* INTERVAL_NO_FOREGROUND = "The interval needs at least one foreground pixel";
}

Interval::Interval( dip::Image image ) : image_( std::move( image )) {
   DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !image_.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( image_.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   for( auto& s : image_.Sizes() ) {
      DIP_THROW_IF( !( s & 1u ), INTERVAL_NOT_ODD );
   }
   DIP_THROW_IF( !Any( image_ == 1 ).As< bool >(), INTERVAL_NO_FOREGROUND );
   image_.Convert( DT_SFLOAT );
   ImageIterator< sfloat > it( image_ );
   do {
      if(( *it != 0.0 ) && ( *it != 1.0 )) {
         *it = X;
      }
   } while( ++it );
}

Interval::Interval( dip::Image hit, dip::Image miss ) {
   DIP_THROW_IF( !hit.IsForged() || !miss.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !hit.IsScalar() || !miss.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !hit.DataType().IsBinary() || !miss.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( hit.Sizes() != miss.Sizes(), E::SIZES_DONT_MATCH );
   for( auto s : hit.Sizes() ) {
      DIP_THROW_IF( !( s & 1u ), INTERVAL_NOT_ODD );
   }
   DIP_THROW_IF( !Any( hit ).As< bool >(), INTERVAL_NO_FOREGROUND );
   DIP_THROW_IF( dip::Any( dip::Infimum( hit, miss )).As< bool >(), "The hit and miss images are not disjoint" );
   image_.ReForge( hit.Sizes(), 1, DT_SFLOAT );
   image_.Fill( X );
   image_.At( std::move( hit )) = 1;
   image_.At( std::move( miss )) = 0; // I'm sure this can be done more efficiently be iterating only once, but we're talking about very small images here,
}

void Interval::Invert() {
   ImageIterator< sfloat > it( image_ );
   do {
      if( *it == 0.0 ) {
         *it = 1.0;
      } else if( *it == 1.0 ) {
         *it = 0.0;
      }
   } while( ++it );
}

void Invert( IntervalArray& array ) {
   // Walk over the array, finding one of each set of intervals with shared data
   BooleanArray shared( array.size(), false );
   for( dip::uint ii = 1; ii < array.size(); ++ii ) {
      for( dip::uint jj = 0; jj < ii; ++jj ) {
         if( array[ ii ].Image().SharesData( array[ jj ].Image() )) {
            shared[ ii ] = true;
            break;
         }
      }
   }
   for( dip::uint ii = 0; ii < array.size(); ++ii ) {
      if( !shared[ ii ] ) {
         array[ ii ].Invert();
      }
   }
}

namespace {

// Rotates 2D sfloat image clockwise
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
   DIP_ASSERT( output.DataType() == DT_SFLOAT );
   dip::sint stepX = output.Stride( 0 );
   dip::sint stepY = output.Stride( 1 );
   // Rotate by shells. Each shell (pixels with the same L_inf distance) rotates independently.
   dip::sfloat* ptr = static_cast< dip::sfloat* >( output.Origin() );
   for( dip::uint shell = 0; shell < len / 2; ++shell ) {
      dip::sint n = static_cast< dip::sint >( len / 2 - shell ); // number of pixels in half an edge
      dip::sint nStepX = n * stepX;
      dip::sint nStepY = n * stepY;
      dip::sint nEndX = 2 * nStepX;
      dip::sint nEndY = 2 * nStepY;
      for( dip::sint ii = 0; ii < n; ++ ii ) {
         dip::sint iX = ii * stepX;
         dip::sint iY = ii * stepY;
         dip::sfloat* ptr1 = ptr + iX;
         dip::sfloat* ptr2 = ptr + nStepY - iY;
         dip::sfloat value = *ptr1;
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

} // namespace

IntervalArray Interval::GenerateRotatedVersions(
      dip::uint rotationAngle,
      String const& rotationDirection
) const {
   DIP_THROW_IF( image_.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint step = 1;
   if( rotationAngle == 45 ) {
      // default step is OK
   } else if( rotationAngle == 90 ) {
      step = 2;
   } else if( rotationAngle == 180 ) {
      step = 4;
   } else {
      DIP_THROW_INVALID_FLAG( std::to_string( rotationAngle ));
   }
   bool interleaved = true;
   bool clockwise = true;
   if( rotationDirection == S::INTERLEAVED_CLOCKWISE ) {
      // defaults are OK
   } else if( rotationDirection == S::INTERLEAVED_COUNTERCLOCKWISE ) {
      clockwise = false;
   } else if( rotationDirection == S::CLOCKWISE ) {
      interleaved = false;
   } else if( rotationDirection == S::COUNTERCLOCKWISE ) {
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
      output[ cur ].image_ = RotateBy45Degrees( image_ );
      for( dip::uint ii = 0; ii < 3; ++ii ) {
         dip::uint next = clockwise ? cur + 2 : cur - 2;
         output[ next ] = output[ cur ];
         output[ next ].image_.Rotation90( 1 );
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
         output[ next ].image_.Rotation90( 1 );
         cur = next;
      }
   } else {
      // 180
      output[ 1 ] = output[ 0 ];
      output[ 1 ].image_.Rotation90( 2 );
   }
   // not interleaved: 0, 45, 90, 135, 180, 225, 270, 315.
   // interleaved:     0, 180, 45, 225, 90, 270, 135, 315.
   if( interleaved ) {
      if( step == 1 ) {
         // not interleaved: 0, 45, 90, 135, 180, 225, 270, 315.
         // swap 1, 2 & 4:   0, 180, 45, 135, 90, 225, 270, 315.
         // swap 3, 5 & 6:   0, 180, 45, 225, 90, 270, 135, 315.
         Interval tmp = std::move( output[ 1 ] );
         output[ 1 ] = std::move( output[ 4 ] );
         output[ 4 ] = std::move( output[ 2 ] );
         output[ 2 ] = std::move( tmp );
         tmp = std::move( output[ 3 ] );
         output[ 3 ] = std::move( output[ 5 ] );
         output[ 5 ] = std::move( output[ 6 ] );
         output[ 6 ] = std::move( tmp );
      } else if( step == 2 ) {
         // not interleaved: 0, 90, 180, 270.
         // swap 1 & 2:      0, 180, 90, 270.
         std::swap( output[ 1 ], output[ 2 ] );
      }
   }
   return output;
}

namespace {

// Copies `in` to `out`, unless `&in==&out` and the input border is already expanded.
// `out` will be the same as `in`, but it'll be possible to access pixel data outside of the bounds,
// however far is needed to accommodate `intervals`.
// `in` is supposed to be binary. No checks are made.
// This function also checks that all elements in `intervals` have the same dimensionality.
void ExpandInputImage(
      dip::Image const& in,
      dip::Image& out,
      IntervalArray const& intervals,
      String const& boundaryCondition
) {
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
   border /= 2;
   // Prepare the output image, adding a border if it's not already there
   if( boundaryCondition != S::ALREADY_EXPANDED ) {
      StringArray bc;
      if( !boundaryCondition.empty() ) {
         bc.push_back( boundaryCondition );
      }
      if( !out.DataType().IsBinary() ) {
         DIP_STACK_TRACE_THIS( out.Strip() ); // Make sure we don't get a data type conversion here
      }
      DIP_STACK_TRACE_THIS( ExtendImage( in, out, border, bc, { "masked" } ));
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
         largerIn.ShiftOriginUnsafe( largerIn.Offset( origin ));
         largerIn.SetSizesUnsafe( std::move( sizes ));
         // Copy
         if( !out.DataType().IsBinary() ) {
            DIP_STACK_TRACE_THIS( out.Strip()); // Make sure we don't get a data type conversion here
         }
         DIP_STACK_TRACE_THIS( out.Copy( largerIn ));
         // Out must be a view on this
         out.ShiftOriginUnsafe( out.Offset( border ));
         out.SetSizesUnsafe( in.Sizes() );
      }
   }
}

enum class PolarityMode : uint8 {
   SupGenerating,
   InfGenerating
};

class SupInfGeneratingLineFilter : public Framework::FullLineFilter {
   public:
      SupInfGeneratingLineFilter( PolarityMode mode )
            : supGenerating_( mode != PolarityMode::InfGenerating ) {}
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /**/, dip::uint nKernelPixels, dip::uint /**/ ) override {
         return lineLength * nKernelPixels;
      }
      void SetNumberOfThreads( dip::uint /**/, PixelTableOffsets const& pixelTable ) override {
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
      void Filter( Framework::FullLineFilterParameters const& params ) override {
         dip::bin* in = static_cast< dip::bin* >( params.inBuffer.buffer );
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
            // For inf-generating operator we do the same as above, but we invert the input and invert the output.
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
      bool const supGenerating_;
      std::vector< dip::sint > offsets_;
      std::vector< dip::bin > hitmiss_;
};

void SupInfGenerating(
      Image const& in,
      Image& out,
      Interval const& interval,
      BoundaryCondition boundaryCondition,
      PolarityMode mode
) {
   DIP_START_STACK_TRACE
      SupInfGeneratingLineFilter lineFilter( mode );
      Framework::Full( in, out, DT_BIN, DT_BIN, DT_BIN, 1, { boundaryCondition }, interval.Image(), lineFilter );
   DIP_END_STACK_TRACE
}

} // namespace

void SupGenerating(
      Image const& in,
      Image& out,
      Interval const& interval,
      String const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_STACK_TRACE_THIS( SupInfGenerating( in, out, interval, StringToBoundaryCondition( boundaryCondition ), PolarityMode::SupGenerating ));
}

void InfGenerating(
      Image const& in,
      Image& out,
      Interval const& interval,
      String const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_STACK_TRACE_THIS( SupInfGenerating( in, out, interval, StringToBoundaryCondition( boundaryCondition ), PolarityMode::InfGenerating ));
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
   Image in;
   DIP_STACK_TRACE_THIS( ExpandInputImage( c_in, in, intervals, boundaryCondition ));
   DIP_STACK_TRACE_THIS( SupInfGenerating( in, out, intervals[ 0 ], BoundaryCondition::ALREADY_EXPANDED, PolarityMode::SupGenerating ));
   Image tmp;
   for( dip::uint ii = 1; ii < intervals.size(); ++ii ) {
      DIP_STACK_TRACE_THIS( SupInfGenerating( in, tmp, intervals[ ii ], BoundaryCondition::ALREADY_EXPANDED, PolarityMode::SupGenerating ));
      DIP_STACK_TRACE_THIS( Supremum( out, tmp, out ));
   }
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
   Image in;
   DIP_STACK_TRACE_THIS( ExpandInputImage( c_in, in, intervals, boundaryCondition ));
   DIP_STACK_TRACE_THIS( SupInfGenerating( in, out, intervals[ 0 ], BoundaryCondition::ALREADY_EXPANDED, PolarityMode::InfGenerating ));
   Image tmp;
   for( dip::uint ii = 1; ii < intervals.size(); ++ii ) {
      DIP_STACK_TRACE_THIS( SupInfGenerating( in, tmp, intervals[ ii ], BoundaryCondition::ALREADY_EXPANDED, PolarityMode::InfGenerating ));
      DIP_STACK_TRACE_THIS( Infimum( out, tmp, out ));
   }
}

namespace {

enum class DirectionMode : uint8 {
   Thickening,
   Thinning
};

class ThickeningThinningLineFilter : public Framework::FullLineFilter {
   public:
      ThickeningThinningLineFilter( DirectionMode mode, Image const& mask, bool& change )
            : mode_( mode ), changed_( change ), mask_( mask ) {}
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /**/, dip::uint nKernelPixels, dip::uint /**/ ) override {
         return lineLength * nKernelPixels;
      }
      void SetNumberOfThreads( dip::uint /**/, PixelTableOffsets const& pixelTable ) override {
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
      void Filter( Framework::FullLineFilterParameters const& params ) override {
         dip::bin* in = static_cast< dip::bin* >( params.inBuffer.buffer ); // yes, it's dip::bin, but this is simpler.
         dip::sint inStride = params.inBuffer.stride;
         dip::bin* out = static_cast< dip::bin* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         dip::bin const* mask = nullptr;
         dip::sint maskStride = 0;
         bool changed = false;
         if( mask_.IsForged() ) {
            mask = static_cast< dip::bin* >( mask_.Pointer( params.position ));
            maskStride = mask_.Stride( params.dimension );
         }
         for( dip::uint ii = 0; ii < length; ++ii ) {
            dip::bin result = false;
            if(( !mask || *mask ) && ( mode_ == DirectionMode::Thickening ? !*in : *in )) {
               result = true;
               for( dip::uint jj = 0; jj < offsets_.size(); ++jj ) {
                  if( in[ offsets_[ jj ]] ^ hitmiss_[ jj ] ) { // Note that this requires our dip::bin image to be correct (i.e. 0 or 1, not other numbers).
                     result = false;
                     break;
                  }
               }
               changed |= result;
            }
            if( mode_ == DirectionMode::Thickening ) {
               // thickening: out = in + result
               *out = *in || result;
            } else {
               // thinning:   out = in - result
               *out = *in && !result;

            }
            out += outStride;
            in += inStride;
            mask += maskStride;
         }
         if( changed ) {
            // TODO: This is more efficient with `#pragma omp atomic write`. But MSVC support that only since 17.4 (2022).
            //       And we'd also need CMake 3.30 or later to enable the MSVC option that enables MSVC to support it.
            //       So for the time being we keep using this, but should update at some point in the future.
            #pragma omp critical( SupInfGeneratingLineFilter )
            changed_ = true;
         }
      }
   private:
      DirectionMode const mode_;
      bool& changed_; // shared among threads, write atomically!
      Image const& mask_;
      std::vector< dip::sint > offsets_;
      std::vector< dip::bin > hitmiss_;
};

void ThickeningThinning(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations,
      String const& boundaryCondition,
      DirectionMode mode // must be either Thickening or Thinning
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( intervals.empty(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( in.Sizes() ));
   }
   Image tmp1;
   Image tmp2;
   DIP_STACK_TRACE_THIS( ExpandInputImage( in, tmp1, intervals, boundaryCondition ));
   DIP_STACK_TRACE_THIS( ExpandInputImage( in, tmp2, intervals, boundaryCondition ));
   bool untilConvergence = iterations == 0;
   bool change = false;
   ThickeningThinningLineFilter lineFilter( mode, mask, change );
   while( true ) {
      change = false;
      for( auto const& inter : intervals ) {
         DIP_STACK_TRACE_THIS( Framework::Full( tmp1, tmp2, DT_BIN, DT_BIN, DT_BIN, 1, { BoundaryCondition::ALREADY_EXPANDED }, inter.Image(), lineFilter ));
         tmp1.swap( tmp2 ); // tmp1 is always the result
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
   out.Copy( tmp1 );
}

} // namespace

void Thickening(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations,
      String const& boundaryCondition
) {
   DIP_STACK_TRACE_THIS( ThickeningThinning( in, mask, out, intervals, iterations, boundaryCondition, DirectionMode::Thickening ));
}

void Thinning(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations,
      String const& boundaryCondition
) {
   DIP_STACK_TRACE_THIS( ThickeningThinning( in, mask, out, intervals, iterations, boundaryCondition, DirectionMode::Thinning ));
}

IntervalArray HomotopicThinningInterval2D( dip::uint connectivity ) {
   constexpr sfloat data1[] = { 0, 0, 0,
                                X, 1, X,
                                1, 1, 1 };
   constexpr sfloat data2[] = { X, 0, 0,
                                1, 1, 0,
                                X, 1, X };
   constexpr sfloat data3[] = { 0, 0, 0,
                                1, 1, 1,
                                X, 1, X };
   IntervalArray out;
   if( connectivity == 1 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in.Copy() ).GenerateRotatedVersions( 45 );
   } else if( connectivity == 2 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in.Copy() ).GenerateRotatedVersions( 90 );
      in = Image( data2, { 3, 3 } );
      IntervalArray more = Interval( in.Copy() ).GenerateRotatedVersions( 90 );
      for( auto& ii : more ) {
         out.push_back( std::move( ii ));
      }
      in = Image( data3, { 3, 3 } );
      more = Interval( in.Copy() ).GenerateRotatedVersions( 90 );
      for( auto& ii : more ) {
         out.push_back( std::move( ii ));
      }
   } else {
      DIP_THROW( E::CONNECTIVITY_NOT_SUPPORTED );
   }
   return out;
}

IntervalArray EndPixelInterval2D( dip::uint connectivity ) {
   constexpr sfloat data1[] = { X, 0, X,
                                0, 1, 0,
                                X, X, X };
   constexpr sfloat data2[] = { 0, 0, 0,
                                0, 1, 0,
                                0, X, X };
   IntervalArray out;
   if( connectivity == 1 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in.Copy() ).GenerateRotatedVersions( 90 );
   } else if( connectivity == 2 ){
      Image in( data2, { 3, 3 } );
      out = Interval( in.Copy() ).GenerateRotatedVersions( 45 );
   } else {
      DIP_THROW( E::CONNECTIVITY_NOT_SUPPORTED );
   }
   return out;
}

IntervalArray HomotopicEndPixelInterval2D( dip::uint connectivity ) {
   constexpr sfloat data1[] = { X, 0, X,
                                0, 1, 0,
                                X, 1, X };
   constexpr sfloat data2[] = { 0, 0, 0,
                                0, 1, 0,
                                X, 1, X };
   IntervalArray out;
   if( connectivity == 1 ) {
      Image in( data1, { 3, 3 } );
      out = Interval( in.Copy() ).GenerateRotatedVersions( 90 );
   } else if( connectivity == 2 ){
      Image in( data2, { 3, 3 } );
      out = Interval( in.Copy() ).GenerateRotatedVersions( 45 );
   } else {
      DIP_THROW( E::CONNECTIVITY_NOT_SUPPORTED );
   }
   return out;
}

Interval SinglePixelInterval( dip::uint nDims ) {
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   UnsignedArray sizes( nDims, 3 );
   Image in( sizes, 1, DT_SFLOAT );
   in.Fill( 0 );
   sizes.fill( 1 ); // reuse `sizes` for the index to the center pixel
   in.At( sizes ) = 1;
   return { in };
}

IntervalArray BranchPixelInterval2D() {
   constexpr sfloat data1[] = { 1, X, X,
                                X, 1, 1,
                                1, X, X };
   constexpr sfloat data2[] = { 1, X, X,
                                X, 1, X,
                                1, X, 1 };
   Image in( data1, { 3, 3 } );
   IntervalArray out = Interval( in.Copy() ).GenerateRotatedVersions( 45 );
   in = Image( data2, { 3, 3 } );
   IntervalArray more = Interval( in.Copy() ).GenerateRotatedVersions( 45 );
   for( auto& ii : more ) {
      out.push_back( std::move( ii ));
   }
   return out;
}

Interval BoundaryPixelInterval2D() {
   constexpr sfloat data[] = { X, X, X,
                               X, 1, 0,
                               X, X, X };
   Image in( data, { 3, 3 } );
   return { in.Copy() };
}

IntervalArray ConvexHullInterval2D() {
   constexpr sfloat data[] = { 1, 1, X,
                               1, 0, X,
                               1, X, X };
   Image in( data, { 3, 3 } );
   return Interval( in.Copy() ).GenerateRotatedVersions( 45 );
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE( "[DIPlib] testing private function RotateBy45Degrees" ) {
   dip::Image in( { 7, 7 }, 1, dip::DT_SFLOAT );
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

#endif // DIP_CONFIG_ENABLE_DOCTEST
