/*
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
   sfloat aspectRatio = 0;
   sfloat sumX = 0;
   sfloat sumX2 = 0;
   sfloat sumY = 0;
   sfloat sumY2 = 0;
   sfloat sumXY = 0;

   void ComputeEllipseParams() {
      ellipseFit = aspectRatio = 0.0;
      if( area > 1 ) { // this is always true!
         sfloat farea = static_cast< sfloat >( area );
         sfloat varX = ( sumX2 - sumX * sumX / farea ); // Actually variance * area
         sfloat varY = ( sumY2 - sumY * sumY / farea );
         sfloat covXY = ( sumXY - sumX * sumY / farea );
         /*
         sfloat varXPlusVarY = varX + varY;
         sfloat sqrtExpr = std::sqrt( varXPlusVarY * varXPlusVarY - 4 * ( varX * varY - covXY * covXY ));
         if( varXPlusVarY > sqrtExpr ) {
            sfloat r1 = 2 * std::sqrt(( varXPlusVarY + sqrtExpr ) / ( 2 * farea ));
            sfloat r2 = 2 * std::sqrt(( varXPlusVarY - sqrtExpr ) / ( 2 * farea ));
            sfloat ellipseArea = static_cast< sfloat >( pi ) * r1 * r2;
            if( ellipseArea > 0 ) {
               ellipseFit = farea / ellipseArea;
               aspectRatio  = r1 / r2;
            }
         }
         */
         sfloat mmu2 = ( varX + varY ) / 2;  // CL: Changed eigenvalue computation, copied from dip::CovarianceMatrix::Eig() because
         sfloat dmu2 = ( varX - varY ) / 2;  //     I didn't recognize the equations above, but they are equivalent.
         sfloat sqroot = std::sqrt( covXY * covXY + dmu2 * dmu2 );
         if( sqroot < mmu2 ) {
            sfloat r1 = 2 * std::sqrt(( mmu2 + sqroot ) / farea );
            sfloat r2 = 2 * std::sqrt(( mmu2 - sqroot ) / farea );
            sfloat ellipseArea = static_cast< sfloat >( pi ) * r1 * r2;
            ellipseFit = farea / ellipseArea;
            aspectRatio  = r1 / r2;
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

   bool MatchesParams( PerObjectEllipsoidFitParameters const& params, sfloat value ) {
      return ( area >= params.minSize ) &&
             ( value <= params.maxThreshold ) &&
             ( ellipseFit >= params.minEllipsoidFit ) &&
             ( aspectRatio >= params.minAspectRatio ) &&
             ( aspectRatio <= params.maxAspectRatio );
   }
};

dip::uint FindRootNode( dip::uint n, std::vector< Node > const& nodes ) {
   if( nodes[ n ].parentNode == n ) {
      return n;
   }
   return FindRootNode( nodes[ n ].parentNode, nodes ); // This is like union-find, but without path compression.
}

void MergeNodes(
      dip::uint rootNode,
      dip::uint otherNode,
      std::vector< Node >& nodes
) {
   otherNode = FindRootNode( otherNode, nodes );
   if( rootNode != otherNode ) {
      //if(( diff == 0 ) && ( rootNode < otherNode )) { // CL: never happens!
      //   std::swap( rootNode, otherNode );
      //}
      nodes[ otherNode ].parentNode = rootNode;
      nodes[ rootNode ] += nodes[ otherNode ];
   }
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
      PerObjectEllipsoidFitParameters const& params,
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
               if( optArea >= nodes[ startE ].area ) {
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
      PerObjectEllipsoidFitParameters const& params,
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
      if(( nodes[ nodes[ e ].parentNode ].area > params.maxArea ) || ( inData[ e ] < params.minThreshold )) {
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

void PerObjectEllipsoidFit(
      Image const& image,
      Image& out,
      PerObjectEllipsoidFitParameters const& params
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
   float const* inData = static_cast< float const* >( floatImg.Origin() );

   // Get a list of indices to all pixels, sorted in descending order.
   // Using a stable sort so that pixels of the same value are processed left to right, top to bottom.
   std::vector< dip::uint > sortedIndices( lenData );
   std::iota( sortedIndices.begin(), sortedIndices.end(), 0 );
   std::stable_sort( sortedIndices.begin(), sortedIndices.end(),
                     [ & ]( dip::uint const& a, dip::uint const& b ) {
                        return inData[ a ] > inData[ b ]; // indices to larger pixel values come first
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
      sfloat currentValue = inData[ currentNode ];
      if( currentValue == extrema.Minimum() ) {
         // We've reached the lowest value, we're done! (added by CL)
         break;
      }
      dip::uint yy = currentNode / width;
      dip::uint xx = currentNode - yy * width;
      // The neighbors to the top and left, of the same value as the current pixel, have already been processed.
      // Therefore we attempt to merge them here.
      if( xx > 0 ) {
         dip::uint neighborNode = currentNode - 1;
         if( inData[ neighborNode ] >= currentValue ) {
            MergeNodes( currentNode, neighborNode, nodes );
         }
      }
      if( yy > 0 ) {
         dip::uint neighborNode = currentNode - width;
         if( inData[ neighborNode ] >= currentValue ) {
            MergeNodes( currentNode, neighborNode, nodes );
         }
      }
      // The neighbors to the right and bottom, of the same value as the current pixel, haven't been processed yet.
      // Therefore we don't attempt to merge them here.
      if( yy + 1 < height ) {
         dip::uint neighborNode = currentNode + width;
         if( inData[ neighborNode ] > currentValue ) {
            MergeNodes( currentNode, neighborNode, nodes );
         }
      }
      if( xx + 1 < width ) {
         dip::uint neighborNode = currentNode + 1;
         if( inData[ neighborNode ] > currentValue ) {
            MergeNodes( currentNode, neighborNode, nodes );
         }
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
