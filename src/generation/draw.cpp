/*
 * DIPlib 3.0
 * This file contains definitions for image drawing functions
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
#include "diplib/generation.h"
#include "diplib/border.h"
#include "diplib/generic_iterators.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
void dip__SetBorder( Image& out, Image::Pixel const& value, dip::uint size ) {
   dip::uint nTensor = out.TensorElements();
   // Copy `value` into an array with the right number of elements, and of the right data type
   std::vector< TPI > borderValues( nTensor, value[ 0 ].As< TPI >() );
   if( !value.IsScalar()) {
      for( dip::uint ii = 1; ii < nTensor; ++ii ) {
         borderValues[ ii ] = value[ ii ].As< TPI >();
      }
   }
   // Process the border
   detail::ProcessBorders< TPI, true, false >(
         out,
         [ &borderValues ]( auto* ptr, dip::sint tStride ) {
            for( auto v : borderValues ) {
               *ptr = v;
               ptr += tStride;
            }
         },
         []( auto, dip::sint ) {}, size );
}

} // namespace

void SetBorder( Image& out, Image::Pixel const& value, dip::uint size ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_OVL_CALL_ALL( dip__SetBorder, ( out, value, size ), out.DataType() );
}

namespace {

template< typename TPI >
void dip__DrawLine(
      Image& out,
      BresenhamLineIterator& iterator,
      Image::Pixel const& value
) {
   dip::uint nTensor = out.TensorElements();
   // Copy `value` into an array with the right number of elements, and of the right data type
   std::vector< TPI > color( nTensor, value[ 0 ].As< TPI >() );
   if( !value.IsScalar()) {
      for( dip::uint ii = 1; ii < nTensor; ++ii ) {
         color[ ii ] = value[ ii ].As< TPI >();
      }
   }
   // Process the line
   dip::sint stride = out.TensorStride();
   TPI* origin = static_cast< TPI* >( out.Origin() );
   do {
      dip::sint offset = *iterator;
      for( dip::uint ii = 0; ii < nTensor; ++ii ) {
         origin[ offset ] = color[ ii ];
         offset += stride;
      }
   } while( ++iterator );
}

} // namespace

void DrawLine(
      Image& out,
      UnsignedArray const& start,
      UnsignedArray const& end,
      Image::Pixel const& value
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( start.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( end.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( !( start < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   DIP_THROW_IF( !( end < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   BresenhamLineIterator iterator( out.Strides(), start, end );
   DIP_OVL_CALL_ALL( dip__DrawLine, ( out, iterator, value ), out.DataType() );
}

} // namespace dip
