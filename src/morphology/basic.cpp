/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the basic morphological operators.
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

#include "diplib.h"
#include "diplib/morphology.h"
#include "diplib/pixel_table.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/saturated_arithmetic.h"


namespace dip {


namespace {

// --- Rectangular morphology ---

template< typename TPI >
class RectangularMorphologyLineFilter : public Framework::SeparableLineFilter {
   public:
      RectangularMorphologyLineFilter( UnsignedArray const& sizes, bool dilation, bool mirror ) :
            sizes_( sizes ), dilation_( dilation ), mirror_( mirror ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
         forwardBuffers_.resize( threads );
         backwardBuffers_.resize( threads );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint procDim = params.dimension;
         dip::uint filterSize = sizes_[ procDim ];
         // Allocate buffer if it's not yet there. It's two buffers, but we allocate only once
         dip::uint margin = filterSize / 2;
         dip::uint bufferSize = length + 2 * margin;
         std::vector< TPI >& buffer = buffers_[ params.thread ];
         if( buffer.size() != 2 * bufferSize ) {
            buffer.resize( 2 * bufferSize );
            forwardBuffers_[ params.thread ] = buffer.data() + margin;
            backwardBuffers_[ params.thread ] = buffer.data() + bufferSize;
         }
         TPI* forwardBuffer = forwardBuffers_[ params.thread ];
         TPI* backwardBuffer = backwardBuffers_[ params.thread ];
         // Fill forward buffer
         in -= inStride * margin;
         TPI* buf = forwardBuffer - margin;
         while( buf < forwardBuffer + length + margin - filterSize ) {
            *buf = *in;
            in += inStride;
            buf++;
            for( dip::uint ii = 1; ii < filterSize; ++ii ) {
               *buf = dilation_ ? std::max( *in, *( buf - 1 )) : std::min( *in, *( buf - 1 ));
               in += inStride;
               buf++;
            }
         }
         dip::sint syncpos = buf - forwardBuffer; // this is needed to align the two buffers
         *buf = *in;
         in += inStride;
         buf++;
         while( buf < forwardBuffer + length + margin ) {
            *buf = dilation_ ? std::max( *in, *( buf - 1 )) : std::min( *in, *( buf - 1 ));
            in += inStride;
            buf++;
         }
         // Fill backward buffer
         in -= inStride; /* undo last increment */
         buf = backwardBuffer + length + margin - 1;
         *buf = *in;
         in -= inStride;
         buf--;
         while( buf >= backwardBuffer + syncpos ) {
            *buf = dilation_ ? std::max( *in, *( buf + 1 )) : std::min( *in, *( buf + 1 ));
            in -= inStride;
            buf--;
         }
         while( buf > backwardBuffer - margin ) {
            *buf = *in;
            in -= inStride;
            buf--;
            for( dip::uint ii = 1; ii < filterSize; ++ii ) {
               *buf = dilation_ ? std::max( *in, *( buf + 1 )) : std::min( *in, *( buf + 1 ));
               in -= inStride;
               buf--;
            }
         }
         // Fill output
         if( mirror_ ) {
            forwardBuffer += filterSize - 1 - margin;
            backwardBuffer -= margin;
         } else {
            forwardBuffer += margin;
            backwardBuffer -= ( filterSize - 1 - margin );
         }
         for( dip::uint ii = 0; ii < length; ++ii ) {
            *out = dilation_ ? std::max( *forwardBuffer, *backwardBuffer ) : std::min( *forwardBuffer, *backwardBuffer );
            out += outStride;
            forwardBuffer++;
            backwardBuffer++;
         }
      }
   private:
      UnsignedArray const& sizes_;
      std::vector< std::vector< TPI >> buffers_; // one for each thread
      std::vector< TPI* > forwardBuffers_; // points to buffers_
      std::vector< TPI* > backwardBuffers_; // points to buffers_
      bool dilation_;
      bool mirror_;
};

void RectangularMorphology(
      Image const& in,
      Image& out,
      FloatArray filterParam,
      BoundaryConditionArray const& bc,
      bool dilation, // true = dilation, false = erosion
      bool mirror // mirror the SE? -- this changes where the origin is placed in the even-sized rectangle
) {
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( filterParam, nDims, 1.0 );
   DIP_END_STACK_TRACE
   BooleanArray process( nDims, false );
   UnsignedArray sizes( nDims, 1 );
   UnsignedArray border( nDims, 0 );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( filterParam[ ii ] > 1.0 ) {
         sizes[ ii ] = static_cast< dip::uint >( std::round( filterParam[ ii ] ));
         process[ ii ] = true;
         border[ ii ] = sizes[ ii ] / 2;
      }
   }
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, RectangularMorphologyLineFilter, ( sizes, dilation, mirror ), dtype );
      Framework::Separable(
            in,
            out,
            dtype,
            dtype,
            process,
            border,
            bc,
            *lineFilter
      );
   DIP_END_STACK_TRACE
}

