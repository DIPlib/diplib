/*
 * (c)2018, Cris Luengo.
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
#include "diplib/microscopy.h"
#include "diplib/math.h"
#include "diplib/mapping.h"

namespace dip {

void BeerLambertMapping(
      Image const& in,
      Image& out,
      Image::Pixel const& background
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal() || !background.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF(( background.TensorElements() != 1 ) && ( background.TensorElements() != in.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DataType dt = DataType::SuggestFloat( in.DataType() );
   Divide( in, background, out, dt );
   Clip( out, out, 1e-6, 1.0 );
   Log10( out, out );
   Invert( out, out );
}

void InverseBeerLambertMapping(
      Image const& in,
      Image& out,
      Image::Pixel const& background
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal() || !background.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF(( background.TensorElements() != 1 ) && ( background.TensorElements() != in.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   if( out.IsProtected() && ( !out.DataType().IsFloat() )) {
      // Use temporary float image for intermediate results
      Image tmp;
      if( in.DataType().IsFloat() ) {
         Invert( in, tmp );
      } else {
         Convert( in, tmp, DataType::SuggestFloat( in.DataType() ));
         Invert( tmp, tmp );
      }
      Exp10( tmp, tmp );
      Clip( tmp, tmp, 0.0, 1.0 );
      MultiplySampleWise( tmp, background, out, tmp.DataType() );
   } else {
      // We can use `out` for intermediate results
      if( in.DataType().IsFloat() ) {
         Invert( in, out );
      } else {
         Convert( in, out, DataType::SuggestFloat( in.DataType() ));
         Invert( out, out );
      }
      Exp10( out, out );
      Clip( out, out, 0.0, 1.0 );
      MultiplySampleWise( out, background, out, out.DataType() );
   }
}

void UnmixStains(
      Image const& in,
      Image& out,
      std::vector< Image::Pixel > const& stains
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint channels = in.TensorElements();
   dip::uint dyes = stains.size();
   DataType dt = DataType::SuggestFloat( in.DataType() );
   Image S{ {}, channels * dyes, dt };
   S.ReshapeTensor( channels, dyes );
   for( dip::uint ii = 0; ii < dyes; ++ii ) {
      DIP_THROW_IF( stains[ ii ].TensorElements() != channels, E::NTENSORELEM_DONT_MATCH );
      S.TensorColumn( ii ) = stains[ ii ];
   }
   Image U = PseudoInverse( S );
   Image col_in = in;
   col_in.ReshapeTensorAsVector();
   Multiply( U, in, out, dt );
   out.ResetColorSpace();
}

void MixStains(
      Image const& in,
      Image& out,
      std::vector< Image::Pixel > const& stains
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint dyes = stains.size();
   DIP_THROW_IF( in.TensorElements() != dyes, E::NTENSORELEM_DONT_MATCH ); // this means dyes >= 1
   dip::uint channels = stains[ 0 ].TensorElements();
   DataType dt = DataType::SuggestFloat( in.DataType() );
   Image S{ {}, channels * dyes, dt };
   S.ReshapeTensor( channels, dyes );
   for( dip::uint ii = 0; ii < dyes; ++ii ) {
      DIP_THROW_IF( stains[ ii ].TensorElements() != channels, E::NTENSORELEM_DONT_MATCH );
      S.TensorColumn( ii ) = stains[ ii ];
   }
   Image col_in = in;
   col_in.ReshapeTensorAsVector();
   Multiply( S, col_in, out, dt );
   if( channels == 3 ) {
      out.SetColorSpace( "RGB" );
   }
}

} // namespace dip
