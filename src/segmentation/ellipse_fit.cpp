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

namespace dip {

namespace {

// Values written to intermediate output image
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

   void ComputeEllipseParams() {
      if( area > 1 ) { // this is always true!
         sfloat varX = ( sumX2 - sumX * sumX / static_cast< sfloat >( area ));
         sfloat varY = ( sumY2 - sumY * sumY / static_cast< sfloat >( area ));
         sfloat covXY = ( sumXY - sumX * sumY / static_cast< sfloat >( area ));
         sfloat varXPlusVarY = varX + varY;
         sfloat sqrtExpr = std::sqrt( varXPlusVarY * varXPlusVarY - 4 * ( varX * varY - covXY * covXY ));
         if( varXPlusVarY > sqrtExpr ) {
            sfloat r1 = 2 * std::sqrt(( varXPlusVarY + sqrtExpr ) / ( 2 * static_cast< sfloat >( area )));
            sfloat r2 = 2 * std::sqrt(( varXPlusVarY - sqrtExpr ) / ( 2 * static_cast< sfloat >( area )));
            sfloat ellipseArea = static_cast< sfloat >( pi ) * r1 * r2;
            if( ellipseArea > 0 ) {
               ellipseFit = static_cast< sfloat >( area ) / ellipseArea;
               majorAxis = 2 * r1;
               minorAxis = 2 * r2;
            }
         } else {
            ellipseFit = majorAxis = minorAxis = 0.0; // CL: added
         }
      }
   }

   void operator+=( Node const& other ) {
      area += other.area;
      sumX += other.sumX;
      sumX2 += other.sumX2;
      sumY += other.sumY;
      sumY2 += other.sumY2;
      sumXY += other.sumXY;
      ComputeEllipseParams();
   }

