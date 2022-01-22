/*
 * (c)2018, Cris Luengo.
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
#include "diplib/analysis.h"
#include "diplib/distance.h"
#include "diplib/histogram.h"
#include "diplib/mapping.h"

namespace dip {

Distribution DistanceDistribution(
      Image const& object_c,
      Image const& region_c,
      dip::uint length
) {
   DIP_THROW_IF( !object_c.IsForged() || !region_c.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !object_c.IsScalar() || !region_c.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !object_c.DataType().IsUnsigned() || !region_c.DataType().IsUnsigned(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( object_c.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( object_c.Sizes() != region_c.Sizes(), E::SIZES_DONT_MATCH );
   Image object = object_c.QuickCopy();
   if( object.DataType().IsBinary() ) {
      object.Convert( DT_UINT8 ); // the binary image can be a labeled image with labels 0 and 1.
   }
   Image region = region_c.QuickCopy();
   if( !region.DataType().IsBinary() ) {
      NotEqual( region, 0, region );
   }
   dfloat maxDistance = length == 0 ? 99.0 : static_cast< dfloat >( length - 1 );
   PixelSize pixelSize = region_c.PixelSize();
   if( !pixelSize.IsPhysical() ) {
      pixelSize = object_c.PixelSize();
   }
   if( pixelSize.IsPhysical() ) {
      dip::uint nDims = std::min( object.Dimensionality(), pixelSize.Size() );
      dfloat psmag = pixelSize[ 0 ].magnitude;
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         psmag = std::min( psmag, pixelSize[ ii ].magnitude );
      }
      maxDistance *= psmag;
      region.SetPixelSize( std::move( pixelSize ));
   }
   Image distance = EuclideanDistanceTransform( region, S::OBJECT, S::TIES );
   dip::Histogram::Configuration configuration( 0, maxDistance, static_cast< int >( length - 1 ));
   configuration.excludeOutOfBoundValues = true;
   return PerObjectHistogram( distance, object, {}, configuration, S::FRACTION, S::INCLUDE );
}

} // namespace dip
