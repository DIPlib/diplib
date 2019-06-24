/*
 * DIPlib 3.0
 * This file contains the thin plate spline functionality
 *
 * (c)2019, Cris Luengo.
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

#include "diplib/library/numeric.h"

#if defined(__GNUG__) || defined(__clang__)
// For this file, turn off -Wsign-conversion, Eigen is really bad at this!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#if __GNUC__ >= 7
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#endif
#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#endif

#include <Eigen/Dense>
#include <Eigen/QR>

namespace dip {

namespace {

dfloat RadialBasis( dfloat r ) {
   return r > 0 ? r * r * std::log( r ) : 0;
}

} // namespace

ThinPlateSpline::ThinPlateSpline(
      FloatCoordinateArray coordinate,    // use std::move if you can!
      FloatCoordinateArray const& value,  // correspondence points
      dfloat lambda
) {
   // NOTE: `source` and `destination` are already checked for sizes
   c_ = std::move( coordinate );
   dip::uint nPoints = c_.size();
   dip::uint nDims = c_[ 0 ].size();

   // Create matrices L and b
   DIP_ASSERT( value.size() == nPoints );
   dip::uint N = nPoints + nDims + 1;
   Eigen::MatrixXd L( N, N );
   L.fill( 0 );
   Eigen::MatrixXd b( N, nDims );
   b.fill( 0 );
   dfloat alpha = 0;
   for( dip::uint ii = 0; ii < nPoints; ++ii ) {
      for( dip::uint jj = 0; jj < ii; ++jj ) {
         L( ii, jj ) = L( jj, ii ); // Previously computed, L is symmetric
      }
      for( dip::uint jj = ii + 1; jj < nPoints; ++jj ) {
         dfloat d = Distance( c_[ ii ], c_[ jj ] );
         L( ii, jj ) = RadialBasis( d );
         alpha += d;
      }
      L( ii, nPoints ) = 1;
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         L( ii, nPoints + 1 + jj ) = c_[ ii ][ jj ];
         b( ii, jj ) = value[ ii ][ jj ] - c_[ ii ][ jj ];
      }
   }
   if( lambda > 0.0 ) {
      alpha /= static_cast< dfloat >( nPoints * ( nPoints - 1 ) / 2 );
      alpha *= alpha * lambda;
      for( dip::uint ii = 0; ii < nPoints; ++ii ) {
         L( ii, ii ) = alpha;
      }
   }
   for( dip::uint jj = 0; jj < nDims + 1; ++jj ) {
      L.row( nPoints + jj ) = L.col( nPoints + jj );
   }

   // Solve equation Lx=b for x
   Eigen::ColPivHouseholderQR <Eigen::Ref< Eigen::MatrixXd >> decomposition( L );
   // Using Eigen::Ref to get in-place decomposition, it re-uses L to store the decomposition.
   // Eigen::HouseholderQR is faster but less accurate. Which one to pick?
   x_.resize( N * nDims );
   Eigen::Map< Eigen::MatrixXd >( x_.data(), N, nDims ) = decomposition.solve( b );
}

// Evaluates the thin plate spline function at point `pt`.
FloatArray ThinPlateSpline::Evaluate( FloatArray const& pt ) {
   dip::uint nPoints = c_.size();
   dip::uint nDims = c_[ 0 ].size();
   dip::uint N = nPoints + nDims + 1;
   Eigen::Map <Eigen::MatrixXd> x( x_.data(), N, nDims );
   // Note: w( ii, jj ) = x( ii, jj ), and a( ii, jj ) = x( nPoints + ii, jj )
   FloatArray res = pt;
   for( dip::uint ii = 0; ii < nPoints; ++ii ) {
      dfloat scale = RadialBasis( Distance( pt, c_[ ii ] ));
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         res[ jj ] += x( ii, jj ) * scale;
      }
   }
   for( dip::uint jj = 0; jj < nDims; ++jj ) {
      res[ jj ] += x( nPoints, jj );
   }
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         res[ jj ] += x( nPoints + 1 + ii, jj ) * pt[ ii ];
      }
   }
   return res;
}

} // namespace dip

#if defined(__GNUG__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
