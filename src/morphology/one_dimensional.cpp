/*
 * DIPlib 3.0
 * This file defines internal functions for 1D morphological operators and their compositions.
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

#include "one_dimensional.h"
#include "diplib/geometry.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/library/copy_buffer.h"

namespace dip {

namespace detail {

// --- 1D Line Filters ---

template< typename TPI >
class OperatorDilation {
   public:
      static TPI max( TPI a, TPI b ) {
         return a > b ? a : b;
      }
      static constexpr TPI init = std::numeric_limits< TPI >::lowest();
};
template< typename TPI >
class OperatorErosion {
   public:
      static TPI max( TPI a, TPI b ) {
         return a < b ? a : b;
      }
      static constexpr TPI init = std::numeric_limits< TPI >::max();
};

template< typename TPI, typename OP >
class DilationLineFilter : public Framework::SeparableLineFilter {
   public:
      // NOTE! This filter needs input and output buffers to be distinct only for the brute-force version (filterLength <= 3)
      DilationLineFilter( UnsignedArray const& filterLengths, Mirror mirror, dip::uint maxSize ) :
            filterLengths_( filterLengths ), mirror_( mirror == Mirror::YES ), maxSize_( maxSize ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         bool needBuffers = false;
         for( dip::uint ii = 0; ii < filterLengths_.size(); ++ii ) {
            if( filterLengths_[ ii ] > 3 ) {
               needBuffers = true;
               break;
            }
         }
         if( needBuffers ) {
            buffers_.resize( threads );
         }
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
         dip::uint filterLength = filterLengths_[ params.dimension ];
         dip::uint margin = params.inBuffer.border; // margin == filterLength/2 || margin == 0
         bool hasMargin = margin == filterLength / 2;
         if( filterLength == 2 ) {
            // Brute-force computation
            TPI prev;
            if( hasMargin ) {
               if( mirror_ ) {
                  in += inStride;
               }
               prev = in[ -inStride ];
            } else {
               prev = *in;
               if( mirror_ ) {
                  in += inStride;
               } else {
                  *out = prev;
                  in += inStride;
                  out += outStride;
               }
               --length;
            }
            for( dip::uint ii = 0; ii < length; ++ii ) {
               *out = OP::max( prev, *in );
               prev = *in;
               in += inStride;
               out += outStride;
            }
            if( !hasMargin && mirror_ ) {
               *out = prev;
            }
         } else if( filterLength == 3 ) {
            // Brute-force computation
            TPI prev2;
            TPI prev1;
            if( hasMargin ) {
               prev2 = in[ -inStride ];
               prev1 = *in;
               in += inStride;
            } else {
               prev2 = *in;
               in += inStride;
               prev1 = *in;
               in += inStride;
               if( length < 3 ) {
                  prev1 = OP::max( prev2, prev1 );
                  *out = prev1;
                  out += outStride;
                  *out = prev1;
                  return;
               }
               length -= 2;
               *out = OP::max( prev2, prev1 );
               out += outStride;
            }
            for( dip::uint ii = 0; ii < length; ++ii ) {
               *out = OP::max( OP::max( prev1, prev2 ), *in );
               prev2 = prev1;
               prev1 = *in;
               in += inStride;
               out += outStride;
            }
            if( !hasMargin ) {
               *out = OP::max( prev2, prev1 );
            }
         } else {
            // Van Herk algorithm
            // Three steps:
            //  1- Fill the forward buffer with the cumulative max over blocks of size filterLength, starting at the
            //     left edge of the image, and past the right edge by filterLength/2.
            //  2- Fill the backward buffer with the cumulative max over blocks of size filterLength, starting at the
            //     right edge of the image, and past the left edge by filterLength/2.
            //     Note that the blocks in the forward and backward buffer must be aligned.
            //  3- Take the max between a value in the forward buffer at pos + right, and a value in
            //     the backward buffer at pos - left. We do this by shifting the two buffers: forward buffer left by
            //     filterLength/2, and backward buffer right by filterLength/2.
            // We could put one of the two buffers in the output array, but, we do not do this for simplicity of the
            // code (note that in and out can be the same, and input and output must be read using strides).
            // How values past the right edge in the forward buffer, and values past the left edge in the
            // backward buffer are filled in depends on the boundary condition. If we don't have a margin (i.e.
            // default boundary condition), we simply extend using the edge pixel. This assures that the max (or min)
            // value selected is always one of the values within the filer.
            // TODO: Gil and Kimmel suggest a way of computing these three steps that further reduces the number of comparisons.
            dip::uint left = filterLength / 2; // The number of pixels on the left side of the filter
            dip::uint right = filterLength - 1 - left; // The number of pixels on the right side
            if( mirror_ ) {
               std::swap( left, right );
            }
            // Allocate buffer if it's not yet there.
            std::vector< TPI >& buffer = buffers_[ params.thread ];
            buffer.resize( std::max( maxSize_, length ) * 2 + filterLength ); // does nothing if already correct size
            TPI* forwardBuffer = buffer.data();    // size = length + right
            TPI* backwardBuffer = forwardBuffer + length + right; // size = length + left
            // Copy input to forward and backward buffers, adding a margin on one side of each buffer
            TPI* tmp;
            TPI* buf;
            TPI prev;
            if( hasMargin ) {
               tmp = in - inStride;
               buf = backwardBuffer + left - 1;
               prev = *buf = *tmp;
               --buf;
               tmp -= inStride;
               for( dip::uint ii = 1; ii < left; ++ii ) {
                  prev = *buf = OP::max( *tmp, prev );
                  --buf;
                  tmp -= inStride;
               }
               backwardBuffer += left;
            } else { // copy edge value out into margin
               for( dip::uint ii = 0; ii < left; ++ii ) {
                  *backwardBuffer = *in;
                  ++backwardBuffer;
               }
            }
            dip::uint nBlocks = length / filterLength;
            dip::uint lastBlockSize = length % filterLength;
            for( dip::uint jj = 0; jj < nBlocks; ++jj ) {
               prev = *forwardBuffer = *in;
               ++forwardBuffer;
               in += inStride;
               for( dip::uint ii = 1; ii < filterLength; ++ii ) {
                  prev = *forwardBuffer = OP::max( *in, prev );
                  ++forwardBuffer;
                  in += inStride;
               }
               tmp = in - inStride;
               backwardBuffer += filterLength;
               buf = backwardBuffer - 1;
               prev = *buf = *tmp;
               --buf;
               tmp -= inStride;
               for( dip::uint ii = 1; ii < filterLength; ++ii ) {
                  prev = *buf = OP::max( *tmp, prev );
                  --buf;
                  tmp -= inStride;
               }
            }
            if( hasMargin ) {
               tmp = in;
               prev = *forwardBuffer = *tmp;
               ++forwardBuffer;
               tmp += inStride;
               for( dip::uint ii = 1; ii < std::min( lastBlockSize + right, filterLength ); ++ii ) {
                  prev = *forwardBuffer = OP::max( *tmp, prev );
                  ++forwardBuffer;
                  tmp += inStride;
               }
               if( lastBlockSize + right > filterLength ) {
                  prev = *forwardBuffer = *tmp;
                  ++forwardBuffer;
                  tmp += inStride;
                  for( dip::uint ii = 1; ii < lastBlockSize + right - filterLength; ++ii ) {
                     prev = *forwardBuffer = OP::max( *tmp, prev );
                     ++forwardBuffer;
                     tmp += inStride;
                  }
               }
            } else {
               if( lastBlockSize > 0 ) {
                  tmp = in;
                  prev = *forwardBuffer = *tmp;
                  ++forwardBuffer;
                  tmp += inStride;
                  for( dip::uint ii = 1; ii < lastBlockSize; ++ii ) {
                     prev = *forwardBuffer = OP::max( *tmp, prev );
                     ++forwardBuffer;
                     tmp += inStride;
                  }
                  for( dip::uint ii = lastBlockSize; ii < std::min( lastBlockSize + right, filterLength ); ++ii ) {
                     *forwardBuffer = prev;
                     ++forwardBuffer;
                  }
                  if( lastBlockSize + right > filterLength ) {
                     prev = *( tmp - inStride ); // copy edge value out into margin
                     for( dip::uint ii = 0; ii < lastBlockSize + right - filterLength; ++ii ) {
                        *forwardBuffer = prev;
                        ++forwardBuffer;
                     }
                  }
               } else {
                  prev = *( in - inStride );
                  for( dip::uint ii = 0; ii < right; ++ii ) {
                     *forwardBuffer = prev;
                     ++forwardBuffer;
                  }
               }
            }
            if( lastBlockSize > 0 ) {
               tmp = in + static_cast< dip::sint >( lastBlockSize - 1 ) * inStride;
               buf = backwardBuffer + ( lastBlockSize - 1 );
               prev = *buf = *tmp;
               --buf;
               tmp -= inStride;
               for( dip::uint ii = 1; ii < lastBlockSize; ++ii ) {
                  prev = *buf = OP::max( *tmp, prev );
                  --buf;
                  tmp -= inStride;
               }
            }
            // Fill output
            forwardBuffer = buffer.data() + right; // shift this buffer left by `right`.
            backwardBuffer = forwardBuffer + length; // this is shifted right by `left`.
            for( dip::uint ii = 0; ii < length; ++ii ) {
               *out = OP::max( *forwardBuffer, *backwardBuffer );
               out += outStride;
               ++forwardBuffer;
               ++backwardBuffer;
            }
         }
      }
   private:
      UnsignedArray const& filterLengths_;
      bool mirror_;
      dip::uint maxSize_;
      std::vector< std::vector< TPI >> buffers_; // one for each thread
};

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewDilationLineFilter(
      UnsignedArray const& sizes, Mirror mirror, dip::uint maxSize ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new DilationLineFilter< TPI, OperatorDilation< TPI >>( sizes, mirror, maxSize ));
}

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewErosionLineFilter(
      UnsignedArray const& sizes, Mirror mirror, dip::uint maxSize ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new DilationLineFilter< TPI, OperatorErosion< TPI >>( sizes, mirror, maxSize ));
}

template< typename TPI, typename OP >
class PeriodicDilationLineFilter : public Framework::SeparableLineFilter {
   public:
      // NOTE! This filer needs input and output buffers to be distinct only for the brute-force version (filterLength <= 3)
      PeriodicDilationLineFilter( dip::uint stepSize, dip::uint filterLength, Mirror mirror, dip::uint maxSize ) :
            stepSize_( stepSize ), filterLength_( filterLength ), mirror_( mirror == Mirror::YES ), maxSize_( maxSize ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         if( filterLength_ / stepSize_ > 3 ) {
            buffers_.resize( threads );
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength * 6; // 3 comparisons, 3 iterations
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         dip::sint stepStride = inStride * static_cast< dip::sint >( stepSize_ );
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint steps = filterLength_ / stepSize_; // stepSize_ > 1, steps > 1
         dip::uint margin = params.inBuffer.border; // margin == filterLength_/2 || margin == 0
         bool hasMargin = margin == filterLength_ / 2;
         if( !hasMargin && ( length <= stepSize_ )) {
            // Short-cut for short lengths, copy in to out.
            for( dip::uint ii = 0; ii < length; ++ii ) {
               *out = *in;
               in += inStride;
               out += outStride;
            }
            return;
         }
         if( steps == 2 ) {
            // Brute-force computation
            // TODO: use the `prev` trick from `DilationLineFilter`. But it needs to be an array with `stepSize_` elements...
            if( hasMargin ) {
               if( mirror_ ) {
                  in += stepStride;
               }
            } else {
               length -= stepSize_;
               if( !mirror_ ) {
                  for( dip::uint ii = 0; ii < stepSize_; ++ii ) {
                     *out = *in;
                     in += inStride;
                     out += outStride;
                  }
               } else {
                  in += stepStride;
               }
            }
            for( dip::uint ii = 0; ii < length; ++ii ) {
               *out = OP::max( in[ -stepStride ], in[ 0 ] );
               in += inStride;
               out += outStride;
            }
            if( !hasMargin && mirror_ ) {
               in -= stepStride;
               for( dip::uint ii = 0; ii < stepSize_; ++ii ) {
                  *out = *in;
                  in += inStride;
                  out += outStride;
               }
            }
         } else if( steps == 3 ) {
            // Brute-force computation
            // TODO: use the `prev` trick from `DilationLineFilter`. But it needs to be an array with `stepSize_` elements...
            if( !hasMargin ) {
               if( length <= 2 * stepSize_ ) {
                  for( dip::uint ii = 0; ii < length - stepSize_; ++ii ) {
                     *out = OP::max( in[ 0 ], in[ stepStride ] );
                     in += inStride;
                     out += outStride;
                  }
                  for( dip::uint ii = 0; ii < 2 * stepSize_ - length; ++ii ) {
                     *out = *in;
                     in += inStride;
                     out += outStride;
                  }
                  for( dip::uint ii = 0; ii < length - stepSize_; ++ii ) {
                     *out = OP::max( in[ -stepStride ], in[ 0 ] );
                     in += inStride;
                     out += outStride;
                  }
                  return;
               }
               length -= 2 * stepSize_;
               for( dip::uint ii = 0; ii < stepSize_; ++ii ) {
                  *out = OP::max( in[ 0 ], in[ stepStride ] );
                  in += inStride;
                  out += outStride;
               }
            }
            for( dip::uint ii = 0; ii < length; ++ii ) {
               *out = OP::max( OP::max( in[ -stepStride ], in[ 0 ] ), in[ stepStride ] );
               in += inStride;
               out += outStride;
            }
            if( !hasMargin ) {
               for( dip::uint ii = 0; ii < stepSize_; ++ii ) {
                  *out = OP::max( in[ -stepStride ], in[ 0 ] );
                  in += inStride;
                  out += outStride;
               }
            }
         } else {
            // Van Herk algorithm, modified from DilationLineFilter
            dip::uint left = ( steps / 2 ) * stepSize_; // The number of pixels on the left side of the filter
            dip::uint right = (( steps - 1 ) / 2 ) * stepSize_; // The number of pixels on the right side
            if( mirror_ ) {
               std::swap( left, right );
            }
            // Allocate buffer if it's not yet there.
            std::vector< TPI >& buffer = buffers_[ params.thread ];
            buffer.resize( std::max( maxSize_, length ) * 2 + filterLength_ ); // does nothing if already correct size
            TPI* forwardBuffer = buffer.data();    // size = length + right
            TPI* backwardBuffer = forwardBuffer + length + right; // size = length + left
            // Copy input to forward and backward buffers, adding a margin on one side of each buffer
            TPI* tmp;
            TPI* buf;
            if( hasMargin ) {
               tmp = in - inStride;
               buf = backwardBuffer + left - 1;
               for( dip::uint ii = 0; ii < std::min( stepSize_, left ); ++ii ) {
                  *buf = *tmp;
                  --buf;
                  tmp -= inStride;
               }
               for( dip::uint ii = stepSize_; ii < left; ++ii ) {
                  *buf = OP::max( *tmp, *( buf + stepSize_ ));
                  --buf;
                  tmp -= inStride;
               }
               backwardBuffer += left;
            } else { // Fill margin with min value
               for( dip::uint ii = 0; ii < left; ++ii ) {
                  *backwardBuffer = OP::init;
                  ++backwardBuffer;
               }
            }
            dip::uint nBlocks = length / filterLength_;
            dip::uint lastBlockSize = length % filterLength_;
            for( dip::uint jj = 0; jj < nBlocks; ++jj ) {
               for( dip::uint ii = 0; ii < stepSize_; ++ii ) {
                  *forwardBuffer = *in;
                  ++forwardBuffer;
                  in += inStride;
               }
               for( dip::uint ii = stepSize_; ii < filterLength_; ++ii ) {
                  *forwardBuffer = OP::max( *in, *( forwardBuffer - stepSize_ ));
                  ++forwardBuffer;
                  in += inStride;
               }
               tmp = in - inStride;
               backwardBuffer += filterLength_;
               buf = backwardBuffer - 1;
               for( dip::uint ii = 0; ii < stepSize_; ++ii ) {
                  *buf = *tmp;
                  --buf;
                  tmp -= inStride;
               }
               for( dip::uint ii = stepSize_; ii < filterLength_; ++ii ) {
                  *buf = OP::max( *tmp, *( buf + stepSize_ ));
                  --buf;
                  tmp -= inStride;
               }
            }
            if( hasMargin ) {
               tmp = in;
               for( dip::uint ii = 0; ii < std::min( stepSize_, lastBlockSize + right ); ++ii ) {
                  *forwardBuffer = *tmp;
                  ++forwardBuffer;
                  tmp += inStride;
               }
               for( dip::uint ii = stepSize_; ii < std::min( lastBlockSize + right, filterLength_ ); ++ii ) {
                  *forwardBuffer = OP::max( *tmp, *( forwardBuffer - stepSize_ ));
                  ++forwardBuffer;
                  tmp += inStride;
               }
               if( lastBlockSize + right > filterLength_ ) {
                  for( dip::uint ii = 0; ii < std::min( stepSize_, lastBlockSize + right - filterLength_ ); ++ii ) {
                     *forwardBuffer = *tmp;
                     ++forwardBuffer;
                     tmp += inStride;
                  }
                  for( dip::uint ii = stepSize_; ii < lastBlockSize + right - filterLength_; ++ii ) {
                     *forwardBuffer = OP::max( *tmp, *( forwardBuffer - stepSize_ ));
                     ++forwardBuffer;
                     tmp += inStride;
                  }
               }
            } else {
               tmp = in;
               for( dip::uint ii = 0; ii < std::min( stepSize_, lastBlockSize ); ++ii ) {
                  *forwardBuffer = *tmp;
                  ++forwardBuffer;
                  tmp += inStride;
               }
               for( dip::uint ii = stepSize_; ii < lastBlockSize; ++ii ) {
                  *forwardBuffer = OP::max( *tmp, *( forwardBuffer - stepSize_ ));
                  ++forwardBuffer;
                  tmp += inStride;
               }
               dip::uint n = 0;
               if( lastBlockSize < stepSize_ ) {
                  n = std::min( stepSize_ - lastBlockSize, right );
                  for( dip::uint ii = 0; ii < n; ++ii ) {
                     *forwardBuffer = OP::init; // Fill margin with min value
                     ++forwardBuffer;
                  }
               }
               for( dip::uint ii = n; ii < right; ++ii ) {
                  *forwardBuffer = *( forwardBuffer - stepSize_ );
                  ++forwardBuffer;
               }
            }
            if( lastBlockSize > 0 ) {
               tmp = in + static_cast< dip::sint >( lastBlockSize - 1 ) * inStride;
               buf = backwardBuffer + ( lastBlockSize - 1 );
               for( dip::uint ii = 0; ii < std::min( stepSize_, lastBlockSize ); ++ii ) {
                  *buf = *tmp;
                  --buf;
                  tmp -= inStride;
               }
               for( dip::uint ii = stepSize_; ii < std::min( lastBlockSize, filterLength_ ); ++ii ) {
                  *buf = OP::max( *tmp, *( buf + stepSize_ ));
                  --buf;
                  tmp -= inStride;
               }
            }
            // Fill output
            forwardBuffer = buffer.data() + right; // shift this buffer left by `right`.
            backwardBuffer = forwardBuffer + length; // this is shifted right by `left`.
            for( dip::uint ii = 0; ii < length; ++ii ) {
               *out = OP::max( *forwardBuffer, *backwardBuffer );
               out += outStride;
               ++forwardBuffer;
               ++backwardBuffer;
            }
         }
      }
   private:
      dip::uint stepSize_;
      dip::uint filterLength_;
      bool mirror_;
      dip::uint maxSize_;
      std::vector< std::vector< TPI >> buffers_; // one for each thread
};

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewPeriodicDilationLineFilter(
      dip::uint stepSize, dip::uint filterLength, Mirror mirror, dip::uint maxSize ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new PeriodicDilationLineFilter< TPI, OperatorDilation< TPI >>( stepSize, filterLength, mirror, maxSize ));
}

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewPeriodicErosionLineFilter(
      dip::uint stepSize, dip::uint filterLength, Mirror mirror, dip::uint maxSize ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new PeriodicDilationLineFilter< TPI, OperatorErosion< TPI >>( stepSize, filterLength, mirror, maxSize ));
}

/*

template< typename TPI >
class OperatorOpening {
   public:
      static bool gt( TPI a, TPI b ) {
         return a > b;
      }
      static bool lt( TPI a, TPI b ) {
         return a < b;
      }
      static TPI max( TPI a, TPI b ) {
         return gt( a, b ) ? a : b;
      }
      static TPI min( TPI a, TPI b ) {
         return lt( a, b ) ? a : b;
      }
};
template< typename TPI >
class OperatorClosing {
   public:
      static bool gt( TPI a, TPI b ) {
         return a < b;
      }
      static bool lt( TPI a, TPI b ) {
         return a > b;
      }
      static TPI max( TPI a, TPI b ) {
         return gt( a, b ) ? a : b;
      }
      static TPI min( TPI a, TPI b ) {
         return lt( a, b ) ? a : b;
      }
};

template< typename TPI, typename OP >
class OpeningLineFilter : public Framework::SeparableLineFilter {
      struct QElem {
         dip::uint start;
         TPI value;
         bool finished;
         QElem( dip::uint start, TPI value, bool finished ) : start( start ), value( value ), finished( finished ) {}
      };
   public:
      OpeningLineFilter( dip::uint filterLength ) : filterLength_( filterLength ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         if( filterLength_ > 3 ) {
            stack_.resize( threads );
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength * 6; // TODO: determine correct number of operations
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint margin = params.inBuffer.border; // margin == filterLength/2 || margin == 0
         bool hasMargin = margin == filterLength_ / 2;
         if( filterLength_ == 2 ) {
            // Brute-force computation
            if( !hasMargin ) {
               *out = OP::min( *in, in[ inStride ] );
               in += inStride;
               out += outStride;
               length -= 2;
            }
            for( dip::uint ii = 0; ii < length; ++ii ) {
               // max(min(a,b),min(b,c)) == min(max(a,c),b)
               TPI v = OP::max( in[ -inStride ], in[ inStride ] );
               *out = OP::min( *in, v );
               in += inStride;
               out += outStride;
            }
            if( !hasMargin ) {
               *out = OP::min( *in, in[ -inStride ] );
            }
         } else if( filterLength_ == 3 ) {
            // Brute-force computation
            // Loop unrolling by factor 2 could save one comparison per output pixel. Not really worth the complication IMO.
            dip::sint inStride2 = 2 * inStride;
            TPI a, b, c;
            if( hasMargin ) {
               // Initialize
               TPI v = OP::min( in[ 0 ], in[ inStride ] );
               a = OP::min( OP::min( in[ 0 ], in[ -inStride ] ), in[ -inStride2 ] );
               b = OP::min( v, in[ -inStride ] );
               c = OP::min( v, in[ inStride2 ] );
               *out = OP::max( OP::max( a, b ), c );
               in += inStride;
               out += outStride;
            } else {
               // Initialize, and handle first two pixels as special case
               if( length == 2 ) {
                  b = OP::min( in[ 0 ], in[ inStride ] );
                  c = b;
                  // The two pixels will be written at the end
                  length = 0; // don't run the main loop
               } else if( length == 3 ) {
                  b = OP::min( OP::min( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
                  c = b;
                  *out = b;
                  out += outStride;
                  // The other two pixels will be written at the end
                  length = 0; // don't run the main loop
               } else {
                  b = OP::min( OP::min( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
                  *out = b;
                  in += inStride;
                  out += outStride;
                  c = OP::min( OP::min( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
                  *out = OP::max( b, c );
                  in += inStride;
                  out += outStride;
                  length -= 3;
               }
            }
            for( dip::uint ii = 1; ii < length; ++ii ) {
               a = b;
               b = c;
               c = OP::min( OP::min( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
               *out = OP::max( OP::max( a, b ), c );
               in += inStride;
               out += outStride;
            }
            if( !hasMargin ) {
               // Handle last two pixels as special case
               *out = OP::max( b, c );
               out += outStride;
               *out = c;
            }
         } else {
            // Vincent Morard's algorithm: build a max-tree, which is pruned as it's build, what is left is the opening.
            // To prevent the stack from growing unnecessarily, nodes are written out as they are finished. This forces
            // `in` and `out` buffers to be distinct. If we build the full max-tree, and then write the output in one
            // go, `in` and `out` can point to the same buffer.
            // We use the boundary-extended `in` by starting to fill the max-tree with data from it, but we never
            // write that data to `out`. At the end of the line, we continue reading and adding to the max-tree,
            // but we again never write to `out` past the end of the line.
            // TODO: Why is Morard's algorithm so slow? This doesn't match what they show in the paper!
            std::vector< QElem >& stack = stack_[ params.thread ]; // We call it stack, but this is the max-tree.
            stack.reserve( length / 2 );
            stack.clear(); // Clear just in case. We do not allocate a new stack for every line, we can re-use memory allocated.
            //if( hasMargin ) {
            //   in -= margin * inStride;
            //   stack.emplace_back( 0, *in, false );
            //   for( dip::uint ii = 1; ii < margin; ++ii ) {
            //      TODO: use margin!
            //   }
            //} else {
            stack.emplace_back( 0, *in, false );
            //}
            for( dip::uint ii = 1; ii < length; ++ii ) {
               in += inStride;
               if( OP::gt( *in, stack.back().value )) {
                  // A new element for the max-tree
                  stack.emplace_back( ii, *in, false );
               } else {
                  while( OP::lt( *in, stack.back().value )) {
                     // Process elements in the max-tree
                     if( stack.back().finished || ( ii - stack.back().start >= filterLength_ )) {
                        // If the last element on the max-tree is long enough, write the whole tree to the output
                        for( dip::uint jj = 0; jj < stack.size() - 1; jj++ ) {
                           for( dip::uint kk = stack[ jj ].start; kk < stack[ jj + 1 ].start; kk++ ) {
                              *out = stack[ jj ].value;
                              out += outStride;
                           }
                        }
                        for( dip::uint kk = stack.back().start; kk < ii; kk++ ) {
                           *out = stack.back().value;
                           out += outStride;
                        }
                        stack.clear();
                        stack.emplace_back( ii, *in, true );
                        break;
                     }
                     // Otherwise, pop last element and repeat
                     dip::uint start = stack.back().start;
                     stack.pop_back();
                     if( stack.empty() || OP::gt( *in, stack.back().value )) {
                        // The last element on the tree is lower, add a new element to the max-tree.
                        stack.emplace_back( start, *in, false );
                        break;
                     }
                  }
               }
            }
            // Process what is left on the max-tree, this code is similar to that above, but we don't add new elements
            dip::uint start = stack.front().start; // Save in case nothing on the stack is long enough
            TPI value = stack.front().value;
            while( !stack.empty() ) {
               if( stack.back().finished || ( length - stack.back().start >= filterLength_ )) {
                  for( dip::uint jj = 0; jj < stack.size() - 1; jj++ ) {
                     for( dip::uint kk = stack[ jj ].start; kk <  stack[ jj + 1 ].start; kk++ ) {
                        *out = stack[ jj ].value;
                        out += outStride;
                     }
                  }
                  for( dip::uint kk = stack.back().start; kk < length; kk++ ) {
                     *out = stack.back().value;
                     out += outStride;
                  }
                  stack.clear();
                  return;
               }
               stack.pop_back();
            }
            // If nothing was long enough, we end up here. Fill last bit of output.
            for( dip::uint kk = start; kk < length; kk++ ) {
               *out = value;
               out += outStride;
            }
         }
      }
   private:
      dip::uint filterLength_;
      std::vector< std::vector< QElem >> stack_; // one for each thread
};

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewOpeningLineFilter( dip::uint filterLength ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>( new OpeningLineFilter< TPI, OperatorOpening< TPI >>( filterLength ));
}

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewClosingLineFilter( dip::uint filterLength ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>( new OpeningLineFilter< TPI, OperatorClosing< TPI >>( filterLength ));
}

*/

