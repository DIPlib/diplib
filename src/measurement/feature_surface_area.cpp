/*
 * DIPlib 3.0
 * This file contains definitions for function that computes 3D surface area.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Original code written by Jim Mullikin, Pattern Recognition Group.
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
#include "diplib/overload.h"
#include "feature_surface_area.h"

namespace dip {

namespace {

// Surface type
constexpr std::array< dip::uint, 64 > stype = {{
      0, 1, 1, 2, 1, 2, 2, 4,
      1, 3, 2, 5, 2, 5, 4, 6,
      1, 2, 3, 5, 2, 4, 5, 6,
      2, 5, 5, 7, 4, 6, 6, 8,
      1, 2, 2, 4, 3, 5, 5, 6,
      2, 5, 4, 6, 5, 7, 6, 8,
      2, 4, 5, 6, 5, 6, 7, 8,
      4, 6, 6, 8, 6, 8, 8, 9
}};

// Nearest neighbour mask bits
constexpr std::array< dip::uint, 6 > nnb = {{
      1 << 0,
      1 << 1,
      1 << 2,
      1 << 3,
      1 << 4,
      1 << 5,
}};

// Nearest neighbour configuration surface area contributions
// We divide all by 2 because we average foreground and background areas
constexpr std::array< dfloat, 10 > sa = {{
      pi             / 2.0,   // pathological situation
      0.8939539326   / 2.0,
      1.340863402    / 2.0,
      8.0 / 3.0      / 2.0,   // pathological situation
      1.587920248    / 2.0,   // Ben's d111 using improved accuracy with Jim's bias3d.m
      0.8939539326   / 2.0,
      1.340863402    / 2.0,   // Ben's d110
      2.0            / 2.0,   // pathological situation
      0.8939539326   / 2.0,   // Ben's d100
      0.0            / 2.0
}};

template< typename TPI >
static void SurfaceAreaInternal(
      Image const& label,
      std::map< dip::uint, dip::uint > const& objectIndex,
      std::vector< dfloat >& surfaceArea,
      std::array< dip::sint, 6 > const& nn
) {
   TPI* ip = static_cast< TPI* >( label.Origin() );
   IntegerArray const& stride = label.Strides();
   UnsignedArray const& dims = label.Sizes();
   for( dip::uint zz = 0; zz < dims[ 2 ]; ++zz ) {
      for( dip::uint yy = 0; yy < dims[ 1 ]; ++yy ) {
         dip::sint pos = static_cast< dip::sint >( zz ) * stride[ 2 ] + static_cast< dip::sint >( yy ) * stride[ 1 ];
         for( dip::uint xx = 0; xx < dims[ 0 ]; ++xx ) {
            // Check whether correct pixel value is a requested objectID
            bool requested = false;
            dip::uint index = 0;
            auto it = objectIndex.find( static_cast< dip::uint >( ip[ pos ] ));
            if( it != objectIndex.end() ) {
               requested = true;
               index = it->second;
            }

            // For each pixel, evaluate its 4 connected neighborhood
            dip::uint nnt = 0;
            std::array< dip::uint, 6 > nnn; // nearest neighbour labels
            for( dip::uint ii = 0; ii < 6; ++ii ) {
               switch( ii ) {
                  case 0:
                     if( xx + 1 == dims[ 0 ] ) continue;
                     break;

                  case 1:
                     if( yy + 1 == dims[ 1 ] ) continue;
                     break;

                  case 2:
                     if( zz + 1 == dims[ 2 ] ) continue;
                     break;

                  case 3:
                     if( xx == 0 ) continue;
                     break;

                  case 4:
                     if( yy == 0 ) continue;
                     break;

                  case 5:
                     if( zz == 0 ) continue;
                     break;

                  default:
                     // since the not so clever code inspector
                     break;
               }

               // If pixel is label of object and on the surface, check
               // which of its neighborhood pixels are as well
               if( requested ) {
                  if( ip[ pos + nn[ ii ] ] == ip[ pos ] ) {
                     nnt |= nnb[ ii ];
                  }
               }
               // If the pixel has a label we don't want to measure,
               // store the labels of its neighborhood
               else {
                  nnn[ ii ] = static_cast< dip::uint >( objectIndex.count( static_cast< dip::uint >( ip[ pos + nn[ ii ]] )) > 0 ? ip[ pos + nn[ ii ] ] : 0 );
               }
            }

            // If we're dealing with a pixel of the surface of the object, add
            // the area of its neighborhood configuration to the total
            if( requested ) {
               surfaceArea[ index ] += sa[ stype[ nnt ]];
            }
            // If the "current" pixel is not an object pixel, calculate the
            // bg SA of all the objects in the pixel's neighborhood
            else {
               for( dip::uint ii = 0; ii < 6; ++ii ) {
                  index = 0;
                  it = objectIndex.find( nnn[ ii ] );
                  if( it == objectIndex.end() ) {
                     continue;
                  }
                  index = it->second;
                  nnt = nnb[ ii ];
                  for( dip::uint jj = ii + 1; jj < 6; ++jj ) {
                     if( nnn[ jj ] == nnn[ ii ] ) {
                        nnn[ jj ] = 0;
                        nnt |= nnb[ jj ];
                     }
                  }
                  // I don't know what Jim meant with this next statement (GvK)
                  nnt = ( nnt ^ 077u ) & 077u;
                  // Let's make sure nnt doesn't overflow stype
                  nnt = std::min( nnt, dip::uint( 64 ) );
                  surfaceArea[ index ] += sa[ stype[ nnt ] ];
               }
            }
            pos += stride[ 0 ];
         }
      }
   }
}

} // namespace

std::vector< dfloat > SurfaceArea(
      Image const& label,
      UnsignedArray const& objectIDs
) {
   if( objectIDs.empty() ) {
      return {};
   }

   // Check image properties
   DIP_STACK_TRACE_THIS( label.CheckProperties( 3, 1, dip::DataType::Class_UInt, Option::ThrowException::DO_THROW ));

   // Initialise the surface area results array
   std::vector< dfloat > surfaceArea( objectIDs.size() );

   // Create lookup table for objectIDs
   std::map< dip::uint, dip::uint > objectIndex;
   for( dip::uint ii = 0; ii < objectIDs.size(); ++ii ) {
      objectIndex.emplace( objectIDs[ ii ], ii );
   }

   // Initialise nearest neighbour offsets
   std::array< dip::sint, 6 > nn;
   nn[ 0 ] =  label.Stride( 0 );
   nn[ 1 ] =  label.Stride( 1 );
   nn[ 2 ] =  label.Stride( 2 );
   nn[ 3 ] = -label.Stride( 0 );
   nn[ 4 ] = -label.Stride( 1 );
   nn[ 5 ] = -label.Stride( 2 );

   DIP_OVL_CALL_UINT( SurfaceAreaInternal, ( label, objectIndex, surfaceArea, nn ), label.DataType() );

   return surfaceArea;
}


} // namespace dip
