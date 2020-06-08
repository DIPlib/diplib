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

#include <utility>

#include "diplib.h"
#include "diplib/morphology.h"
#include "diplib/kernel.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/overload.h"
#include "diplib/library/copy_buffer.h"

#include "one_dimensional.h"

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
      // TODO: ShapeCode::OCTAGONAL and ShapeCode::PERIODIC_LINE could be converted to ShapeCode::CUSTOM, but only if the image dimensionality is known.
      default:
         DIP_THROW( "Cannot create kernel for this structuring element shape" );
   }
   if( mirror_ ) {
      out.Mirror();
   }
   return out;
}

namespace detail {

namespace {

// Extend the image by `2*boundary`, setting a view around the input + 1*boundary. This allows a first operation
// to read past the image boundary, and still save results outside the original image boundary. These results
// can then be used by a second operation for correct results.
void ExtendImageDoubleBoundary(
      Image const& in,
      Image& out,
      UnsignedArray const& boundary,
      BoundaryConditionArray const&bc
) {
   // Expand by 2*boundary using `bc`.
   UnsignedArray doubleBoundary = boundary;
   for( auto& b : doubleBoundary ) {
      b *= 2;
   }
   ExtendImage( in, out, doubleBoundary, bc );
   // Crop the image by 1*boundary, leaving it larger than `in` by 1*boundary.
   UnsignedArray outSizes = out.Sizes();
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < out.Dimensionality(); ++ii ) {
      outSizes[ ii ] -= doubleBoundary[ ii ];
      offset += static_cast< dip::sint >( boundary[ ii ] ) * out.Stride( ii );
   }
   out.SetSizesUnsafe( outSizes );
   out.SetOriginUnsafe( out.Pointer( offset ));
   // Later after the first processing step, crop the image to the original size.
}

// --- Pixel table morphology ---

template< typename TPI >
class FlatSEMorphologyLineFilter : public Framework::FullLineFilter {
   public:
      FlatSEMorphologyLineFilter( Polarity polarity ) : dilation_( polarity == Polarity::DILATION ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint nKernelPixels, dip::uint nRuns ) override {
         // Number of operations depends on data, so we cannot guess as to how many we'll do. On average:
         dip::uint averageRunLength = div_ceil( nKernelPixels, nRuns );
         dip::uint timesNoMaxInFilter = lineLength / averageRunLength;
         dip::uint timesMaxInFilter = lineLength - timesNoMaxInFilter;
         return timesMaxInFilter * (
                     nRuns * 4                        // number of multiply-adds and comparisons
                     + nRuns )                        // iterating over pixel table runs
                + timesNoMaxInFilter * (
                     nKernelPixels * 2                // number of comparisons
                     + 2 * nKernelPixels + nRuns );   // iterating over pixel table
      }
      virtual void SetNumberOfThreads( dip::uint, PixelTableOffsets const& pixelTable ) override {
         // Let's determine how to process the neighborhood
         dip::uint averageRunLength = div_ceil( pixelTable.NumberOfPixels(), pixelTable.Runs().size() );
         bruteForce_ = averageRunLength < 4; // Experimentally determined
         //std::cout << ( bruteForce_ ? "   Using brute force method\n" : "   Using run length method\n" );
         if( bruteForce_ ) {
            offsets_ = pixelTable.Offsets();
         }
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         if( bruteForce_ ) {
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI max = std::numeric_limits< TPI >::lowest();
                  for( auto it = offsets_.begin(); it != offsets_.end(); ++it ) {
                     max = std::max( max, in[ *it ] );
                  }
                  *out = max;
                  out += outStride;
                  in += inStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI min = std::numeric_limits< TPI >::max();
                  for( auto it = offsets_.begin(); it != offsets_.end(); ++it ) {
                     min = std::min( min, in[ *it ] );
                  }
                  *out = min;
                  out += outStride;
                  in += inStride;
               }
            }
         } else {
            PixelTableOffsets const& pixelTable = params.pixelTable;
            if( dilation_ ) {
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
                        } else if( val > max ) {
                           max = val;
                           index = len;
                        }
                     }
                  } else {
                     // Maximum is no longer in the filter. Find maximum by looping over all pixels in the table.
                     index = 0;
                     max = std::numeric_limits< TPI >::lowest();
                     //for( auto it = pixelTable.begin(); !it.IsAtEnd(); ++it ) {
                     for( auto const& run : pixelTable.Runs() ) {
                        dip::sint offset = run.offset;
                        for( dip::uint jj = 0; jj < run.length; ++jj ) {
                           TPI val = in[ offset ];
                           if( max == val ) {
                              index = std::max( index, static_cast< dip::sint >( jj ));
                           } else if( val > max ) {
                              max = val;
                              index = static_cast< dip::sint >( jj );
                           }
                           offset += pixelTable.Stride();
                        }
                     }
                  }
                  *out = max;
                  out += outStride;
                  in += inStride;
                  index--;
               }
            } else {
               TPI min = 0; // The minimum value within the filter
               dip::sint index = -1; // Location of the minimum value w.r.t. the left edge
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  // Check whether minimum is in filter
                  if( index >= 0 ) {
                     // Minimum is in filter. Check to see if a smaller value came in to the filter.
                     for( auto const& run : pixelTable.Runs() ) {
                        dip::sint len = static_cast< dip::sint >( run.length - 1 );
                        dip::sint position = run.offset + len * inStride;
                        TPI val = in[ position ];
                        if( min == val ) {
                           index = std::max( index, static_cast< dip::sint >( len ));
                        } else if( val < min ) {
                           min = val;
                           index = len;
                        }
                     }
                  } else {
                     // Minimum is no longer in the filter. Find minimum by looping over all pixels in the table.
                     index = 0;
                     min = std::numeric_limits< TPI >::max();
                     //for( auto it = pixelTable.begin(); !it.IsAtEnd(); ++it ) {
                     for( auto const& run : pixelTable.Runs() ) {
                        dip::sint offset = run.offset;
                        for( dip::uint jj = 0; jj < run.length; ++jj ) {
                           TPI val = in[ offset ];
                           if( min == val ) {
                              index = std::max( index, static_cast< dip::sint >( jj ));
                           } else if( val < min ) {
                              min = val;
                              index = static_cast< dip::sint >( jj );
                           }
                           offset += pixelTable.Stride();
                        }
                     }
                  }
                  *out = min;
                  out += outStride;
                  in += inStride;
                  index--;
               }
            }
         }
      }
   private:
      bool dilation_;
      bool bruteForce_ = false;
      std::vector< dip::sint > offsets_; // used when bruteForce_

};

