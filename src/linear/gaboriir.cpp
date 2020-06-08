/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the IIR Gabor filter.
 *
 * (c)2018, Erik Schuitema, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * This function was originally written by Lucas van Vliet.
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
#include "diplib/framework.h"

namespace dip {

namespace {

constexpr dip::uint MAX_IIR_ORDER = 6;

struct GaborIIRParams
{
   dfloat sigma;
   dfloat frequency;
   dip::uint border;
   std::array< dip::uint, 6 > iir_order_num;
   std::array< dip::uint, 6 > iir_order_den;
   std::array< dcomplex, MAX_IIR_ORDER > a1;
   std::array< dcomplex, MAX_IIR_ORDER > a2;
   std::array< dcomplex, MAX_IIR_ORDER > b1;
   std::array< dcomplex, MAX_IIR_ORDER > b2;
   dcomplex cc;
};

// Get Gabor IIR filter parameters for one dimension
GaborIIRParams FillGaborIIRParams(
   dfloat sigma,
   dfloat frequency,
   dfloat truncation
) {
   GaborIIRParams params;
   params.sigma = sigma;
   params.frequency = frequency;
   params.border = static_cast< dip::uint >( std::round( sigma * truncation ));

   params.iir_order_num = {{ 0, 0, 0, 0, 0, 0 }};
   params.iir_order_den = {{ 3, 1, 3, 3, 1, 3 }};

   // Init a1, a2, b1, b2 and cc with zero
   params.a1.fill( 0.0 );
   params.a2.fill( 0.0 );
   params.b1.fill( 0.0 );
   params.b2.fill( 0.0 );
   params.cc = 0.0;

   dfloat q;
   if( sigma > 0 ) {
      if( sigma >= 2.5 ) {
         q = sigma * 0.98711 - 0.96330;
      } else {
         q = 3.97156 - 4.14554 * sqrt( 1.0 - 0.26891 * sigma );
      }
   } else {
      q = -sigma;
   }

   dfloat w = frequency;

   // Scale the denominator coefficients
   dfloat m0 = 1.1668048;
   dfloat m1 = 1.1078345;
   dfloat m2 = 1.4058574;
   dfloat scale = (m0 + q)*(m1*m1 + m2*m2 + 2 * m1*q + q*q);

   // Non-recursive forward/backward scan
   params.a1[ 0 ] = 1.0;
   params.a2[ 0 ] = 1.0;

   // recursive forward/backward scan
   params.b1[ 0 ].real( 1.0 );
   params.b1[ 1 ].real( -1 * q*(2 * m0*m1 + m1*m1 + m2*m2 + (2 * m0 + 4 * m1)*q + 3 * q*q) / scale );
   params.b1[ 2 ].real( q*q*(m0 + 2 * m1 + 3 * q) / scale );
   params.b1[ 3 ].real( -1 * (q*q*q) / scale );

   // Normalization
   params.cc.real(( params.b1[ 0 ].real() + params.b1[ 1 ].real() + params.b1[ 2 ].real() + params.b1[ 3 ].real() ));
   params.cc.real(( params.cc.real() * params.cc.real() ) / ( params.a1[ 0 ].real() * params.a2[ 0 ].real() ));
   params.cc.imag( 0.0 );

   // Actual coefficients
   params.b1[ 0 ].imag( 0.0 );
   params.b1[ 1 ].imag( std::sin( 1 * 2 * dip::pi*w ) * params.b1[ 1 ].real() );
   params.b1[ 2 ].imag( std::sin( 2 * 2 * dip::pi*w ) * params.b1[ 2 ].real() );
   params.b1[ 3 ].imag( std::sin( 3 * 2 * dip::pi*w ) * params.b1[ 3 ].real() );

   params.b1[ 0 ].real( params.b1[ 0 ].real() );
   params.b1[ 1 ].real( std::cos( 1 * 2 * dip::pi*w ) * params.b1[ 1 ].real() );
   params.b1[ 2 ].real( std::cos( 2 * 2 * dip::pi*w ) * params.b1[ 2 ].real() );
   params.b1[ 3 ].real( std::cos( 3 * 2 * dip::pi*w ) * params.b1[ 3 ].real() );

   params.b2[ 0 ].imag( 0.0 );
   params.b2[ 1 ].imag( -1.0 * params.b1[ 1 ].imag() );
   params.b2[ 2 ].imag( -1.0 * params.b1[ 2 ].imag() );
   params.b2[ 3 ].imag( -1.0 * params.b1[ 3 ].imag() );

   params.b2[ 0 ].real( 1.0 );
   params.b2[ 1 ].real( params.b1[ 1 ].real() );
   params.b2[ 2 ].real( params.b1[ 2 ].real() );
   params.b2[ 3 ].real( params.b1[ 3 ].real() );

   return params;
}

// Gabor IIR separable line filter
class GaborIIRLineFilter : public Framework::SeparableLineFilter
{
public:
   GaborIIRLineFilter( std::vector< GaborIIRParams > const& filterParams ) : filterParams_( filterParams ) {}
   virtual void SetNumberOfThreads( dip::uint threads ) override {
      buffers_.resize( threads );
   }
   virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint /*procDim*/ ) override {
      // TODO: figure out how filter parameters affect amount of computation
      //GaborIIRParams const& fParams = filterParams_[ procDim ];
      return lineLength * 40;
   }
   virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
      dcomplex* in = static_cast< dcomplex* >(params.inBuffer.buffer);
      dcomplex* out = static_cast< dcomplex* >(params.outBuffer.buffer);
      DIP_ASSERT( params.inBuffer.stride == 1 );
      DIP_ASSERT( params.outBuffer.stride == 1 );
      GaborIIRParams const& fParams = filterParams_[ params.dimension ];
      DIP_ASSERT( fParams.border == params.inBuffer.border );

