/*
 * (c)2016-2017, Cris Luengo.
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
#include "diplib/framework.h"

namespace dip {
namespace Framework {

// Part of the next two functions
void SingletonExpandedSize(
      UnsignedArray& size,
      UnsignedArray const& size2
) {
   if( size.size() < size2.size() ) {
      size.resize( size2.size(), 1 );
   }
   for( dip::uint jj = 0; jj < size2.size(); ++jj ) {
      if( size[ jj ] != size2[ jj ] ) {
         if( size[ jj ] == 1 ) {
            size[ jj ] = size2[ jj ];
         } else if( size2[ jj ] != 1 ) {
            DIP_THROW( E::SIZES_DONT_MATCH );
         }
      }
   }
}

// Figure out what the size of the images must be.
UnsignedArray SingletonExpandedSize(
      ImageConstRefArray const& in
) {
   UnsignedArray size = in[ 0 ].get().Sizes();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      UnsignedArray size2 = in[ ii ].get().Sizes();
      SingletonExpandedSize( size, size2 );
   }
   return size;
}

// Idem as above.
UnsignedArray SingletonExpandedSize(
      ImageArray const& in
) {
   UnsignedArray size = in[ 0 ].Sizes();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      UnsignedArray size2 = in[ ii ].Sizes();
      SingletonExpandedSize( size, size2 );
   }
   return size;
}

dip::uint SingletonExpendedTensorElements(
      ImageArray const& in
) {
   dip::uint tsize = in[ 0 ].TensorElements();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      dip::uint tsize2 = in[ ii ].TensorElements();
      if( tsize != tsize2 ) {
         if( tsize == 1 ) {
            tsize = tsize2;
         } else if( tsize2 != 1 ) {
            DIP_THROW( E::SIZES_DONT_MATCH );
         }
      }
   }
   return tsize;
}


static dip::uint OptimalProcessingDim_internal(
      UnsignedArray const& sizes,
      IntegerArray const& strides
) {
   constexpr dip::uint SMALL_IMAGE = 63;  // A good value would depend on the size of cache.
   dip::uint processingDim = 0;
   for( dip::uint ii = 1; ii < strides.size(); ++ii ) {
      if(( strides[ ii ] != 0 ) && ( std::abs( strides[ ii ] ) < std::abs( strides[ processingDim ] ))) {
         if( ( sizes[ ii ] > SMALL_IMAGE ) || ( sizes[ ii ] > sizes[ processingDim ] ) ) {
            processingDim = ii;
         }
      } else if( ( sizes[ processingDim ] <= SMALL_IMAGE ) && ( sizes[ ii ] > sizes[ processingDim ] ) ) {
         processingDim = ii;
      }
   }
   return processingDim;
}

// Find best processing dimension, which is the one with the smallest stride,
// except if that dimension is very small and there's a longer dimension.
dip::uint OptimalProcessingDim(
      Image const& in
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   return OptimalProcessingDim_internal( in.Sizes(), in.Strides() );
}

// Find the best processing dimension as above, but giving preference to a dimension
// where `kernelSizes` is large also.
dip::uint OptimalProcessingDim(
      Image const& in,
      UnsignedArray const& kernelSizes
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray sizes = in.Sizes();
   DIP_THROW_IF( sizes.size() != kernelSizes.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      if( kernelSizes[ ii ] == 1 ) {
         sizes[ ii ] = 1; // this will surely force the algorithm to not return this dimension as optimal processing dimension
      }
   }
   // TODO: a kernel of 1000x2 should definitely return the dimension where it's 1000 as the optimal dimension. Or?
   return OptimalProcessingDim_internal( sizes, in.Strides() );
}

} // namespace Framework
} // namespace dip
