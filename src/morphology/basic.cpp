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
#include "diplib/geometry.h"
#include "diplib/kernel.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/overload.h"
#include "diplib/library/copy_buffer.h"

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

enum class Polarity {
      DILATION,
      EROSION
};
enum class Mirror {
      NO,
      YES
};
inline Mirror GetMirrorParam( bool mirror ) {
   return mirror ? Mirror::YES : Mirror::NO;
}
inline Mirror InvertMirrorParam( Mirror mirror ) {
   return mirror == Mirror::YES ? Mirror::NO : Mirror::YES;
}

inline BoundaryConditionArray BoundaryConditionForDilation( BoundaryConditionArray const& bc ) {
   return bc.empty() ? BoundaryConditionArray{ BoundaryCondition::ADD_MIN_VALUE } : bc;
}
inline BoundaryConditionArray BoundaryConditionForErosion( BoundaryConditionArray const& bc ) {
   return bc.empty() ? BoundaryConditionArray{ BoundaryCondition::ADD_MAX_VALUE } : bc;
}

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
   ExtendImageLowLevel( in, out, doubleBoundary, bc, {} );
   // Crop the image by 1*boundary, leaving it larger than `in` by 1*boundary.
   UnsignedArray outSizes = out.Sizes();
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < out.Dimensionality(); ++ii ) {
      outSizes[ ii ] -= doubleBoundary[ ii ];
      offset += static_cast< dip::sint >( boundary[ ii ] ) * out.Stride( ii );
   }
   out.dip__SetSizes( outSizes );
   out.dip__SetOrigin( out.Pointer( offset ));
   // Later after the first processing step, crop the image to the original size.
}

// --- Rectangular morphology ---

template< typename TPI >
class RectangularMorphologyLineFilter : public Framework::SeparableLineFilter {
   public:
      RectangularMorphologyLineFilter( UnsignedArray const& sizes, Polarity polarity, Mirror mirror ) :
            sizes_( sizes ), dilation_( polarity == Polarity::DILATION ), mirror_( mirror == Mirror::YES ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength * 6; // 3 comparisons, 3 iterations
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint filterSize = sizes_[ params.dimension ];
         if( filterSize == 2 ) {
            // Brute-force computation
            if( mirror_ ) {
               in += inStride;
            }
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( in[ -inStride ], in[ 0 ] );
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( in[ -inStride ], in[ 0 ] );
                  in += inStride;
                  out += outStride;
               }
            }
         } else if( filterSize == 3 ) {
            // Brute-force computation
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( std::max( in[ -inStride ], in[ 0 ] ), in[ inStride ] );
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( std::min( in[ -inStride ], in[ 0 ] ), in[ inStride ] );
                  in += inStride;
                  out += outStride;
               }
            }
         } else if( filterSize == 4 ) {
            // Brute-force computation
            if( mirror_ ) {
               in += inStride;
            }
            dip::sint inStride2 = 2 * inStride;
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( std::max( in[ -inStride2 ], in[ -inStride ] ),
                                   std::max( in[ 0 ], in[ inStride ] ));
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( std::min( in[ -inStride2 ], in[ -inStride ] ),
                                   std::min( in[ 0 ], in[ inStride ] ));
                  in += inStride;
                  out += outStride;
               }
            }
         } else if( filterSize == 5 ) {
            // Brute-force computation
            dip::sint inStride2 = 2 * inStride;
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( in[ 0 ], std::max(
                                   std::max( in[ -inStride2 ], in[ -inStride ] ),
                                   std::max( in[ inStride ], in[ inStride2 ] )));
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( in[ 0 ], std::min(
                                   std::min( in[ -inStride2 ], in[ -inStride ] ),
                                   std::min( in[ inStride ], in[ inStride2 ] )));
                  in += inStride;
                  out += outStride;
               }
            }
         } else {
            // Van Herke algorithm
            dip::uint margin = params.inBuffer.border; // TODO: this function must be changed so that margin != filterSize/2
            // Allocate buffer if it's not yet there. It's two buffers, but we allocate only once
            dip::uint bufferSize = length + 2 * margin;
            std::vector< TPI >& buffer = buffers_[ params.thread ];
            buffer.resize( 2 * bufferSize ); // does nothing if already correct size
            TPI* forwardBuffer = buffer.data() + margin;
            TPI* backwardBuffer = forwardBuffer + bufferSize;
            // Fill forward buffer
            in -= inStride * static_cast< dip::sint >( margin );
            TPI* buf = forwardBuffer - margin;
            TPI prev;
            if( dilation_ ) {
               while( buf < forwardBuffer + length + margin - filterSize ) {
                  prev = *buf = *in;
                  in += inStride;
                  ++buf;
                  for( dip::uint ii = 1; ii < filterSize; ++ii ) {
                     prev = *buf = std::max( *in, prev );
                     in += inStride;
                     ++buf;
                  }
               }
            } else {
               while( buf < forwardBuffer + length + margin - filterSize ) {
                  prev = *buf = *in;
                  in += inStride;
                  ++buf;
                  for( dip::uint ii = 1; ii < filterSize; ++ii ) {
                     prev = *buf = std::min( *in, prev );
                     in += inStride;
                     ++buf;
                  }
               }
            }
            dip::sint syncpos = buf - forwardBuffer; // this is needed to align the two buffers
            prev = *buf = *in;
            in += inStride;
            ++buf;
            if( dilation_ ) {
               while( buf < forwardBuffer + length + margin ) {
                  prev = *buf = std::max( *in, prev );
                  in += inStride;
                  ++buf;
               }
            } else {
               while( buf < forwardBuffer + length + margin ) {
                  prev = *buf = std::min( *in, prev );
                  in += inStride;
                  ++buf;
               }
            }
            // Fill backward buffer
            in -= inStride; // undo last increment
            buf = backwardBuffer + length + margin - 1;
            prev = *buf = *in;
            in -= inStride;
            --buf;
            if( dilation_ ) {
               while( buf >= backwardBuffer + syncpos ) {
                  prev = *buf = std::max( *in, prev );
                  in -= inStride;
                  --buf;
               }
               while( buf > backwardBuffer - margin ) {
                  prev = *buf = *in;
                  in -= inStride;
                  --buf;
                  for( dip::uint ii = 1; ii < filterSize; ++ii ) {
                     prev = *buf = std::max( *in, prev );
                     in -= inStride;
                     --buf;
                  }
               }
            } else {
               while( buf >= backwardBuffer + syncpos ) {
                  prev = *buf = std::min( *in, prev );
                  in -= inStride;
                  --buf;
               }
               while( buf > backwardBuffer - margin ) {
                  prev = *buf = *in;
                  in -= inStride;
                  --buf;
                  for( dip::uint ii = 1; ii < filterSize; ++ii ) {
                     prev = *buf = std::min( *in, prev );
                     in -= inStride;
                     --buf;
                  }
               }
            }
            // Fill output
            if( mirror_ ) {
               forwardBuffer += margin;
               backwardBuffer -= filterSize - 1 - margin;
            } else {
               forwardBuffer += filterSize - 1 - margin;
               backwardBuffer -= margin;
            }
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( *forwardBuffer, *backwardBuffer );
                  out += outStride;
                  ++forwardBuffer;
                  ++backwardBuffer;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( *forwardBuffer, *backwardBuffer );
                  out += outStride;
                  ++forwardBuffer;
                  ++backwardBuffer;
               }
            }
         }
      }
   private:
      UnsignedArray const& sizes_;
      std::vector< std::vector< TPI >> buffers_; // one for each thread
      bool dilation_;
      bool mirror_;
};