      in -= fParams.border;
      out -= fParams.border;
      dip::uint length = params.inBuffer.length + fParams.border * 2;
      buffers_[ params.thread ].resize( length ); // won't do anything if buffer is already of correct size.
      dcomplex* p1 = buffers_[ params.thread ].data();

      auto const& a1 = fParams.a1;
      auto const& a2 = fParams.a2;
      auto const& b1 = fParams.b1;
      auto const& b2 = fParams.b2;
      dcomplex c = fParams.cc;

      auto const& orderMA = fParams.iir_order_num;
      auto const& orderAR = fParams.iir_order_den;
      dip::uint order1 = std::max( orderAR[ 0 ], orderMA[ 0 ] );
      dip::uint order2 = std::max( orderAR[ 3 ], orderMA[ 3 ] );
      bool copy_forward = false;
      bool copy_backward = false;
      if(( orderMA[ 0 ] == 0 ) && ( a1[ 0 ] == 1.0 )) {
         copy_forward = true;
      }
      if(( orderMA[ 3 ] == 0 ) && ( a2[ 0 ] == 1.0 )) {
         copy_backward = true;
      }

      // Recursive forward scan
      dcomplex* p0 = in;
      for( dip::uint ii = 0; ii < order1; ii++ ) {
         p1[ ii ] = p0[ ii ];
      }
      for( dip::uint ii = order1; ii < length; ii++ ) {
         if( !copy_forward ) {
            p1[ ii ] = 0.0;
            for( dip::uint jj = orderMA[ 1 ]; jj <= orderMA[ 2 ]; jj++ ) {
               p1[ ii ] += dcomplex(( a1[ jj ].real() * p0[ ii - jj ].real() ) - ( a1[ jj ].imag() * p0[ ii - jj ].imag() ),
                                    ( a1[ jj ].real() * p0[ ii - jj ].imag() ) - ( a1[ jj ].imag() * p0[ ii - jj ].real() ));
            }
         } else {
            p1[ ii ] = p0[ ii ];
         }

         for(dip::uint jj = orderAR[ 1 ]; jj <= orderAR[ 2 ]; jj++ ) {
            p1[ ii ] -= dcomplex(( b1[ jj ].real() * p1[ ii - jj ].real() ) - ( b1[ jj ].imag() * p1[ ii - jj ].imag() ),
                                 ( b1[ jj ].real() * p1[ ii - jj ].imag() ) + ( b1[ jj ].imag() * p1[ ii - jj ].real() ));
         }
      }

