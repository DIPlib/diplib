/*
 * DIPlib 3.0
 * This file contains the definition for the CumulativeSum function.
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
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
class CumSumFilter : public Framework::SeparableLineFilter {
      void Filter( Framework::SeparableLineFilterParameters const& params ) {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         TPI sum = 0;
         for( dip::uint ii = 0; ii < length; ++ii ) {
            sum += *in;
            *out = sum;
            in += inStride;
            out += outStride;
         }
      }
};

} // namespace

void CumulativeSum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DataType dataType = DataType::SuggestFlex( in.DataType() );
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_FLEX( lineFilter, CumSumFilter, (), dataType );
   if( mask.IsForged() ) {
      Select( in, Image( 0, dataType ), mask, out );
      Framework::Separable( out, out, dataType, dataType,
                            process, { 0 }, {}, *lineFilter,
                            Framework::Separable_AsScalarImage );
   }
   Framework::Separable( in, out, dataType, dataType,
                         process, { 0 }, {}, *lineFilter,
                         Framework::Separable_AsScalarImage );
}

} // namespace dip