void RectangularMorphology(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      Mirror mirror,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
) {
   dip::uint nDims = in.Dimensionality();
   BooleanArray process( nDims, false );
   UnsignedArray sizes( nDims, 1 );
   UnsignedArray border( nDims, 0 );
   dip::uint nProcess = 0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( filterParam[ ii ] > 1.0 ) && ( in.Size( ii ) > 1 )) {
         sizes[ ii ] = static_cast< dip::uint >( std::round( filterParam[ ii ] ));
         process[ ii ] = true;
         ++nProcess;
         //if( !bc.empty() ) {
            // If the boundary condition is default, we don't need a boundary extension at all.
            border[ ii ] = sizes[ ii ] / 2;
         //}
      }
   }
   DataType dtype = in.DataType();
   DataType ovltype = dtype;
   if( ovltype.IsBinary() ) {
      ovltype = DT_UINT8; // Dirty trick: process a binary image with the same filter as a UINT8 image, but don't convert the type -- for some reason this is faster!
   }
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   if( nProcess == 0 ) {
      out.Copy( in );
   //} else if( nProcess = 1 ) {
      // TODO: apply 1D opening/closing
   } else {
      DIP_START_STACK_TRACE
         switch( operation ) {
            case BasicMorphologyOperation::DILATION:
               DIP_OVL_NEW_REAL( lineFilter, RectangularMorphologyLineFilter, ( sizes, Polarity::DILATION, mirror ), ovltype );
               //Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               Framework::Separable( in, out, dtype, dtype, process, border, BoundaryConditionForDilation( bc ), *lineFilter );
               break;
            case BasicMorphologyOperation::EROSION:
               DIP_OVL_NEW_REAL( lineFilter, RectangularMorphologyLineFilter, ( sizes, Polarity::EROSION, mirror ), ovltype );
               //Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               Framework::Separable( in, out, dtype, dtype, process, border, BoundaryConditionForErosion( bc ), *lineFilter );
               break;
            case BasicMorphologyOperation::CLOSING:
               DIP_OVL_NEW_REAL( lineFilter, RectangularMorphologyLineFilter, ( sizes, Polarity::DILATION, mirror ), ovltype );
               //Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               Framework::Separable( in, out, dtype, dtype, process, border, BoundaryConditionForDilation( bc ), *lineFilter );
               DIP_OVL_NEW_REAL( lineFilter, RectangularMorphologyLineFilter, ( sizes, Polarity::EROSION, InvertMirrorParam( mirror )), ovltype );
               //Framework::Separable( out, out, dtype, dtype, process, border, bc, *lineFilter );
               Framework::Separable( out, out, dtype, dtype, process, border, BoundaryConditionForErosion( bc ), *lineFilter );
               break;
            case BasicMorphologyOperation::OPENING:
               DIP_OVL_NEW_REAL( lineFilter, RectangularMorphologyLineFilter, ( sizes, Polarity::EROSION, mirror ), ovltype );
               //Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               Framework::Separable( in, out, dtype, dtype, process, border, BoundaryConditionForErosion( bc ), *lineFilter );
               DIP_OVL_NEW_REAL( lineFilter, RectangularMorphologyLineFilter, ( sizes, Polarity::DILATION, InvertMirrorParam( mirror )), ovltype );
               //Framework::Separable( out, out, dtype, dtype, process, border, bc, *lineFilter );
               Framework::Separable( out, out, dtype, dtype, process, border, BoundaryConditionForDilation( bc ), *lineFilter );
               break;
         }
      DIP_END_STACK_TRACE
   }
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
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         if( bruteForce_ ) {
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI max = std::numeric_limits< TPI >::lowest();
                  for( PixelTableOffsets::iterator it = pixelTable.begin(); !it.IsAtEnd(); ++it ) {
                     max = std::max( max, in[ *it ] );
                  }
                  *out = max;
                  out += outStride;
                  in += inStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI min = std::numeric_limits< TPI >::max();
                  for( PixelTableOffsets::iterator it = pixelTable.begin(); !it.IsAtEnd(); ++it ) {
                     min = std::min( min, in[ *it ] );
                  }
                  *out = min;
                  out += outStride;
                  in += inStride;
               }
            }
         } else {
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
                     for( PixelTableOffsets::iterator it = pixelTable.begin(); !it.IsAtEnd(); ++it ) {
                        TPI val = in[ *it ];
                        if( max == val ) {
                           index = std::max( index, static_cast< dip::sint >( it.Index() ));
                        } else if( val > max ) {
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
                     for( PixelTableOffsets::iterator it = pixelTable.begin(); !it.IsAtEnd(); ++it ) {
                        TPI val = in[ *it ];
                        if( min == val ) {
                           index = std::max( index, static_cast< dip::sint >( it.Index() ));
                        } else if( val < min ) {
                           min = val;
                           index = static_cast< dip::sint >( it.Index() );
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
};

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
               std::cout << in;
               std::cout << "[GeneralSEMorphology] calling ExtendImageDoubleBoundary()\n";
               std::cout << out;
               opts += Framework::Full_BorderAlreadyExpanded;
            }
            if( hasWeights ) {
               DIP_OVL_NEW_REAL( lineFilter, GreyValueSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            } else {
               DIP_OVL_NEW_REAL( lineFilter, FlatSEMorphologyLineFilter, ( Polarity::DILATION ), ovltype );
            }
            Framework::Full( bc.empty() ? in : out, out, dtype, dtype, dtype, 1, BoundaryConditionForDilation( bc ), kernel, *lineFilter, opts );
            // Note that the output image has a newly-allocated data segment, we've lost the boundary extension we had.
            if( !bc.empty() ) {
               std::cout << out;
               out.Crop( originalImageSize );
               std::cout << out;
               std::cout << "[GeneralSEMorphology] calling Image::Crop()\n";
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
               opts += Framework::Full_BorderAlreadyExpanded;
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

// --- Line morphology ---

template< typename TPI >
class PeriodicLineMorphologyLineFilter : public Framework::SeparableLineFilter {
      // This is an identical copy of RectangularMorphologyLineFilter, but we fill the buffers using the step size.
      // TODO: Can we merge these two line filters into the same code path?
   public:
      PeriodicLineMorphologyLineFilter( dip::uint stepSize, dip::uint length, Polarity polarity, Mirror mirror ) :
            stepSize_( stepSize ), frameLength_( length ), dilation_( polarity == Polarity::DILATION ), mirror_( mirror == Mirror::YES ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength * 6; // 3 comparisons, 3 iterations
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.inBuffer.length;
         dip::uint margin = params.inBuffer.border; // TODO: this function must be changed so that margin != filterSize/2
         dip::uint steps = frameLength_ / stepSize_;
         if( steps == 2 ) {
            // Brute-force computation
            dip::sint stride = inStride * static_cast< dip::sint >( stepSize_ );
            if( mirror_ ) {
               in += stride;
            }
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( in[ -stride ], in[ 0 ] );
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( in[ -stride ], in[ 0 ] );
                  in += inStride;
                  out += outStride;
               }
            }
         } else if( steps == 3 ) {
            // Brute-force computation
            dip::sint stride = inStride * static_cast< dip::sint >( stepSize_ );
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( std::max( in[ -stride ], in[ 0 ] ), in[ stride ] );
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( std::min( in[ -stride ], in[ 0 ] ), in[ stride ] );
                  in += inStride;
                  out += outStride;
               }
            }
         } else if( steps == 4 ) {
            // Brute-force computation
            dip::sint stride = inStride * static_cast< dip::sint >( stepSize_ );
            if( mirror_ ) {
               in += stride;
            }
            dip::sint stride2 = 2 * stride;
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( std::max( in[ -stride2 ], in[ -stride ] ),
                                   std::max( in[ 0 ], in[ stride ] ));
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( std::min( in[ -stride2 ], in[ -stride ] ),
                                   std::min( in[ 0 ], in[ stride ] ));
                  in += inStride;
                  out += outStride;
               }
            }
         } else if( steps == 5 ) {
            // Brute-force computation
            dip::sint stride = inStride * static_cast< dip::sint >( stepSize_ );
            dip::sint stride2 = 2 * stride;
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( in[ 0 ], std::max(
                        std::max( in[ -stride2 ], in[ -stride ] ),
                        std::max( in[ stride ], in[ stride2 ] )));
                  in += inStride;
                  out += outStride;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( in[ 0 ], std::min(
                        std::min( in[ -stride2 ], in[ -stride ] ),
                        std::min( in[ stride ], in[ stride2 ] )));
                  in += inStride;
                  out += outStride;
               }
            }
         } else {
            // Van Herke algorithm
            // Allocate buffer if it's not yet there. It's two buffers, but we allocate only once
            dip::uint bufferSize = length + 2 * margin;
            std::vector< TPI >& buffer = buffers_[ params.thread ];
            buffer.resize( 2 * bufferSize ); // does nothing if already correct size
            TPI* forwardBuffer = buffer.data() + margin;
            TPI* backwardBuffer = forwardBuffer + bufferSize;
            // Copy input data over to buffers, will simplify filling them later
            in -= inStride * static_cast< dip::sint >( margin );
            TPI* buf = forwardBuffer - margin;
            TPI* buf2 = backwardBuffer - margin;
            while( buf < forwardBuffer + length + margin ) {
               *buf2 = *buf = *in;
               in += inStride;
               ++buf;
               ++buf2;
            }
            // Fill forward buffer
            buf = forwardBuffer - margin;
            if( dilation_ ) {
               while( buf < forwardBuffer + length + margin - frameLength_ ) {
                  buf += stepSize_;
                  for( dip::uint ii = stepSize_; ii < frameLength_; ++ii ) {
                     *buf = std::max( *buf, *( buf - stepSize_ ));
                     ++buf;
                  }
               }
            } else {
               while( buf < forwardBuffer + length + margin - frameLength_ ) {
                  buf += stepSize_;
                  for( dip::uint ii = stepSize_; ii < frameLength_; ++ii ) {
                     *buf = std::min( *buf, *( buf - stepSize_ ));
                     ++buf;
                  }
               }
            }
            dip::sint syncpos = buf - forwardBuffer; // this is needed to align the two buffers
            buf += stepSize_;
            if( dilation_ ) {
               while( buf < forwardBuffer + length + margin ) {
                  *buf = std::max( *buf, *( buf - stepSize_ ));
                  ++buf;
               }
            } else {
               while( buf < forwardBuffer + length + margin ) {
                  *buf = std::min( *buf, *( buf - stepSize_ ));
                  ++buf;
               }
            }
            // Fill backward buffer
            buf = backwardBuffer + length + margin - 1;
            buf -= stepSize_;
            if( dilation_ ) {
               while( buf >= backwardBuffer + syncpos ) {
                  *buf = std::max( *buf, *( buf + stepSize_ ));
                  --buf;
               }
            } else {
               while( buf >= backwardBuffer + syncpos ) {
                  *buf = std::min( *buf, *( buf + stepSize_ ));
                  --buf;
               }
            }
            buf = backwardBuffer + syncpos - 1; // in case `buf -= stepSize_` passed its mark, and the `while` loop didn't run at all.
            if( dilation_ ) {
               while( buf > backwardBuffer - margin ) {
                  buf -= stepSize_;
                  for( dip::uint ii = stepSize_; ii < frameLength_; ++ii ) {
                     *buf = std::max( *buf, *( buf + stepSize_ ));
                     --buf;
                  }
               }
            } else {
               while( buf > backwardBuffer - margin ) {
                  buf -= stepSize_;
                  for( dip::uint ii = stepSize_; ii < frameLength_; ++ii ) {
                     *buf = std::min( *buf, *( buf + stepSize_ ));
                     --buf;
                  }
               }
            }
            // Fill output
            dip::uint nSteps = frameLength_ / stepSize_;
            dip::uint filterLength = ( nSteps - 1 ) * stepSize_ + 1;
            margin = ( nSteps / 2 ) * stepSize_;
            if( mirror_ ) {
               forwardBuffer += margin;
               backwardBuffer -= filterLength - 1 - margin;
            } else {
               forwardBuffer += filterLength - 1 - margin;
               backwardBuffer -= margin;
            }
            if( dilation_ ) {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::max( *forwardBuffer, *backwardBuffer );
                  out += outStride;
                  ++forwardBuffer;
                  ++backwardBuffer;
               }
            } else {
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::min( *forwardBuffer, *backwardBuffer );
                  out += outStride;
                  ++forwardBuffer;
                  ++backwardBuffer;
               }
            }
         }
      }
   private:
      dip::uint stepSize_;
      dip::uint frameLength_;
      std::vector< std::vector< TPI >> buffers_; // one for each thread
      bool dilation_;
      bool mirror_;
};

