/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the IIR Gaussian filter.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * This function originally written by Lucas van Vliet.
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

using ComplexArray = std::array< dcomplex, 10 >;

constexpr dip::uint MAX_IIR_ORDER = 6;

struct GaussIIRParams {
   dfloat sigma;
   dip::uint border;
   std::array< dip::uint, 6 > iir_order_num;
   std::array< dip::uint, 6 > iir_order_den;
   std::array< dfloat, MAX_IIR_ORDER > a1;
   std::array< dfloat, MAX_IIR_ORDER > a2;
   std::array< dfloat, MAX_IIR_ORDER > b1;
   std::array< dfloat, MAX_IIR_ORDER > b2;
   dfloat cc;
};

enum class DesignMethod {
      FORWARD_BACKWARD = 1,
      DISCRETE_TIME_FIT = 2
};

dfloat q2sigma (
      dip::uint nn,
      ComplexArray& pp,
      dfloat q
) {
   dfloat var = 0.0;
   for( dip::uint mm = 1; mm <= ( nn - ( nn & 1 ) ); mm += 2 ) {
      dfloat modulus = std::sqrt( pp[ mm ].real() * pp[ mm ].real() + pp[ mm ].imag() * pp[ mm ].imag() );
      dfloat phase = std::atan( pp[ mm ].imag() / pp[ mm ].real() );
      modulus = std::exp( std::log( modulus ) / q );
      phase = phase / q;
      dfloat re = modulus * std::cos( phase );
      dfloat im = modulus * std::sin( phase );
      dfloat d = 1 - 2 * re + re * re + im * im;
      d *= d;
      var += ( 4 * ( re + ( re - 2 ) * ( re * re + im * im ) ) ) / d;
   }
   if( nn & 1 ) {
      dfloat modulus = std::sqrt( pp[ nn ].real() * pp[ nn ].real() + pp[ nn ].imag() * pp[ nn ].imag() );
      dfloat phase = std::atan( pp[ nn ].imag() / pp[ nn ].real() );
      modulus = std::exp( std::log( modulus ) / q );
      phase = phase / q;
      dfloat re = modulus * std::cos( phase );
      //dfloat im = modulus * std::sin( phase );
      var += ( 2 * re ) / (( re - 1 ) * ( re - 1 ));
   }
   return std::sqrt( var );
}

void FillPoleCoefficients (
      dip::uint nn,
      ComplexArray& pp,
      dip::uint order,
      DesignMethod& method
) {
   // Store order of recursive filters in pp[ 0 ]
   pp[ 0 ] = { static_cast< dfloat >( nn ), 0.0 };
   if( method == DesignMethod::DISCRETE_TIME_FIT ) {
      method = DesignMethod::DISCRETE_TIME_FIT;
      switch( nn ) {
         case 5:
            switch( order ) {
               case 2:
                  pp[ 1 ] = { 0.70381, 1.38271 };
                  pp[ 2 ] = { 0.70381, -1.38271 };
                  pp[ 3 ] = { 1.42239, 0.77978 };
                  pp[ 4 ] = { 1.42239, -0.77978 };
                  pp[ 5 ] = { 1.69319, 0.0 };
                  break;

               case 1:
                  pp[ 1 ] = { 0.70237, 1.38717 };
                  pp[ 2 ] = { 0.70237, -1.38717 };
                  pp[ 3 ] = { 1.43280, 0.77903 };
                  pp[ 4 ] = { 1.43280, -0.77903 };
                  pp[ 5 ] = { 1.70346, 0.0 };
                  break;

               case 0:
               default:
                  pp[ 1 ] = { 0.85991, 1.45235 };
                  pp[ 2 ] = { 0.85991, -1.45235 };
                  pp[ 3 ] = { 1.60953, 0.83009 };
                  pp[ 4 ] = { 1.60953, -0.83009 };
                  pp[ 5 ] = { 1.87040, 0.0 };
                  break;
            }
            break;
         case 4:
            switch( order ) {
               case 2:
                  pp[ 1 ] = { 0.94576, 1.21364 };
                  pp[ 2 ] = { 0.94576, -1.21364 };
                  pp[ 3 ] = { 1.59892, 0.42668 };
                  pp[ 4 ] = { 1.59892, -0.42668 };
                  break;

               case 1:
                  pp[ 1 ] = { 1.04198, 1.25046};
                  pp[ 2 ] = { 1.04198, -1.25046};
                  pp[ 3 ] = { 1.69337, 0.45006};
                  pp[ 4 ] = { 1.69337, -0.45006};
                  break;

               case 0:
               default:
                  pp[ 1 ] = { 1.13231, 1.28122};
                  pp[ 2 ] = { 1.13231, -1.28122};
                  pp[ 3 ] = { 1.78532, 0.46766};
                  pp[ 4 ] = { 1.78532, -0.46766};
                  break;
            }
            break;
         case 3:
            switch( order ) {
               case 2:
                  pp[ 1 ] = { 1.21969, 0.91724 };
                  pp[ 2 ] = { 1.21969, -0.91724 };
                  pp[ 3 ] = { 1.69485, 0.0 };
                  break;

               case 1:
                  pp[ 1 ] = { 1.32094, 0.97057 };
                  pp[ 2 ] = { 1.32094, -0.97057 };
                  pp[ 3 ] = { 1.77635, 0.0 };
                  break;

               case 0:
               default:
                  pp[ 1 ] = { 1.41650, 1.00829 };
                  pp[ 2 ] = { 1.41650, -1.00829 };
                  pp[ 3 ] = { 1.86131, 0.0 };
                  break;
            }
            break;
         case 2:
            pp[ 1 ] = { 1.69580, 0.59955 };
            pp[ 2 ] = { 1.69580, -0.59955 };
            break;
         case 1:
            pp[ 1 ] = { 2.00000, 0.00000 };
            break;
         default:
            // Abramowitz and Stegun
            method = DesignMethod::FORWARD_BACKWARD;
            pp[ 0 ] = { 3.0, 0.0 };
            pp[ 1 ] = { 2.1078345, 1.4058574 };
            pp[ 2 ] = { 2.1078345, -1.4058574 };
            pp[ 3 ] = { 2.1668048, 0.0 };
            break;
      }
   } else {
      method = DesignMethod::FORWARD_BACKWARD;
      switch( nn ) {
         case 5:
            pp[ 1 ] = { 2.19406, 1.90251 };
            pp[ 2 ] = { 2.19406, -1.90251 };
            pp[ 3 ] = { 2.31029, 0.598022 };
            pp[ 4 ] = { 2.31029, -0.598022 };
            pp[ 5 ] = { 7.64071, 0.0 };
            break;
         case 4:
            pp[ 1 ] = { 2.06954, 1.90416 };
            pp[ 2 ] = { 2.06954, -1.90416 };
            pp[ 3 ] = { 2.18403, 0.593635 };
            pp[ 4 ] = { 2.18403, -0.593635 };
            break;
         case 3:
         default:
            pp[ 0 ] = { 3.0, 0.0 };
            pp[ 1 ] = { 2.1078345, 1.4058574 };
            pp[ 2 ] = { 2.1078345, -1.4058574 };
            pp[ 3 ] = { 2.1668048, 0.0 };
            break;
      }
   }
}

