/*
 * DIPlib 3.0
 * This file contains the definition for the Label function.
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
#include "diplib/regions.h"
#include "diplib/union_find.h"
#include "diplib/iterators.h"

#include "labelingGrana2016.h"

namespace dip {

dip::uint Label(
      Image const& c_in,
      Image& out,
      dip::uint connectivity,
      dip::uint minSize,
      dip::uint maxSize,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( connectivity > nDims, E::PARAMETER_OUT_OF_RANGE );

   Image in = c_in.QuickCopy();
   auto pixelSize = in.PixelSize();
   out.ReForge( in, DT_LABEL );
   out.SetPixelSize( pixelSize );
   out.Fill( 0 );

   LabelRegionList regions{ std::plus< dip::uint >{} };

   // First scan
   if(( nDims == 2 ) && ( connectivity == 2 )) {
      LabelingGrana2016( in, out, regions );
   } else {
      DIP_THROW( E::NOT_IMPLEMENTED );
      //LabelingFirstScan( in, out, regions );
   }

   // Handle boundary condition
   if( !boundaryCondition.empty() ) {
      BoundaryConditionArray bc;
      DIP_START_STACK_TRACE
         bc = StringArrayToBoundaryConditionArray( boundaryCondition );
         ArrayUseParameter( bc, nDims, BoundaryCondition::ADD_ZEROS ); // any default that is not PERIODIC will do...
      DIP_END_STACK_TRACE
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( bc[ ii ] == BoundaryCondition::PERIODIC ) {
            // Merge labels for objects touching opposite sides of image along this dimension
            // TODO: take connectivity into account, now we do connectivity==1 only.
            dip::sint lastPixelOffset = out.Stride( ii ) * static_cast< dip::sint >( out.Size( ii ) - 1 );
            auto it = ImageIterator< LabelType >( out, ii );
            do {
               LabelType lab1 = *it;
               LabelType lab2 = *( it.Pointer() + lastPixelOffset );
               if(( lab1 > 0 ) && ( lab2 > 0 )) {
                  regions.Union( lab1, lab2 );
               }
            } while( ++it );

         }
      }
   }

   // Relabel
   dip::uint nLabel;
   if(( minSize > 0 ) && ( maxSize > 0 )) {
      nLabel = regions.Relabel(
            [ & ]( dip::uint size ){ return ( size >= minSize ) && ( size <= maxSize ); }
      );
   } else if( minSize > 0 ) {
      nLabel = regions.Relabel(
            [ & ]( dip::uint size ){ return size >= minSize; }
      );
   } else if( maxSize > 0 ) {
      nLabel = regions.Relabel(
            [ & ]( dip::uint size ){ return size <= maxSize; }
      );
   } else {
      nLabel = regions.Relabel();
   }

   // Second scan
   auto it = ImageIterator< LabelType >( out );
   do {
      if( *it > 0 ) {
         *it = regions.Label( *it );
      }
   } while( ++it );

   return nLabel;
}

} // namespace dip
