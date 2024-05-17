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

#include "diplib/microscopy.h"

#include <cmath>
#include <memory>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/generation.h"
#include "diplib/statistics.h"

namespace dip {

constexpr dfloat hopkinsOftCutoff = 0.0001;

void IncoherentOTF(
      Image& out,
      dfloat defocus,
      dfloat oversampling,
      dfloat amplitude,
      String const& method
) {
   DIP_THROW_IF( out.Dimensionality() > 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( oversampling <= 0.0, E::INVALID_PARAMETER );
   DIP_THROW_IF( amplitude <= 0.0, E::INVALID_PARAMETER );
   bool stokseth{};
   DIP_STACK_TRACE_THIS( stokseth = BooleanFromString( method, S::STOKSETH, "Hopkins" ));
   if( out.Dimensionality() == 0 ) {
      // No sizes given
      DIP_STACK_TRACE_THIS( out.Strip() ); // just in case...
      out.SetSizes( { 256, 256 } );
   }
   DIP_STACK_TRACE_THIS( out.ReForge( out.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));

   Image radius;
   if( out.DataType().IsFloat() ) {
      // A floating-point output: we can use it for the radius value
      radius = out.QuickCopy();
   } else {
      // Otherwise, we need a new image for the radius value.
      radius.ReForge( out, DT_SFLOAT );
   }
   DIP_STACK_TRACE_THIS( FillRadiusCoordinate( radius, { S::FREQUENCY } ));

   std::unique_ptr< Framework::ScanLineFilter > filter;
   if( defocus == 0 ) {    // In-focus OTF

      filter = Framework::NewMonadicScanLineFilter< sfloat >(
            [ = ]( auto its ) {

               dfloat r = *its[ 0 ] * 2.0 * oversampling;
               if( r >= 1.0 ) { return 0.0f; }
               if( r == 0.0 ) { return static_cast< sfloat >( amplitude ); }
               return static_cast< sfloat >( amplitude * ( 2.0 / pi ) * ( std::acos( r ) - r * std::sqrt( 1 - r * r )));

            }, 1 );

   } else {
      if( stokseth ) {     // Out-of-focus OTF: Stokseth formulation

         filter = Framework::NewMonadicScanLineFilter< sfloat >(
               [ = ]( auto its ) {

                  dfloat r = *its[ 0 ] * 2.0 * oversampling;
                  if( r >= 1.0 ) { return 0.0f; }
                  if( r == 0.0 ) { return static_cast< sfloat >( amplitude ); }

                  dfloat s = 2.0 * r;
                  dfloat rr = ( 1.0 - 0.69 * s + 0.0076 * s * s + 0.043 * s * s * s );
                  dfloat x = 4.0 * pi * defocus * s * ( 1.0 - r );
                  s = 2.0 * BesselJ1( x ) / x;
                  return static_cast< sfloat >( amplitude * rr * s );

               }, 1 );

      } else {             // Out-of-focus OTF: Hopkins formulation

         filter = Framework::NewMonadicScanLineFilter< sfloat >(
               [ = ]( auto its ) {

                  dfloat r = *its[ 0 ] * 2.0 * oversampling;
                  if( r >= 1.0 ) { return 0.0f; }
                  if( r == 0.0 ) { return static_cast< sfloat >( amplitude ); }

                  dfloat s = 2.0 * r;
                  dfloat a = 4.0 * pi * defocus * s;
                  dfloat beta = std::acos( 0.5 * s );
                  dfloat bj1 = beta * BesselJ1( a );
                  dfloat sign = 1.0;
                  dip::uint n2 = 2;
                  dfloat sinb = sign * std::sin( static_cast< dfloat >( n2 ) * beta ) / static_cast< dfloat >( n2 );
                  dfloat bjn = sinb * ( BesselJN( a, n2 - 1 ) - BesselJN( a, n2 + 1 ));
                  dfloat sum1 = bj1 + bjn;
                  while( std::fabs( bjn ) / std::fabs( sum1 ) > hopkinsOftCutoff ) {
                     n2 += 2;
                     sign *= -1.0;
                     sinb = sign * std::sin( static_cast< dfloat >( n2 ) * beta ) / static_cast< dfloat >( n2 );
                     bjn = sinb * ( BesselJN( a, n2 - 1 ) - BesselJN( a, n2 + 1 ));
                     sum1 += bjn;
                  }
                  sum1 *= std::cos( 0.5 * a * s );

                  sign = 1.0;
                  n2 = 0;
                  dfloat sina = std::sin( 0.5 * a * s );
                  sinb = sign * std::sin( static_cast< dfloat >( n2 + 1 ) * beta ) / static_cast< dfloat >( n2 + 1 );
                  bjn = sina * sinb * ( BesselJN( a, n2 ) - BesselJN( a, n2 + 2 ));
                  dfloat sum2 = bjn;
                  while( std::fabs( bjn ) / std::fabs( sum1 - sum2 ) > hopkinsOftCutoff ) {
                     n2 += 2;
                     sign *= -1.0;
                     sinb = sign * std::sin( static_cast< dfloat >( n2 + 1 ) * beta ) / static_cast< dfloat >( n2 + 1 );
                     bjn = sina * sinb * ( BesselJN( a, n2 ) - BesselJN( a, n2 + 2 ));
                     sum2 += bjn;
                  }
                  return static_cast< sfloat >( amplitude * ( 4.0 / ( pi * a )) * ( sum1 - sum2 ));

               }, 1 );
      }
   }
   DIP_STACK_TRACE_THIS( dip::Framework::ScanMonadic( radius, out, DT_SFLOAT, out.DataType(), 1, *filter ));
}

void IncoherentPSF(
      Image& out,
      dfloat oversampling,
      dfloat amplitude
) {
   DIP_THROW_IF( out.Dimensionality() > 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( oversampling <= 0.0, E::INVALID_PARAMETER );
   DIP_THROW_IF( amplitude <= 0.0, E::INVALID_PARAMETER );
   if( out.Dimensionality() == 0 ) {
      // No sizes given
      DIP_STACK_TRACE_THIS( out.Strip() ); // just in case...
      dip::uint size = static_cast< dip::uint >( std::ceil( oversampling * 19 ));
      out.SetSizes( { size, size } );
   }
   DIP_STACK_TRACE_THIS( out.ReForge( out.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   Image radius;
   if( out.DataType().IsFloat() ) {
      // A floating-point output: we can use it for the radius value
      radius = out.QuickCopy();
   } else {
      // Otherwise, we need a new image for the radius value.
      radius.ReForge( out, DT_SFLOAT );
   }
   DIP_STACK_TRACE_THIS( FillRadiusCoordinate( radius ));
   dfloat cutoff = pi / ( 2.0 * oversampling );
   std::unique_ptr< Framework::ScanLineFilter > filter = Framework::NewMonadicScanLineFilter< sfloat >(
         [ = ]( auto its ) {
            dfloat r = *its[ 0 ] * cutoff;
            if( r == 0.0 ) { return 1.0f; }
            dfloat b = 2.0 * BesselJ1( r ) / r;
            return static_cast< sfloat >( b * b );
         }, 1 );
   DIP_STACK_TRACE_THIS( dip::Framework::ScanMonadic( radius, out, DT_SFLOAT, out.DataType(), 1, *filter ));
   amplitude /= Sum( out ).As< double >();
   out *= amplitude;
}

} // namespace dip
