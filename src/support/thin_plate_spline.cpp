/*
 * (c)2019-2022, Cris Luengo.
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

#include <cmath>
#include <utility>
#include "diplib/library/numeric.h"

#include "diplib.h"

#if defined(__GNUG__) || defined(__clang__)
   // For Eigen, turn off -Wsign-conversion
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wsign-conversion"
   #ifndef __clang__
      #pragma GCC diagnostic ignored "-Wclass-memaccess"
   #endif
   #if ( __GNUC__ >= 11 ) && ( __GNUC__ <= 14 )
      #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
   #endif
#endif

#include <Eigen/QR> // IWYU pragma: keep
#include <Eigen/Cholesky> // IWYU pragma: keep

#if defined(__GNUG__) || defined(__clang__)
   #pragma GCC diagnostic pop
#endif


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
) : c_( std::move( coordinate )) {
   // NOTE: `source` and `destination` are already checked for sizes
   dip::sint nPoints = static_cast< dip::sint >( c_.size() );
   dip::sint nDims = static_cast< dip::sint >( c_[ 0 ].size() );

   // Create matrices L and b
   DIP_ASSERT( value.size() == c_.size() );
   dip::sint N = nPoints + nDims + 1;
   Eigen::MatrixXd L( N, N );
   L.fill( 0 );
   Eigen::MatrixXd b( N, nDims );
   b.fill( 0 );
   dfloat alpha = 0;
   for( dip::sint ii = 0; ii < nPoints; ++ii ) {
      for( dip::sint jj = 0; jj < ii; ++jj ) {
         L( ii, jj ) = L( jj, ii ); // Previously computed, L is symmetric
      }
      for( dip::sint jj = ii + 1; jj < nPoints; ++jj ) {
         dfloat d = Distance( c_[ static_cast< dip::uint >( ii ) ], c_[ static_cast< dip::uint >( jj ) ] );
         L( ii, jj ) = RadialBasis( d );
         alpha += d;
      }
      L( ii, nPoints ) = 1;
      for( dip::sint jj = 0; jj < nDims; ++jj ) {
         L( ii, nPoints + 1 + jj ) = c_[ static_cast< dip::uint >( ii ) ][ static_cast< dip::uint >( jj ) ];
         b( ii, jj ) = value[ static_cast< dip::uint >( ii ) ][ static_cast< dip::uint >( jj ) ]
                       - c_[ static_cast< dip::uint >( ii ) ][ static_cast< dip::uint >( jj ) ];
      }
   }
   if( lambda > 0.0 ) {
      alpha /= static_cast< dfloat >(( nPoints * ( nPoints - 1 )) / 2 ); // NOLINT(*-integer-division)
      alpha *= alpha * lambda;
      for( dip::sint ii = 0; ii < nPoints; ++ii ) {
         L( ii, ii ) = alpha;
      }
   }
   for( dip::sint jj = 0; jj < nDims + 1; ++jj ) {
      L.row( nPoints + jj ) = L.col( nPoints + jj );
   }

   // Solve equation Lx=b for x
   // Using Eigen::Ref to get in-place decomposition, it re-uses L to store the decomposition.
   Eigen::HouseholderQR< Eigen::Ref< Eigen::MatrixXd >> decomposition( L );
   //Eigen::ColPivHouseholderQR <Eigen::Ref< Eigen::MatrixXd >> decomposition( L );
   // TODO: Eigen::HouseholderQR is faster but less accurate than Eigen::ColPivHouseholderQR. Which one to pick?
   x_.resize( static_cast< dip::uint >( N * nDims ));
   Eigen::Map< Eigen::MatrixXd >( x_.data(), N, nDims ) = decomposition.solve( b );
}

// Evaluates the thin plate spline function at point `pt`.
FloatArray ThinPlateSpline::Evaluate( FloatArray const& pt ) {
   dip::sint nPoints = static_cast< dip::sint >( c_.size() );
   dip::sint nDims = static_cast< dip::sint >( c_[ 0 ].size() );
   dip::sint N = nPoints + nDims + 1;
   Eigen::Map< Eigen::MatrixXd > x( x_.data(), N, nDims );
   // Note: w( ii, jj ) = x( ii, jj ), and a( ii, jj ) = x( nPoints + ii, jj )
   FloatArray res = pt;
   for( dip::sint ii = 0; ii < nPoints; ++ii ) {
      dfloat scale = RadialBasis( Distance( pt, c_[ static_cast< dip::uint >( ii ) ] ));
      for( dip::sint jj = 0; jj < nDims; ++jj ) {
         res[ static_cast< dip::uint >( jj ) ] += x( ii, jj ) * scale;
      }
   }
   for( dip::sint jj = 0; jj < nDims; ++jj ) {
      res[ static_cast< dip::uint >( jj ) ] += x( nPoints, jj );
   }
   for( dip::sint ii = 0; ii < nDims; ++ii ) {
      for( dip::sint jj = 0; jj < nDims; ++jj ) {
         res[ static_cast< dip::uint >( jj ) ] += x( nPoints + 1 + ii, jj ) * pt[ static_cast< dip::uint >( ii ) ];
      }
   }
   return res;
}

} // namespace dip