void FilterCoefficients (
            dip::uint mm,
            dip::uint nn,
            ComplexArray& pp,
            dip::uint start,
            dip::uint stop,
            dcomplex tmp,
            ComplexArray& bb
      ) {
   // Initialize temporary product to 1.
   // Initialize result to 0.
   if( ( start == mm ) && ( stop == nn ) ) {
      tmp = { 1.0, 0.0 };
      bb[ nn - mm ] = { 0.0, 0.0 };
   }
   if( start > 1 ) {
      for( dip::uint ii = start; ii <= stop; ++ii ) {
         FilterCoefficients( mm, nn, pp, start - 1, ii - 1, tmp * pp[ ii ], bb );
      }
   } else if( start == 1 ) {
      for( dip::uint ii = start; ii <= stop; ++ii ) {
         bb[ nn - mm ] = ( tmp * pp[ ii ] ) + bb[ nn - mm ];
      }
   } else if( ( start == 0 ) && ( mm == 0 ) ) {
      bb[ nn - mm ] = { 1.0, 0.0 };
   }
}

GaussIIRParams FillGaussIIRParams(
      dfloat sigma,
      dip::uint order,
      dip::uint filterOrder,
      DesignMethod method,
      dfloat truncation
) {
   GaussIIRParams params;
      params.border = std::max< dip::uint >( 5, static_cast< dip::uint >( sigma * truncation + 0.5 ));
      params.sigma = sigma;

      params.a1.fill( 0.0 );
      params.a2.fill( 0.0 );
      params.b1.fill( 0.0 );
      params.b2.fill( 0.0 );

      // Fetch the desired poles, depending on the filter order and derivative order
      dip::uint nn = filterOrder;
      ComplexArray pp;
   FillPoleCoefficients( nn, pp, order, method ); // modifies `method`!
      nn = static_cast< dip::uint >( pp[ 0 ].real() );

      // Compute the correct value for q based on the poles in the z-domain
      dfloat q;
      if( method == DesignMethod::FORWARD_BACKWARD ) {
         if( sigma > 0 ) {
            dfloat q0term = -sigma * sigma;
            dfloat q1term = 0.0;
            dfloat q2term = 0.0;
            for( dip::uint mm = 1; mm <= ( nn - ( nn & 1 ) ); mm += 2 ) {
               dfloat re = pp[ mm ].real();
               dfloat im = pp[ mm ].imag();
               dfloat t1 = 4 * ( -1 + 3 * re - 3 * re * re - im * im + re * ( re * re + im * im ) );
               dfloat t2 = 4 * ( 1 - 2 * re + re * re - im * im );
               dfloat d =  1 - 2 * re + re * re + im * im;
               d *= d;
               q1term += t1 / d;
               q2term += t2 / d;
            }
            if( nn & 1 ) {
               dfloat d = pp[ nn ].real() - 1.0;
               q1term += 2.0 / d;
               q2term += 2.0 / ( d * d );
            }
            dfloat t = std::sqrt( q1term * q1term - 4.0 * q2term * q0term );
            dfloat q1 = ( -q1term + t ) / ( 2 * q2term );
            dfloat q2 = ( -q1term - t ) / ( 2 * q2term );
            if( q1 > q2 ) {
               q = q1;
            } else {
               q = q2;
            }
         } else {
            q = -sigma;
         }
      } else if( method == DesignMethod::DISCRETE_TIME_FIT ) {
         q = std::fabs( sigma ) / 2.0;
         dfloat q0 = q;
         dfloat qs = q2sigma( nn, pp, q );
         while( std::fabs( std::fabs( sigma ) - qs ) > 0.000001 ) {
            q += q0 - qs / 2.0;
            qs = q2sigma( nn, pp, q );
         }
      } else {
         q = std::fabs( sigma );
      }

      // Scale the poles
      if( method == DesignMethod::FORWARD_BACKWARD ) {
         for( dip::uint mm = 1; mm <= nn; ++mm ) {
            pp[ mm ] = { 1.0 + ( pp[ mm ].real() - 1.0 ) / q, pp[ mm ].imag() / q };
         }
      } else if( method == DesignMethod::DISCRETE_TIME_FIT ) {
         for( dip::uint mm = 1; mm <= nn; ++mm ) {
            dfloat modulus = std::sqrt( pp[ mm ].real() * pp[ mm ].real() + pp[ mm ].imag() * pp[ mm ].imag() );
            dfloat phase = std::atan( pp[ mm ].imag() / pp[ mm ].real() );
            modulus = std::exp( std::log( modulus ) / q );
            phase = phase / q;
            pp[ mm ] = { modulus * std::cos( phase ), modulus * std::sin( phase ) };
         }
      }
      // Compute the new variance
      dfloat var = 0.0;
      for( dip::uint mm = 1; mm <= ( nn - ( nn & 1 ) ); mm += 2 ) {
         dfloat re = pp[ mm ].real();
         dfloat im = pp[ mm ].imag();
         dfloat d = 1 - 2 * re + re * re + im * im;
         d *= d;
         var += ( 4 * ( re + ( re - 2 ) * ( re * re + im * im ) ) ) / d;
      }
      if( nn & 1 ) {
         dfloat re = pp[ nn ].real();
         //dfloat im = pp[ nn ].imag();
         var += ( 2 * re ) / (( re - 1 ) * ( re - 1 ));
      }

      // Compute the actual filter coefficients
      ComplexArray bb;
      for( dip::uint mm = 0; mm <= nn; ++mm ) {
         FilterCoefficients( mm, nn, pp, mm, nn, {}, bb );
      }
      for( dip::uint mm = 0; mm <= nn; ++mm ) {
         bb[ nn - mm ] *= std::pow( -1.0, nn - mm ) / bb[ 0 ].real();
         params.b1[ nn - mm ] = params.b2[ nn - mm ] = bb[ nn - mm ].real();
      }

      // Normalization
      params.cc = 0.0;
      for( dip::uint mm = 0; mm <= nn; ++mm ) {
         params.cc += params.b1[ mm ];
      }
      params.cc = ( params.cc * params.cc );

      params.iir_order_den[ 0 ] = nn;
      params.iir_order_den[ 1 ] = 1;
      params.iir_order_den[ 2 ] = nn;

      params.iir_order_den[ 3 ] = nn;
      params.iir_order_den[ 4 ] = 1;
      params.iir_order_den[ 5 ] = nn;


      // Non-recursive forward/backward scan
      switch( order ) {
         case 0:
            params.a1[ 0 ] = 1.0;
            params.a2[ 0 ] = 1.0;
            break;
         case 1:
            params.a1[ 0 ] = 0.5;
            params.a1[ 2 ] = -0.5;
            params.a2[ 1 ] = 1.0;
            break;
         case 2:
            params.a1[ 0 ] = 1.0;
            params.a1[ 1 ] = -1.0;
            params.a2[ 1 ] = 1.0;
            params.a2[ 0 ] = -1.0;
            break;
         case 3:
            params.a1[ 0 ] = 1.0;
            params.a1[ 1 ] = -2.0;
            params.a1[ 2 ] = 1.0;
            params.a2[ 2 ] = 0.5;
            params.a2[ 0 ] = -0.5;
            break;
         case 4:
            params.a1[ 0 ] = 1.0;
            params.a1[ 1 ] = -2.0;
            params.a1[ 2 ] = 1.0;
            params.a2[ 2 ] = 1.0;
            params.a2[ 1 ] = -2.0;
            params.a2[ 0 ] = 1.0;
            break;
         default:
            DIP_THROW( E::NOT_IMPLEMENTED );
            break;
      }

      params.iir_order_num.fill( MAX_IIR_ORDER + 1 );
      for( dip::uint jj = 0; jj < MAX_IIR_ORDER; ++jj ) {
         if(( params.a1[ jj ] != 0.0 ) && ( params.iir_order_num[ 1 ] > MAX_IIR_ORDER )) {
            params.iir_order_num[ 1 ] = jj;
         }
         if(( params.a2[ jj ] != 0.0 ) && ( params.iir_order_num[ 4 ] > MAX_IIR_ORDER )) {
            params.iir_order_num[ 4 ] = jj;
         }
      }

      for( dip::uint jj = MAX_IIR_ORDER; jj > 0; ) {
         --jj;
         if(( params.a1[ jj ] != 0.0 ) && ( params.iir_order_num[ 2 ] > MAX_IIR_ORDER )) {
            params.iir_order_num[ 2 ] = jj;
         }
         if(( params.a2[ jj ] != 0.0 ) && ( params.iir_order_num[ 5 ] > MAX_IIR_ORDER )) {
            params.iir_order_num[ 5 ] = jj;
         }
      }
      params.iir_order_num[ 0 ] = params.iir_order_num[ 2 ];
      params.iir_order_num[ 3 ] = params.iir_order_num[ 5 ];

/*
      for (jj = 0; jj < 6; ++jj)
         printf("%1d %1d\n",
            params.iir_order_den[jj], params.iir_order_num[jj]);

      for (jj = params.iir_order_num[1];
           jj <= params.iir_order_num[2]; ++jj)
      {
         printf("params.a1[%1d] = %8.5f \n", jj, params.a1[jj]);
      }
      for (jj = params.iir_order_num[4];
           jj <= params.iir_order_num[5]; ++jj)
      {
         printf("params.a2[%1d] = %8.5f \n", jj, params.a2[jj]);
      }
      for (jj = 0; jj <= params.iir_order_den[2]; ++jj)
      {
         printf("params.b1[%1d] = %8.5f \tparams.b2[%1d] = %8.5f\n",
             jj, params.b1[jj], jj, params.b2[jj]);
      }
      printf("c[%1d]  = %g \n", 0, cc[0]);
*/

   return params;
}

