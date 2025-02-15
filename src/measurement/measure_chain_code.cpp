/*
 * (c)2016-2022, Cris Luengo.
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

#include "diplib/chain_code.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include "diplib.h"
#include "diplib/polygon.h"

namespace dip {

namespace {

dfloat Length8Connected( std::vector< ChainCode::Code > const& codes, bool includeBoundaryPixels ) {
   dip::uint Ne = 0;
   dip::uint No = 0;
   dip::uint Nc = 0;
   ChainCode::Code prev = codes.back();
   for( auto code : codes ) {
      if( includeBoundaryPixels || !code.IsBorder() ) {
         // count the number of even and odd codes
         if( code.IsOdd() ) {
            No++;
         } else {
            Ne++;
         }
         if( code != prev ) {
            Nc++;
         }
      }
      prev = code;
   }
   return 0.980 * static_cast< dfloat >( Ne ) +
          1.406 * static_cast< dfloat >( No ) -
          0.091 * static_cast< dfloat >( Nc );
}

dfloat Length4Connected( std::vector< ChainCode::Code > const& codes, bool includeBoundaryPixels ) {
   dip::uint Ne = 0;
   dip::uint Nc = 0;
   ChainCode::Code prev = codes.back();
   for( auto code : codes ) {
      if( includeBoundaryPixels || !code.IsBorder() ) {
         Ne++;
         if( code != prev ) {
            Nc++;
         }
      }
      prev = code;
   }
   return 0.948 * static_cast< dfloat >( Ne ) - 0.278 * static_cast< dfloat >( Nc );
}

} // namespace

dfloat ChainCode::Length( String const& boundaryPixels ) const {
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)" );
   bool includeBoundaryPixels{};
   DIP_STACK_TRACE_THIS( includeBoundaryPixels = BooleanFromString( boundaryPixels, S::INCLUDE, S::EXCLUDE ));
   if( codes.empty() ) {
      return 0;
   }
   return is8connected ? Length8Connected( codes, includeBoundaryPixels )
                       : Length4Connected( codes, includeBoundaryPixels );
}

FeretValues ChainCode::Feret( dfloat angleStep ) const {
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)" );
   FeretValues feret;
   if( Empty() ) {
      return feret;
   }
   if( codes.empty() ) {
      // Fill in some values
      feret.maxDiameter = 1.0;
      feret.minDiameter = 1.0;
      feret.maxPerpendicular = 1.0;
      feret.maxAngle = 0;
      feret.minAngle = pi / 2;
      return feret;
   }

   // Rotate the chain for each angle, and find largest x & y diameter
   feret.minDiameter = std::numeric_limits< dfloat >::max(); // other values initialized to 0.
   for( dfloat angle = 0.0; angle <= pi / 2.0; angle += angleStep ) {
      std::array< dfloat, 8 > cosval{};
      std::array< dfloat, 8 > sinval{};
      if( is8connected ) {
         for( dip::uint ii = 0; ii < 8; ii++ ) {
            cosval[ ii ] =  ( 1.0 + ( std::sqrt( 2.0 ) - 1.0 ) * static_cast< dfloat >( ii % 2 ))
                                      * std::cos( static_cast< dfloat >( ii ) * pi / 4.0 + angle );
            sinval[ ii ] = -( 1.0 + ( std::sqrt( 2.0 ) - 1.0 ) * static_cast< dfloat >( ii % 2 ))
                                      * std::sin( static_cast< dfloat >( ii ) * pi / 4.0 + angle );
         }
      } else {
         for( dip::uint ii = 0; ii < 4; ii++ ) {
            cosval[ ii ] =  std::cos( static_cast< dfloat >( ii ) * pi / 2.0 + angle );
            sinval[ ii ] = -std::sin( static_cast< dfloat >( ii ) * pi / 2.0 + angle );
         }
      }
      // Rotate the chain and find bounding box
      dfloat x = 0.0, y = 0.0, xMin = 0.0, xMax = 0.0, yMin = 0.0, yMax = 0.0;
      for( auto const& code :codes ) {
         x += cosval[ code ];
         y += sinval[ code ];
         xMin = std::min( xMin, x );
         xMax = std::max( xMax, x );
         yMin = std::min( yMin, y );
         yMax = std::max( yMax, y );
      }
      // Get x & y diameters
      dfloat xDiam = xMax - xMin + 1.0;
      dfloat yDiam = yMax - yMin + 1.0;
      // See whether diameters for this angle "break records"
      if(( xDiam > feret.maxDiameter ) || ( angle == 0.0 )) {
         feret.maxDiameter = xDiam;
         feret.maxAngle = angle;
      }
      if( yDiam > feret.maxDiameter ) {
         feret.maxDiameter = yDiam;
         feret.maxAngle = angle + pi / 2.0;
      }
      if(( xDiam < feret.minDiameter ) || ( angle == 0.0 )) {
         feret.minDiameter = xDiam;
         feret.minAngle = angle;
         feret.maxPerpendicular = yDiam;
      }
      if( yDiam < feret.minDiameter ) {
         feret.minDiameter = yDiam;
         feret.minAngle = angle + pi / 2.0;
         feret.maxPerpendicular = xDiam;
      }
   }

   return feret;
}

dfloat ChainCode::BendingEnergy() const {
   constexpr dfloat kulpa_weights[ 8 ] = { 0.9481, 1.3408, 0.9481, 1.3408, 0.9481, 1.3408, 0.9481, 1.3408 };
   constexpr dfloat fourc_cornerc_weights[ 2 ] = { 0.948, 0.278 };
   dip::uint size = codes.size();
   if( size <= 1 ) {
      return 0;
   }
   // Compute angular difference, divide by curve element length computed using Kulpa weights.
   std::vector< dfloat > diff( size, 0 );
   std::vector< dfloat > delta_s( size, 0 );
   auto prev = codes.back();
   for( dip::uint ii = 0; ii < size; ++ii ) {
      delta_s[ ii ] = is8connected
                      ? 0.5 * ( kulpa_weights[ codes[ ii ]] + kulpa_weights[ prev ] )
                      : fourc_cornerc_weights[ 0 ] + fourc_cornerc_weights[ 1 ] * ( codes[ ii ] != prev );
      diff[ ii ] = static_cast< dfloat >( codes[ ii ] ) - static_cast< dfloat >( prev );
      if( !is8connected ) { diff[ ii ] *= 2; }
      if( diff[ ii ] > 3 ) { diff[ ii ] -= 8; }
      if( diff[ ii ] < -3 ) { diff[ ii ] += 8; }
      diff[ ii ] /= delta_s[ ii ];
      prev = codes[ ii ];
   }
   // Three times uniform filtering of diff
   constexpr dip::uint k = 5; // filter size
   if( size > k ) {
      dip::uint size1 = size - k;
      for( dip::uint jj = 0; jj < 3; ++jj ) {
         dfloat sum = 0;
         dfloat stored[ k ];
         for( dip::uint ii = 0; ii < k; ++ii ) {
            stored[ ii ] = diff[ ii ];
            sum += diff[ ii ];
         }
         for( dip::uint ii = 0; ii < size1; ++ii ) {
            dfloat saved = diff[ ii ];
            diff[ ii ] = sum / k;
            sum += diff[ ii + k ] - saved;
         }
         for( dip::uint ii = size1; ii < size; ++ii ) {
            dfloat saved = diff[ ii ];
            diff[ ii ] = sum / k;
            sum += stored[ ii - size1 ] - saved;
         }
      }
   }
   // Integrate the curvature squared weighted by the curve element length
   dfloat be = 0;
   for( dip::uint ii = 0; ii < size; ++ii ) {
      be += diff[ ii ] * diff[ ii ] * delta_s[ ii ];
   }
   // Convert chain code into actual angle in radian
   be *= dip::pi * dip::pi / 16.0;
   return be;
}

BoundingBoxInteger ChainCode::BoundingBox() const {
   VertexInteger current = start;
   dip::BoundingBoxInteger bb{ current };
   if( is8connected ) {
      for( auto code : codes ) {
         current += code.Delta8();
         bb.Expand( current );
      }
   } else {
      for( auto code : codes ) {
         current += code.Delta4();
         bb.Expand( current );
      }
   }
   return bb;
}

dip::uint ChainCode::LongestRun() const {
   dip::uint longestRun = 0;
   dip::uint currentRun = 0;
   Code prev = codes.back();
   // Two loops around the perimeter, the second one is to count the chain code run that starts before the first code
   for( dip::uint ii = 0; ii <= 1; ++ii ) {
      for( auto code : codes ) {
         if( !code.IsBorder() && ( code == prev )) {
            currentRun++;
         } else {
            longestRun = std::max( longestRun, currentRun );
            currentRun = 0;
            // On the second run, quit after the first change in direction
            if ( ii == 1 ) {
               break;
            }
         }
         prev  = code;
      }
   }
   return longestRun;
}


} // namespace dip
