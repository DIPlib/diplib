/*
 * DIPlib 3.0
 * This file contains per object ellipse fit algorithm.
 *
 * (c)2019, Cris Luengo.
 * (c)2015, Petter Ranefall.
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
#include "diplib/segmentation.h"
#include "diplib/statistics.h"
#include "diplib/iterators.h"

namespace dip {

namespace {

// Values written to outputArray
constexpr uint8 UNDEFINED = 0;
constexpr uint8 NOT_OBJECT = 1;
constexpr uint8 MAYBE_NOT_OBJECT = 2;
constexpr uint8 OBJECT = 3;

struct Node {
   dip::uint parentNode = 0;
   dip::uint area = 1;
   sfloat ellipseFit = 0;
   sfloat majorAxis = 0;
   sfloat minorAxis = 0;
   sfloat sumX = 0;
   sfloat sumX2 = 0;
   sfloat sumY = 0;
   sfloat sumY2 = 0;
   sfloat sumXY = 0;
};

dip::uint findNode( dip::uint n, std::vector< Node > const& nodes ) {
   if( nodes[ n ].parentNode == n ) {
      return n;
   }
   return findNode( nodes[ n ].parentNode, nodes );
}

dip::uint mergeNodes(
      dip::uint n1,
      dip::uint n2,
      sfloat const* imData,
      std::vector< Node >& nodes
) {
   dip::uint par;
   dip::uint child;
   if( imData[ n1 ] == imData[ n2 ] ) {
      par = std::max( n1, n2 );
      child = std::min( n1, n2 );
   } else { // imData[n1] is always > imData[n2] here
      par = n2;
      child = n1;
   }
   nodes[ par ].area += nodes[ child ].area;
   nodes[ child ].parentNode = par;
   nodes[ par ].sumX += nodes[ child ].sumX;
   nodes[ par ].sumX2 += nodes[ child ].sumX2;
   nodes[ par ].sumY += nodes[ child ].sumY;
   nodes[ par ].sumY2 += nodes[ child ].sumY2;
   nodes[ par ].sumXY += nodes[ child ].sumXY;

   //if( nodes[ par ].area > 1 ) { // this is always true!
      sfloat varX = ( nodes[ par ].sumX2 - nodes[ par ].sumX * nodes[ par ].sumX / static_cast< sfloat >( nodes[ par ].area ));
      sfloat varY = ( nodes[ par ].sumY2 - nodes[ par ].sumY * nodes[ par ].sumY / static_cast< sfloat >( nodes[ par ].area ));
      sfloat covXY = ( nodes[ par ].sumXY - nodes[ par ].sumX * nodes[ par ].sumY / static_cast< sfloat >( nodes[ par ].area ));
      sfloat varXPlusVarY = varX + varY;
      sfloat sqrtExpr = std::sqrt( varXPlusVarY * varXPlusVarY - 4 * ( varX * varY - covXY * covXY ));
      if( varXPlusVarY > sqrtExpr ) {
         sfloat r1 = 2 * std::sqrt(( varXPlusVarY + sqrtExpr ) / ( 2 * static_cast< sfloat >( nodes[ par ].area )));
         sfloat r2 = 2 * std::sqrt(( varXPlusVarY - sqrtExpr ) / ( 2 * static_cast< sfloat >( nodes[ par ].area )));
         sfloat ellipseArea = static_cast< sfloat >( pi ) * r1 * r2;
         if( ellipseArea > 0 ) {
            nodes[ par ].ellipseFit = static_cast< sfloat >( nodes[ par ].area ) / ellipseArea;
            nodes[ par ].majorAxis = 2 * r1;
            nodes[ par ].minorAxis = 2 * r2;
         }
      }
   //}
   return par;
}

void findBestEllipseLevel(
      dip::uint inputE,
      PerObjectEllipseFitParameters const& params,
      std::vector< Node >& nodes,
      std::vector< uint8 >& outputArray,
      sfloat const* imData
) {
   constexpr dip::uint INVALID = std::numeric_limits< dip::uint >::max();
   dip::uint e = inputE;
   dip::uint optE = INVALID;
   sfloat optEf = 0;
   dip::uint optArea = 0;
   dip::uint startE = e;
   while( true ) {
      dip::uint firstE = e;
      while( imData[ e ] == imData[ nodes[ e ].parentNode ] && nodes[ e ].parentNode != e ) {
         e = nodes[ e ].parentNode;
      }
      if( outputArray[ e ] == OBJECT ) {
         while( startE != e ) {
            outputArray[ startE ] = OBJECT;
            startE = nodes[ startE ].parentNode;
         }
         return;
      }
      if( outputArray[ e ] == NOT_OBJECT ) {
         if( optE != INVALID ) {
            while( startE != optE ) {
               outputArray[ startE ] = OBJECT;
               startE = nodes[ startE ].parentNode;
            }
            outputArray[ optE ] = OBJECT;
            startE = nodes[ optE ].parentNode;
         }
         while( startE != e ) {
            outputArray[ startE ] = NOT_OBJECT;
            startE = nodes[ startE ].parentNode;
         }
         return;
      }
      if( nodes[ e ].parentNode == e ) // root
      {
         if( optE != INVALID ) {
            while( startE != optE ) {
               outputArray[ startE ] = OBJECT;
               startE = nodes[ startE ].parentNode;
            }
            outputArray[ optE ] = OBJECT;
            startE = nodes[ optE ].parentNode;
         }
         while( startE != e ) {
            outputArray[ startE ] = NOT_OBJECT;
            startE = nodes[ startE ].parentNode;
         }
         return;
      }
      if( nodes[ e ].area > params.maxArea || imData[ e ] < params.minThreshold ) {
         // set from firstE
         if( optE != INVALID ) {
            while( startE != optE ) {
               outputArray[ startE ] = OBJECT;
               startE = nodes[ startE ].parentNode;
            }
            outputArray[ optE ] = OBJECT;
            startE = nodes[ optE ].parentNode;
            while( startE != e ) {
               outputArray[ startE ] = MAYBE_NOT_OBJECT;
               if( optArea > nodes[ startE ].area ) {
                  nodes[ startE ].area = 0;
               } else {
                  nodes[ startE ].area -= optArea;
               }
               startE = nodes[ startE ].parentNode;
            }
         }
         while( firstE != nodes[ firstE ].parentNode && outputArray[ nodes[ firstE ].parentNode ] == UNDEFINED ) {
            firstE = nodes[ firstE ].parentNode;
            outputArray[ firstE ] = NOT_OBJECT;
         }
         outputArray[ firstE ] = NOT_OBJECT;
         return;
      }
      sfloat majorMinorRatio = 1.0;
      if( nodes[ e ].minorAxis > 0 ) {
         majorMinorRatio = nodes[ e ].majorAxis / nodes[ e ].minorAxis;
      }
      if(( nodes[ e ].area >= params.minArea )
         && ( imData[ e ] <= params.maxThreshold )
         && ( nodes[ e ].ellipseFit > optEf )
         && ( nodes[ e ].ellipseFit >= params.minEllipseFit )
         && ( nodes[ e ].majorAxis >= params.minMajorAxis )
         && ( nodes[ e ].majorAxis <= params.maxMajorAxis )
         && ( nodes[ e ].minorAxis >= params.minMinorAxis )
         && ( nodes[ e ].minorAxis <= params.maxMinorAxis )
         && ( majorMinorRatio >= params.minMajorMinorRatio )
         && ( majorMinorRatio <= params.maxMajorMinorRatio )) {
         optEf = nodes[ e ].ellipseFit;
         optArea = nodes[ e ].area;
         optE = e;
      }

      e = nodes[ e ].parentNode;
   }
}

void findObjBelow(
      dip::uint startE,
      PerObjectEllipseFitParameters const& params,
      std::vector< Node >& nodes,
      std::vector< uint8 >& outputArray,
      sfloat const* imData
) {
   dip::uint e = startE;
   uint8 res = NOT_OBJECT;
   while( true ) {
      if( nodes[ e ].parentNode == e ) {
         res = NOT_OBJECT;
         break;
      }
      if( outputArray[ nodes[ e ].parentNode ] == OBJECT ) {
         res = OBJECT;
         break;
      }
      if( nodes[ nodes[ e ].parentNode ].area > params.maxArea || imData[ e ] < params.minThreshold ) {
         res = NOT_OBJECT;
         break;
      }
      e = nodes[ e ].parentNode;
   }
   dip::uint j = startE;
   while( j != e ) {
      outputArray[ j ] = res;
      j = nodes[ j ].parentNode;
   }
}

} // namespace

void PerObjectEllipseFit(
      Image const& image,
      Image& out,
      PerObjectEllipseFitParameters const& params
) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !image.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( image.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !image.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );

   dip::uint height = image.Size( 1 );
   dip::uint width = image.Size( 0 );
   dip::uint lenData = width * height;

   auto extrema = MaximumAndMinimum( image );
   DIP_THROW_IF( !std::isfinite( extrema.Maximum() ) || !std::isfinite( extrema.Minimum() ), "Image has non-finite values" );
   DIP_THROW_IF( extrema.Maximum() == extrema.Minimum(), "Image is constant" );

   // Get image data pointer.
   // We first convert the image to SFLOAT, this way we avoid dealing with multiple data types and strides.
   Image floatImg = Convert( image, DT_SFLOAT );
   DIP_ASSERT( floatImg.HasNormalStrides() );
   float const* imData = static_cast< float const* >( floatImg.Origin() );

   // Get a list of indices to all pixels, sorted in descending order.
   std::vector< dip::uint > sortedIndices( lenData );
   std::iota( sortedIndices.begin(), sortedIndices.end(), 0 );
   std::stable_sort( sortedIndices.begin(), sortedIndices.end(),
                     [ & ]( dip::uint const& a, dip::uint const& b ) {
                        return imData[ a ] > imData[ b ];
                     } );

   // Allocate and initialize intermediate data
   std::vector< Node > nodes( lenData );
   dip::uint ii = 0;
   for( dip::uint yy = 0; yy < height; ++yy ) {
      sfloat y = static_cast< sfloat >( yy );
      for( dip::uint xx = 0; xx < width; ++xx ) {
         sfloat x = static_cast< sfloat >( xx );
         nodes[ ii ].sumX = x;
         nodes[ ii ].sumX2 = x * x;
         nodes[ ii ].sumY = y;
         nodes[ ii ].sumY2 = y * y;
         nodes[ ii ].sumXY = x * y;
         nodes[ ii ].parentNode = ii;
         ++ii;
      }
   }
   std::vector< uint8 > outputArray( lenData, 0 ); // Actually the output image...

   for( dip::uint jj : sortedIndices ) {
      dip::uint curNode = jj;
      dip::uint yy = jj / width;
      dip::uint xx = jj - yy * width;

      dip::uint nextY = yy + 1;
      if( nextY < height ) {
         dip::uint kk = xx + width * nextY;
         if( imData[ kk ] >= imData[ jj ] ) {
            dip::uint adjNode = findNode( kk, nodes );
            if( curNode != adjNode ) {
               curNode = mergeNodes( adjNode, curNode, imData, nodes );
            }
         }
      }
      dip::uint nextX = xx + 1;
      if( nextX < width ) {
         dip::uint kk = nextX + width * yy;
         if( imData[ kk ] >= imData[ jj ] ) {
            dip::uint adjNode = findNode( kk, nodes );
            if( curNode != adjNode ) {
               curNode = mergeNodes( adjNode, curNode, imData, nodes );
            }
         }
      }
      if( xx > 0 ) {
         dip::uint kk = xx - 1 + width * yy;
         if( imData[ kk ] > imData[ jj ] ) {
            dip::uint adjNode = findNode( kk, nodes );
            if( curNode != adjNode ) {
               curNode = mergeNodes( adjNode, curNode, imData, nodes );
            }
         }
      }
      if( yy > 0 ) {
         dip::uint kk = xx + width * ( yy - 1 );
         if( imData[ kk ] > imData[ jj ] ) {
            dip::uint adjNode = findNode( kk, nodes );
            if( curNode != adjNode ) {
               curNode = mergeNodes( adjNode, curNode, imData, nodes );
            }
         }
      }
   }

   // Handle UNDEFINED
   for( dip::uint jj : sortedIndices ) {
      if( outputArray[ jj ] == UNDEFINED ) {
         dip::uint e = jj;
         while( imData[ e ] == imData[ nodes[ e ].parentNode ] && outputArray[ e ] == UNDEFINED && e != nodes[ e ].parentNode  ) {
            e = nodes[ e ].parentNode;
         }
         if( outputArray[ e ] == UNDEFINED ) {
            findBestEllipseLevel( jj, params, nodes, outputArray, imData );
         } else {
            dip::uint e1 = jj;
            while( e1 != e ) {
               outputArray[ e1 ] = outputArray[ e ];
               e1 = nodes[ e1 ].parentNode;
            }
         }
      }
   }

   // Handle MAYBE
   for( dip::uint jj : sortedIndices ) {
      if( outputArray[ jj ] == MAYBE_NOT_OBJECT ) {
         findBestEllipseLevel( jj, params, nodes, outputArray, imData );
      }
   }

   // Handle MAYBE
   for( dip::uint jj : sortedIndices ) {
      if( outputArray[ jj ] == MAYBE_NOT_OBJECT ) {
         findObjBelow( jj, params, nodes, outputArray, imData );
      }
   }

   // Create output image and copy over
   out.ReForge( image, DT_BIN ); // Note: copies over pixel sizes.
   ImageIterator< bin > outIt( out );
   auto dataIt = outputArray.begin();
   do {
      *outIt = *dataIt == OBJECT;
   } while( ++dataIt, ++outIt );
}

} // namespace dip