template< typename TPI >
class GreyValueSEMorphologyLineFilter : public Framework::FullLineFilter {
   public:
      GreyValueSEMorphologyLineFilter( Polarity polarity ) : dilation_( polarity == Polarity::DILATION ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint nKernelPixels, dip::uint ) override {
         return lineLength * nKernelPixels * 3;
      }
      virtual void SetNumberOfThreads( dip::uint, PixelTableOffsets const& pixelTable ) override {
         offsets_ = pixelTable.Offsets();
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         std::vector< dfloat > const& weights =  params.pixelTable.Weights();
         if( dilation_ ) {
            for( dip::uint ii = 0; ii < length; ++ii ) {
               TPI max = std::numeric_limits< TPI >::lowest();
               auto ito = offsets_.begin();
               auto itw = weights.begin();
               while( ito != offsets_.end() ) {
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
               auto ito = offsets_.begin();
               auto itw = weights.begin();
               while( ito != offsets_.end() ) {
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
      std::vector< dip::sint > offsets_;
};

void GeneralSEMorphology(
      Image const& in,
      Image& out,
      Kernel& kernel,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
) {
   bool hasWeights = kernel.HasWeights();
   UnsignedArray originalImageSize = in.Sizes();
   Framework::FullOptions opts = {};
   DataType dtype = in.DataType();
   DataType ovltype = dtype;
   if( ovltype.IsBinary() ) {
      ovltype = DT_UINT8; // Dirty trick: process a binary image with the same filter as a UINT8 image, but don't convert the type -- for some reason this is faster!
      DIP_THROW_IF( hasWeights, E::DATA_TYPE_NOT_SUPPORTED );
   }
   std::unique_ptr< Framework::FullLineFilter > lineFilter;
   DIP_START_STACK_TRACE
      switch( operation ) {
         case BasicMorphologyOperation::DILATION:
            if( hasWeights ) {
               DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            } else {
               DIP_OVL_NEW_REAL( lineFilter, FlatSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            }
            Framework::Full( in, out, dtype, dtype, dtype, 1, BoundaryConditionForDilation( bc ), kernel, *lineFilter );
            break;
         case BasicMorphologyOperation::EROSION:
            if( hasWeights ) {
               DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( Polarity::EROSION ), ovltype );
            } else {
               DIP_OVL_NEW_REAL( lineFilter, FlatSEMorphologyLineFilter, ( Polarity::EROSION ), ovltype );
            }
            Framework::Full( in, out, dtype, dtype, dtype, 1, BoundaryConditionForErosion( bc ), kernel, *lineFilter );
            break;
         case BasicMorphologyOperation::CLOSING:
            if( !bc.empty() ) {
               UnsignedArray boundary = kernel.Boundary( in.Dimensionality() );
               ExtendImageDoubleBoundary( in, out, boundary, bc );
               opts += Framework::FullOption::BorderAlreadyExpanded;
            }
            if( hasWeights ) {
               DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            } else {
               DIP_OVL_NEW_REAL( lineFilter, FlatSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            }
            Framework::Full( bc.empty() ? in : out, out, dtype, dtype, dtype, 1, BoundaryConditionForDilation( bc ), kernel, *lineFilter, opts );
            // Note that the output image has a newly-allocated data segment, we've lost the boundary extension we had.
            if( !bc.empty() ) {
               out.Crop( originalImageSize );
            }
            kernel.Mirror();
            if( hasWeights ) {
               DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( Polarity::EROSION ), ovltype );
            } else {
               DIP_OVL_NEW_REAL( lineFilter, FlatSEMorphologyLineFilter, ( Polarity::EROSION ), ovltype );
            }
            Framework::Full( out, out, dtype, dtype, dtype, 1, BoundaryConditionForErosion( bc ), kernel, *lineFilter, opts );
            break;
         case BasicMorphologyOperation::OPENING:
            if( !bc.empty() ) {
               UnsignedArray boundary = kernel.Boundary( in.Dimensionality() );
               ExtendImageDoubleBoundary( in, out, boundary, bc );
               opts += Framework::FullOption::BorderAlreadyExpanded;
            }
            if( hasWeights ) {
               DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( Polarity::EROSION ), ovltype );
            } else {
               DIP_OVL_NEW_REAL( lineFilter, FlatSEMorphologyLineFilter, ( Polarity::EROSION ), ovltype );
            }
            Framework::Full( bc.empty() ? in : out, out, dtype, dtype, dtype, 1, BoundaryConditionForErosion( bc ), kernel, *lineFilter, opts );
            // Note that the output image has a newly-allocated data segment, we've lost the boundary extension we had.
            if( !bc.empty() ) {
               out.Crop( originalImageSize );
            }
            kernel.Mirror();
            if( hasWeights ) {
               DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            } else {
               DIP_OVL_NEW_REAL( lineFilter, FlatSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            }
            Framework::Full( out, out, dtype, dtype, dtype, 1, BoundaryConditionForDilation( bc ), kernel, *lineFilter, opts );
            break;
      }
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
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         // Actual cost depends on data!
         return lineLength * 12;
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         TPI lambda = static_cast< TPI >( 1.0 / ( params_[ params.dimension ] * params_[ params.dimension ] ));
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
            out += static_cast< dip::sint >( length - 1 ) * outStride;
            --buf;
            *out = *buf;
            out -= outStride;
            --buf;
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
      BasicMorphologyOperation operation
) {
   dip::uint nDims = in.Dimensionality();
   BooleanArray process( nDims, false );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( filterParam[ ii ] > 0.0 ) {
         process[ ii ] = true;
      }
   }
   DataType dtype = DataType::SuggestFlex( in.DataType() ); // Returns either float or complex. If complex, DIP_OVL_NEW_FLOAT will throw.
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_START_STACK_TRACE
      switch( operation ) {
         case BasicMorphologyOperation::DILATION:
            DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, Polarity::DILATION ), dtype );
            Framework::Separable( in, out, dtype, dtype, process, { 0 }, bc, *lineFilter );
            break;
         case BasicMorphologyOperation::EROSION:
            DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, Polarity::EROSION ), dtype );
            Framework::Separable( in, out, dtype, dtype, process, { 0 }, bc, *lineFilter );
            break;
         case BasicMorphologyOperation::CLOSING:
            DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, Polarity::DILATION ), dtype );
            Framework::Separable( in, out, dtype, dtype, process, { 0 }, bc, *lineFilter );
            DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, Polarity::EROSION ), dtype );
            Framework::Separable( out, out, dtype, dtype, process, { 0 }, bc, *lineFilter );
            break;
         case BasicMorphologyOperation::OPENING:
            DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, Polarity::EROSION ), dtype );
            Framework::Separable( in, out, dtype, dtype, process, { 0 }, bc, *lineFilter );
            DIP_OVL_NEW_FLOAT( lineFilter, ParabolicMorphologyLineFilter, ( filterParam, Polarity::DILATION ), dtype );
            Framework::Separable( out, out, dtype, dtype, process, { 0 }, bc, *lineFilter );
            break;
      }
   DIP_END_STACK_TRACE
}