// --- Pixel table morphology ---

template< typename TPI >
class PixelTableMorphologyLineFilter : public Framework::FullLineFilter {
   public:
      PixelTableMorphologyLineFilter( bool dilation ) : dilation_( dilation ) {}
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         TPI max = 0; // The maximum value within the filter
         dip::sint index = -1; // Location of the maximum value w.r.t. the left edge
         for( dip::uint xx = 0; xx < length; ++xx ) {
            // Check whether maximum is in filter
            if( index >= 0 ) {
               // Maximum is in filter. Check to see if a larger value came in to the filter.
               for( auto const& run : pixelTable.Runs() ) {
                  dip::sint len = run.length - 1;
                  dip::sint position = run.offset + len * inStride;
                  TPI val = in[ position ];
                  if( max == val ) {
                     index = std::max( index, static_cast< dip::sint >( len ));
                  }
                  if( dilation_ ? val > max : val < max ) {
                     max = val;
                     index = len;
                  }
               }
            } else {
               // Maximum is no longer in the filter. Find maximum by looping over all pixels in the table.
               index = 0;
               max = dilation_ ? std::numeric_limits< TPI >::lowest() : std::numeric_limits< TPI >::max();
               for( PixelTableOffsets::iterator it = pixelTable.begin(); !it.IsAtEnd(); ++it ) {
                  TPI val = in[ *it ];
                  if( max == val ) {
                     index = std::max( index, static_cast< dip::sint >( it.Index() ));
                  }
                  if( dilation_ ? val >= max : val <= max ) {
                     max = val;
                     index = it.Index();
                  }
               }
            }
            *out = max;
            out += outStride;
            in += inStride;
            index--;
         }
      }
   private:
      bool dilation_;
};

void PixelTableMorphology(
      Image const& in,
      Image& out,
      PixelTable& pixelTable,
      BoundaryConditionArray const& bc,
      bool dilation, // true = dilation, false = erosion
      bool mirror // mirror the SE? -- this changes where the origin is placed in the even-sized bounding box
) {
   if( mirror ) {
      auto sizes = pixelTable.Sizes();
      IntegerArray offset( sizes.size(), 0 );
      for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
         if( !( sizes[ ii ] & 1 )) {
            offset[ ii ] = -1;
         }
      }
      pixelTable.ShiftOrigin( offset );
   }
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, PixelTableMorphologyLineFilter, ( dilation ), dtype );
      Framework::Full(
            in,
            out,
            dtype,
            dtype,
            dtype,
            1,
            bc,
            pixelTable,
            *lineFilter
      );
   DIP_END_STACK_TRACE
}

// --- Grey-value pixel table morphology ---

template< typename TPI >
class GreyValueSEMorphologyLineFilter : public Framework::FullLineFilter {
   public:
      GreyValueSEMorphologyLineFilter( bool dilation ) : dilation_( dilation ) {}
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         std::vector< dfloat > const& weights = pixelTable.Weights();
         if( dilation_ ) {
            for( dip::uint xx = 0; xx < length; xx++ ) {
               TPI max = dilation_ ? std::numeric_limits< TPI >::lowest() : std::numeric_limits< TPI >::max();
               auto ito = pixelTable.begin();
               auto itw = weights.begin();
               for( dip::uint ii = 0; ii < pixelTable.NumberOfPixels(); ++ii ) {
                  max = std::max( max, clamp_cast< TPI >( static_cast< dfloat >( in[ *ito ] ) + *itw ));
               }
               *out = max;
               in += inStride;
               out += outStride;
            }
         } else {
            for( dip::uint xx = 0; xx < length; xx++ ) {
               TPI min = dilation_ ? std::numeric_limits< TPI >::lowest() : std::numeric_limits< TPI >::max();
               auto ito = pixelTable.begin();
               auto itw = weights.begin();
               for( dip::uint ii = 0; ii < pixelTable.NumberOfPixels(); ++ii ) {
                  min = std::min( min, clamp_cast< TPI >( static_cast< dfloat >( in[ *ito ] ) - *itw ));
               }
               *out = min;
               in += inStride;
               out += outStride;
            }
         }
      }
   private:
      bool dilation_;
};