      // Backward scan
      dcomplex* p2 = out;
      for( dip::uint ii = length - order2; ii < length; ii++ ) {
         p2[ ii ] = p1[ ii ];
      }
      for( dip::uint ii = length - order2; ii-- > 0; ) {
         if( !copy_backward ) {
            p2[ ii ] = 0.0;
            for( dip::uint jj = orderMA[ 4 ]; jj <= orderMA[ 5 ]; jj++ ) {
               p2[ ii ] += dcomplex(( a2[ jj ].real() * p1[ ii + jj ].real() ) - ( a2[ jj ].imag() * p1[ ii + jj ].imag() ),
                                    ( a2[ jj ].real() * p1[ ii + jj ].imag() ) - ( a2[ jj ].imag() * p1[ ii + jj ].real() ));
            }
         } else {
            p2[ ii ] = p1[ ii ];
         }

         for( dip::uint jj = orderAR[ 4 ]; jj <= orderAR[ 5 ]; jj++ ) {
            p2[ ii ] -= dcomplex(( b2[ jj ].real() * p2[ ii + jj ].real() ) - ( b2[ jj ].imag() * p2[ ii + jj ].imag() ),
                                 ( b2[ jj ].real() * p2[ ii + jj ].imag() ) + ( b2[ jj ].imag() * p2[ ii + jj ].real() ));
         }
      }

      // Normalization
      for(dip::uint ii = 0; ii < length; ii++ ) {
         p2[ ii ].real( c.real() * p2[ ii ].real() );
         p2[ ii ].imag( c.real() * p2[ ii ].imag() );
      }
   }
private:
   std::vector< GaborIIRParams > const& filterParams_; // one of each dimension
   std::vector< std::vector< dcomplex >> buffers_; // one for each thread
};

} // namespace

void GaborIIR(
   Image const& in,
   Image& out,
   FloatArray sigmas,
   FloatArray const& frequencies,
   StringArray const& boundaryCondition,
   BooleanArray process,
   IntegerArray, // filterOrder is ignored, treated as 0
   dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sigmas.empty(), E::ARRAY_PARAMETER_EMPTY ); // This must be given, but can be a scalar
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( frequencies.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( process, nDims, true );
   DIP_END_STACK_TRACE
   if( truncation <= 0.0 ) {
      truncation = 3;   // Default truncation
   }

   // Filter parameters
   std::vector< GaborIIRParams > filterParams( nDims );
   UnsignedArray border( nDims );
   for( dip::uint ii = 0; ii < nDims; ii++ ) {
      if( process[ ii ] && ( sigmas[ ii ] > 0.0 ) && ( in.Size( ii ) > 1 )) {
         bool found = false;
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            if( process[ jj ] && ( sigmas[ jj ] == sigmas[ ii ] ) && ( frequencies[ jj ] == frequencies[ ii ] )) {
               filterParams[ ii ] = filterParams[ jj ];
               found = true;
               break;
            }
         }
         if( !found ) {
            filterParams[ ii ] = FillGaborIIRParams( sigmas[ ii ], frequencies[ ii ], truncation );
         }
         border[ ii ] = filterParams[ ii ].border;
      } else {
         process[ ii ] = false;
      }
   }
   // Call the separable framework
   DIP_START_STACK_TRACE
      // handle boundary condition array (checks are made in Framework::Separable, no need to repeat them here)
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      // Create Gabor callback function
      GaborIIRLineFilter lineFilter( filterParams );
      Framework::Separable(
         in,
         out,
         DT_DCOMPLEX,
         DataType::SuggestComplex( in.DataType() ),
         process,
         border,
         bc,
         lineFilter,
         Framework::SeparableOption::AsScalarImage
         + Framework::SeparableOption::UseOutputBorder
         + Framework::SeparableOption::UseInputBuffer // ensures that there's no strides
         + Framework::SeparableOption::UseOutputBuffer // ensures that there's no strides
      );
   DIP_END_STACK_TRACE
}

} // namespace dip
