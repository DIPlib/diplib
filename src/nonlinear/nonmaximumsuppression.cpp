/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the non-maximum suppression.
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
#include "diplib/nonlinear.h"
#include "diplib/math.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
void NonMaximumSuppression1D(
      Image const& gradmag,
      Image const& mask,
      Image& out
) {
   dip::uint size = gradmag.Size( 0 );
   dip::sint instride = gradmag.Stride( 0 );
   TPI const* pin = static_cast< TPI const* >( gradmag.Origin() );
   dip::sint outstride = out.Stride( 0 );
   TPI* pout = static_cast< TPI* >( out.Origin() );
   dip::sint maskstride = 0;
   bin* pmask = nullptr;
   if( mask.IsForged() ) {
      maskstride = mask.Stride( 0 );
      pmask = static_cast< bin* >( mask.Origin() );
   }
   // First pixel
   *pout = 0;
   pin += instride;
   pout += outstride;
   pmask += maskstride; // if !pmask, adds zero to the nullptr.
   // The bulk of the pixels
   for( dip::uint ii = 1; ii < size - 1; ++ii ) {
      if(( !pmask || *pmask ) && ( *pin > 0 )) {
         TPI m1 = *(pin-instride);
         TPI m2 = *(pin+instride);
         if((( *pin > m1 ) && ( *pin >= m2 )) || (( *pin >= m1 ) && ( *pin > m2 ))) {
            *pout = *pin;
         } else {
            *pout = 0;
         }
      } else {
         *pout = 0;
      }
      pin += instride;
      pout += outstride;
      pmask += maskstride;
   }
   // Last pixel
   *pout = 0;
}

template< typename TPI >
void NonMaximumSuppression2D(
      Image const& gradmag,
      Image const& gradient,
      Image const& mask,
      Image& out
) {
   dip::uint sizex = gradmag.Size( 0 );
   dip::uint sizey = gradmag.Size( 1 );

   dip::sint gmstridex = gradmag.Stride( 0 );
   dip::sint gmstridey = gradmag.Stride( 1 );
   TPI const* pgm = static_cast< TPI const* >( gradmag.Origin() );

   dip::sint gvstridex = gradient.Stride( 0 );
   dip::sint gvstridey = gradient.Stride( 1 );
   dip::sint one = gradient.TensorStride();
   TPI const* pgv = static_cast< TPI const* >( gradient.Origin() );

   dip::sint outstridex = out.Stride( 0 );
   dip::sint outstridey = out.Stride( 1 );
   TPI* pout = static_cast< TPI* >( out.Origin() );

   dip::sint maskstridex = 0;
   dip::sint maskstridey = 0;
   bin* pmask = nullptr;
   if( mask.IsForged() ) {
      maskstridex = mask.Stride( 0 );
      maskstridey = mask.Stride( 1 );
      pmask = static_cast< bin* >( mask.Origin() );
   }

   // First row of pixels
   TPI* ppout = pout;
   for( dip::uint xx = 0; xx < sizex; ++xx ) {
      *ppout = 0;
      ppout += outstridex;
   }

   // The bulk of the rows of pixels
   for( dip::uint yy = 1; yy < sizey - 1; ++yy ) {

      pgm += gmstridey;
      pgv += gvstridey;
      pout += outstridey;
      pmask += maskstridey;

      TPI const* ppgm = pgm;
      TPI const* ppgv = pgv;
      ppout = pout;
      bin* ppmask = pmask;

      // First pixel
      *ppout = 0;
      ppgm += gmstridex;
      ppgv += gvstridex;
      ppout += outstridex;
      ppmask += maskstridex; // if !ppmask, adds zero to the nullptr.

      // The bulk of the pixels
      for( dip::uint xx = 1; xx < sizex - 1; ++xx ) {
         if(( !ppmask || *ppmask ) && ( *ppgm > 0 )) {

            // Get gradient values to interpolate between
            TPI dx = ppgv[ 0 ];
            TPI dy = ppgv[ one ];
            TPI absdx = std::abs( dx );
            TPI absdy = std::abs( dy );
            TPI delta, mag1, mag2, mag3, mag4;
            if( absdy > absdx ) {
               delta = absdx / absdy;
               // Get values above and below the current point
               mag2 = *( ppgm - gmstridey );
               mag4 = *( ppgm + gmstridey );
               // Get values diagonally w.r.t. the current point
               if( std::signbit( dx ) != std::signbit( dy )) {
                  mag1 = *( ppgm - gmstridey + gmstridex );
                  mag3 = *( ppgm + gmstridey - gmstridex );
               } else {
                  mag1 = *( ppgm - gmstridey - gmstridex );
                  mag3 = *( ppgm + gmstridey + gmstridex );
               }
            } else {
               delta = absdy / absdx;
               // Get values left and right of the current point
               mag2 = *( ppgm + gmstridex );
               mag4 = *( ppgm - gmstridex );
               // Get values diagonally w.r.t. the current point
               if( std::signbit( dx ) != std::signbit( dy )) {
                  mag1 = *( ppgm - gmstridey + gmstridex );
                  mag3 = *( ppgm + gmstridey - gmstridex );
               } else {
                  mag1 = *( ppgm + gmstridey + gmstridex );
                  mag3 = *( ppgm - gmstridey - gmstridex );
               }
            }

            // Interpolate gradient magnitude values
            TPI m1 = delta * mag1 + ( 1 - delta ) * mag2;
            TPI m2 = delta * mag3 + ( 1 - delta ) * mag4;

            // Determine if the current point is a maximum point
            // Note that at most one side is allowed to be 'flat'
            if((( *ppgm > m1 ) && ( *ppgm >= m2 )) || (( *ppgm >= m1 ) && ( *ppgm > m2 ))) {
               *ppout = *ppgm;
            } else {
               *ppout = 0;
            }
         } else {
            *ppout = 0;
         }
         ppgm += gmstridex;
         ppgv += gvstridex;
         ppout += outstridex;
         ppmask += maskstridex;
      }

      // Last pixel
      *ppout = 0;
      std::cout << "." << std::endl;
   }

   // Last row of pixels
   pout += outstridey;
   ppout = pout;
   for( dip::uint xx = 0; xx < sizex; ++xx ) {
      *ppout = 0;
      ppout += outstridex;
   }
}