void GreyValueSEMorphology(
      Image const& in,
      Image& out,
      PixelTable& pixelTable,
      BoundaryConditionArray const& bc,
      bool dilation, // true = dilation, false = erosion
      bool mirror // mirror the SE? -- this changes where the origin is placed in the even-sized bounding box
) {
   DIP_ASSERT( pixelTable.HasWeights() );
   if( mirror ) {
      auto sizes = pixelTable.Sizes();
      IntegerArray offset( sizes.size(), 0 );
      for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
         if( !( sizes[ ii ] & 1 )) {
            offset[ ii ] = -1;
         }
      }
      pixelTable.ShiftOrigin( offset );
   }
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( dilation ), dtype );
      Framework::Full(
            in,
            out,
            dtype,
            dtype,
            dtype,
            1,
            bc,
            pixelTable,
            *lineFilter
      );
   DIP_END_STACK_TRACE
}

// --- Parabolic morphology ---

template< typename TPI >
class ParabolicMorphologyLineFilter : public Framework::SeparableLineFilter {
   public:
      ParabolicMorphologyLineFilter( FloatArray const& params, bool dilation ) :
            params_( params ), dilation_( dilation ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint procDim = params.dimension;
         dfloat lambda = 1.0 / ( params_[ procDim ] * params_[ procDim ] );
         // Allocate buffer if it's not yet there.
         if( buffers_[ params.thread ].size() != length ) {
            buffers_[ params.thread ].resize( length );
         }
         TPI* buf = buffers_[ params.thread ].data();
         *buf = *in;
         in += inStride;
         ++buf;
         dip::sint index = 0;
         if( dilation_ ) {
            // Start with processing the line from left to right
            for( dip::uint ii = 1; ii < length; ++ii ) {
               --index;
               if( *in >= *( buf - 1 )) {
                  *buf = *in;
                  index = 0;
               } else {
                  dfloat max = std::numeric_limits< dfloat >::lowest();
                  for( dip::sint jj = index; jj <= 0; ++jj ) {
                     dfloat val = in[ jj * inStride ] - lambda * jj * jj;
                     if( val >= max ) {
                        max = val;
                        index = jj;
                     }
                  }
                  *buf = max;
               }
               in += inStride;
               ++buf;
            }
            // Now process the line from right to left
            out += ( length - 1 ) * outStride;
            --buf;
            *out = *buf;
            out -= outStride;
            --buf;
            index = 0;
            for( dip::uint ii = 1; ii < length; ++ii ) {
               ++index;
               if( *buf >= *( out + outStride )) {
                  *out = *buf;
                  index = 0;
               } else {
                  dfloat max = std::numeric_limits< dfloat >::lowest();
                  for(dip::sint jj = index; jj >= 0; --jj ) {
                     dfloat val = buf[ jj ] - lambda * jj * jj;
                     if( val >= max ) {
                        max = val;
                        index = jj;
                     }
                  }
                  *out = max;
               }
               out -= outStride;
               --buf;
            }
         } else {
            // Start with processing the line from left to right
            for( dip::uint ii = 1; ii < length; ++ii ) {
               --index;
               if( *in <= *( buf - 1 )) {
                  *buf = *in;
                  index = 0;
               } else {
                  dfloat min = std::numeric_limits< dfloat >::max();
                  for( dip::sint jj = index; jj <= 0; ++jj ) {
                     dfloat val = in[ jj * inStride ] + lambda * jj * jj;
                     if( val <= min ) {
                        min = val;
                        index = jj;
                     }
                  }
                  *buf = min;
               }
               in += inStride;
               ++buf;
            }
            // Now process the line from right to left
            out += ( length - 2 ) * outStride;
            buf -= 2;
            index = 0;
            for( dip::uint ii = 1; ii < length; ++ii ) {
               ++index;
               if( *buf <= *( out + outStride )) {
                  *out = *buf;
                  index = 0;
               } else {
                  dfloat min = std::numeric_limits< dfloat >::max();
                  for(dip::sint jj = index; jj >= 0; --jj ) {
                     dfloat val = buf[ jj ] + lambda * jj * jj;
                     if( val <= min ) {
                        min = val;
                        index = jj;
                     }
                  }
                  *out = min;
               }
               out -= outStride;
               --buf;
            }
         }
      }
   private:
      FloatArray const& params_;
      std::vector< std::vector< TPI >> buffers_; // one for each thread
      bool dilation_;
};