// --- Composed SEs ---

void LineMorphology(
      Image const& in,
      Image& out,
      FloatArray filterParam, // by copy
      Mirror mirror,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
) {
   // Normalize direction so that, for even-sized lines, the origin is in a consistent place.
   if( filterParam[ 0 ] < 0 ) {
      for( auto& l: filterParam ) {
         l = -l;
      }
   }
   dip::uint maxSize;
   dip::uint steps;
   std::tie( maxSize, steps ) = PeriodicLineParameters( filterParam );
   if( steps == maxSize ) {
      // This means that all filterParam are the same (or 1)
      FastLineMorphology( in, out, filterParam, StructuringElement::ShapeCode::FAST_LINE, mirror, bc, operation );
   } else {
      if(( steps > 1 ) && ( maxSize > 5 )) { // TODO: an optimal threshold here is impossible to determine. It depends on the processing dimension and the angle of the line.
         dip::uint nDims = in.Dimensionality();
         FloatArray discreteLineParam( nDims, 0.0 );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            discreteLineParam[ ii ] = std::copysign( std::round( std::abs( filterParam[ ii ] )), filterParam[ ii ] ) / static_cast< dfloat >( steps );
         }
         // If the periodic line with even number of points, then the discrete line has origin at left side, to
         // correct for origin displacement of periodic line
         Kernel discreteLineKernel( ( steps & 1 ) ? Kernel::ShapeCode::LINE : Kernel::ShapeCode::LEFT_LINE, discreteLineParam );
         if( mirror == Mirror::YES ) {
            discreteLineKernel.Mirror();
         }
         switch( operation ) {
            default:
               //case BasicMorphologyOperation::DILATION:
               //case BasicMorphologyOperation::EROSION:
               GeneralSEMorphology( in, out, discreteLineKernel, bc, operation );
               FastLineMorphology( out, out, filterParam, StructuringElement::ShapeCode::PERIODIC_LINE, mirror, bc, operation );
               break;
            case BasicMorphologyOperation::CLOSING:
               GeneralSEMorphology( in, out, discreteLineKernel, bc, BasicMorphologyOperation::DILATION );
               FastLineMorphology( out, out, filterParam, StructuringElement::ShapeCode::PERIODIC_LINE, mirror, bc, BasicMorphologyOperation::CLOSING );
               discreteLineKernel.Mirror();
               GeneralSEMorphology( out, out, discreteLineKernel, bc, BasicMorphologyOperation::EROSION );
               break;
            case BasicMorphologyOperation::OPENING:
               GeneralSEMorphology( in, out, discreteLineKernel, bc, BasicMorphologyOperation::EROSION );
               FastLineMorphology( out, out, filterParam, StructuringElement::ShapeCode::PERIODIC_LINE, mirror, bc, BasicMorphologyOperation::OPENING );
               discreteLineKernel.Mirror();
               GeneralSEMorphology( out, out, discreteLineKernel, bc, BasicMorphologyOperation::DILATION );
               break;
         }
      } else {
         // One step, no need to do a periodic line with a single point
         Kernel kernel( Kernel::ShapeCode::LINE, filterParam );
         if( mirror == Mirror::YES ) {
            kernel.Mirror();
         }
         GeneralSEMorphology( in, out, kernel, bc, operation );
      }
   }
}