std::pair< dip::uint, dip::uint > PeriodicLineParameters( FloatArray const& filterParam ) {
   dip::uint maxSize = 0;
   dip::uint steps = 0;
   for( dip::uint ii = 0; ii < filterParam.size(); ++ii ) {
      dip::uint length = static_cast< dip::uint >( std::round( std::abs( filterParam[ ii ] )));
      maxSize = std::max( maxSize, length );
      if( length > 1 ) {
         if( steps > 0 ) {
            steps = gcd( steps, length );
         } else {
            steps = length;
         }
      }
   }
   if( steps == 0 ) {
      // This happens if all length <= 1
      DIP_ASSERT( maxSize == 0 );
      steps = 1;
      maxSize = 1;
   }
   return std::make_pair( maxSize, steps );
}

void SkewLineMorphology(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      Mirror mirror,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
) {
   dip::uint nDims = in.Dimensionality();
   dfloat length = std::round( std::abs( filterParam[ 0 ] ));
   dip::uint axis = 0;
   dip::uint nLarger1 = length > 1.0 ? 1 : 0;
   for( dip::uint ii = 1; ii < nDims; ++ii ) {
      dfloat param = std::round( std::abs( filterParam[ ii ] ));
      if( param > length ) {
         length = param;
         axis = ii;
      }
      nLarger1 += param > 1.0 ? 1 : 0;
   }
   if( nLarger1 > 1 ) {
      // 1- Skew in all dimensions perpendicular to `axis`
      FloatArray shearArray( nDims, 0.0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( ii != axis ) {
            shearArray[ ii ] = std::copysign( std::round( std::abs( filterParam[ ii ] )) / length, filterParam[ ii ] );
         }
      }
      Image tmp;
      Skew( in, tmp, shearArray, axis, 0, "linear", bc ); // TODO: how to fill in default boundary condition here?
      // 2- Call RectangularMorphology
      FloatArray rectSize( nDims, 1.0 );
      rectSize[ axis ] = length;
      RectangularMorphology( tmp, tmp, rectSize, mirror, bc, operation );
      // 3- Skew back and crop to original size
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         shearArray[ ii ] = -shearArray[ ii ];
      }
      Skew( tmp, tmp, shearArray, axis, 0, "linear", bc );
      //Skew( tmp, tmp, shearArray, axis, 0, "linear", { BoundaryCondition::PERIODIC } ); // Using periodic boundary condition so it can be done in-place.
      // TODO: when using periodic skew to go back to original geometry, the origin needs to be computed. Image::Crop can't help us.
      tmp.Crop( in.Sizes());
      out.Copy( tmp );
      out.SetPixelSize( in.PixelSize() );
   } else if( std::round( length ) > 1 ) {
      FloatArray rectSize( nDims, 1.0 );
      rectSize[ axis ] = length;
      RectangularMorphology( in, out, rectSize, mirror, bc, operation );
   } else {
      out.Copy( in );
   }
}