   bool MatchesParams( PerObjectEllipseFitParameters const& params, sfloat value ) {
      if(( area >= params.minArea ) &&
         ( value <= params.maxThreshold ) &&
         ( ellipseFit >= params.minEllipseFit ) &&
         ( majorAxis >= params.minMajorAxis ) &&
         ( majorAxis <= params.maxMajorAxis ) &&
         ( minorAxis >= params.minMinorAxis ) &&
         ( minorAxis <= params.maxMinorAxis )) {
         // `minorAxis >= params.minMinorAxis` ensures `minorAxis > 0`!
         sfloat majorMinorRatio = majorAxis / minorAxis;
         return ( majorMinorRatio >= params.minMajorMinorRatio ) &&
                ( majorMinorRatio <= params.maxMajorMinorRatio );
      }
      return false;
   }

};

dip::uint FindRootNode( dip::uint n, std::vector< Node > const& nodes ) {
   if( nodes[ n ].parentNode == n ) {
      return n;
   }
   return FindRootNode( nodes[ n ].parentNode, nodes ); // This is like union-find, but without path compression.
}

dip::uint ConditionallyMergeNodes(
      dip::uint currentNode,
      dip::uint neighborNode,
      std::vector< Node >& nodes,
      sfloat const* inData
) {
   if( inData[ neighborNode ] >= inData[ currentNode ] ) {
      neighborNode = FindRootNode( neighborNode, nodes );
      if( currentNode != neighborNode ) {
         if(( currentNode < neighborNode ) && ( inData[ neighborNode ] == inData[ currentNode ] )) {
            std::swap( currentNode, neighborNode );
         }
         nodes[ neighborNode ].parentNode = currentNode;
         nodes[ currentNode ] += nodes[ neighborNode ];
      }
   }
   return currentNode;
}

void MarkParents(
      dip::uint startE,
      dip::uint e,
      uint8 value,
      std::vector< Node >& nodes,
      uint8* outData
) {
   while( startE != e ) {
      outData[ startE ] = value;
      startE = nodes[ startE ].parentNode;
   }
}

constexpr dip::uint INVALID = std::numeric_limits< dip::uint >::max();

void ProcessParents(
      dip::uint startE,
      dip::uint e,
      dip::uint optE,
      std::vector< Node >& nodes,
      uint8* outData
) {
   if( optE != INVALID ) {
      MarkParents( startE, e, OBJECT, nodes, outData );
      outData[ optE ] = OBJECT;
      startE = nodes[ optE ].parentNode;
   }
   MarkParents( startE, e, NOT_OBJECT, nodes, outData );
}

void FindBestEllipseLevel(
      dip::uint e,
      PerObjectEllipseFitParameters const& params,
      std::vector< Node >& nodes,
      uint8* outData,
      sfloat const* inData
) {
   dip::uint optE = INVALID;
   sfloat optEf = 0;
   dip::uint optArea = 0;
   dip::uint startE = e;
   while( true ) {
      dip::uint firstE = e;
      while(( inData[ e ] == inData[ nodes[ e ].parentNode ] ) && ( nodes[ e ].parentNode != e )) {
         e = nodes[ e ].parentNode;
      }
      if( outData[ e ] == OBJECT ) {
         MarkParents( startE, e, OBJECT, nodes, outData );
         return;
      }
      if( outData[ e ] == NOT_OBJECT ) {
         ProcessParents( startE, e, optE, nodes, outData );
         return;
      }
      if( nodes[ e ].parentNode == e ) { // root
         ProcessParents( startE, e, optE, nodes, outData );
         return;
      }
      if(( nodes[ e ].area > params.maxArea ) || ( inData[ e ] < params.minThreshold )) {
         // set from firstE
         if( optE != INVALID ) {
            MarkParents( startE, optE, OBJECT, nodes, outData );
            outData[ optE ] = OBJECT;
            startE = nodes[ optE ].parentNode;
            while( startE != e ) { // same as MarkParents() but additionally changing the area for each visited node.
               outData[ startE ] = MAYBE_NOT_OBJECT;
               if( optArea > nodes[ startE ].area ) {
                  nodes[ startE ].area = 0;
               } else {
                  nodes[ startE ].area -= optArea;
               }
               startE = nodes[ startE ].parentNode;
            }
         }
         while(( firstE != nodes[ firstE ].parentNode ) && ( outData[ nodes[ firstE ].parentNode ] == UNDEFINED )) {
            firstE = nodes[ firstE ].parentNode;
            outData[ firstE ] = NOT_OBJECT;
         }
         outData[ firstE ] = NOT_OBJECT;
         return;
      }
      if(( nodes[ e ].ellipseFit > optEf ) && ( nodes[ e ].MatchesParams( params, inData[ e ] ))) {
         optEf = nodes[ e ].ellipseFit;
         optArea = nodes[ e ].area;
         optE = e;
      }
      e = nodes[ e ].parentNode;
   }
}

void FindObjectBelow(
      dip::uint startE,
      PerObjectEllipseFitParameters const& params,
      std::vector< Node >& nodes,
      uint8* outData,
      sfloat const* inData
) {
   dip::uint e = startE;
   uint8 res;
   while( true ) {
      if( nodes[ e ].parentNode == e ) {
         res = NOT_OBJECT;
         break;
      }
      if( outData[ nodes[ e ].parentNode ] == OBJECT ) {
         res = OBJECT;
         break;
      }
      if( nodes[ nodes[ e ].parentNode ].area > params.maxArea || inData[ e ] < params.minThreshold ) {
         res = NOT_OBJECT;
         break;
      }
      e = nodes[ e ].parentNode;
   }
   dip::uint j = startE;
   while( j != e ) {
      outData[ j ] = res;
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

   DIP_THROW_IF( params.minMinorAxis <= 0, "params.minMinorAxis must be positive" ); // actually they all should be, but with this test we prevent division by zero later on.

   // Get image data pointer.
   // We first convert the image to SFLOAT, this way we avoid dealing with multiple data types and strides.
   Image floatImg = Convert( image, DT_SFLOAT );
   DIP_ASSERT( floatImg.HasNormalStrides() );
   float const* inData = static_cast< float const* >( floatImg.Origin() );

   // Get a list of indices to all pixels, sorted in descending order.
   std::vector< dip::uint > sortedIndices( lenData );
   std::iota( sortedIndices.begin(), sortedIndices.end(), 0 );
   std::stable_sort( sortedIndices.begin(), sortedIndices.end(),
                     [ & ]( dip::uint const& a, dip::uint const& b ) {
                        return inData[ a ] > inData[ b ];
                     } );

   // Allocate and initialize nodes
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

   // Build tree
   for( dip::uint currentNode : sortedIndices ) {
      dip::uint yy = currentNode / width;
      dip::uint xx = currentNode - yy * width;
      dip::uint rootNode = currentNode;
      if( yy + 1 < height ) {
         rootNode = ConditionallyMergeNodes( rootNode, currentNode + width, nodes, inData );
      }
      if( xx + 1 < width ) {
         rootNode = ConditionallyMergeNodes( rootNode, currentNode + 1, nodes, inData );
      }
      if( xx > 0 ) {
         rootNode = ConditionallyMergeNodes( rootNode, currentNode - 1, nodes, inData );
      }
      if( yy > 0 ) {
         /*rootNode =*/ ConditionallyMergeNodes( rootNode, currentNode - width, nodes, inData );
      }
   }

   // Allocate and initialize intermediate output image
   // By using a separate array we don't have to deal with output image strides.
   Image tmpOutput( image.Sizes(), 1, DT_UINT8 );
   tmpOutput.Fill( UNDEFINED );
   uint8* outData = static_cast< uint8* >( tmpOutput.Origin() );

   // Handle UNDEFINED
   for( dip::uint jj : sortedIndices ) {
      if( outData[ jj ] == UNDEFINED ) {
         dip::uint e = jj;
         while(( inData[ e ] == inData[ nodes[ e ].parentNode ] ) && ( outData[ e ] == UNDEFINED ) && ( e != nodes[ e ].parentNode )) {
            e = nodes[ e ].parentNode;
         }
         if( outData[ e ] == UNDEFINED ) {
            FindBestEllipseLevel( jj, params, nodes, outData, inData );
         } else {
            while( jj != e ) {
               outData[ jj ] = outData[ e ];
               jj = nodes[ jj ].parentNode;
            }
         }
      }
   }

   // Handle MAYBE
   for( dip::uint jj : sortedIndices ) {
      if( outData[ jj ] == MAYBE_NOT_OBJECT ) {
         FindBestEllipseLevel( jj, params, nodes, outData, inData );
      }
   }

   // Handle MAYBE
   for( dip::uint jj : sortedIndices ) {
      if( outData[ jj ] == MAYBE_NOT_OBJECT ) {
         FindObjectBelow( jj, params, nodes, outData, inData );
      }
   }

   // Create output image and copy over
   out.ReForge( image, DT_BIN ); // Note: copies over pixel sizes.
   Equal( tmpOutput, OBJECT, out );
}

} // namespace dip
