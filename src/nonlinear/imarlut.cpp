/*
 * DIPlib 3.0
 * This file contains definitions of functions for lookup responses from an image array.
 *
 * (c)2018, Erik Schuitema.
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

#include "diplib/nonlinear.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip
{

// First input buffer data type is DFLOAT, output data type is TPO and is equal to the vals buffer data type
template< typename TPO >
class ImageArrayLUTLineFilter : public Framework::ScanLineFilter {
public:
   virtual void Filter( Framework::ScanLineFilterParameters const& params )  override {

   }

   //virtual void SetNumberOfThreads( dip::uint  threads ) { (void)threads; }
   virtual dip::uint GetNumberOfOperations( dip::uint nInput, dip::uint nOutput, dip::uint nTensorElements ) override {
      // TODO: find a sensible return value
      return std::max( nInput, nOutput ) * nTensorElements;
   }

};

DIP_EXPORT void ImageArrayLUT(
   Image const& in,
   Image& out,
   FloatArray index,
   ImageArray const& vals,
   String const& interpolationMethod
) {
   // Make sure the sizes of bins and vals correspond and are >= 1
   DIP_THROW_IF( index.size() != vals.size(), "Number of bins must equal number of vals" );
   DIP_THROW_IF( index.size() < 1, "At least 1 bin and val needed" );
   // Make sure in and vals have equal number of tensor elements
   // Make sure all vals entries have equal data types
   dip::uint tensorElements = in.TensorElements();
   DataType valsDataType = vals.front().DataType();
   for( ImageArray::const_iterator itVals = vals.begin(); itVals != vals.end(); ++itVals ) {
      DIP_THROW_IF( itVals->TensorElements() != tensorElements, "Vals image must have equal number of tensor elements as input image" );
      DIP_THROW_IF( itVals->DataType() != valsDataType, "Vals images must have equal data type" );
   }

   // Input images: [ in, vals... ]
   ImageConstRefArray inRefs = CreateImageConstRefArray( vals );
   inRefs.insert( inRefs.begin(), in );
   // Input buffer data types: [ DFLOAT, valsDataType... ]
   // The input buffer type for `in` is chosen as DFLOAT so that these values can be easily looked up in the bins array
   DataTypeArray inBufferTypes( vals.size(), valsDataType );
   inBufferTypes.insert( 0, in.DataType() );

   // Obtain output data type that can hold interpolated values between vals
   DataType outDataType = DataType::SuggestFlex( valsDataType );
   DataTypeArray outBufferTypes( { outDataType } );
   ImageRefArray outRefs{ out };

   // Call Scan framework
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, ImageArrayLUTLineFilter, (), outDataType );
   Framework::Scan( inRefs, outRefs, inBufferTypes, outBufferTypes, { outDataType }, { tensorElements }, *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim );
}

} // namespace dip
