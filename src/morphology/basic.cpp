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
#include "diplib/kernel.h"
#include "diplib/framework.h"
#include "diplib/overload.h"


namespace dip {

// This function defined here, not in the header, to avoid pulling in kernel.h and its dependencies there.
dip::Kernel StructuringElement::Kernel() const {
   dip::Kernel out;
   switch( shape_ ) {
      case ShapeCode::RECTANGULAR:
         out = { Kernel::ShapeCode::RECTANGULAR, params_ };
         break;
      case ShapeCode::ELLIPTIC:
         out = { Kernel::ShapeCode::ELLIPTIC, params_ };
         break;
      case ShapeCode::DIAMOND:
         out = { Kernel::ShapeCode::DIAMOND, params_ };
         break;
      case ShapeCode::DISCRETE_LINE:
         out = { Kernel::ShapeCode::LINE, params_ };
         break;
      case ShapeCode::CUSTOM:
         out = { image_ };
         break;
      default:
         DIP_THROW( "Cannot create kernel for this structuring element shape" );
   }
   if( mirror_ ) {
      out.Mirror();
   }
   return out;
}

namespace {

enum class Polarity {
      DILATION,
      EROSION
};
enum class Mirror {
      NO,
      YES
};

bool IsIsotropic( FloatArray params ) {
   dfloat param = 0.0;
   for( dfloat p : params ) {
      p = std::abs( p );
      if( p > 1.0 ) {
         if( param == 0.0 ) {
            param = p;
         } else if( p != param ) {
            return false;
         }
      }
   }
   return true;
}

// --- Rectangular morphology ---

template< typename TPI >
class RectangularMorphologyLineFilter : public Framework::SeparableLineFilter {
   public:
      RectangularMorphologyLineFilter( UnsignedArray const& sizes, Polarity polarity, Mirror mirror ) :
            sizes_( sizes ), dilation_( polarity == Polarity::DILATION ), mirror_( mirror == Mirror::YES ) {}
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
         in -= inStride * static_cast< dip::sint >( margin );
         TPI* buf = forwardBuffer - margin;
         while( buf < forwardBuffer + length + margin - filterSize ) {
            *buf = *in;
            in += inStride;
            ++buf;
            for( dip::uint ii = 1; ii < filterSize; ++ii ) {
               *buf = dilation_ ? std::max( *in, *( buf - 1 )) : std::min( *in, *( buf - 1 ));
               in += inStride;
               ++buf;
            }
         }
         dip::sint syncpos = buf - forwardBuffer; // this is needed to align the two buffers
         *buf = *in;
         in += inStride;
         ++buf;
         while( buf < forwardBuffer + length + margin ) {
            *buf = dilation_ ? std::max( *in, *( buf - 1 )) : std::min( *in, *( buf - 1 ));
            in += inStride;
            ++buf;
         }
         // Fill backward buffer
         in -= inStride; // undo last increment
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
            ++forwardBuffer;
            ++backwardBuffer;
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
      FloatArray const& filterParam,
      BoundaryConditionArray const& bc,
      Polarity polarity,
      Mirror mirror // this changes where the origin is placed in the even-sized rectangle
) {
   dip::uint nDims = in.Dimensionality();
   BooleanArray process( nDims, false );
   UnsignedArray sizes( nDims, 1 );
   UnsignedArray border( nDims, 0 );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( filterParam[ ii ] > 1.0 ) && ( in.Size( ii ) > 1 )) {
         sizes[ ii ] = static_cast< dip::uint >( std::round( filterParam[ ii ] ));
         process[ ii ] = true;
         border[ ii ] = sizes[ ii ] / 2;
      }
   }
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, RectangularMorphologyLineFilter, ( sizes, polarity, mirror ), dtype );
      Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
   DIP_END_STACK_TRACE
}

// --- Pixel table morphology ---