template< typename TPI, typename OP1, typename OP2 >
class OpeningLineFilter : public Framework::SeparableLineFilter {
   public:
      // NOTE! This filter needs input and output buffers to be distinct only for the brute-force version (filterLength <= 3)
      OpeningLineFilter( UnsignedArray const& filterLengths, dip::uint maxSize, BoundaryConditionArray const& bc ) :
            erosion_( filterLengths, Mirror::NO, maxSize ), dilation_( filterLengths, Mirror::YES, maxSize ),
            filterLengths_( filterLengths ), maxSize_( maxSize ), boundaryCondition_( bc ) {
         // Exactly one of `filterLengths_` is larger than 1, find it.
         filterLength_ = 0;
         for( dip::uint ii = 0; ii < filterLengths_.size(); ++ii ) {
            filterLength_ = std::max( filterLength_, filterLengths_[ ii ] );
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         if( filterLength_ > 3 ) {
            erosion_.SetNumberOfThreads( threads );
            dilation_.SetNumberOfThreads( threads );
            buffer_.resize( threads );
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return erosion_.GetNumberOfOperations( lineLength, 0, 0, 0 ) +
                dilation_.GetNumberOfOperations( lineLength, 0, 0, 0 );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         dip::uint length = params.inBuffer.length;
         dip::uint margin = params.inBuffer.border;
         if( filterLength_ > 3 ) {
            std::vector< TPI >& buffer = buffer_[ params.thread ];
            buffer.resize( std::max( maxSize_, length ) + 2 * margin ); // does nothing if already correct size
            TPI* tmp = buffer.data() + margin;
            Framework::SeparableBuffer tmpBuffer{ tmp, length, margin, 1, 0, 1 };
            Framework::SeparableLineFilterParameters paramsTmpIn{
                  params.inBuffer,
                  tmpBuffer,
                  params.dimension,
                  params.pass,
                  params.nPasses,
                  params.position,
                  params.tensorToSpatial,
                  params.thread
            };
            erosion_.Filter( paramsTmpIn );
            if( margin > 0 ) {
               ExpandBuffer( tmpBuffer.buffer, DataType( TPI( 0 )), 1, 1, length, 1, margin, margin, boundaryCondition_[ 0 ] );
            }
            Framework::SeparableLineFilterParameters paramsTmpOut{
                  tmpBuffer,
                  params.outBuffer,
                  params.dimension,
                  params.pass,
                  params.nPasses,
                  params.position,
                  params.tensorToSpatial,
                  params.thread
            };
            dilation_.Filter( paramsTmpOut );
         } else {
            TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
            dip::sint inStride = params.inBuffer.stride;
            TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
            dip::sint outStride = params.outBuffer.stride;
            bool hasMargin = margin == filterLength_ / 2; // margin == filterLength/2 || margin == 0
            if( filterLength_ == 2 ) {
               // Brute-force computation
               if( !hasMargin ) {
                  *out = OP2::max( *in, in[ inStride ] );
                  in += inStride;
                  out += outStride;
                  length -= 2;
               }
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  // max(min(a,b),min(b,c)) == min(max(a,c),b)
                  TPI v = OP1::max( in[ -inStride ], in[ inStride ] );
                  *out = OP2::max( *in, v );
                  in += inStride;
                  out += outStride;
               }
               if( !hasMargin ) {
                  *out = OP2::max( *in, in[ -inStride ] );
               }
            } else { // filterLength_ == 3
               // Brute-force computation
               // Loop unrolling by factor 2 could save one comparison per output pixel. Not really worth the complication IMO.
               dip::sint inStride2 = 2 * inStride;
               TPI a, b, c;
               if( hasMargin ) {
                  // Initialize
                  TPI v = OP2::max( in[ 0 ], in[ inStride ] );
                  a = OP2::max( OP2::max( in[ 0 ], in[ -inStride ] ), in[ -inStride2 ] );
                  b = OP2::max( v, in[ -inStride ] );
                  c = OP2::max( v, in[ inStride2 ] );
                  *out = OP1::max( OP1::max( a, b ), c );
                  in += inStride;
                  out += outStride;
               } else {
                  // Initialize, and handle first two pixels as special case
                  if( length == 2 ) {
                     b = OP2::max( in[ 0 ], in[ inStride ] );
                     c = b;
                     // The two pixels will be written at the end
                     length = 0; // don't run the main loop
                  } else if( length == 3 ) {
                     b = OP2::max( OP2::max( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
                     c = b;
                     *out = b;
                     out += outStride;
                     // The other two pixels will be written at the end
                     length = 0; // don't run the main loop
                  } else {
                     b = OP2::max( OP2::max( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
                     *out = b;
                     in += inStride;
                     out += outStride;
                     c = OP2::max( OP2::max( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
                     *out = OP1::max( b, c );
                     in += inStride;
                     out += outStride;
                     length -= 3;
                  }
               }
               for( dip::uint ii = 1; ii < length; ++ii ) {
                  a = b;
                  b = c;
                  c = OP2::max( OP2::max( in[ 0 ], in[ inStride ] ), in[ inStride2 ] );
                  *out = OP1::max( OP1::max( a, b ), c );
                  in += inStride;
                  out += outStride;
               }
               if( !hasMargin ) {
                  // Handle last two pixels as special case
                  *out = OP1::max( b, c );
                  out += outStride;
                  *out = c;
               }
            }
         }
      }
   private:
      DilationLineFilter< TPI, OP1 > erosion_;
      DilationLineFilter< TPI, OP2 > dilation_;
      UnsignedArray const& filterLengths_;
      dip::uint filterLength_;
      dip::uint maxSize_;
      BoundaryConditionArray const& boundaryCondition_;
      std::vector< std::vector< TPI >> buffer_; // one for each thread
};

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewOpeningLineFilter(
      UnsignedArray const& filterLengths, dip::uint maxSize, BoundaryConditionArray const& bc ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new OpeningLineFilter< TPI, OperatorErosion< TPI >, OperatorDilation< TPI >>( filterLengths, maxSize, bc ));
}

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewClosingLineFilter(
      UnsignedArray const& filterLengths, dip::uint maxSize, BoundaryConditionArray const& bc ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new OpeningLineFilter< TPI, OperatorDilation< TPI >, OperatorErosion< TPI >>( filterLengths, maxSize, bc ));
}

template< typename TPI, typename OP1, typename OP2 >
class PeriodicOpeningLineFilter : public Framework::SeparableLineFilter {
   public:
      // NOTE! This filter does not need input and output buffers to be distinct
      PeriodicOpeningLineFilter( dip::uint stepSize, dip::uint filterLength, dip::uint maxSize, BoundaryConditionArray const& bc ) :
            erosion_( stepSize, filterLength, Mirror::NO, maxSize ), dilation_( stepSize, filterLength, Mirror::YES, maxSize ),
            filterLength_( filterLength ), maxSize_( maxSize ), boundaryCondition_( bc ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         erosion_.SetNumberOfThreads( threads );
         dilation_.SetNumberOfThreads( threads );
         buffer_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return erosion_.GetNumberOfOperations( lineLength, 0, 0, 0 ) +
                dilation_.GetNumberOfOperations( lineLength, 0, 0, 0 );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         dip::uint length = params.inBuffer.length;
         dip::uint margin = params.inBuffer.border;
         std::vector< TPI >& buffer = buffer_[ params.thread ];
         buffer.resize( std::max( maxSize_, length ) + 2 * margin ); // does nothing if already correct size
         TPI* tmp = buffer.data() + margin;
         Framework::SeparableBuffer tmpBuffer{ tmp, length, margin, 1, 0, 1 };
         Framework::SeparableLineFilterParameters paramsTmpIn {
               params.inBuffer,
               tmpBuffer,
               params.dimension,
               params.pass,
               params.nPasses,
               params.position,
               params.tensorToSpatial,
               params.thread
         };
         erosion_.Filter( paramsTmpIn );
         if( margin > 0 ) {
            ExpandBuffer( tmpBuffer.buffer, DataType( TPI( 0 )), 1, 1, length, 1, margin, margin, boundaryCondition_[ 0 ] );
         }
         Framework::SeparableLineFilterParameters paramsTmpOut {
               tmpBuffer,
               params.outBuffer,
               params.dimension,
               params.pass,
               params.nPasses,
               params.position,
               params.tensorToSpatial,
               params.thread
         };
         dilation_.Filter( paramsTmpOut );
      }
   private:
      PeriodicDilationLineFilter< TPI, OP1 > erosion_;
      PeriodicDilationLineFilter< TPI, OP2 > dilation_;
      dip::uint filterLength_;
      dip::uint maxSize_;
      BoundaryConditionArray const& boundaryCondition_;
      std::vector< std::vector< TPI >> buffer_; // one for each thread
};

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewPeriodicOpeningLineFilter(
      dip::uint stepSize, dip::uint filterLength, dip::uint maxSize, BoundaryConditionArray const& bc ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new PeriodicOpeningLineFilter< TPI, OperatorErosion< TPI >, OperatorDilation< TPI >>( stepSize, filterLength, maxSize, bc ));
}

template< typename TPI >
inline std::unique_ptr< Framework::SeparableLineFilter > NewPeriodicClosingLineFilter(
      dip::uint stepSize, dip::uint filterLength, dip::uint maxSize, BoundaryConditionArray const& bc ) {
   return static_cast< std::unique_ptr< Framework::SeparableLineFilter >>(
         new PeriodicOpeningLineFilter< TPI, OperatorDilation< TPI >, OperatorErosion< TPI >>( stepSize, filterLength, maxSize, bc ));
}

// --- Rectangular morphology ---

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
         if( !bc.empty() ) {
            // If the boundary condition is default, we don't need a boundary extension at all.
            border[ ii ] = sizes[ ii ] / 2;
         }
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
   } else {
      DIP_START_STACK_TRACE
         switch( operation ) {
            case BasicMorphologyOperation::DILATION:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewDilationLineFilter, ( sizes, mirror, 0 ), ovltype );
               Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               break;
            case BasicMorphologyOperation::EROSION:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewErosionLineFilter, ( sizes, mirror, 0 ), ovltype );
               Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               break;
            case BasicMorphologyOperation::CLOSING:
               if( nProcess == 1 ) {
                  DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewClosingLineFilter, ( sizes, 0, bc ), ovltype );
                  Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               } else {
                  DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewDilationLineFilter, ( sizes, mirror, 0 ), ovltype );
                  Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
                  DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewErosionLineFilter, ( sizes, InvertMirrorParam( mirror ), 0 ), ovltype );
                  Framework::Separable( out, out, dtype, dtype, process, border, bc, *lineFilter );
               }
               break;
            case BasicMorphologyOperation::OPENING:
               if( nProcess == 1 ) {
                  DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewOpeningLineFilter, ( sizes, 0, bc ), ovltype );
                  Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
               } else {
                  DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewErosionLineFilter, ( sizes, mirror, 0 ), ovltype );
                  Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter );
                  DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewDilationLineFilter, ( sizes, InvertMirrorParam( mirror ), 0 ), ovltype );
                  Framework::Separable( out, out, dtype, dtype, process, border, bc, *lineFilter );
               }
               break;
         }
      DIP_END_STACK_TRACE
   }
}