class GaussIIRLineFilter : public Framework::SeparableLineFilter {
   public:
      GaussIIRLineFilter( std::vector< GaussIIRParams > const& filterParams ) : filterParams_( filterParams ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint /*procDim*/ ) override {
         // TODO: figure out how filter parameters affect amount of computation
         //GaussIIRParams const& fParams = filterParams_[ procDim ];
         return lineLength * 40;
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         dfloat* in = static_cast< dfloat* >( params.inBuffer.buffer );
         dfloat* out = static_cast< dfloat* >( params.outBuffer.buffer );
         DIP_ASSERT( params.inBuffer.stride == 1 );
         DIP_ASSERT( params.outBuffer.stride == 1 );
         GaussIIRParams const& fParams = filterParams_[ params.dimension ];
         DIP_ASSERT( fParams.border == params.inBuffer.border );

         in -= fParams.border;
         out -= fParams.border;
         dip::uint length = params.inBuffer.length + fParams.border * 2;
         buffers_[ params.thread ].resize( length ); // won't do anything if buffer is already of correct size.
         dfloat* p1 = buffers_[ params.thread ].data();

         auto const& a1 = fParams.a1;
         auto const& a2 = fParams.a2;
         auto const& b1 = fParams.b1;
         auto const& b2 = fParams.b2;
         dfloat c = ( fParams.cc );

         auto const& orderMA = fParams.iir_order_num;
         auto const& orderAR = fParams.iir_order_den;
         dip::uint order1 = std::max( orderAR[ 0 ], orderMA[ 0 ] );
         dip::uint order2 = std::max( orderAR[ 3 ], orderMA[ 3 ] );
         bool copy_forward = false;
         bool copy_backward = false;
         if( ( orderMA[ 0 ] == 0 ) && ( a1[ 0 ] == 1.0 ) ) {
            copy_forward = true;
         }
         if( ( orderMA[ 3 ] == 0 ) && ( a2[ 0 ] == 1.0 ) ) {
            copy_backward = true;
         }

         // Recursive forward scan
         dfloat* p0 = in;
         dip::uint ii = 0;
         dfloat r1, r2, r3, r4, r5;
         switch( order1 ) {
            case 3:
               if( copy_forward ) {
                  r1 = r2 = r3 = p0[ 0 ] / ( 1.0 + b1[ 1 ] + b1[ 2 ] + b1[ 3 ] );
                  for( ; ii < length - 3; ii += 3 ) {
                     r3 = p1[ ii ] = p0[ ii ] - b1[ 1 ] * r1 - b1[ 2 ] * r2 - b1[ 3 ] * r3;
                     r2 = p1[ ii + 1 ] = p0[ ii + 1 ] - b1[ 1 ] * r3 - b1[ 2 ] * r1 - b1[ 3 ] * r2;
                     r1 = p1[ ii + 2 ] = p0[ ii + 2 ] - b1[ 1 ] * r2 - b1[ 2 ] * r3 - b1[ 3 ] * r1;
                  }
               }
               break;

            case 4:
               if( copy_forward ) {
                  r1 = r2 = r3 = r4 = p0[ 0 ] / ( 1.0 + b1[ 1 ] + b1[ 2 ] + b1[ 3 ] + b1[ 4 ] );
                  for( ; ii < length - 4; ii += 4 ) {
                     r4 = p1[ ii ] = p0[ ii ]
                                     - b1[ 1 ] * r1 - b1[ 2 ] * r2 - b1[ 3 ] * r3 - b1[ 4 ] * r4;
                     r3 = p1[ ii + 1 ] = p0[ ii + 1 ]
                                         - b1[ 1 ] * r4 - b1[ 2 ] * r1 - b1[ 3 ] * r2 - b1[ 4 ] * r3;
                     r2 = p1[ ii + 2 ] = p0[ ii + 2 ]
                                         - b1[ 1 ] * r3 - b1[ 2 ] * r4 - b1[ 3 ] * r1 - b1[ 4 ] * r2;
                     r1 = p1[ ii + 3 ] = p0[ ii + 3 ]
                                         - b1[ 1 ] * r2 - b1[ 2 ] * r3 - b1[ 3 ] * r4 - b1[ 4 ] * r1;
                  }
               } else if( a1[ 0 ] == 0.5 && a1[ 1 ] == 0.0 && a1[ 2 ] == -0.5 && a1[ 3 ] == 0.0 ) {
                  r1 = r2 = r3 = r4 = ( p0[ 1 ] - p0[ 0 ] ) /
                                      ( 1.0 + b1[ 1 ] + b1[ 2 ] + b1[ 3 ] + b1[ 4 ] );
                  for( ii = 0; ii < 2; ++ii ) {
                     p1[ ii ] = r1;
                  }
                  for( ; ii < length - 4; ii += 4 ) {
                     r4 = p1[ ii ] = 0.5 * ( p0[ ii ] - p0[ ii - 2 ] )
                                     - b1[ 1 ] * r1 - b1[ 2 ] * r2 - b1[ 3 ] * r3 - b1[ 4 ] * r4;
                     r3 = p1[ ii + 1 ] = 0.5 * ( p0[ ii + 1 ] - p0[ ii - 1 ] )
                                         - b1[ 1 ] * r4 - b1[ 2 ] * r1 - b1[ 3 ] * r2 - b1[ 4 ] * r3;
                     r2 = p1[ ii + 2 ] = 0.5 * ( p0[ ii + 2 ] - p0[ ii ] )
                                         - b1[ 1 ] * r3 - b1[ 2 ] * r4 - b1[ 3 ] * r1 - b1[ 4 ] * r2;
                     r1 = p1[ ii + 3 ] = 0.5 * ( p0[ ii + 3 ] - p0[ ii + 1 ] )
                                         - b1[ 1 ] * r2 - b1[ 2 ] * r3 - b1[ 3 ] * r4 - b1[ 4 ] * r1;
                  }
               }
               break;

            case 5:
               if( copy_forward ) {
                  r1 = r2 = r3 = r4 = r5 = p0[ 0 ] /
                                           ( 1.0 + b1[ 1 ] + b1[ 2 ] + b1[ 3 ] + b1[ 4 ] + b1[ 5 ] );
                  for( ; ii < length - 5; ii += 5 ) {
                     r5 = p1[ ii ] = p0[ ii ]
                                     - b1[ 1 ] * r1 - b1[ 2 ] * r2 - b1[ 3 ] * r3 - b1[ 4 ] * r4 - b1[ 5 ] * r5;
                     r4 = p1[ ii + 1 ] = p0[ ii + 1 ]
                                         - b1[ 1 ] * r5 - b1[ 2 ] * r1 - b1[ 3 ] * r2 - b1[ 4 ] * r3 - b1[ 5 ] * r4;
                     r3 = p1[ ii + 2 ] = p0[ ii + 2 ]
                                         - b1[ 1 ] * r4 - b1[ 2 ] * r5 - b1[ 3 ] * r1 - b1[ 4 ] * r2 - b1[ 5 ] * r3;
                     r2 = p1[ ii + 3 ] = p0[ ii + 3 ]
                                         - b1[ 1 ] * r3 - b1[ 2 ] * r4 - b1[ 3 ] * r5 - b1[ 4 ] * r1 - b1[ 5 ] * r2;
                     r1 = p1[ ii + 4 ] = p0[ ii + 4 ]
                                         - b1[ 1 ] * r2 - b1[ 2 ] * r3 - b1[ 3 ] * r4 - b1[ 4 ] * r5 - b1[ 5 ] * r1;
                  }
               } else if( a1[ 0 ] == 1.0 && a1[ 1 ] == -1.0 && a1[ 2 ] == 0.0 && a1[ 3 ] == 0.0 ) {
                  r1 = r2 = r3 = r4 = r5 = ( p0[ 1 ] - p0[ 0 ] ) /
                                           ( 1.0 + b1[ 1 ] + b1[ 2 ] + b1[ 3 ] + b1[ 4 ] + b1[ 5 ] );
                  p1[ ii++ ] = r1;
                  for( ; ii < length - 5; ii += 5 ) {
                     r5 = p1[ ii ] = p0[ ii ] - p0[ ii - 1 ]
                                     - b1[ 1 ] * r1 - b1[ 2 ] * r2 - b1[ 3 ] * r3 - b1[ 4 ] * r4 - b1[ 5 ] * r5;
                     r4 = p1[ ii + 1 ] = p0[ ii + 1 ] - p0[ ii ]
                                         - b1[ 1 ] * r5 - b1[ 2 ] * r1 - b1[ 3 ] * r2 - b1[ 4 ] * r3 - b1[ 5 ] * r4;
                     r3 = p1[ ii + 2 ] = p0[ ii + 2 ] - p0[ ii + 1 ]
                                         - b1[ 1 ] * r4 - b1[ 2 ] * r5 - b1[ 3 ] * r1 - b1[ 4 ] * r2 - b1[ 5 ] * r3;
                     r2 = p1[ ii + 3 ] = p0[ ii + 3 ] - p0[ ii + 2 ]
                                         - b1[ 1 ] * r3 - b1[ 2 ] * r4 - b1[ 3 ] * r5 - b1[ 4 ] * r1 - b1[ 5 ] * r2;
                     r1 = p1[ ii + 4 ] = p0[ ii + 4 ] - p0[ ii + 3 ]
                                         - b1[ 1 ] * r2 - b1[ 2 ] * r3 - b1[ 3 ] * r4 - b1[ 4 ] * r5 - b1[ 5 ] * r1;
                  }
               }
               break;

            default:
               break;
         }

         // Compute the first order1 values for arbitrary coefficients a & b
         dfloat val = 0.0;
         for( dip::uint jj = orderMA[ 1 ]; jj <= orderMA[ 2 ]; ++jj ) {
            val += ( a1[ jj ] * p0[ orderMA[ 2 ] - jj ] );
         }
         r1 = val / ( 1.0 + b1[ 1 ] + b1[ 2 ] + b1[ 3 ] + b1[ 4 ] + b1[ 5 ] );
         for( ; ii < order1; ++ii ) {
            p1[ ii ] = r1;
         }
         for( ; ii < length; ++ii ) {
            if( !copy_forward ) {
               val = 0.0;
               for( dip::uint jj = orderMA[ 1 ]; jj <= orderMA[ 2 ]; ++jj ) {
                  val += ( a1[ jj ] * p0[ ii - jj ] );
               }
            } else {
               val = p0[ ii ];
            }
            for( dip::uint jj = orderAR[ 1 ]; jj <= orderAR[ 2 ]; ++jj ) {
               val -= ( b1[ jj ] * p1[ ii - jj ] );
            }
            p1[ ii ] = val;
         }

         // Iterative & recursive backward scan
         dfloat* p2 = out;
         ii = length - 1;
         switch( order2 ) {
            case 3:
               if( copy_backward ) {
                  r1 = r2 = r3 = c * p1[ length - 1 ] / ( 1.0 + b2[ 1 ] + b2[ 2 ] + b2[ 3 ] );
                  for( ; ii >= 3; ii -= 3 ) {
                     r3 = p2[ ii ] = c * p1[ ii ] - b2[ 1 ] * r1 - b2[ 2 ] * r2 - b2[ 3 ] * r3;
                     r2 = p2[ ii - 1 ] = c * p1[ ii - 1 ] - b2[ 1 ] * r3 - b2[ 2 ] * r1 - b2[ 3 ] * r2;
                     r1 = p2[ ii - 2 ] = c * p1[ ii - 2 ] - b2[ 1 ] * r2 - b2[ 2 ] * r3 - b2[ 3 ] * r1;
                  }
               }
               break;

            case 4:
               if( copy_backward ) {
                  r1 = r2 = r3 = r4 = c * p1[ length - 1 ] /
                                      ( 1.0 + b2[ 1 ] + b2[ 2 ] + b2[ 3 ] + b2[ 4 ] );
                  for( ; ii >= 4; ii -= 4 ) {
                     r4 = p2[ ii ] = c * p1[ ii ]
                                     - b2[ 1 ] * r1 - b2[ 2 ] * r2 - b2[ 3 ] * r3 - b2[ 4 ] * r4;
                     r3 = p2[ ii - 1 ] = c * p1[ ii - 1 ]
                                         - b2[ 1 ] * r4 - b2[ 2 ] * r1 - b2[ 3 ] * r2 - b2[ 4 ] * r3;
                     r2 = p2[ ii - 2 ] = c * p1[ ii - 2 ]
                                         - b2[ 1 ] * r3 - b2[ 2 ] * r4 - b2[ 3 ] * r1 - b2[ 4 ] * r2;
                     r1 = p2[ ii - 3 ] = c * p1[ ii - 3 ]
                                         - b2[ 1 ] * r2 - b2[ 2 ] * r3 - b2[ 3 ] * r4 - b2[ 4 ] * r1;
                  }
               } else if( a2[ 0 ] == 0.0 && a2[ 1 ] == 1.0 && a2[ 2 ] == 0.0 && a2[ 3 ] == 0.0 ) {
                  r1 = r2 = r3 = r4 = c * p1[ length - 1 ] /
                                      ( 1.0 + b2[ 1 ] + b2[ 2 ] + b2[ 3 ] + b2[ 4 ] );
                  p2[ ii ] = r1;
                  ii -= 1;
                  for( ; ii >= 4; ii -= 4 ) {
                     r4 = p2[ ii ] = c * p1[ ii + 1 ]
                                     - b2[ 1 ] * r1 - b2[ 2 ] * r2 - b2[ 3 ] * r3 - b2[ 4 ] * r4;
                     r3 = p2[ ii - 1 ] = c * p1[ ii ]
                                         - b2[ 1 ] * r4 - b2[ 2 ] * r1 - b2[ 3 ] * r2 - b2[ 4 ] * r3;
                     r2 = p2[ ii - 2 ] = c * p1[ ii - 1 ]
                                         - b2[ 1 ] * r3 - b2[ 2 ] * r4 - b2[ 3 ] * r1 - b2[ 4 ] * r2;
                     r1 = p2[ ii - 3 ] = c * p1[ ii - 2 ]
                                         - b2[ 1 ] * r2 - b2[ 2 ] * r3 - b2[ 3 ] * r4 - b2[ 4 ] * r1;
                  }
               }
               break;

            case 5:
               if( copy_backward ) {
                  r1 = r2 = r3 = r4 = r5 = c * p1[ length - 1 ] /
                                           ( 1.0 + b2[ 1 ] + b2[ 2 ] + b2[ 3 ] + b2[ 4 ] + b2[ 5 ] );
                  for( ; ii >= 5; ii -= 5 ) {
                     r5 = p2[ ii ] = c * p1[ ii ]
                                     - b2[ 1 ] * r1 - b2[ 2 ] * r2 - b2[ 3 ] * r3 - b2[ 4 ] * r4 - b2[ 5 ] * r5;
                     r4 = p2[ ii - 1 ] = c * p1[ ii - 1 ]
                                         - b2[ 1 ] * r5 - b2[ 2 ] * r1 - b2[ 3 ] * r2 - b2[ 4 ] * r3 - b2[ 5 ] * r4;
                     r3 = p2[ ii - 2 ] = c * p1[ ii - 2 ]
                                         - b2[ 1 ] * r4 - b2[ 2 ] * r5 - b2[ 3 ] * r1 - b2[ 4 ] * r2 - b2[ 5 ] * r3;
                     r2 = p2[ ii - 3 ] = c * p1[ ii - 3 ]
                                         - b2[ 1 ] * r3 - b2[ 2 ] * r4 - b2[ 3 ] * r5 - b2[ 4 ] * r1 - b2[ 5 ] * r2;
                     r1 = p2[ ii - 4 ] = c * p1[ ii - 4 ]
                                         - b2[ 1 ] * r2 - b2[ 2 ] * r3 - b2[ 3 ] * r4 - b2[ 4 ] * r5 - b2[ 5 ] * r1;
                  }
               } else if( a2[ 0 ] == -1.0 && a2[ 1 ] == 1.0 && a2[ 2 ] == 0.0 && a2[ 3 ] == 0.0 ) {
                  r1 = r2 = r3 = r4 = r5 = c * ( -p1[ length - 2 ] + p1[ length - 1 ] ) /
                                           ( 1.0 + b2[ 1 ] + b2[ 2 ] + b2[ 3 ] + b2[ 4 ] + b2[ 5 ] );
                  p2[ ii ] = r1;
                  ii -= 1;
                  for( ; ii >= 5; ii -= 5 ) {
                     r5 = p2[ ii ] = c * ( -p1[ ii ] + p1[ ii + 1 ] )
                                     - b2[ 1 ] * r1 - b2[ 2 ] * r2 - b2[ 3 ] * r3 - b2[ 4 ] * r4 - b2[ 5 ] * r5;
                     r4 = p2[ ii - 1 ] = c * ( -p1[ ii - 1 ] + p1[ ii ] )
                                         - b2[ 1 ] * r5 - b2[ 2 ] * r1 - b2[ 3 ] * r2 - b2[ 4 ] * r3 - b2[ 5 ] * r4;
                     r3 = p2[ ii - 2 ] = c * ( -p1[ ii - 2 ] + p1[ ii - 1 ] )
                                         - b2[ 1 ] * r4 - b2[ 2 ] * r5 - b2[ 3 ] * r1 - b2[ 4 ] * r2 - b2[ 5 ] * r3;
                     r2 = p2[ ii - 3 ] = c * ( -p1[ ii - 3 ] + p1[ ii - 2 ] )
                                         - b2[ 1 ] * r3 - b2[ 2 ] * r4 - b2[ 3 ] * r5 - b2[ 4 ] * r1 - b2[ 5 ] * r2;
                     r1 = p2[ ii - 4 ] = c * ( -p1[ ii - 4 ] + p1[ ii - 3 ] )
                                         - b2[ 1 ] * r2 - b2[ 2 ] * r3 - b2[ 3 ] * r4 - b2[ 4 ] * r5 - b2[ 5 ] * r1;
                  }
               }
               break;

            default:
               break;
         }

         // Compute the first order2 values for arbitrary coefficients a & b
         val = 0.0;
         for( dip::uint jj = orderMA[ 4 ]; jj <= orderMA[ 5 ]; ++jj ) {
            val += ( a2[ jj ] * p1[ length - 1 - orderMA[ 5 ] + jj ] );
         }
         r1 = val / ( 1.0 + b2[ 1 ] + b2[ 2 ] + b2[ 3 ] + b2[ 4 ] + b2[ 5 ] );
         for( ; ii > length - 1 - order2; --ii ) {
            p2[ ii ] = c * r1;
         }
         ++ii;
         while( ii > 0 ) {
            --ii;
            if( !copy_backward ) {
               val = 0.0;
               for( dip::uint jj = orderMA[ 4 ]; jj <= orderMA[ 5 ]; ++jj ) {
                  val += ( a2[ jj ] * p1[ ii + jj ] );
               }
               val = c * val;
            } else {
               val = c * p1[ ii ];
            }

            for( dip::uint jj = orderAR[ 4 ]; jj <= orderAR[ 5 ]; ++jj ) {
               val -= ( b2[ jj ] * p2[ ii + jj ] );
            }
            p2[ ii ] = val;
         }
      }
   private:
      std::vector< GaussIIRParams > const& filterParams_; // one of each dimension
      std::vector< std::vector< dfloat >> buffers_; // one for each thread
};

} // namespace

void GaussIIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      StringArray const& boundaryCondition,
      UnsignedArray filterOrder,
      String const& designMethod,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
   DIP_END_STACK_TRACE
   if( truncation <= 0.0 ) {
      truncation = 3;
   }
   if( filterOrder.empty() ) {
      filterOrder.resize( nDims, 3 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         filterOrder[ ii ] = ( order[ ii ] > 2 ) ? 5 : order[ ii ] + 3;
      }
   } else {
      DIP_STACK_TRACE_THIS( ArrayUseParameter< dip::uint >( filterOrder, nDims, 3 ));
   }
   DesignMethod method;
   DIP_STACK_TRACE_THIS( method = BooleanFromString( designMethod, S::FORWARD_BACKWARD, S::DISCRETE_TIME_FIT )
                                  ? DesignMethod::FORWARD_BACKWARD : DesignMethod::DISCRETE_TIME_FIT );
   // Filter parameters
   std::vector< GaussIIRParams > filterParams( nDims );
   UnsignedArray border( nDims );
   BooleanArray process( nDims, true );
   for( dip::uint ii = 0; ii < nDims; ii++ ) {
      if(( sigmas[ ii ] > 0.0 ) && ( in.Size( ii ) > 1 )) {
         bool found = false;
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            if( process[ jj ] && ( sigmas[ jj ] == sigmas[ ii ] ) && ( order[ jj ] == order[ ii ] ) && ( filterOrder[ jj ] == filterOrder[ ii ] )) {
               filterParams[ ii ] = filterParams[ jj ];
               found = true;
               break;
            }
         }
         if( !found ) {
            filterParams[ ii ] = FillGaussIIRParams( sigmas[ ii ], order[ ii ], filterOrder[ ii ], method, truncation );
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
      // Get callback function
      GaussIIRLineFilter lineFilter( filterParams );
      Framework::Separable(
            in,
            out,
            DT_DFLOAT,
            DataType::SuggestFlex( in.DataType() ),
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

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"
#include "diplib/iterators.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE("[DIPlib] testing the IIR Gaussian filter") {

   // Test 0th order
   dip::Image img{ dip::UnsignedArray{ 256 }, 1, dip::DT_DFLOAT };
   img.Fill( 0.0 );
   img.At( 128 ) = 1.0;
   dip::dfloat sigma = 5.0;
   dip::dfloat amplitude = 1.0 / ( std::sqrt( 2.0 * dip::pi ) * sigma );

   dip::Image r1 = dip::GaussIIR( img, { sigma }, { 0 }, {}, { 5 }, "discrete time fit" );
   DOCTEST_CHECK( std::abs( r1.At( 128 ).As< dip::dfloat >() - amplitude ) < 0.00015 );
   DOCTEST_CHECK( dip::Sum( r1 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   dip::Image r2 = dip::GaussIIR( img, { sigma }, { 0 }, {}, { 4 }, "discrete time fit"  );
   DOCTEST_CHECK( dip::testing::CompareImages( r1, r2, 0.0003 ));
   r2 = dip::GaussIIR( img, { sigma }, { 0 }, {}, { 3 }, "discrete time fit"  );
   DOCTEST_CHECK( dip::testing::CompareImages( r1, r2, 0.002 ));
   r2 = dip::GaussIIR( img, { sigma }, { 0 }, {}, { 2 }, "discrete time fit"  );
   DOCTEST_CHECK( dip::testing::CompareImages( r1, r2, 0.004 ));
   r2 = dip::GaussIIR( img, { sigma }, { 0 }, {}, { 5 }, "forward backward"  );
   DOCTEST_CHECK( dip::testing::CompareImages( r1, r2, 0.003 ));
   r2 = dip::GaussIIR( img, { sigma }, { 0 }, {}, { 4 }, "forward backward"  );
   DOCTEST_CHECK( dip::testing::CompareImages( r1, r2, 0.002 ));
   r2 = dip::GaussIIR( img, { sigma }, { 0 }, {}, { 3 }, "forward backward"  );
   DOCTEST_CHECK( dip::testing::CompareImages( r1, r2, 0.009 ));

   // Test first order derivative
   dip::ImageIterator< dip::dfloat > it( img );
   for( dip::dfloat x = -128; it; ++it, ++x ) {
      *it = x;
   }
   r1 = dip::GaussIIR( img, { sigma }, { 1 }, {}, { 5 }, "discrete time fit" );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 1 }, {}, { 4 }, "discrete time fit"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 1 }, {}, { 3 }, "discrete time fit"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 1 }, {}, { 5 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 1 }, {}, { 4 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 1 }, {}, { 3 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));

   // Test second order derivative
   dip::Image grad = img;
   img = img * img;
   r1 = dip::GaussIIR( img, { sigma }, { 2 }, {}, { 5 }, "discrete time fit" );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 2 }, {}, { 4 }, "discrete time fit"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 2 }, {}, { 3 }, "discrete time fit"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 2 }, {}, { 5 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 2 }, {}, { 4 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 2 }, {}, { 3 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));

   // Test third order derivative
   img = img * grad;
   r1 = dip::GaussIIR( img, { sigma }, { 3 }, {}, { 5 }, "discrete time fit" );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 6.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 3 }, {}, { 4 }, "discrete time fit"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 6.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 3 }, {}, { 5 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 6.0 ));
   r1 = dip::GaussIIR( img, { sigma }, { 3 }, {}, { 4 }, "forward backward"  );
   DOCTEST_CHECK( r1.At( 128 ).As< dip::dfloat >() == doctest::Approx( 6.0 ));
}

#endif // DIP__ENABLE_DOCTEST