void FastLineMorphology(
      Image const& c_in,
      Image& c_out,
      FloatArray const& filterParam,
      StructuringElement::ShapeCode mode, // PERIODIC_LINE, FAST_LINE
      Mirror mirror,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
) {
   /* This is the general idea for this algorithm:
    *  - We find the (image-wide) Bresenham line that has the angle given by filterParam.
    *  - We make sure that this line has unit steps along the x-axis, and negative steps along all other axes.
    *    (This can be accomplished by swapping and mirroring dimensions.)
    *  - To tessellate the image with this line, we need to always start it at x=0.
    *  - We iterate over all coordinates that have x=0 (i.e. we iterate over all image lines), but including
    *    coordinates outside of the image domain, such that part of the line still touches the image domain.
    *  - At each of these positions, we can copy the input pixels into a buffer, and copy the output pixels back.
    *  - If the angle is such that steps in all dimensions are either 0 or 1, we can define a stride to reach each
    *    pixel along the line, and don't need to use the buffers.
    *  - When walking along a line that starts outside the image domain, we can compute at which x-position the
    *    rounded coordinates will fall within the image domain.
    *  - Likewise, we can compute at which x-poxision the rounded coordinates will exit the image domain.
    */

   BoundaryCondition boundaryCondition1 = BoundaryCondition::DEFAULT;
   BoundaryCondition boundaryCondition2 = BoundaryCondition::DEFAULT;
   if( !bc.empty() ) {
      if( bc.size() > 1 ) {
         // TODO: Make sure they're all the same?
      }
      boundaryCondition1 = bc[ 0 ];
      boundaryCondition2 = bc[ 0 ];
   }

   // Determine SE parameters
   dip::uint nDims = c_in.Dimensionality();
   dfloat length = std::round( std::abs( filterParam[ 0 ] ));
   dip::uint axis = 0;
   dip::uint nLarger1 = length > 1.0 ? 1 : 0;
   for( dip::uint ii = 1; ii < nDims; ++ii ) {
      dfloat param = std::round( std::abs( filterParam[ ii ] ));
      if( param > length ) {
         length = param;
         axis = ii;
      }
      nLarger1 += param > 1.0 ? 1 : 0;
   }

   // Determine periodic line SE parameters
   dip::uint periodicStepSize = 1;
   if( mode == StructuringElement::ShapeCode::PERIODIC_LINE ) {
      dip::uint maxSize = 0;
      dip::uint nSteps = 0;
      std::tie( maxSize, nSteps ) = PeriodicLineParameters( filterParam );
      if( nSteps == 1 ) {
         // The periodic line has just one point, make it so that we just copy the input below.
         nLarger1 = 1;
         length = 1.0;
      } else {
         periodicStepSize = maxSize / nSteps;
         if( periodicStepSize == 1 ) {
            // The periodic line is continuous, use the more efficient code path.
            mode = StructuringElement::ShapeCode::FAST_LINE;
         }
      }
   }

   //std::cout << "[FastLineMorphology] nDims = " << nDims << ", filterParam = " << filterParam << ", mode = "
   //          << ( mode == StructuringElement::ShapeCode::FAST_LINE ? "FAST_LINE" : "PERIODIC_LINE" ) << std::endl;
   //std::cout << "[FastLineMorphology] length = " << length << ", axis = " << axis << ", nLarger1 = " << nLarger1
   //          << ", periodicStepSize = " << periodicStepSize << std::endl;

   // Do easy cases first
   if( length <= 1.0 ) {
      c_out.Copy( c_in );
      return;
   }
   if( nLarger1 == 1 ) {
      // This is the case where the line is along an image axis, there's no angled lines involved
      // If periodic, the step size will be 1
      FloatArray rectSize( nDims, 1.0 );
      rectSize[ axis ] = length;
      DIP_STACK_TRACE_THIS( RectangularMorphology( c_in, c_out, rectSize, mirror, bc, operation ));
      return;
   }

   // Determine step sizes along each dimension
   FloatArray stepSize( nDims, 0 );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      stepSize[ ii ] = std::copysign( std::round( std::abs( filterParam[ ii ] )), filterParam[ ii ] ) / ( length );
   }
   if( stepSize[ axis ] < 0.0 ) {
      // We can flip all dimensions and get the same line
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         stepSize[ ii ] = -stepSize[ ii ];
      }
   }

   // Create output
   Image in = c_in.QuickCopy();
   c_out.ReForge( in );            // Create an image identical to `in`. It's OK if it happened to point at data from `in`, we can work in-place
   Image out = c_out.QuickCopy();
   // Reorder image dimensions so that the first dimension is axis.
   if( axis != 0 ) {
      in.SwapDimensions( axis, 0 );
      out.SwapDimensions( axis, 0 );
      std::swap( stepSize[ axis ], stepSize[ 0 ] );
   }
   DIP_ASSERT( stepSize[ 0 ] == 1.0 );

   //std::cout << "[FastLineMorphology] (1) stepSize = " << stepSize
   //          << ", in.Sizes = " << in.Sizes() << ", in.Strides = " << in.Strides() << std::endl;

   // Make all other dimensions have negative step sizes
   BooleanArray ps( nDims, false );
   bool processDiagonally = true; // This is a special case: we can define a stride to walk along the line.
   for( dip::uint ii = 1; ii < nDims; ++ii ) {
      if( stepSize[ ii ] > 0.0 ) {
         ps[ ii ] = true;
         stepSize[ ii ] = -stepSize[ ii ];
      }
      if(( stepSize[ ii ] != 0.0 ) && ( stepSize[ ii ] != -1.0 )) {
         processDiagonally = false;
      }
   }
   in.Mirror( ps );
   out.Mirror( ps );

   //std::cout << "[FastLineMorphology] (2) stepSize = " << stepSize
   //          << ", in.Sizes = " << in.Sizes() << ", in.Strides = " << in.Strides() << std::endl;
   //std::cout << "[FastLineMorphology] processDiagonally = " << processDiagonally << std::endl;

   // Find the line filter to use
   dip::uint filterLength = static_cast< dip::uint >( length );
   DataType dtype = in.DataType();
   DataType ovltype = dtype;
   if( ovltype.IsBinary() ) {
      ovltype = DT_UINT8; // Dirty trick: process a binary image with the same filter as a UINT8 image, but don't convert the type -- for some reason this is faster!
   }
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter1;
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter2;
   UnsignedArray sizes( 1, filterLength ); // This needs to be kept alive, RectangularMorphologyLineFilter holds a reference to it
   if( mode == StructuringElement::ShapeCode::PERIODIC_LINE ) {
      DIP_START_STACK_TRACE
         switch( operation ) {
            case BasicMorphologyOperation::DILATION:
               DIP_OVL_NEW_REAL( lineFilter1, PeriodicLineMorphologyLineFilter, ( periodicStepSize, filterLength, Polarity::DILATION, mirror ), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MIN_VALUE;
               }
               break;
            case BasicMorphologyOperation::EROSION:
               DIP_OVL_NEW_REAL( lineFilter1, PeriodicLineMorphologyLineFilter, ( periodicStepSize, filterLength, Polarity::EROSION, mirror ), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MAX_VALUE;
               }
               break;
            case BasicMorphologyOperation::CLOSING:
               DIP_OVL_NEW_REAL( lineFilter1, PeriodicLineMorphologyLineFilter, ( periodicStepSize, filterLength, Polarity::DILATION, mirror ), ovltype );
               DIP_OVL_NEW_REAL( lineFilter2, PeriodicLineMorphologyLineFilter, ( periodicStepSize, filterLength, Polarity::EROSION, InvertMirrorParam( mirror )), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MIN_VALUE;
                  boundaryCondition2 = BoundaryCondition::ADD_MAX_VALUE;
               }
               break;
            case BasicMorphologyOperation::OPENING:
               DIP_OVL_NEW_REAL( lineFilter1, PeriodicLineMorphologyLineFilter, ( periodicStepSize, filterLength, Polarity::EROSION, mirror ), ovltype );
               DIP_OVL_NEW_REAL( lineFilter2, PeriodicLineMorphologyLineFilter, ( periodicStepSize, filterLength, Polarity::DILATION, InvertMirrorParam( mirror )), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MAX_VALUE;
                  boundaryCondition2 = BoundaryCondition::ADD_MIN_VALUE;
               }
               break;
         }
      DIP_END_STACK_TRACE
   } else { // mode == StructuringElement::ShapeCode::FAST_LINE
      DIP_START_STACK_TRACE
         switch( operation ) {
            case BasicMorphologyOperation::DILATION:
               DIP_OVL_NEW_REAL( lineFilter1, RectangularMorphologyLineFilter, ( sizes, Polarity::DILATION, mirror ), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MIN_VALUE;
               }
               break;
            case BasicMorphologyOperation::EROSION:
               DIP_OVL_NEW_REAL( lineFilter1, RectangularMorphologyLineFilter, ( sizes, Polarity::EROSION, mirror ), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MAX_VALUE;
               }
               break;
            case BasicMorphologyOperation::CLOSING:
               DIP_OVL_NEW_REAL( lineFilter1, RectangularMorphologyLineFilter, ( sizes, Polarity::DILATION, mirror ), ovltype );
               DIP_OVL_NEW_REAL( lineFilter2, RectangularMorphologyLineFilter, ( sizes, Polarity::EROSION, InvertMirrorParam( mirror )), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MIN_VALUE;
                  boundaryCondition2 = BoundaryCondition::ADD_MAX_VALUE;
               }
               break;
            case BasicMorphologyOperation::OPENING:
               DIP_OVL_NEW_REAL( lineFilter1, RectangularMorphologyLineFilter, ( sizes, Polarity::EROSION, mirror ), ovltype );
               DIP_OVL_NEW_REAL( lineFilter2, RectangularMorphologyLineFilter, ( sizes, Polarity::DILATION, InvertMirrorParam( mirror )), ovltype );
               if( bc.empty() ) {
                  boundaryCondition1 = BoundaryCondition::ADD_MAX_VALUE;
                  boundaryCondition2 = BoundaryCondition::ADD_MIN_VALUE;
               }
               break;
         }
      DIP_END_STACK_TRACE
   }

   // Initialize the line filters (we don't do multithreading here)
   lineFilter1->SetNumberOfThreads( 1 );
   if( lineFilter2 ) {
      lineFilter2->SetNumberOfThreads( 1 );
   }

   // Determine parameters for buffers
   dip::uint border = 0;
   //if( !bc.empty() ) {
      // If the boundary condition is default, we don't need a boundary extension at all.
      border = filterLength / 2;
   //}
   dip::uint maxLineLength = in.Size( 0 );
   dip::uint sizeOf = dtype.SizeOf();

   // Create input buffer data struct and allocate buffer
   dip::uint inBufferSize = ( maxLineLength + 2 * border ) * sizeOf;
   std::vector< uint8 > inBuffer;
   Framework::SeparableBuffer inBufferStruct;
   inBufferStruct.tensorLength = 1;
   inBufferStruct.tensorStride = 1;
   inBufferStruct.border = border;
   bool useInBuffer = !processDiagonally || ( border > 0 ) || lineFilter2;
   if( useInBuffer ) {
      inBuffer.resize( inBufferSize );
      inBufferStruct.stride = 1;
      inBufferStruct.buffer = inBuffer.data() + border * sizeOf;
   } else {
      // processDiagonally == true
      inBufferStruct.stride = in.Stride( 0 );
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         if( stepSize[ ii ] != 0.0 ) { // meaning it's -1.0
            inBufferStruct.stride -= in.Stride( ii );
         }
      }
      inBufferStruct.buffer = nullptr;
   }

   // Create output buffer data struct and allocate buffer
   dip::uint outBufferSize = maxLineLength * sizeOf;
   std::vector< uint8 > outBuffer;
   Framework::SeparableBuffer outBufferStruct;
   outBufferStruct.tensorLength = 1;
   outBufferStruct.tensorStride = 1;
   outBufferStruct.border = 0;
   bool useOutBuffer = !processDiagonally;
   if( useOutBuffer ) {
      outBuffer.resize( outBufferSize );
      outBufferStruct.stride = 1;
      outBufferStruct.buffer = outBuffer.data();
   } else {
      // processDiagonally == true
      outBufferStruct.stride = out.Stride( 0 );
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         if( stepSize[ ii ] != 0.0 ) { // meaning it's -1.0
            outBufferStruct.stride -= out.Stride( ii );
         }
      }
      outBufferStruct.buffer = nullptr;
   }

   Framework::SeparableLineFilterParameters params1{ inBufferStruct, lineFilter2 ? inBufferStruct : outBufferStruct, 0, 0, 1, {}, false, 0 };
   Framework::SeparableLineFilterParameters params2{ inBufferStruct, outBufferStruct, 0, 0, 1, {}, false, 0 };
   // if( lineFilter2 ): We apply 2 filters in sequence, the first one uses the input buffer also for output

   // Compute how far out we need to go along dimensions 1..nDims-1 so that our tessellated lines cover the whole image
   UnsignedArray itSizes( nDims, 0 );
   for( dip::uint ii = 1; ii < nDims; ++ii ) {
      itSizes[ ii ] = in.Size( ii ) - 1 + static_cast< dip::uint >( -std::floor( static_cast< dfloat >( in.Size( 0 )) * stepSize[ ii ] ));
   }
   //std::cout << "[FastLineMorphology] itSizes = " << itSizes << std::endl;

   // Iterate over itSizes
   dip::sint inOffset = 0;
   dip::sint outOffset = 0;
   dip::sint ssizeOf = static_cast< dip::sint >( sizeOf );
   UnsignedArray coords( nDims, 0 );      // These are the start coordinates for the Bresenham line
   FloatArray bresenhamCoords( nDims );   // These are the coordinates to round to get the Bresenham line
   FloatArray bresenhamCoords2( nDims );   // These are the coordinates to round to get the Bresenham line
   while( true ) {

      // Determine the start and end x-coordinate for this line
      dip::uint start = 0;
      dip::uint end = in.Size( 0 ) - 1;
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         dfloat x;
         if( coords[ ii ] >= in.Size( ii )) {
            x = -static_cast< dfloat >( coords[ ii ] + 1 - in.Size( ii )) / stepSize[ ii ];
            start = std::max( start, static_cast< dip::uint >( std::ceil( x )));
         } // otherwise the line starts within the image domain
         x = -static_cast< dfloat >( coords[ ii ] + 1 ) / stepSize[ ii ];
         end = std::min( end, static_cast< dip::uint >( std::ceil( x )) - 1 );
      }
      DIP_ASSERT( start <= end );

      // Find offsets for the start coordinates
      bresenhamCoords[ 0 ] = static_cast< dfloat >( start );
      dip::sint inOffsetStart = inOffset + static_cast< dip::sint >( start ) * in.Stride( 0 );
      dip::sint outOffsetStart = outOffset + static_cast< dip::sint >( start ) * out.Stride( 0 );
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         bresenhamCoords[ ii ] = 0.99999 + bresenhamCoords[ 0 ] * stepSize[ ii ];
         inOffsetStart += static_cast< dip::sint >( std::floor( bresenhamCoords[ ii ] )) * in.Stride( ii );
         outOffsetStart += static_cast< dip::sint >( std::floor( bresenhamCoords[ ii ] )) * out.Stride( ii );
         bresenhamCoords[ ii ] += static_cast< dfloat >( coords[ ii ] );
         DIP_ASSERT( static_cast< dip::uint >( std::floor( bresenhamCoords[ ii ] )) < in.Size( ii ));
      }

      //std::cout << "[FastLineMorphology] coords = " << coords << ", start = " << start << ", end = " << end
      //          << ", bresenhamCoords = " << bresenhamCoords
      //          << ", inOffsetStart = " << inOffsetStart << ", outOffsetStart = " << outOffsetStart << std::endl;

      // Prepare line filter parameters
      dip::uint lineLength = end - start + 1;
      inBufferStruct.length = lineLength;
      outBufferStruct.length = lineLength;
      if( useOutBuffer ) {
         bresenhamCoords2 = bresenhamCoords; // copy before we modify it
      } else {
         outBufferStruct.buffer = out.Pointer( outOffsetStart );
      }

      // Copy from input image to input buffer
      if( useInBuffer ) {
         uint8* src = static_cast< uint8* >( in.Pointer( inOffsetStart ));
         uint8* dest = static_cast< uint8* >( inBufferStruct.buffer );
         for( dip::uint ss = 0; ss < lineLength; ++ss ) {
            std::memcpy( dest, src, sizeOf );
            dest += sizeOf;
            src += in.Stride( 0 ) * ssizeOf;
            for( dip::uint ii = 1; ii < nDims; ++ii ) {
               dfloat old = std::floor( bresenhamCoords[ ii ] );
               bresenhamCoords[ ii ] += stepSize[ ii ];
               if( std::floor( bresenhamCoords[ ii ] ) != old ) {
                  src -= in.Stride( ii ) * ssizeOf; // we're always moving towards smaller coordinates
               }
            }
         }
         if( border > 0 ) {
            ExpandBuffer( inBufferStruct.buffer, ovltype, 1, 1, lineLength, 1, border, border, boundaryCondition1 );
         }
      } else {
         inBufferStruct.buffer = in.Pointer( inOffsetStart );
      }

      // Execute the line filter(s)
      DIP_STACK_TRACE_THIS( lineFilter1->Filter( params1 ));
      if( lineFilter2 ) {
         if( border > 0 ) {
            ExpandBuffer( inBufferStruct.buffer, ovltype, 1, 1, lineLength, 1, border, border, boundaryCondition2 );
         }
         DIP_STACK_TRACE_THIS( lineFilter2->Filter( params2 ));
      }

      // Copy output buffer to output image
      if( useOutBuffer ) {
         uint8* src = static_cast< uint8* >( outBufferStruct.buffer );
         uint8* dest = static_cast< uint8* >( out.Pointer( outOffsetStart ));
         for( dip::uint ss = 0; ss < lineLength; ++ss ) {
            std::memcpy( dest, src, sizeOf );
            src += sizeOf;
            dest += out.Stride( 0 ) * ssizeOf;
            for( dip::uint ii = 1; ii < nDims; ++ii ) {
               dfloat old = std::floor( bresenhamCoords2[ ii ] );
               bresenhamCoords2[ ii ] += stepSize[ ii ];
               if( std::floor( bresenhamCoords2[ ii ] ) != old ) {
                  dest -= out.Stride( ii ) * ssizeOf; // we're always moving towards smaller coordinates
               }
            }
         }
      }

      // Find next start point
      dip::uint dd;
      for( dd = 1; dd < nDims; ++dd ) { // Loop over all dimensions except the first one
         // Increment coordinate and adjust pointer
         ++coords[ dd ];
         inOffset += in.Stride( dd );
         outOffset += out.Stride( dd );
         // Check whether we reached the last pixel of the line
         if( coords[ dd ] < itSizes[ dd ] ) {
            break;
         }
         // Rewind, the next loop iteration will increment the next coordinate
         inOffset -= static_cast< dip::sint >( coords[ dd ] ) * in.Stride( dd );
         outOffset -= static_cast< dip::sint >( coords[ dd ] ) * out.Stride( dd );
         coords[ dd ] = 0;
      }
      if( dd == nDims ) {
         // We're done!
         break;
      }
   }
}

