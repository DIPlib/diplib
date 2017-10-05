/*
 * DIPlib 3.0
 * This file contains the definition for the dip::MeasurementToObject function.
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

#include <algorithm>

#include "diplib.h"
#include "diplib/measurement.h"
#include "diplib/lookup_table.h"

namespace dip {

void ObjectToMeasurement(
      Image const& label,
      Image& out,
      Measurement::IteratorFeature const& featureValues
) {
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   UnsignedArray const& objects = featureValues.Objects();
   dip::uint maxObject = *std::max_element( objects.begin(), objects.end() );
   dip::uint nElements = featureValues.NumberOfValues();
   bool protect = out.IsProtected();
   if( !protect ) {
      // If the user didn't protect the output image, we set it to SFLOAT, which is the default output type
      out.ReForge( label.Sizes(), nElements, DT_SFLOAT );
      out.Protect();
   }
   Image lutIm( { maxObject + 1 }, nElements, DT_DFLOAT );
   lutIm.Fill( 0.0 );
   DIP_ASSERT( lutIm.TensorStride() == 1 );
   dfloat* data = static_cast< dfloat* >( lutIm.Origin() );
   dip::sint stride = lutIm.Stride( 0 );
   auto it = featureValues.FirstObject();
   while( it ) {
      dfloat* dest = data + static_cast< dip::sint >( it.ObjectID() ) * stride;
      std::copy( it.begin(), it.end(), dest );
      ++it;
   }
   LookupTable lut( lutIm );
   lut.Apply( label, out );
   out.Protect( protect );
}

} // namespace dip