template< typename TPI >
class FlatSEMorphologyLineFilter : public Framework::FullLineFilter {
   public:
      FlatSEMorphologyLineFilter( Polarity polarity ) : dilation_( polarity == Polarity::DILATION ) {}
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         TPI max = 0; // The maximum value within the filter
         dip::sint index = -1; // Location of the maximum value w.r.t. the left edge
         for( dip::uint ii = 0; ii < length; ++ii ) {
            // Check whether maximum is in filter
            if( index >= 0 ) {
               // Maximum is in filter. Check to see if a larger value came in to the filter.
               for( auto const& run : pixelTable.Runs() ) {
                  dip::sint len = static_cast< dip::sint >( run.length - 1 );
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
                     index = static_cast< dip::sint >( it.Index() );
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

void FlatSEMorphology(
      Image const& in,
      Image& out,
      Kernel const& kernel,
      BoundaryConditionArray const& bc,
      Polarity polarity
) {
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, FlatSEMorphologyLineFilter, ( polarity ), dtype );
      Framework::Full( in, out, dtype, dtype, dtype, 1, bc, kernel, *lineFilter );
   DIP_END_STACK_TRACE
}

// --- Grey-value pixel table morphology ---

template< typename TPI >
class GreyValueSEMorphologyLineFilter : public Framework::FullLineFilter {
   public:
      GreyValueSEMorphologyLineFilter( Polarity polarity ) : dilation_( polarity == Polarity::DILATION ) {}
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         std::vector< dfloat > const& weights = pixelTable.Weights();
         if( dilation_ ) {
            for( dip::uint ii = 0; ii < length; ++ii ) {
               TPI max = std::numeric_limits< TPI >::lowest();
               auto ito = pixelTable.begin();
               auto itw = weights.begin();
               while( !ito.IsAtEnd() ) {
                  max = std::max( max, clamp_cast< TPI >( static_cast< dfloat >( in[ *ito ] ) + *itw ));
                  ++ito;
                  ++itw;
               }
               *out = max;
               in += inStride;
               out += outStride;
            }
         } else {
            for( dip::uint ii = 0; ii < length; ++ii ) {
               TPI min = std::numeric_limits< TPI >::max();
               auto ito = pixelTable.begin();
               auto itw = weights.begin();
               while( !ito.IsAtEnd() ) {
                  min = std::min( min, clamp_cast< TPI >( static_cast< dfloat >( in[ *ito ] ) - *itw ));
                  ++ito;
                  ++itw;
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
      Kernel const& kernel,
      BoundaryConditionArray const& bc,
      Polarity polarity
) {
   DIP_ASSERT( kernel.HasWeights() );
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( polarity ), dtype );
      Framework::Full( in, out, dtype, dtype, dtype, 1, bc, kernel, *lineFilter );
   DIP_END_STACK_TRACE
}

// --- Parabolic morphology ---

template< typename TPI >
class ParabolicMorphologyLineFilter : public Framework::SeparableLineFilter {
   public:
      ParabolicMorphologyLineFilter( FloatArray const& params, Polarity polarity ) :
            params_( params ), dilation_( polarity == Polarity::DILATION ) {}
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
         TPI lambda = static_cast< TPI >( 1.0 / ( params_[ procDim ] * params_[ procDim ] ));
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
                  TPI max = std::numeric_limits< TPI >::lowest();
                  for( dip::sint jj = index; jj <= 0; ++jj ) {
                     TPI val = in[ jj * inStride ] - lambda * static_cast< TPI >( jj * jj );
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
            out += static_cast< dip::sint >( length - 1 ) * outStride;
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
                  TPI max = std::numeric_limits< TPI >::lowest();
                  for(dip::sint jj = index; jj >= 0; --jj ) {
                     TPI val = buf[ jj ] - lambda * static_cast< TPI >( jj * jj );
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
                  TPI min = std::numeric_limits< TPI >::max();
                  for( dip::sint jj = index; jj <= 0; ++jj ) {
                     TPI val = in[ jj * inStride ] + lambda * static_cast< TPI >( jj * jj );
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
            out += static_cast< dip::sint >( length - 2 ) * outStride;
            buf -= 2;
            index = 0;
            for( dip::uint ii = 1; ii < length; ++ii ) {
               ++index;
               if( *buf <= *( out + outStride )) {
                  *out = *buf;
                  index = 0;
               } else {
                  TPI min = std::numeric_limits< TPI >::max();
                  for(dip::sint jj = index; jj >= 0; --jj ) {
                     TPI val = buf[ jj ] + lambda * static_cast< TPI >( jj * jj );
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
      FloatArray const& filterParam,
      BoundaryConditionArray const& bc, // will not be used, as border==0.
      Polarity polarity
) {
   dip::uint nDims = in.Dimensionality();
   BooleanArray process( nDims, false );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( filterParam[ ii ] > 0.0 ) {
         process[ ii ] = true;
      }
   }
   DIP_START_STACK_TRACE
      DataType dtype = DataType::SuggestFlex( in.DataType() ); // Returns either float or complex. If complex, DIP_OVL_NEW_FLOAT will throw.
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, polarity ), dtype );
      Framework::Separable( in, out, dtype, dtype, process, { 0 }, bc, *lineFilter );
   DIP_END_STACK_TRACE
}

// --- Line morphology ---

void SkewLineMorphology(
      Image const& /*in*/,
      Image& /*out*/,
      FloatArray const& /*filterParam*/,
      BoundaryConditionArray const& /*bc*/,
      StructuringElement::ShapeCode /*mode*/,
      Polarity /*polarity*/,
      Mirror /*mirror*/
) {
   // 1- Skew (using interpolation if INTERPOLATED_LINE)
   // 2- Call RectangularMorphology (or a different, new function if PERIODIC_LINE)
   // 3- Skew back (using interpolation if INTERPOLATED_LINE)
   // TODO: Implement SkewLineMorphology (requires implementing Skew() first)
   DIP_THROW( E::NOT_IMPLEMENTED );
}

void LineMorphology(
      Image const& in,
      Image& out,
      FloatArray filterParam,
      BoundaryConditionArray const& bc,
      Polarity polarity,
      Mirror mirror
) {
   // TEMPORARY:
   Kernel se( Kernel::ShapeCode::LINE, filterParam );
   if( mirror == Mirror::YES ) {
      se.Mirror();
   }
   FlatSEMorphology( in, out, se, bc, polarity );
   //

   /*
   dip::uint maxSize = 0;
   dip::uint steps = 0;
   for( dip::uint ii = 0; ii < filterParam.size(); ++ii ) {
      dip::uint length = static_cast< dip::uint >( std::abs( std::round( filterParam[ ii ] )));
      maxSize = std::max( maxSize, length );
      if( steps > 0 ) {
         steps = gcd( steps, length );
      } else {
         steps = length;
      }
   }
   if( steps == 1 ) {
      // This means that one of the sizes is 1.
      // TODO: don't take the size==1 into account when doing the gcd()!
      // TODO: call RectangularMorphology
   } else if( steps == maxSize ){
      // This means that all filterParam are the same
      // TODO: we should get here too if some size==1, but all the rest are equal.
      // TODO: call SkewLineMorphology with FAST_LINE
   } else {
      SkewLineMorphology( in, out, filterParam, bc, StructuringElement::ShapeCode::PERIODIC_LINE, polarity, mirror );
      for( dip::uint ii = 0; ii < filterParam.size(); ++ii ) {
         filterParam[ ii ] = std::round( filterParam[ ii ] ) / static_cast< dfloat >( steps );
      }
      Kernel se( Kernel::ShapeCode::LINE, filterParam );
      if( mirror == Mirror::YES ) {
         se.Mirror();
      }
      FlatSEMorphology( out, out, se, bc, polarity );
   }
   */
}

void DiamondMorphology(
      Image const& in,
      Image& out,
      FloatArray size, // by copy
      BoundaryConditionArray const& bc,
      Polarity polarity
) {
   if( !IsIsotropic( size )) {
      Kernel kernel{ Kernel::ShapeCode::DIAMOND, size };
      FlatSEMorphology( in, out, kernel, bc, polarity );
      return;
   }
   DIP_START_STACK_TRACE
      // Determine values for all operations
      bool skipOperation1 = true;
      bool skipOperation2 = true;
      FloatArray unitSize( size.size(), 1.0 );
      IntegerArray shift( size.size(), 0 );
      bool needShift = false;
      for( dip::uint ii = 0; ii < size.size(); ++ii ) {
         if( std::round( size[ ii ] ) > 4.0 ) {
            // at least 5 pixels in this dimension
            skipOperation1 = false;
            skipOperation2 = false;
            dfloat halfSize = std::max( 3.0, std::floor( size[ ii ] / 4.0 ) * 2.0 + 1.0 ); // an odd value
            unitSize[ ii ] = halfSize;
            size[ ii ] = static_cast< dfloat >(( static_cast< dip::uint >( size[ ii ] - halfSize ) + 1 ) / 2 ) + 1;
            if( !(static_cast< dip::sint >( size[ ii ] ) & 1 )) {
               shift[ ii ] = -1;
               needShift = true;
            }
         } else if( size[ ii ] < 3.0 ) {
            // a single pixel in this dimension
            size[ ii ] = 1.0;
         } else {
            // three pixels in this dimension
            skipOperation1 = false;
            unitSize[ ii ] = 3.0;
            size[ ii ] = 1.0;
         }
      }
      if( skipOperation1 ) {
         out.Copy( in );
      } else {
         // Step 1: apply operation with a unit diamond
         Kernel unitDiamond{ Kernel::ShapeCode::DIAMOND, unitSize };
         if( needShift ) {
            unitDiamond.Shift( shift );
         }
         FlatSEMorphology( in, out, unitDiamond, bc, polarity );
         if( !skipOperation2 ) {
            // Step 2: apply operation with line SEs
            // To get all directions, we flip signs on all elements of size array except the first one.
            // This is exponential in the number of dimensions: 2 in 2D, 4 in 3D, 8 in 4D, etc.
            dip::uint nDims = size.size();
            while( true ) {
               // TODO: this can be the fast skew line morphology, since lines are always at 0 or 45 degrees.
               LineMorphology( out, out, size, bc, polarity, Mirror::NO );
               dip::uint dd;
               for( dd = 1; dd < nDims; ++dd ) {
                  if( std::abs( size[ dd ] ) > 1.0 ) {
                     size[ dd ] = -size[ dd ];
                     if( size[ dd ] < 0 ) {
                        break;
                     }
                  }
               }
               if( dd == nDims ) {
                  break;
               }
            }
         } // else we're done!
      }
   DIP_END_STACK_TRACE
}

void OctagonMorphology(
      Image const& in,
      Image& out,
      FloatArray size, // by copy
      BoundaryConditionArray const& bc,
      Polarity polarity
) {
   // An octagon is formed by a diamond of size n, and a rectangle of size m = n - 2 or m = n.
   // Both n and m are odd integers. The octagon then has a size of n + m - 1.
   // We allow anisotropic octagons by increasing some dimensions of the rectangle (but not decreasing).
   // That is, the diamond will be isotropic, and the rectangle will have at least one side of size m,
   // other dimensions of the rectangle can be larger.
   // Any dimension with an extension of 1 is not included in these calculations.
   DIP_START_STACK_TRACE
      // Determine the smallest dimension (excluding dimensions of size 1)
      dfloat smallestSize = 0.0;
      for( dfloat& sz : size ) {
         sz = std::floor(( sz - 1 ) / 2 ) * 2 + 1; // an odd integer smaller or equal to sz.
         if( sz >= 3.0 ) {
            if( smallestSize == 0.0 ) {
               smallestSize = sz;
            } else {
               smallestSize = std::min( smallestSize, sz );
            }
         } else {
            sz = 1.0;
         }
      }
      if( smallestSize == 0.0 ) {
         // No dimension >= 3
         out.Copy( in );
         return;
      }
      // Given size = n + m + 1, determine n, the size of the diamond
      dfloat n = 2.0 * floor(( smallestSize + 1.0 ) / 4.0 ) + 1.0;
      bool skipRect = true;
      FloatArray rectSize( size.size(), 1.0 );
      for( dip::uint ii = 0; ii < size.size(); ++ii ) {
         if( size[ ii ] >= 3.0 ) {
            // at least 3 pixels in this dimension
            rectSize[ ii ] = size[ ii ] - n + 1.0;
            if( rectSize[ ii ] > 1 ) {
               skipRect = false;
            }
            size[ ii ] = n;
         }
      }
      // Step 1: apply operation with a diamond
      DiamondMorphology( in, out, size, bc, polarity );
      if( !skipRect ) {
         // Step 2: apply operation with a rectangle
         RectangularMorphology( out, out, rectSize, bc, polarity, Mirror::NO );
      }
   DIP_END_STACK_TRACE
}

// --- Dispatch ---

void BasicMorphology(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition,
      Polarity polarity,
      Mirror mirror = Mirror::NO
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      if( bc.empty() ) {
         bc.resize( 1 );
         bc[ 0 ] = polarity == Polarity::DILATION ? BoundaryCondition::ADD_MIN_VALUE : BoundaryCondition::ADD_MAX_VALUE;
      }
      switch( se.Shape() ) {
         case StructuringElement::ShapeCode::RECTANGULAR:
            RectangularMorphology( in, out, se.Params( in.Sizes() ), bc, polarity, mirror );
            break;
         case StructuringElement::ShapeCode::DIAMOND:
            DiamondMorphology( in, out, se.Params( in.Sizes() ), bc, polarity );
            break;
         case StructuringElement::ShapeCode::OCTAGON:
            OctagonMorphology( in, out, se.Params( in.Sizes() ), bc, polarity );
            break;
         case StructuringElement::ShapeCode::LINE:
            LineMorphology( in, out, se.Params( in.Sizes() ), bc, polarity, mirror );
            break;
         case StructuringElement::ShapeCode::FAST_LINE:
         case StructuringElement::ShapeCode::PERIODIC_LINE:
         case StructuringElement::ShapeCode::INTERPOLATED_LINE:
            SkewLineMorphology( in, out, se.Params( in.Sizes() ), bc, se.Shape(), polarity, mirror );
            break;
         case StructuringElement::ShapeCode::PARABOLIC:
            ParabolicMorphology( in, out, se.Params( in.Sizes() ), bc, polarity );
            break;
         default: {
            Kernel kernel = se.Kernel();
            if( mirror == Mirror::YES ) {
               kernel.Mirror();
            }
            if( kernel.HasWeights()) {
               GreyValueSEMorphology( in, out, kernel, bc, polarity );
            } else {
               FlatSEMorphology( in, out, kernel, bc, polarity );
            }
            break;
         }
      }
   DIP_END_STACK_TRACE
}

} // namespace

// --- Public functions ---

void Dilation(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, se, boundaryCondition, Polarity::DILATION );
   DIP_END_STACK_TRACE
}

void Erosion(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, se, boundaryCondition, Polarity::EROSION );
   DIP_END_STACK_TRACE
}

// TODO: For opening and closing, extend the input image, then apply the two operations without boundary extension.

void Opening(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, se, boundaryCondition, Polarity::EROSION, Mirror::NO );
      BasicMorphology( out, out, se, boundaryCondition, Polarity::DILATION, Mirror::YES );
   DIP_END_STACK_TRACE
}

void Closing(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      BasicMorphology( in, out, se, boundaryCondition, Polarity::DILATION, Mirror::NO );
      BasicMorphology( out, out, se, boundaryCondition, Polarity::EROSION, Mirror::YES );
   DIP_END_STACK_TRACE
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the basic morphological filters") {
   dip::Image in( { 128, 69 }, 1, dip::DT_UINT8 );
   in = 0;
   dip::uint pval = 5 * 5;
   in.At( 64, 35 ) = pval;
   dip::Image out;

   // Rectangular morphology
   dip::BasicMorphology( in, out, { { 10, 1 }, "rectangular" }, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   dip::BasicMorphology( in, out, { { 11, 1 }, "rectangular" }, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 11 );
   dip::BasicMorphology( in, out, { { 10, 11 }, "rectangular" }, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10*11 );
   dip::BasicMorphology( out, out, { { 10, 11 }, "rectangular" }, {}, dip::Polarity::EROSION, dip::Mirror::YES );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 64, 35 ) == pval ); // Is that pixel in the right place?

   // PixelTable morphology
   dip::BasicMorphology( in, out, { { 1, 10 }, "elliptic" }, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 11 ); // rounded!
   dip::BasicMorphology( in, out, { { 1, 11 }, "elliptic" }, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 11 );
   dip::BasicMorphology( in, out, { { 10, 11 }, "elliptic" }, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 89 );
   dip::BasicMorphology( out, out, { { 10, 11 }, "elliptic" }, {}, dip::Polarity::EROSION, dip::Mirror::YES );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 64, 35 ) == pval ); // Is that pixel in the right place?

   // PixelTable morphology -- mirroring
   dip::Image se( { 10, 10 }, 1, dip::DT_BIN );
   se = 1;
   dip::BasicMorphology( in, out, se, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 100 );
   dip::BasicMorphology( out, out, se, {}, dip::Polarity::EROSION, dip::Mirror::YES );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 64, 35 ) == pval ); // Is that pixel in the right place?

   // Parabolic morphology
   dip::BasicMorphology( in, out, { { 10.0, 0.0 }, "parabolic" }, {}, dip::Polarity::DILATION );
   dip::dfloat result = 0.0;
   for( dip::uint ii = 1; ii < 50; ++ii ) { // 50 = 10.0 * sqrt( pval )
      result += dip::dfloat( pval ) - dip::dfloat( ii * ii ) / 100.0;
   }
   result = dip::dfloat( pval ) + result * 2.0;
   DOCTEST_CHECK( dip::Sum( out ).As< dip::dfloat >() == doctest::Approx( result ));
   DOCTEST_CHECK( out.At( 64, 35 ) == pval ); // Is the origin in the right place?

   dip::BasicMorphology( out, out, { { 10.0, 0.0 }, "parabolic" }, {}, dip::Polarity::EROSION, dip::Mirror::YES );
   result = 0.0;
   for( dip::uint ii = 1; ii < 50; ++ii ) { // 50 = 10.0 * sqrt( pval )
      result += dip::dfloat( ii * ii ) / 100.0; // 100.0 = 10.0 * 10.0
   }
   result = dip::dfloat( pval ) + result * 2.0;
   DOCTEST_CHECK( dip::Sum( out ).As< dip::dfloat >() == doctest::Approx( result ));
   DOCTEST_CHECK( out.At( 64, 35 ) == pval ); // Is the origin in the right place?

   // Grey-value SE morphology
   se.ReForge( { 5, 6 }, 1, dip::DT_SFLOAT );
   se = -dip::infinity;
   se.At( 0, 0 ) = 0;
   se.At( 4, 5 ) = -5;
   se.At( 0, 5 ) = -5;
   se.At( 4, 0 ) = -10;
   se.At( 2, 3 ) = 0;
   dip::BasicMorphology( in, out, se, {}, dip::Polarity::DILATION );
   DOCTEST_CHECK( dip::Sum( out ).As< dip::uint >() == 5 * pval - 5 - 5 - 10 );
   dip::BasicMorphology( out, out, se, {}, dip::Polarity::EROSION, dip::Mirror::YES );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 64, 35 ) == pval ); // Is the main pixel in the right place and with the right value?
}

#endif // DIP__ENABLE_DOCTEST