template< typename TPI >
void NonMaximumSuppression3D(
      Image const& gradmag,
      Image const& gradient,
      Image const& mask,
      Image& out
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

} // namespace

void NonMaximumSuppression(
      Image const& c_gradmag,
      Image const& c_gradient,
      Image const& c_mask,
      Image& out
) {
   Image gradient;
   Image gradmag;
   dip::uint nDims;
   if( c_gradmag.IsForged() && ( c_gradmag.Dimensionality() == 1 )) {

      // In this case we can ignore c_gradient
      gradmag = c_gradmag.QuickCopy();
      nDims = 1;

   } else {

      gradient = c_gradient.QuickCopy();

      DIP_THROW_IF( !gradient.IsForged(), E::IMAGE_NOT_FORGED );
      nDims = gradient.Dimensionality();
      DIP_THROW_IF(( nDims < 1 ) || ( nDims > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
      DIP_THROW_IF( gradient.TensorElements() != nDims, E::NTENSORELEM_DONT_MATCH );
      DIP_THROW_IF( !gradient.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );

      gradmag = c_gradmag.QuickCopy();
      if( gradmag.IsForged()) {
         DIP_THROW_IF( !gradmag.IsScalar(), E::IMAGE_NOT_SCALAR );
         DIP_THROW_IF( gradmag.Sizes() != gradient.Sizes(), E::SIZES_DONT_MATCH );
         DIP_THROW_IF( gradmag.DataType() != gradient.DataType(), E::DATA_TYPES_DONT_MATCH );
      } else {
         DIP_STACK_TRACE_THIS( gradmag = Norm( gradient ));
      }

   }

   Image mask;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( gradient.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( gradient.Sizes() );
      DIP_END_STACK_TRACE
   }

   PixelSize ps = gradmag.HasPixelSize() ? gradmag.PixelSize() : gradient.PixelSize();
   out.ReForge( gradmag );
   out.SetPixelSize( ps );

   out.Fill( 45 );

   switch( nDims ) {
      case 1:
         DIP_OVL_CALL_FLOAT( NonMaximumSuppression1D, ( gradmag, mask, out ), gradmag.DataType() );
         break;
      case 2:
         DIP_OVL_CALL_FLOAT( NonMaximumSuppression2D, ( gradmag, gradient, mask, out ), gradmag.DataType() );
         break;
      case 3:
      default: // we've already tested for 1 <= nDims <= 3
         DIP_OVL_CALL_FLOAT( NonMaximumSuppression3D, ( gradmag, gradient, mask, out ), gradmag.DataType() );
         break;
   }
}

} // namespace dip
