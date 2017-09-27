/*
 * DIPlib 3.0 viewer
 * This file contains functionality for controlling the display options.
 *
 * (c)2017, Wouter Caarls
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

#undef DIP__ENABLE_DOCTEST
#include "diplib/viewer/viewer.h"

namespace dip { namespace viewer {

namespace {

template< typename TPI >
void viewer__ColorMap(Image const& slice, Image& out, ViewingOptions &options)
{
   auto mapping = options.mapping_;
   auto lut = options.lut_;
   auto element = options.element_;
   auto color_elements = options.color_elements_;

   dip::uint width = slice.Size( 0 );
   dip::uint height = slice.Size( 1 );
   dip::sint sliceStride0 = slice.Stride( 0 );
   dip::sint sliceStride1 = slice.Stride( 1 );
   dip::sint outStride0 = out.Stride( 0 );
   dip::sint outStride1 = out.Stride( 1 );
   dip::sint sliceStrideT = slice.TensorStride();

   double offset, scale;
   if( mapping == ViewingOptions::Mapping::Logarithmic )
   {
      offset = options.mapping_range_.first-1.;
      scale = 1./std::log(options.mapping_range_.second-offset);
   }
   else
   {
      offset = options.mapping_range_.first;
      scale = 1./(options.mapping_range_.second-options.mapping_range_.first);
   }

   TPI* slicePtr = static_cast< TPI* >( slice.Origin() );
   uint8* outPtr = static_cast< uint8* >( out.Origin() );
   for( dip::uint jj = 0; jj < height; ++jj, slicePtr += sliceStride1, outPtr += outStride1 )
   {
      TPI* iPtr = slicePtr;
      uint8* oPtr = outPtr;
      dip::uint ii;

      switch (lut)
      {
         case ViewingOptions::LookupTable::RGB:
            for (dip::uint kk=0; kk < 3; ++kk)
            {
               dip::sint elem = color_elements[kk];
               if (elem >= 0)
                  for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
                     oPtr[kk] = rangeMap((dip::sfloat)iPtr[elem*sliceStrideT], offset, scale, mapping);
               else
                  for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
                     oPtr[kk] = 0;
            }
            break;
         default:
            for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
               oPtr[0] = oPtr[1] = oPtr[2] = rangeMap((dip::sfloat)iPtr[(dip::sint)element*sliceStrideT], offset, scale, mapping);
            break;
      }
   }
}

} // namespace

void ApplyViewerColorMap(dip::Image &in, dip::Image &out, ViewingOptions &options)
{
   if (in.Dimensionality() == 0)
      out = dip::Image(dip::UnsignedArray {1, 1}, 3, DT_UINT8);
   else
      out = dip::Image(in.Sizes(), 3, DT_UINT8);

   DIP_OVL_CALL_NONCOMPLEX( viewer__ColorMap, ( in, out, options ), in.DataType() );

   switch(options.lut_)
   {
      case ViewingOptions::LookupTable::Sequential:
         ApplyColorMap(out[0], out, "linear");
         break;
      case ViewingOptions::LookupTable::Divergent:
         ApplyColorMap(out[0], out, "diverging");
         break;
      case ViewingOptions::LookupTable::Cyclic:
         ApplyColorMap(out[0], out, "cyclic");
         break;
      case ViewingOptions::LookupTable::Label:
         ApplyColorMap(out[0], out, "label");
         break;
      default:
         // Nothing to do
         break;
   }
}

}} // namespace dip::viewer