void TwoStepDiamondMorphology(
      Image& out,
      FloatArray size, // by copy, we'll modify it again
      dip::uint procDim,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation // should be either DILATION or EROSION.
) {
   // To get all directions, we flip signs on all elements of `size` array except `procDim`.
   // This is exponential in the number of dimensions: 2 in 2D, 4 in 3D, 8 in 4D, etc.
   // `size` array must start off with all positive elements.
   dip::uint nDims = size.size();
   while( true ) {
      // This can be the fast line morphology, since lines are always at 0 or 45 degrees.
      FastLineMorphology( out, out, size, StructuringElement::ShapeCode::FAST_LINE, Mirror::NO, bc, operation );
      dip::uint dd;
      for( dd = 0; dd < nDims; ++dd ) {
         if(( dd != procDim ) && ( std::abs( size[ dd ] ) > 1.0 )) {
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
}

void DiamondMorphology(
      Image const& in,
      Image& out,
      FloatArray size, // by copy, we'll modify it
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
) {
   dip::uint nDims = in.Dimensionality();
   dfloat param = 0.0;
   bool isotropic = true;
   dip::uint procDim = 0;
   dip::uint nProcDims = 0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      size[ ii ] = std::floor( size[ ii ] / 2 ) * 2 + 1; // an odd size, same as in `dip::PixelTable::PixelTable(S::DIAMOND)`
      if( size[ ii ] > 1.0 ) {
         ++nProcDims;
         if( param == 0.0 ) {
            param = size[ ii ];
            procDim = ii;
         } else if( size[ ii ] != param ) {
            isotropic = false;
            break;
         }
      }
   }
   if( nProcDims <= 1 ) {
      DIP_STACK_TRACE_THIS( RectangularMorphology( in, out, size, Mirror::NO, bc, operation ));
   } else if( !isotropic || ( param < 7.0 )) {
      // The threshold on 7 is determined empirically, for a large 2D image.
      // If we didn't need to rely on intermediate buffers for the short lines, we could have left the threshold at 5,
      // which is the minimum size that we can decompose.
      DIP_START_STACK_TRACE
         Kernel kernel{ Kernel::ShapeCode::DIAMOND, size };
         GeneralSEMorphology( in, out, kernel, bc, operation );
      DIP_END_STACK_TRACE
   } else {
      // We can separate the SE if param is at least 5, and the SE is isotropic (excluding dimensions where it's 1, those dimensions we don't process at all).
      FloatArray unitSize( nDims, 1.0 );
      dfloat lineLength = std::round(( param - 3.0 ) / 2.0 + 1.0 ); // rounding just in case there's a rounding error, but in principle this always gives a round number.
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( size[ ii ] == param ) {
            unitSize[ ii ] = 3.0;
            size[ ii ] = lineLength;
         } else {
            size[ ii ] = 1.0;
         }
      }
      Kernel unitDiamond{ Kernel::ShapeCode::DIAMOND, unitSize };
      if( !( static_cast< dip::sint >( lineLength ) & 1 )) {
         IntegerArray shift( nDims, 0 );
         shift[ procDim ] = -1;
         unitDiamond.Shift( shift );
      }
      DIP_START_STACK_TRACE
         switch( operation ) {
            default:
            //case BasicMorphologyOperation::DILATION:
            //case BasicMorphologyOperation::EROSION:
               // TODO: For fully correct operation, we should do boundary expansion first, then these two operations, then crop.
               // Step 1: apply operation with a unit diamond
               GeneralSEMorphology( in, out, unitDiamond, bc, operation );
               // Step 2: apply operation with line SEs
               TwoStepDiamondMorphology( out, size, procDim, bc, operation );
               break;
            case BasicMorphologyOperation::CLOSING:
               // TODO: It would be more efficient to apply dilation with lines first, then closing with the unit diamond, then erosion with the lines.
               // ...but for that we need a version of TwoStepDiamond with separate input and output images.
               GeneralSEMorphology( in, out, unitDiamond, bc, BasicMorphologyOperation::DILATION );
               TwoStepDiamondMorphology( out, size, procDim, bc, BasicMorphologyOperation::DILATION );
               TwoStepDiamondMorphology( out, size, procDim, bc, BasicMorphologyOperation::EROSION );
               GeneralSEMorphology( out, out, unitDiamond, bc, BasicMorphologyOperation::EROSION );
               break;
            case BasicMorphologyOperation::OPENING:
               GeneralSEMorphology( in, out, unitDiamond, bc, BasicMorphologyOperation::EROSION );
               TwoStepDiamondMorphology( out, size, procDim, bc, BasicMorphologyOperation::EROSION );
               TwoStepDiamondMorphology( out, size, procDim, bc, BasicMorphologyOperation::DILATION );
               GeneralSEMorphology( out, out, unitDiamond, bc, BasicMorphologyOperation::DILATION );
               break;
         }
      DIP_END_STACK_TRACE
   }
}

void OctagonalMorphology(
      Image const& in,
      Image& out,
      FloatArray size, // by copy
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
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
      switch( operation ) {
         default:
         //case BasicMorphologyOperation::DILATION:
         //case BasicMorphologyOperation::EROSION:
            // Step 1: apply operation with a diamond
            DiamondMorphology( in, out, size, bc, operation );
            if( !skipRect ) {
               // Step 2: apply operation with a rectangle
               RectangularMorphology( out, out, rectSize, Mirror::NO, bc, operation );
            }
            break;
         case BasicMorphologyOperation::CLOSING:
            if( skipRect ) {
               DiamondMorphology( in, out, size, bc, BasicMorphologyOperation::CLOSING );
            } else {
               RectangularMorphology( in, out, rectSize, Mirror::NO, bc, BasicMorphologyOperation::DILATION );
               DiamondMorphology( out, out, size, bc, BasicMorphologyOperation::CLOSING );
               RectangularMorphology( out, out, rectSize, Mirror::YES, bc, BasicMorphologyOperation::EROSION );
            }
            break;
         case BasicMorphologyOperation::OPENING:
            if( skipRect ) {
               DiamondMorphology( in, out, size, bc, BasicMorphologyOperation::OPENING );
            } else {
               RectangularMorphology( in, out, rectSize, Mirror::NO, bc, BasicMorphologyOperation::EROSION );
               DiamondMorphology( out, out, size, bc, BasicMorphologyOperation::OPENING );
               RectangularMorphology( out, out, rectSize, Mirror::YES, bc, BasicMorphologyOperation::DILATION );
            }
            break;
      }
   DIP_END_STACK_TRACE
}

} // namespace