void LineMorphology(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
) {
   FloatArray filterParam = se.Params( in.Sizes() );
   dip::uint maxSize;
   dip::uint steps;
   std::tie( maxSize, steps ) = PeriodicLineParameters( filterParam );
   if( steps == maxSize ) {
      // This means that all filterParam are the same (or 1)
      FastLineMorphology( in, out, filterParam, StructuringElement::ShapeCode::FAST_LINE,
                          GetMirrorParam( se.IsMirrored()), bc, operation );
   } else {
      if(( steps > 1 ) && ( maxSize > 5 )) { // TODO: an optimal threshold here is impossible to determine. It depends on the processing dimension and the angle of the line.
         dip::uint nDims = in.Dimensionality();
         FloatArray discreteLineParam( nDims, 0.0 );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            discreteLineParam[ ii ] = std::round( filterParam[ ii ] ) / static_cast< dfloat >( steps );
         }
         Kernel discreteLineKernel( Kernel::ShapeCode::LINE, discreteLineParam );
         if( !( steps & 1 )) {
            // Periodic line with even number of points
            // Discrete line has origin at left side, to correct for origin displacement of periodic line
            IntegerArray shift( nDims, 0 );
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               shift[ ii ] = - static_cast< dip::sint >( discreteLineParam[ ii ] - 1 ) / 2;
            }
            discreteLineKernel.Shift( shift );
         }
         bool mirror = se.IsMirrored();
         if( mirror ) {
            discreteLineKernel.Mirror();
         }
         switch( operation ) {
            default:
            //case BasicMorphologyOperation::DILATION:
            //case BasicMorphologyOperation::EROSION:
               GeneralSEMorphology( in, out, discreteLineKernel, bc, operation );
               FastLineMorphology( out, out, filterParam, StructuringElement::ShapeCode::PERIODIC_LINE,
                                   GetMirrorParam( mirror ), bc, operation );
               break;
            case BasicMorphologyOperation::CLOSING:
               GeneralSEMorphology( in, out, discreteLineKernel, bc, BasicMorphologyOperation::DILATION );
               FastLineMorphology( out, out, filterParam, StructuringElement::ShapeCode::PERIODIC_LINE,
                                   GetMirrorParam( mirror ), bc, BasicMorphologyOperation::CLOSING );
               discreteLineKernel.Mirror();
               GeneralSEMorphology( out, out, discreteLineKernel, bc, BasicMorphologyOperation::EROSION );
               break;
            case BasicMorphologyOperation::OPENING:
               GeneralSEMorphology( in, out, discreteLineKernel, bc, BasicMorphologyOperation::EROSION );
               FastLineMorphology( out, out, filterParam, StructuringElement::ShapeCode::PERIODIC_LINE,
                                   GetMirrorParam( mirror ), bc, BasicMorphologyOperation::OPENING );
               discreteLineKernel.Mirror();
               GeneralSEMorphology( out, out, discreteLineKernel, bc, BasicMorphologyOperation::DILATION );
               break;
         }
      } else {
         // One step, no need to do a periodic line with a single point
         Kernel kernel( Kernel::ShapeCode::LINE, filterParam );
         if( se.IsMirrored() ) {
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
      size[ ii ] = std::floor( size[ ii ] );
      if( size[ ii ] > 1.0 ) {
         if( param == 0.0 ) {
            param = size[ ii ];
            procDim = ii;
         } else if( size[ ii ] != param ) {
            isotropic = false;
            break;
         }
         ++nProcDims;
      }
   }
   if( !isotropic || ( param < 26.0 ) || ( nProcDims == 1 )) {
      // Threshold of 26 determined empirically for an image of size 2000x1900, and without multithreading.
      // Surely it's different for other image sizes, dimensionalities, and contents.
      DIP_START_STACK_TRACE
         Kernel kernel{ Kernel::ShapeCode::DIAMOND, size };
         GeneralSEMorphology( in, out, kernel, bc, operation );
      DIP_END_STACK_TRACE
   } else {
      // Determine values for all operations
      FloatArray unitSize( nDims, 1.0 );
      dfloat halfSize = std::max( 3.0, std::floor( param / 4.0 ) * 2.0 + 1.0 ); // an odd value
      dfloat remainderSize = static_cast< dfloat >(( static_cast< dip::uint >( param - halfSize ) + 1 ) / 2 ) + 1;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( size[ ii ] == param ) {
            unitSize[ ii ] = halfSize;
            size[ ii ] = remainderSize;
         } else {
            size[ ii ] = 1.0;
         }
      }
      Kernel unitDiamond{ Kernel::ShapeCode::DIAMOND, unitSize };
      if( !( static_cast< dip::sint >( remainderSize ) & 1 )) {
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
            LineMorphology( in, out, se, bc, operation );
            break;
         case StructuringElement::ShapeCode::FAST_LINE:
         case StructuringElement::ShapeCode::PERIODIC_LINE:
            FastLineMorphology( in, out, se.Params( in.Sizes()), se.Shape(), mirror, bc, operation );
            break;
         case StructuringElement::ShapeCode::INTERPOLATED_LINE:
            SkewLineMorphology( in, out, se.Params( in.Sizes()), mirror, bc, operation );
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
   dip::StructuringElement se = {{ 10, 1 }, "rectangular" };
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

   // Line morphology
   se = {{ 10, 4 }, "discrete line" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 10, 4 }, "fast line" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 8, 4 }, "fast line" };
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 8 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 10, 4 }, "line" }; // periodic component n=2, discrete line {5,2}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 10 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 8, 4 }, "line" }; // periodic component n=4, discrete line {2,1}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 8 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 9, 6 }, "line" }; // periodic component n=3, discrete line {3,2}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 9 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 12, 9 }, "line" }; // periodic component n=3, discrete line {4,3}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 12 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?

   se = {{ 8, 9 }, "line" }; // periodic component n=1, discrete line {8,9}
   dip::detail::BasicMorphology( in, out, se, {}, dip::detail::BasicMorphologyOperation::DILATION );
   DOCTEST_CHECK( dip::Count( out ) == 9 );
   se.Mirror();
   dip::detail::BasicMorphology( out, out, se, {}, dip::detail::BasicMorphologyOperation::EROSION );
   DOCTEST_CHECK( dip::Count( out ) == 1 ); // Did the erosion return the image to a single pixel?
   DOCTEST_CHECK( out.At( 32, 20 ) == pval ); // Is that pixel in the right place?
}

#endif // DIP__ENABLE_DOCTEST