// --- Line morphology ---

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
   // This is the general idea for this algorithm:
   //  - We find the (image-wide) Bresenham line that has the angle given by filterParam.
   //  - We make sure that this line has unit steps along the x-axis, and negative steps along all other axes.
   //    (This can be accomplished by swapping and mirroring dimensions.)
   //  - To tessellate the image with this line, we need to always start it at x=0.
   //  - We iterate over all coordinates that have x=0 (i.e. we iterate over all image lines), but including
   //    coordinates outside of the image domain, such that part of the line still touches the image domain.
   //  - At each of these positions, we can copy the input pixels into a buffer, and copy the output pixels back.
   //  - If the angle is such that steps in all dimensions are either 0 or 1, we can define a stride to reach each
   //    pixel along the line, and don't need to use the buffers.
   //  - When walking along a line that starts outside the image domain, we can compute at which x-position the
   //    rounded coordinates will fall within the image domain.
   //  - Likewise, we can compute at which x-position the rounded coordinates will exit the image domain.
   if( bc.size() > 1 ) {
      // TODO: Make sure they're all the same? Does this matter?
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
      //std::cout << "periodic line: periodicStepSize = " << periodicStepSize << ", nSteps = " << nSteps << std::endl;
   }

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

   // Find the line filter to use
   dip::uint maxLineLength = in.Size( 0 );
   dip::uint filterLength = static_cast< dip::uint >( length );
   DataType dtype = in.DataType();
   DataType ovltype = dtype;
   if( ovltype.IsBinary() ) {
      ovltype = DT_UINT8; // Dirty trick: process a binary image with the same filter as a UINT8 image, but don't convert the type -- for some reason this is faster!
   }
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   UnsignedArray sizes( 1, filterLength ); // This needs to be kept alive, DilationLineFilter holds a reference to it
   bool requireBuffer = false; // for some line filters, input and output must be distinct, so we need at least one buffer
   if( mode == StructuringElement::ShapeCode::PERIODIC_LINE ) {
      DIP_START_STACK_TRACE
         switch( operation ) {
            case BasicMorphologyOperation::DILATION:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewPeriodicDilationLineFilter, ( periodicStepSize, filterLength, mirror, maxLineLength ), ovltype );
               requireBuffer = filterLength / periodicStepSize <= 3;
               break;
            case BasicMorphologyOperation::EROSION:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewPeriodicErosionLineFilter, ( periodicStepSize, filterLength, mirror, maxLineLength ), ovltype );
               requireBuffer = filterLength / periodicStepSize <= 3;
               break;
            case BasicMorphologyOperation::CLOSING:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewPeriodicClosingLineFilter, ( periodicStepSize, filterLength, maxLineLength, bc ), ovltype );
               break;
            case BasicMorphologyOperation::OPENING:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewPeriodicOpeningLineFilter, ( periodicStepSize, filterLength, maxLineLength, bc ), ovltype );
               break;
         }
      DIP_END_STACK_TRACE
   } else { // mode == StructuringElement::ShapeCode::FAST_LINE
      DIP_START_STACK_TRACE
         requireBuffer = filterLength / periodicStepSize <= 3; // All of these line filters need input and output buffers to be distinct for small SEs.
         switch( operation ) {
            case BasicMorphologyOperation::DILATION:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewDilationLineFilter, ( sizes, mirror, maxLineLength ), ovltype );
               break;
            case BasicMorphologyOperation::EROSION:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewErosionLineFilter, ( sizes, mirror, maxLineLength ), ovltype );
               break;
            case BasicMorphologyOperation::CLOSING:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewClosingLineFilter, ( sizes, maxLineLength, bc ), ovltype );
               break;
            case BasicMorphologyOperation::OPENING:
               DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewOpeningLineFilter, ( sizes, maxLineLength, bc ), ovltype );
               break;
         }
      DIP_END_STACK_TRACE
   }

   // Initialize the line filter (we don't do multithreading here)
   lineFilter->SetNumberOfThreads( 1 );

   // Determine parameters for buffers
   dip::uint border = 0;
   if( !bc.empty() ) {
      // If the boundary condition is default, we don't need a boundary extension at all.
      border = filterLength / 2;
   }
   dip::uint sizeOf = dtype.SizeOf();

   // Create input buffer data struct and allocate buffer
   dip::uint inBufferSize = ( maxLineLength + 2 * border ) * sizeOf;
   std::vector< uint8 > inBuffer;
   Framework::SeparableBuffer inBufferStruct;
   inBufferStruct.tensorLength = 1;
   inBufferStruct.tensorStride = 1;
   inBufferStruct.border = border;
   bool useInBuffer = requireBuffer || !processDiagonally || ( border > 0 );
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

   Framework::SeparableLineFilterParameters params1{ inBufferStruct, outBufferStruct, 0, 0, 1, {}, false, 0 };

   constexpr dfloat epsilon = 1e-5;
   constexpr dfloat delta = 1.0 - epsilon;

   // Compute how far out we need to go along dimensions 1..nDims-1 so that our tessellated lines cover the whole image
   //
   // The equation for the coordinates `(x,y)` is:
   //    y = n + floor( delta + x * stepSize )
   // where `n` is the start y coordinate (that is, the line starts at `(0,n)`).
   // For more than 2 dimensions, `y` -> `pos(ii)` and `stepSize` -> `stepSize(ii)`, with `ii > 0`. `x` is always `x`.
   //
   // From this equation it is easy to compute `n` given `y` = `in.Size(1)-1` and `x` = `in.Size(0)-1`. That is the
   // last `n` to use. `itSizes` is the number of start positions, much like image size, and is given by `n+1`.
   UnsignedArray itSizes( nDims, 0 );
   for( dip::uint ii = 1; ii < nDims; ++ii ) {
      itSizes[ ii ] =  in.Size( ii ) - static_cast< dip::uint >( std::floor( delta + static_cast< dfloat >( in.Size( 0 ) - 1 ) * stepSize[ ii ] ));
   }

   // Iterate over itSizes
   dip::sint inOffset = 0;
   dip::sint outOffset = 0;
   dip::sint ssizeOf = static_cast< dip::sint >( sizeOf );
   UnsignedArray coords( nDims, 0 );      // These are the start coordinates for the Bresenham line
   FloatArray bresenhamCoords( nDims );   // These are the coordinates to round to get the Bresenham line
   FloatArray bresenhamCoords2( nDims );  // These are the coordinates to round to get the Bresenham line
   IntegerArray inStridesBytes;
   if( useInBuffer ) {
      inStridesBytes = in.Strides();
      for( auto& s: inStridesBytes ) {
         s *= ssizeOf;
      }
   }
   IntegerArray outStridesBytes;
   if( useOutBuffer ) {
      outStridesBytes = out.Strides();
      for( auto& s: outStridesBytes ) {
         s *= ssizeOf;
      }
   }
   uint8* inOrigin = static_cast< uint8* >( in.Origin() );
   uint8* outOrigin = static_cast< uint8* >( out.Origin() );
   while( true ) {

      // Determine the start and end x-coordinate for this line
      dip::uint start = 0;
      dip::uint end = in.Size( 0 ) - 1;
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         // Computing `start` and `end`:
         // For each dimension (ii>0), `start` can increase if necessary, and `end` can decrease.
         // Given the line equation as above, we need to compute the first integer `x` for which
         //    y = n + floor( delta + x * stepSize(1) ) == in.Size(1)
         // (note there can be multiple values of `x` satisfying this equation). We compute:
         //    x = ceil( ( n - in.Size(1) + delta ) / -stepSize(1) ).
         // We do this for each dimension independently.
         // For `end` we do the same thing. We compute the first integer `x` for which `y` == -1,
         // using the same math, then subtract one. That is the last integer `x` for which `y` == 0.
         // NOTE: stepSize(ii) <= 0 for any ii>0.
         dfloat x;
         if( coords[ ii ] >= in.Size( ii )) {
            x = ( static_cast< dfloat >( coords[ ii ] - in.Size( ii )) + delta ) / -stepSize[ ii ];
            start = std::max( start, static_cast< dip::uint >( std::ceil( x )));
         } // otherwise the line starts within the image domain
         x = ( static_cast< dfloat >( coords[ ii ] ) + delta ) / -stepSize[ ii ];
         end = std::min( end, static_cast< dip::uint >( std::ceil( x )) - 1 );
      }
      DIP_ASSERT( start <= end );

      // Find offsets for the start coordinates
      bresenhamCoords[ 0 ] = static_cast< dfloat >( start );
      dip::sint inOffsetStart = inOffset + static_cast< dip::sint >( start ) * in.Stride( 0 );
      dip::sint outOffsetStart = outOffset + static_cast< dip::sint >( start ) * out.Stride( 0 );
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         bresenhamCoords[ ii ] = delta + bresenhamCoords[ 0 ] * stepSize[ ii ];
         inOffsetStart += static_cast< dip::sint >( std::floor( bresenhamCoords[ ii ] )) * in.Stride( ii );
         outOffsetStart += static_cast< dip::sint >( std::floor( bresenhamCoords[ ii ] )) * out.Stride( ii );
         bresenhamCoords[ ii ] += static_cast< dfloat >( coords[ ii ] );
         //DIP_ASSERT( static_cast< dip::uint >( std::floor( bresenhamCoords[ ii ] )) < in.Size( ii ));
      }

      if( start == end ) {
         // Short-cut: the line has a single pixel, let's just copy the pixel from input to output
         if( inOrigin != outOrigin ) {
            uint8* src = inOrigin + inOffsetStart * ssizeOf;
            uint8* dest = outOrigin + outOffsetStart * ssizeOf;
            std::memcpy( dest, src, sizeOf );
         }
      } else {

         // Prepare line filter parameters
         dip::uint lineLength = end - start + 1;
         inBufferStruct.length = lineLength;
         outBufferStruct.length = lineLength;
         if( useOutBuffer ) {
            bresenhamCoords2 = bresenhamCoords; // copy before we modify it
         } else {
            outBufferStruct.buffer = outOrigin + outOffsetStart * ssizeOf;
         }

         // Copy from input image to input buffer
         if( useInBuffer ) {
            uint8* src = inOrigin + inOffsetStart * ssizeOf;
            uint8* dest = static_cast< uint8* >( inBufferStruct.buffer );
            for( dip::uint ss = 0; ss < lineLength; ++ss ) {
               std::memcpy( dest, src, sizeOf );
               dest += sizeOf;
               src += inStridesBytes[ 0 ];
               for( dip::uint ii = 1; ii < nDims; ++ii ) {
                  dfloat old = std::floor( bresenhamCoords[ ii ] );
                  bresenhamCoords[ ii ] += stepSize[ ii ];
                  if( std::floor( bresenhamCoords[ ii ] ) != old ) {
                     src -= inStridesBytes[ ii ]; // we're always moving towards smaller coordinates
                  }
               }
            }
            if( border > 0 ) {
               ExpandBuffer( inBufferStruct.buffer, ovltype, 1, 1, lineLength, 1, border, border, bc[ 0 ] );
            }
         } else {
            inBufferStruct.buffer = inOrigin + inOffsetStart * ssizeOf;
         }

         // Execute the line filter
         DIP_STACK_TRACE_THIS( lineFilter->Filter( params1 ));

         // Copy output buffer to output image
         if( useOutBuffer ) {
            uint8* src = static_cast< uint8* >( outBufferStruct.buffer );
            uint8* dest = outOrigin + outOffsetStart * ssizeOf;
            for( dip::uint ss = 0; ss < lineLength; ++ss ) {
               std::memcpy( dest, src, sizeOf );
               src += sizeOf;
               dest += outStridesBytes[ 0 ];
               for( dip::uint ii = 1; ii < nDims; ++ii ) {
                  dfloat old = std::floor( bresenhamCoords2[ ii ] );
                  bresenhamCoords2[ ii ] += stepSize[ ii ];
                  if( std::floor( bresenhamCoords2[ ii ] ) != old ) {
                     dest -= outStridesBytes[ ii ]; // we're always moving towards smaller coordinates
                  }
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

} // namespace detail

} // namespace dip
