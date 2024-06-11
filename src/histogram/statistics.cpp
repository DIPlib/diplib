/*
 * (c)2017-2021, Cris Luengo.
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

#include <algorithm>
#include "diplib/histogram.h"

#include "diplib.h"

namespace dip {

FloatArray Mean( Histogram const& in ) {
   DIP_THROW_IF( !in.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   dip::uint nDims = in.Dimensionality();
   FloatArray mean( nDims, 0 );
   dfloat weight = 0;
   std::vector< FloatArray > binCenters( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      binCenters[ ii ] = in.BinCenters( ii );
   }
   ImageIterator< Histogram::CountType > it( in.GetImage() );
   // Histogram always has normal strides, it.Optimize() would not do anything here.
   do {
      UnsignedArray const& coord = it.Coordinates();
      dfloat v = static_cast< dfloat >( *it );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         dfloat bin = binCenters[ ii ][ coord[ ii ]];
         mean[ ii ] += bin * v;
      }
      weight += v;
   } while( ++it );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      mean[ ii ] /= weight;
   }
   return mean;
}

namespace {

FloatArray Covariance( Histogram const& in, FloatArray& mean ) {
   dip::uint nDims = in.Dimensionality();
   dfloat weight = 0;
   FloatArray cov( nDims * ( nDims + 1 ) / 2, 0 );
   std::vector< FloatArray > binCenters( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      binCenters[ ii ] = in.BinCenters( ii );
   }
   ImageIterator< Histogram::CountType > it( in.GetImage() );
   // Histogram always has normal strides, it.Optimize() would not do anything here.
   FloatArray diff( nDims, 0 );
   do {
      UnsignedArray const& coord = it.Coordinates();
      dfloat w = static_cast< dfloat >( *it );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         diff[ ii ] = binCenters[ ii ][ coord[ ii ]] - mean[ ii ];
      }

      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         cov[ ii ] += diff[ ii ] * diff[ ii ] * w;
      }
      dip::uint index = nDims;
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            cov[ index ] += diff[ ii ] * diff[ jj ] * w;
            ++index;
         }
      }
      weight += w;
   } while( ++it );
   dfloat norm = 1.0 / ( weight - 1 );
   for( auto& c : cov ) {
      c *= norm;
   }
   return cov;
}

} // namespace

FloatArray Covariance( Histogram const& in ) {
   DIP_THROW_IF( !in.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   FloatArray mean = Mean( in );
   return Covariance( in, mean );
}

RegressionParameters Regression( Histogram const& in ) {
   DIP_THROW_IF( !in.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   FloatArray mean = Mean( in );
   FloatArray cov = Covariance( in, mean );
   RegressionParameters out;
   out.slope = ( cov[ 0 ] != 0.0 ) ? ( cov[ 2 ] / cov[ 0 ] ) : ( 0.0 );
   out.intercept = mean[ 1 ] - out.slope * mean[ 0 ];
   return out;
}

FloatArray MarginalPercentile( Histogram const& in, dfloat percentile ) {
   DIP_THROW_IF( !in.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   dip::uint nDims = in.Dimensionality();
   FloatArray output( nDims );
   Histogram cum = CumulativeHistogram( in ); // we look along the last line in each direction
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
      while( static_cast< dfloat >( *pcum ) / n < percentile / 100 ) {
         ++jj;
         pcum += stride;
      }
      output[ ii ] = in.BinCenter( static_cast< dip::uint >( jj ), ii );
   }
   return output;
}

FloatArray Mode( Histogram const& in ) {
   DIP_THROW_IF( !in.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   dip::uint nDims = in.Dimensionality();
   UnsignedArray coord( nDims, 0 );
   Histogram::CountType maxVal = 0;
   ImageIterator< Histogram::CountType > it( in.GetImage() );
   // Histogram always has normal strides, it.Optimize() would not do anything here.
   do {
      if( *it > maxVal ) {
         maxVal = *it;
         coord = it.Coordinates();
      }
   } while( ++it );
   FloatArray mode( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      mode[ ii ] = in.BinCenter( coord[ ii ], ii );
   }
   return mode;
}

dfloat MutualInformation( Histogram const& hist ) {
   DIP_THROW_IF( !hist.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   DIP_THROW_IF( hist.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );

   Image const& histImg = hist.GetImage();
   dip::uint n1 = histImg.Size( 0 );
   dip::uint n2 = histImg.Size( 1 );
   // Make sure data is contiguous as we'd expect from default strides:
   DIP_ASSERT( histImg.Stride( 0 ) == 1 );
   DIP_ASSERT( histImg.Stride( 1 ) == static_cast< dip::sint >( n1 ));

   Histogram c1 = hist.GetMarginal( 0 );
   Image const& c1Img = c1.GetImage();
   DIP_ASSERT( c1Img.Dimensionality() == 1 );
   DIP_ASSERT( c1Img.NumberOfPixels() == n1 );
   DIP_ASSERT( c1Img.Stride( 0 ) == 1 );

   Histogram c2 = hist.GetMarginal( 1 );
   Image const& c2Img = c2.GetImage();
   DIP_ASSERT( c2Img.Dimensionality() == 1 );
   DIP_ASSERT( c2Img.NumberOfPixels() == n2 );
   DIP_ASSERT( c2Img.Stride( 0 ) == 1 );

   dfloat norm = 1.0 / static_cast< dfloat >( hist.Count() );
   dfloat out = 0.0;
   Histogram::CountType const* hPtr = static_cast< Histogram::CountType const* >( histImg.Origin() );
   Histogram::CountType const* c2Ptr = static_cast< Histogram::CountType const* >( c2Img.Origin() );
   for( dip::uint jj = 0; jj < n2; jj++ ) {
      Histogram::CountType const* c1Ptr = static_cast< Histogram::CountType const* >( c1Img.Origin() );
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
   DIP_THROW_IF( !hist.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
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

std::vector< GaussianParameters > GaussianMixtureModel(
      Histogram const& in,
      dip::uint numberOfGaussians,
      dip::uint maxIter
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   auto pData = static_cast< Histogram::CountType const* >( hist.Origin() );
   // Copy data and apply mixture model
   std::vector< dfloat > data( nBins );
   std::transform( pData, pData + nBins, data.data(), []( Histogram::CountType v ){ return static_cast< dfloat >( v ); } );
   std::vector< GaussianParameters > params = GaussianMixtureModel( data.data(), nullptr, nBins, numberOfGaussians, maxIter );
   // Scale data
   dfloat scale = in.BinSize();
   dfloat offset = in.LowerBound() + scale / 2; // bin[ii] = offset + ii * scale;
   std::cout << "scale = " << scale << ", offset = " << offset << '\n';
   for( auto& p : params ) {
      p.position = p.position * scale + offset;
      p.sigma *= scale;
   }
   return params;
}

} // namespace dip
