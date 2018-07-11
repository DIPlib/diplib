/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the FIR Gabor filter.
 *
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
#include "diplib/linear.h"
#include "diplib/generation.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"

namespace dip {

namespace {

inline dip::uint HalfGaborSize(
      dfloat sigma,
      dfloat truncation
) {
   return clamp_cast< dip::uint >( std::ceil( truncation * sigma ));
}

// Creates half of a complex-valued Gabor kernel, with the x=0 at the right end (last element) of the output array.
// `out[0] + i*out[1]` is the first element, etc. As expected by `OneDimensionalFilter`.
std::vector< dfloat > MakeHalfGabor(
      dfloat sigma,
      dfloat frequency,
      dfloat truncation
) {
   dip::uint halfFilterSize = 1 + HalfGaborSize( sigma, truncation );
   std::vector< dfloat > filter( 2 * halfFilterSize );
   auto ptr = reinterpret_cast< dcomplex* >( filter.data() );
   dip::uint r0 = halfFilterSize - 1;
   dfloat factor = -1.0 / ( 2.0 * sigma * sigma );
   dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigma );
   frequency *= 2 * pi;
   ptr[ r0 ] = norm;
   for( dip::uint rr = 1; rr < halfFilterSize; rr++ ) {
      dfloat rad = static_cast< dfloat >( rr );
      dfloat g = std::exp( factor * ( rad * rad )) * norm;
      dfloat phi = rad * frequency;
      ptr[ r0 - rr ] = g * dcomplex{ std::cos( phi ), -std::sin( phi ) };
   }
   return filter;
}

// Create 1D full Gabor filter
std::vector< dfloat > MakeGabor(
      dfloat sigma,
      dfloat frequency,
      dfloat truncation
) {
   // Handle sigma == 0.0
   if( sigma == 0.0 ) {
      return { 1.0 };
   }
   // Create half Gabor
   std::vector< dfloat > gabor;
   DIP_STACK_TRACE_THIS( gabor = MakeHalfGabor( sigma, frequency, truncation ));
   dip::uint halfFilterSize = gabor.size() / 2 - 1;
   // Complete the Gabor
   gabor.resize( 2 * ( halfFilterSize * 2 + 1 ));
   auto ptr = reinterpret_cast< dcomplex* >( gabor.data() );
   for( dip::uint ii = 1; ii <= halfFilterSize; ++ii ) {
      ptr[ halfFilterSize + ii ] = std::conj( ptr[ halfFilterSize - ii ] );
   }
   return gabor;
}

} // namespace

void CreateGabor(
      Image& out,
      FloatArray const& sigmas,
      FloatArray const& frequencies,
      dfloat truncation
) {
   // Verify dimensionality
   dip::uint nDims = sigmas.size();
   DIP_THROW_IF( frequencies.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );

   // Adjust truncation to default if needed
   if( truncation <= 0.0 ) {
      truncation = 3.0;
   }

   // Create 1D gaussian for each dimension
   std::vector< std::vector< dfloat >> gabors( nDims );
   std::vector< dcomplex const* > ptrs( nDims );
   UnsignedArray outSizes( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_STACK_TRACE_THIS( gabors[ ii ] = MakeGabor( sigmas[ ii ], frequencies[ ii ], truncation ));
      outSizes[ ii ] = gabors[ ii ].size() / 2;
      ptrs[ ii ] =  reinterpret_cast< dcomplex* >( gabors[ ii ].data() );
   }

   // Create output image
   std::cout << "[CreateGabor] sigmas = " << sigmas << '\n';
   std::cout << "[CreateGabor] outSizes = " << outSizes << '\n';
   out.ReForge( outSizes, 1, DT_DCOMPLEX );
   std::cout << "[CreateGabor] out = " << out << '\n';
   ImageIterator< dcomplex > itOut( out );
   do {
      const UnsignedArray& coords = itOut.Coordinates();
      // Multiply 1D Kernels
      dcomplex value = 1.0;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         value *= ptrs[ ii ][ coords[ ii ]];
      }
      *itOut = value;
   } while( ++itOut );
}

void GaborFIR(
      Image const& in_c,
      Image& out_c,
      FloatArray sigmas,
      FloatArray const& frequencies,
      StringArray const& boundaryCondition,
      BooleanArray process,
      dfloat truncation
) {
   DIP_THROW_IF( !in_c.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in_c.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( process, nDims, true );
   DIP_END_STACK_TRACE
   OneDimensionalFilterArray filter( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( sigmas[ ii ] > 0.0 ) && ( in_c.Size( ii ) > 1 )) {
         bool found = false;
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            if( process[ jj ] && ( sigmas[ jj ] == sigmas[ ii ] ) && ( frequencies[ jj ] == frequencies[ ii ] )) {
               filter[ ii ] = filter[ jj ];
               found = true;
               break;
            }
         }
         if( !found ) {
            filter[ ii ].symmetry = S::CONJ;
            filter[ ii ].isComplex = true;
            filter[ ii ].filter = MakeHalfGabor( sigmas[ ii ], frequencies[ ii ], truncation );
            // NOTE: origin defaults to the middle of the filter, so we don't need to set it explicitly here.
         }
      } else {
         process[ ii ] = false;
      }
   }
   Image in = in_c;
   if( out_c.Aliases( in ) ) {
      out_c.Strip();
   }
   SeparableConvolution( in, out_c, filter, boundaryCondition, process );
}

} // namespace dip
