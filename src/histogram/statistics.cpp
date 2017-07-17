/*
 * DIPlib 3.0
 * This file contains definitions for histogram-related functionality.
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
#include "diplib/histogram.h"

namespace dip {

Image::CastPixel< dfloat > Mean( Histogram const& in ) {
   dip::uint nDims = in.Dimensionality();
   Image::CastPixel< dfloat > mean( DT_DFLOAT, nDims );
   mean = 0;
   dfloat* pmean = static_cast< dfloat* >( mean.Origin() );
   dfloat weight = 0;
   std::vector< FloatArray > binCenters( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      binCenters[ ii ] = in.BinCenters( ii );
   }
   ImageIterator< Histogram::CountType > it( in.GetImage() );
   do {
      UnsignedArray const& coord = it.Coordinates();
      dfloat v = static_cast< dfloat >( *it );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         dfloat bin = binCenters[ ii ][ coord[ ii ] ];
         pmean[ ii ] += bin * v;
      }
      weight += v;
   } while( ++it );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      pmean[ ii ] /= weight;
   }
   return mean;
}

Image::CastPixel< dfloat > Covariance( Histogram const& in ) {
   dip::uint nDims = in.Dimensionality();
   Tensor tensor( Tensor::Shape::SYMMETRIC_MATRIX, nDims, nDims );
   Image::CastPixel< dfloat > mean = Mean( in );
   dfloat* pmean = static_cast< dfloat* >( mean.Origin() );
   dfloat weight = 0;
   Image::CastPixel< dfloat > cov( DT_DFLOAT, tensor.Elements() );
   cov.ReshapeTensor( tensor );
   std::vector< dip::sint > lut = tensor.LookUpTable();
   cov = 0;
   dfloat* pcov = static_cast< dfloat* >( cov.Origin() );
   std::vector< FloatArray > binCenters( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      binCenters[ ii ] = in.BinCenters( ii );
   }
   ImageIterator< Histogram::CountType > it( in.GetImage() );
   FloatArray diff( nDims, 0 );
   do {
      UnsignedArray const& coord = it.Coordinates();
      dfloat w = static_cast< dfloat >( *it );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         diff[ ii ] = binCenters[ ii ][ coord[ ii ] ] - pmean[ ii ];
      }
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         for( dip::uint jj = 0; jj <= ii; ++jj ) {
            pcov[ static_cast< dip::uint >( lut[ ii * nDims + jj ] ) ] += diff[ ii ] * diff[ jj ] * w;
         }
      }
      weight += w;
   } while( ++it );
   dfloat norm = 1.0 / ( weight - 1 );
   for( dip::uint ii = 0; ii < cov.TensorElements(); ++ii ) {
      pcov[ ii ] *= norm;
   }
   return cov;
}

Image::CastPixel< dfloat > MarginalMedian( Histogram const& in ) {
   dip::uint nDims = in.Dimensionality();
   Image::CastPixel< dfloat > median( DT_DFLOAT, nDims );
   dfloat* pmedian = static_cast< dfloat* >( median.Origin() );
   Histogram cum = in.Cumulative(); // we look along the last line in each direction
   Image const& cumImg = cum.GetImage();
   Histogram::CountType* pcum = static_cast< Histogram::CountType* >( cumImg.Origin() );
   dfloat n = static_cast< dfloat >( pcum[ cumImg.NumberOfPixels() - 1 ] );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      pcum = static_cast< Histogram::CountType* >( cumImg.Origin() );
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         if( jj != ii ) {
            pcum += static_cast< dip::sint >( cumImg.Size( jj ) - 1 ) * cumImg.Stride( jj );
         }
      }
      dip::sint stride = cumImg.Stride( ii );
      dip::sint jj = 0;
      while( static_cast< dfloat >( *pcum ) / n < 0.5 ) {
         ++jj;
         pcum += stride;
      }
      pmedian[ ii ] = in.BinCenter( static_cast< dip::uint >( jj ), ii );
   }
   return median;
}

Image::CastPixel< dfloat > Mode( Histogram const& in ) {
   dip::uint nDims = in.Dimensionality();
   UnsignedArray coord( nDims, 0 );
   Histogram::CountType maxVal = 0;
   ImageIterator< Histogram::CountType > it( in.GetImage() );
   do {
      if( *it > maxVal ) {
         maxVal = *it;
         coord = it.Coordinates();
      }
   } while( ++it );
   Image::CastPixel< dfloat > mode( DT_DFLOAT, nDims );
   dfloat* pmode = static_cast< dfloat* >( mode.Origin() );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      pmode[ ii ] = in.BinCenter( coord[ ii ], ii );
   }
   return mode;
}

dfloat MutualInformation( Histogram const& hist ) {
   DIP_THROW_IF( hist.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );

   Image const& histImg = hist.GetImage();
   dip::uint n1 = histImg.Size( 0 );
   dip::uint n2 = histImg.Size( 1 );
   // Make sure data is contiguous as we'd expect from default strides:
   DIP_ASSERT( histImg.Stride( 0 ) == 1 );
   DIP_ASSERT( histImg.Stride( 1 ) == static_cast< dip::sint >( n1 ));

   Histogram c1 = hist.Marginal( 0 );
   Image const& c1Img = c1.GetImage();
   DIP_ASSERT( c1Img.Dimensionality() == 1 );
   DIP_ASSERT( c1Img.NumberOfPixels() == n1 );
   DIP_ASSERT( c1Img.Stride( 0 ) == 1 );

   Histogram c2 = hist.Marginal( 1 );
   Image const& c2Img = c2.GetImage();
   DIP_ASSERT( c2Img.Dimensionality() == 1 );
   DIP_ASSERT( c2Img.NumberOfPixels() == n2 );
   DIP_ASSERT( c2Img.Stride( 0 ) == 1 );

   dfloat norm = 1.0 / static_cast< dfloat >( hist.Count() );
   dfloat out = 0.0;
   Histogram::CountType* hPtr = static_cast< Histogram::CountType* >( histImg.Origin() );
   Histogram::CountType* c2Ptr = static_cast< Histogram::CountType* >( c2Img.Origin() );
   for( dip::uint jj = 0; jj < n2; jj++ ) {
      Histogram::CountType* c1Ptr = static_cast< Histogram::CountType* >( c1Img.Origin() );
      for( dip::uint ii = 0; ii < n1; ii++ ) {
         if( *hPtr > 0 ) { // This means that c1 and c2 both have data here also
            // out += h * norm * log2( h * norm / ( c1 * norm * c2 * norm ) )
            // =>  out/norm += h * log2( h / ( c1 * c2 * norm ) )
            out += static_cast< dfloat >( *hPtr ) *
                   std::log2( static_cast< dfloat >( *hPtr ) / ( static_cast< dfloat >( *c1Ptr ) * static_cast< dfloat >( *c2Ptr ) * norm ));
         }
         ++hPtr;
         ++c1Ptr;
      }
      ++c2Ptr;
   }
   return out * norm;
}

dfloat Entropy( Histogram const& hist ) {
   DIP_THROW_IF( hist.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& histImg = hist.GetImage();
   dip::uint n = histImg.Size( 0 );
   dfloat norm = 1.0 / static_cast< dfloat >( hist.Count() );
   dfloat out = 0.0;
   Histogram::CountType* hPtr = static_cast< Histogram::CountType* >( histImg.Origin() );
   for( dip::uint ii = 0; ii < n; ii++ ) {
      if( *hPtr > 0 ) {
         dfloat c = static_cast< dfloat >( *hPtr ) * norm;
         out += c * std::log2( c );
      }
      ++hPtr;
   }
   return -out;
}

} // namespace dip