void ParabolicMorphology(
      Image const& in,
      Image& out,
      FloatArray filterParam,
      BoundaryConditionArray const& bc, // will not be used, as border==0.
      bool dilation // true = dilation, false = erosion
) {
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( filterParam, nDims, 0.0 );
   DIP_END_STACK_TRACE
   BooleanArray process( nDims, false );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( filterParam[ ii ] > 0.0 ) {
         process[ ii ] = true;
      }
   }
   DIP_START_STACK_TRACE
      DataType dtype = DataType::SuggestFlex( in.DataType() ); // Returns either float or complex. If complex, DIP_OVL_NEW_FLOAT will throw.
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, dilation ), dtype );
      Framework::Separable(
            in,
            out,
            dtype,
            dtype,
            process,
            { 0 },
            bc,
            *lineFilter
      );
   DIP_END_STACK_TRACE
}

// --- Line morphology ---

void LineMorphology(
      Image const& in,
      Image& out,
      FloatArray filterParam,
      BoundaryConditionArray const& bc,
      bool interpolated, // true = "interpolated line", false = "discrete line"
      bool dilation // true = dilation, false = erosion
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

// --- Dispatch ---

void BasicMorphology(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      String const& filterShape,
      StringArray const& boundaryCondition,
      bool dilation, // true = dilation, false = erosion
      bool mirror = false // mirror the SE?
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
   BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
   if( bc.empty() ) {
      bc.resize( 1 );
      bc[ 0 ] = dilation ? BoundaryCondition::ADD_MIN_VALUE : BoundaryCondition::ADD_MAX_VALUE;
   }
   if( filterShape == "rectangular" ) {
         RectangularMorphology(in, out, filterParam, bc, dilation, mirror );
   } else if( filterShape == "parabolic" ) {
         ParabolicMorphology(in, out, filterParam, bc, dilation );
   } else if( filterShape == "interpolated line" ) {
         LineMorphology(in, out, filterParam, bc, true, dilation );
   } else if( filterShape == "discrete line" ) {
         LineMorphology(in, out, filterParam, bc, false, dilation );
   } else {
      dip::uint procDim = Framework::OptimalProcessingDim( in, UnsignedArray{ filterParam } );
      PixelTable pixelTable( filterShape, filterParam, procDim );
      PixelTableMorphology( in, out, pixelTable, bc, dilation, mirror );
   }
   DIP_END_STACK_TRACE
}

void BasicMorphology(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition,
      bool dilation, // true = dilation, false = erosion
      bool mirror = false // mirror the SE?
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !se.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !se.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
   DIP_THROW_IF( se.Dimensionality() != in.Dimensionality(), E::DIMENSIONALITIES_DONT_MATCH );
   BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
   dip::uint procDim = Framework::OptimalProcessingDim( in, se.Sizes() );
   Image se_ = se.QuickCopy();
   if( mirror ) {
      se_.Mirror();
   }
   if( se.DataType().IsBinary() ) {
         PixelTable pixelTable( se, {}, procDim );
         PixelTableMorphology( in, out, pixelTable, bc, dilation, mirror );
   } else {
         // TODO: make a pixel table of IsFinite(se).
         PixelTable pixelTable( se > -1e15, {}, procDim );
         pixelTable.AddWeights( se );
         GreyValueSEMorphology( in, out, pixelTable, bc, dilation, mirror );
   }
   DIP_END_STACK_TRACE
}

} // namespace

// --- Public functions ---

void Dilation(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      String const& filterShape,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, filterParam, filterShape, boundaryCondition, true );
   DIP_END_STACK_TRACE
}

void Dilation(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, se, out, boundaryCondition, true );
   DIP_END_STACK_TRACE
}

void Erosion(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      String const& filterShape,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, filterParam, filterShape, boundaryCondition, false );
   DIP_END_STACK_TRACE
}

void Erosion(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, se, out, boundaryCondition, false );
   DIP_END_STACK_TRACE
}

// TODO: For opening and closing, extend the input image, then apply the two operations without boundary extension.

void Opening(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      String const& filterShape,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, filterParam, filterShape, boundaryCondition, false, false );
      BasicMorphology( out, out, filterParam, filterShape, boundaryCondition, true, true );
   DIP_END_STACK_TRACE
}

void Opening(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, se, out, boundaryCondition, false, false );
      BasicMorphology( out, se, out, boundaryCondition, true, true );
   DIP_END_STACK_TRACE
}

void Closing(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      String const& filterShape,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, filterParam, filterShape, boundaryCondition, true, false );
      BasicMorphology( out, out, filterParam, filterShape, boundaryCondition, false, true );
   DIP_END_STACK_TRACE
}

