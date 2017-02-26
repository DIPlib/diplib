/*
 * DIPlib 3.0
 * This file contains definitions for functions that measure chain codes.
 *
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

#include <limits>
#include <cmath>
#include <array>

#include "diplib.h"
#include "diplib/chain_code.h"

namespace dip {

dfloat ChainCode::Length() const {
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)" );
   if( codes.empty() ) {
      return pi;
   } else if( is8connected ) {
      dip::uint Ne = 0;
      dip::uint No = 0;
      dip::uint Nc = 0;
      Code prev = codes.back();
      for( auto code : codes ) {
         if( !code.IsBorder() ) {
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
      return 0.980 * Ne + 1.406 * No - 0.091 * Nc;
   } else {
      dip::uint Ne = 0;
      dip::uint Nc = 0;
      Code prev = codes.back();
      for( auto code : codes ) {
         if( !code.IsBorder() ) {
            Ne++;
            if( code != prev ) {
               Nc++;
            }
         }
         prev = code;
      }
      return 0.948 * Ne - 0.278 * Nc;
   }
}

FeretValues ChainCode::Feret( dfloat angleStep ) const {
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)" );
   FeretValues feret;
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
      std::array< dfloat, 8 > cosval;
      std::array< dfloat, 8 > sinval;
      if( is8connected ) {
         for( dip::uint ii = 0; ii < 8; ii++ ) {
            cosval[ ii ] =  ( 1.0 + std::sqrt( 2.0 ) - 1.0 * ( ii % 2 ) ) * std::cos( ii * pi / 4.0 + angle );
            sinval[ ii ] = -( 1.0 + std::sqrt( 2.0 ) - 1.0 * ( ii % 2 ) ) * std::sin( ii * pi / 4.0 + angle );
         }
      } else {
         for( dip::uint ii = 0; ii < 4; ii++ ) {
            cosval[ ii ] =  1.0 * std::cos( ii * pi / 2.0 + angle );
            sinval[ ii ] = -1.0 * std::sin( ii * pi / 2.0 + angle );
         }
      }
      // Rotate the chain and find bounding box
      dfloat x, y, xMin, xMax, yMin, yMax;
      x = y = xMin = xMax = yMin = yMax = 0;
      for( auto const& code :codes ) {
         x += cosval[ int( code ) ];
         y += sinval[ int( code ) ];
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
   constexpr dfloat kulpa_weights[8] = { 0.9481, 1.3408, 0.9481, 1.3408, 0.9481, 1.3408, 0.9481, 1.3408 };
   dip::uint size = codes.size();
   if( size > 1 ) {
      // Compute angular difference, divide by curve element length computed using Kulpa weights.
      dip::uint size1 = size - 1;
      FloatArray diff( size, 0 );
      FloatArray delta_s( size, 0 );
      for( dip::uint ii = 0; ii < size1; ++ii ) {
         delta_s[ ii ] = 0.5 * ( kulpa_weights[ codes[ ii ] ] + kulpa_weights[ codes[ ii + 1 ] ] ); // TODO: does not work for 4-connected CCs
         diff[ ii ] = codes[ ii + 1 ] - codes[ ii ];
         if( !is8connected ) { diff[ ii ] *= 2; }
         if( diff[ ii ] > 3 ) { diff[ ii ] -= 8; }
         if( diff[ ii ] < -3 ) { diff[ ii ] += 8; }
         diff[ ii ] = diff[ ii ] / delta_s[ ii ];
      }
      diff[ size1 ] = codes.front() - codes.back();
      // Three times uniform filtering of diff
      if( size > 5 ) {
         for( dip::uint jj = 0; jj < 3; ++jj ) {
            dfloat sum = 0;
            dfloat stored[5];
            dfloat saved;
            for( dip::uint ii = 0; ii < 5; ++ii ) {
               stored[ ii ] = diff[ ii ];
               sum += diff[ ii ];
            }
            size1 = size - 5;
            for( dip::uint ii = 0; ii < size1; ++ii ) {
               saved = diff[ ii ];
               diff[ ii ] = sum / 5;
               sum += diff[ ii + 5 ] - saved;
            }
            for( dip::uint ii = size1; ii < size; ++ii ) {
               saved = diff[ ii ];
               diff[ ii ] = sum / 5;
               sum += stored[ 5 - size + ii ] - saved;
            }
         }
      }
      // Integrate the curvature squared weighted by the curve element length
      sfloat be = 0;
      for( dip::uint ii = 0; ii < size; ++ii ) {
         be += diff[ii] * diff[ii] * delta_s[ii];
      }
      // Convert chain code into actual angle in radian
      be *= dip::pi * dip::pi / 16;
      return be;
   }
   else {
      return 0;
   }
}

dip::BoundingBox ChainCode::BoundingBox() const {
   VertexInteger current = start;
   dip::BoundingBox bb{ current };
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