// --- Dispatch ---

void BasicMorphology(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition,
      BasicMorphologyOperation operation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      Mirror mirror = GetMirrorParam( se.IsMirrored() );
      switch( se.Shape() ) {
         case StructuringElement::ShapeCode::RECTANGULAR:
            RectangularMorphology( in, out, se.Params( in.Sizes() ), mirror, bc, operation );
            break;
         case StructuringElement::ShapeCode::DIAMOND:
            DiamondMorphology( in, out, se.Params( in.Sizes() ), bc, operation );
            break;
         case StructuringElement::ShapeCode::OCTAGONAL:
            OctagonalMorphology( in, out, se.Params( in.Sizes() ), bc, operation );
            break;
         case StructuringElement::ShapeCode::LINE:
            LineMorphology( in, out, se.Params( in.Sizes() ), mirror, bc, operation );
            break;
         case StructuringElement::ShapeCode::FAST_LINE:
         case StructuringElement::ShapeCode::PERIODIC_LINE:
            FastLineMorphology( in, out, se.Params( in.Sizes() ), se.Shape(), mirror, bc, operation );
            break;
         case StructuringElement::ShapeCode::INTERPOLATED_LINE:
            SkewLineMorphology( in, out, se.Params( in.Sizes() ), mirror, bc, operation );
            break;
         case StructuringElement::ShapeCode::PARABOLIC:
            ParabolicMorphology( in, out, se.Params( in.Sizes() ), bc, operation );
            break;
         //case StructuringElement::ShapeCode::DISCRETE_LINE:
         //case StructuringElement::ShapeCode::ELLIPTIC:
         //case StructuringElement::ShapeCode::CUSTOM:
         default: {
            Kernel kernel = se.Kernel();
            GeneralSEMorphology( in, out, kernel, bc, operation );
            break;
         }
      }
   DIP_END_STACK_TRACE
}

} // namespace detail

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the basic morphological filters") {
   dip::Image in( { 64, 41 }, 1, dip::DT_UINT8 );
   in = 0;
   dip::uint pval = 3 * 3;
   in.At( 32, 20 ) = pval;
   dip::Image out;

   // Rectangular morphology
   dip::StructuringElement se = {{ 2, 1 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 2 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 3, 1 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 3 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 10, 1 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   se = {{ 11, 1 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 11 );
   se = {{ 10, 11 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10*11 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 2, 1 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );
   se = {{ 1, 3 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );
   se = {{ 10, 1 }, "rectangular" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   // PixelTable morphology
   se = {{ 1, 10 }, "elliptic" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 11 ); // rounded!
   se = {{ 1, 11 }, "elliptic" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 11 );
   se = {{ 3, 3 }, "elliptic" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 9 );
   se = {{ 10, 11 }, "elliptic" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 89 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   // PixelTable morphology -- mirroring
   dip::Image seImg( { 10, 10 }, 1, dip::DT_BIN );
   seImg.Fill( 1 );
   se = seImg;
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 100 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   // Parabolic morphology
   se = {{ 10.0, 0.0 }, "parabolic" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   dip::dfloat result = 0.0;
   for( dip::uint ii = 1; ii < 30; ++ii ) { // 30 = 10.0 * sqrt( pval )
      result += dip::dfloat( pval ) - dip::dfloat( ii * ii ) / 100.0; // 100.0 = 10.0 * 10.0
   }
   result = dip::dfloat( pval ) + result * 2.0;
   DOCTEST_CHECK( dip::Sum( out ).As< dip::dfloat >() == doctest::Approx( result ));
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is the origin in the right place?

   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   result = 0.0;
   for( dip::uint ii = 1; ii < 30; ++ii ) { // 30 = 10.0 * sqrt( pval )
      result += dip::dfloat( ii * ii ) / 100.0; // 100.0 = 10.0 * 10.0
   }
   result = dip::dfloat( pval ) + result * 2.0;
   DOCTEST_CHECK( dip::Sum( out ).As< dip::dfloat >() == doctest::Approx( result ));
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is the origin in the right place?

   // Grey-value SE morphology
   seImg = dip::Image( { 5, 6 }, 1, dip::DT_SFLOAT );
   seImg = -dip::infinity;
   seImg.At( 0, 0 ) = 0;
   seImg.At( 4, 5 ) = -5;
   seImg.At( 0, 5 ) = -5;
   seImg.At( 4, 0 ) = -8;
   seImg.At( 2, 3 ) = 0;
   se = seImg;
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Sum( out ).As< dip::uint >() == 5 * pval - 5 - 5 - 8 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is the main pixel in the right place and with the right value?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   // Line morphology
   se = {{ 10, 4 }, "discrete line" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   se = {{ 10, 4 }, "fast line" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   se = {{ 8, 4 }, "fast line" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 8 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   se = {{ 10, 4 }, "line" }; // periodic component n=2, discrete line {5,2}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   se = {{ 8, 4 }, "line" }; // periodic component n=4, discrete line {2,1}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 8 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   se = {{ 9, 6 }, "line" }; // periodic component n=3, discrete line {3,2}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 9 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   se = {{ 12, 9 }, "line" }; // periodic component n=3, discrete line {4,3}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 12 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );

   se = {{ 8, 9 }, "line" }; // periodic component n=1, discrete line {8,9}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 9 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::CLOSING );
   DOCTEST_CHECK( dip::Count( out ) == 1 );
   DOCTEST_CHECK( out.At( 32, 20 ) == pval );
}

#ifdef _OPENMP

#include "diplib/multithreading.h"
#include "diplib/generation.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE("[DIPlib] testing the full framework under multithreading") {

   // Compute using one thread
   dip::SetNumberOfThreads( 1 );

   // Generate test image
   dip::Image img{ dip::UnsignedArray{ 256, 192, 59 }, 1, dip::DT_DFLOAT };
   img.Fill( 0 );
   dip::Random random( 0 );
   dip::GaussianNoise( img, img, random );

   // Apply separable filter using one thread
   dip::Image out1 = dip::Dilation( img, { 5, "elliptic" } );

   // Reset number of threads
   dip::SetNumberOfThreads( 0 );

   // Apply separable filter using all threads
   dip::Image out2 = dip::Dilation( img, { 5, "elliptic" } );

   // Compare
   DOCTEST_CHECK( dip::testing::CompareImages( out1, out2, dip::Option::CompareImagesMode::EXACT ));
}

#endif // _OPENMP

#endif // DIP__ENABLE_DOCTEST