void Closing(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, se, out, boundaryCondition, true, false );
      BasicMorphology( out, se, out, boundaryCondition, false, true );
   DIP_END_STACK_TRACE
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/math.h"

DOCTEST_TEST_CASE("[DIPlib] testing the basic morphological filters") {
   dip::Image in( { 256, 69 }, 1, dip::DT_UINT8 );
   in = 0;
   in.At( 127, 35 ) = 100;
   dip::Image out;

   // Rectangular morphology
   dip::BasicMorphology( in, out, { 10, 1 }, "rectangular", {}, true /*compute dilation*/ );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   dip::BasicMorphology( in, out, { 11, 1 }, "rectangular", {}, true /*compute dilation*/ );
   DOCTEST_CHECK( dip::Count( out ) == 11 );
   dip::BasicMorphology( in, out, { 10, 11 }, "rectangular", {}, true /*compute dilation*/ );
   DOCTEST_CHECK( dip::Count( out ) == 10*11 );
   dip::BasicMorphology( out, out, { 10, 11 }, "rectangular", {}, false /*compute erosion*/, true );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK(( dip::sint )out.At( 127, 35 ) == 100 ); // Is that pixel in the right place?

   // PixelTable morphology
   dip::BasicMorphology( in, out, { 1, 10 }, "elliptic", {}, true /*compute dilation*/ );
   DOCTEST_CHECK( dip::Count( out ) == 11 ); // rounded!
   dip::BasicMorphology( in, out, { 1, 11 }, "elliptic", {}, true /*compute dilation*/ );
   DOCTEST_CHECK( dip::Count( out ) == 11 );
   dip::BasicMorphology( in, out, { 10, 11 }, "elliptic", {}, true /*compute dilation*/ );
   DOCTEST_CHECK( dip::Count( out ) == 89 );
   dip::BasicMorphology( out, out, { 10, 11 }, "elliptic", {}, false /*compute erosion*/, true );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK(( dip::sint )out.At( 127, 35 ) == 100 ); // Is that pixel in the right place?

   // PixelTable morphology -- mirroring
   dip::Image se( { 10, 10 }, 1, dip::DT_BIN );
   se = 1;
   dip::BasicMorphology( in, se, out, {}, true /*compute dilation*/ );
   DOCTEST_CHECK( dip::Count( out ) == 100 );
   dip::BasicMorphology( out, se, out, {}, false /*compute erosion*/, true );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK(( dip::sint )out.At( 127, 35 ) == 100 ); // Is that pixel in the right place?

   // Parabolic morphology
   dip::BasicMorphology( in, out, { 10.0, 0.0 }, "parabolic", {}, true /*compute dilation*/ );
   dip::dfloat result = 0.0;
   for( dip::uint ii = 1; ii < 100; ++ii ) {
      result += 100.0 - ii * ii / 100.0;
   }
   result = 100.0 + result * 2.0;
   DOCTEST_CHECK(( dip::dfloat )dip::Sum( out ) == doctest::Approx( result ) );
   DOCTEST_CHECK(( dip::dfloat )out.At( 127, 35 ) == 100.0 ); // Is the origin in the right place?

   dip::BasicMorphology( out, out, { 10.0, 0.0 }, "parabolic", {}, false /*compute erosion*/ );
   result = 0.0;
   for( dip::uint ii = 1; ii < 100; ++ii ) {
      result += ii * ii / 100.0;
   }
   result = 100.0 + result * 2.0;
   DOCTEST_CHECK(( dip::dfloat )dip::Sum( out ) == doctest::Approx( result ) );
   DOCTEST_CHECK(( dip::dfloat )out.At( 127, 35 ) == 100.0 ); // Is the origin in the right place?

   // Grey-value SE morphology
   se.ReForge( { 5, 6 }, 1, dip::DT_SFLOAT );
   se = std::numeric_limits< dip::dfloat >::infinity();
   se.At( 0, 0 ) = 0;
   se.At( 4, 5 ) = 5;
   se.At( 0, 5 ) = 5;
   se.At( 4, 0 ) = -5;
   dip::BasicMorphology( in, se, out, {}, true /*compute dilation*/ );
   DOCTEST_CHECK(( dip::dfloat )dip::Sum( out ) == 100 + 105 + 105 + 95 );
   dip::BasicMorphology( out, se, out, {}, false /*compute erosion*/, true );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK(( dip::sint )out.At( 127, 35 ) == 100 ); // Is that pixel in the right place?
}

#endif // DIP__ENABLE_DOCTEST
