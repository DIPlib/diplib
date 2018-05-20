/*
 * DIPlib 3.0
 * This file contains definitions for the granulometry
 *
 * (c)2018, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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
#include "diplib/statistics.h"
#include "diplib/morphology.h"
#include "diplib/generation.h"
#include "diplib/geometry.h"
#include "diplib/mapping.h"

namespace dip {

Distribution Granulometry(
      Image const& in,
      Image const& mask,
      std::vector< dfloat > const& in_scales,
      String const& type,
      String const& polarity,
      StringSet const& options
) {
   static std::vector< dfloat > const default_scales{ 1.41, 2.00, 2.83, 4.00, 5.66, 8.00, 11.31, 16.00, 22.63, 32.00, 45.25, 64.00 };
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   // Scales
   std::vector< dfloat > scales = in_scales.empty() ? default_scales : in_scales; // copy
   std::sort( scales.begin(), scales.end() );
   DIP_THROW_IF( scales[ 0 ] <= 1.0, E::PARAMETER_OUT_OF_RANGE ); // all scales must be larger than 1
   // Type
   bool isotropic;
   DIP_STACK_TRACE_THIS( isotropic = BooleanFromString( type, "isotropic", "length" ));
   // Polarity
   bool opening;
   DIP_STACK_TRACE_THIS( opening = BooleanFromString( polarity, S::OPENING, S::CLOSING ));
   // Options
   bool reconstruction = false;
   bool shifted = false;
   bool interpolate = false;
   bool subsample = false;
   bool constrained = true;
   bool robust = false;
   for( auto& option : options ) {
      if( isotropic && ( option == "reconstruction" )) {
         reconstruction = true;
      } else if( isotropic && ( option == "shifted" )) {
         shifted = true;
      } else if( isotropic && ( option == "interpolate" )) {
         interpolate = true;
      } else if( isotropic && ( option == "subsample" )) {
         subsample = true;
      } else if( !isotropic && ( option == "non-constrained" )) {
         constrained = false;
      } else if( !isotropic && ( option == "robust" )) {
         robust = true;
      } else {
         DIP_THROW_INVALID_FLAG( option );
      }
   }

   // Scaling
   auto maxmin = MaximumAndMinimum( in, mask );
   dfloat offset = Mean( in, mask ).As< dfloat >();
   dfloat gain = 1 / (( opening ? maxmin.Minimum() : maxmin.Maximum() ) - offset );

   // Output
   Distribution out( scales );

   if( isotropic ) {
      // Isotropic opening/closing

      // Shifted SEs
      FloatArray center;
      if( shifted ) {
         switch( nDims ) {
            case 1:
               center = { 0.25 };
               break;
            case 2:
               center = { 0.19, 0.31 };
               break;
            case 3:
               center = { 0.16, 0.24, 0.34 };
               break;
            default:
               // For higher dimensionalities we don't know the proper shift, let's ignore the flag
               shifted = false;
               break;
         }
      }
      Image radiusSE;
      if( shifted ) {
         dip::uint maxDiameter = static_cast< dip::uint >( std::ceil( scales.back() / 2 )) * 2 + 3;
         UnsignedArray sz( nDims, maxDiameter );
         radiusSE = CreateRamp( sz, 0 );
         radiusSE += center[ 0 ];
         if( nDims > 1 ) {
            Square( radiusSE, radiusSE );
            auto tmp = CreateRamp( sz, 1 );
            tmp += center[ 1 ];
            Square( tmp, tmp );
            radiusSE += tmp;
            if( nDims > 2 ) {
               CreateRamp( tmp, sz, 2 );
               tmp += center[ 2 ];
               Square( tmp, tmp );
               radiusSE += tmp;
            }
            Sqrt( radiusSE, radiusSE );
         }
      }

      dfloat currentZoom = 1;
      Image scaledIn = in.QuickCopy();
      Image scaledMask = mask.IsForged() ? mask.QuickCopy() : Image{};
      Image tmp;
      for( dip::uint ii = 0; ii < scales.size(); ++ii ) {

         // Do we want to scale?
         dfloat zoom = 1;
         if( subsample && scales[ ii ] > 64 ) {
            zoom = 1 / std::ceil( scales[ ii ] / 64 );
            if( zoom != currentZoom ) {
               // Subsample
               if( opening ) {
                  Erosion( in, scaledIn, { 1 / zoom, "rectangular" } );
               } else {
                  Dilation( in, scaledIn, { 1 / zoom, "rectangular" } );
               }
               Subsampling( scaledIn, scaledIn, { static_cast< dip::uint >( 1 / zoom ) } );
               if( mask.IsForged()) {
                  Subsampling( mask, scaledMask, { static_cast< dip::uint >( 1 / zoom ) } );
               }
               currentZoom = zoom;
               //std::cout << "Subsampling, currentZoom = " << currentZoom << ", sizes = " << scaledIn.Sizes() << '\n';
            }
         } else if( interpolate && scales[ ii ] < 8 ) {
            zoom = 8 / scales[ ii ];
            if( zoom != currentZoom ) {
               // Interpolate
               Resampling( in, scaledIn, { zoom }, { 0 }, S::CUBIC_ORDER_3 );
               Clip( scaledIn, scaledIn, maxmin.Minimum(), maxmin.Maximum(), S::BOTH );
               if( mask.IsForged()) {
                  Resampling( mask, scaledMask, { zoom }, { 0 }, S::NEAREST );
               }
               currentZoom = zoom;
               //std::cout << "Resampling, currentZoom = " << currentZoom << ", sizes = " << scaledIn.Sizes() << '\n';
            }
         } else if( currentZoom != 1 ) {
            scaledIn = in.QuickCopy();
            if( mask.IsForged()) {
               scaledMask = mask.QuickCopy();
            }
            currentZoom = 1;
            //std::cout << "Resetting, currentZoom = " << currentZoom << ", sizes = " << scaledIn.Sizes() << '\n';
         }

         // Filter
         StructuringElement se;
         if( shifted ) {
            se = radiusSE < ( scales[ ii ] * currentZoom / 2.0 );
         } else {
            se = { scales[ ii ] * currentZoom, S::ELLIPTIC };
         }
         if( reconstruction ) {
            opening ? OpeningByReconstruction( scaledIn, tmp, se ) : ClosingByReconstruction( scaledIn, tmp, se );
            //std::cout << "By reconstruction, scale = " << scales[ ii ] << ", currentZoom = " << currentZoom << '\n';
         } else {
            opening ? Opening( scaledIn, tmp, se ) : Closing( scaledIn, tmp, se );
            //std::cout << "Structural, scale = " << scales[ ii ] << ", currentZoom = " << currentZoom << '\n';
         }

         // Normalized average
         dfloat result = Mean( tmp, mask ).As< dfloat >();
         out[ ii ].Y() = clamp(( result - offset ) * gain, 0.0, 1.0 ); // Clamping is necessary if we interpolate and/or subsample
      }

   } else {
      // Path opening/closing

      String mode = constrained ? S::CONSTRAINED : S::NORMAL;
      Image tmp;
      StructuringElement robustSE{ 2, "rectangular" };
      for( dip::uint ii = 0; ii < scales.size(); ++ii ) {
         if( robust ) {
            opening ? Dilation( in, tmp, robustSE ) : Erosion( in, tmp, robustSE );
            PathOpening( tmp, {}, tmp, static_cast< dip::uint >( scales[ ii ] ), polarity, mode );
            opening ? Erosion( tmp, tmp, robustSE ) : Dilation( tmp, tmp, robustSE );
            //std::cout << "Robust path, scale = " << scales[ ii ] << ", mode = " << mode << '\n';
         } else {
            PathOpening( in, {}, tmp, static_cast< dip::uint >( scales[ ii ] ), polarity, mode );
            //std::cout << "Path, scale = " << scales[ ii ] << ", mode = " << mode << '\n';
         }
         dfloat result = Mean( tmp, mask ).As< dfloat >();
         out[ ii ].Y() = ( result - offset ) * gain;
      }

   }

   return out;
}

} // namespace dip
