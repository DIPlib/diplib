/*
 * DIPlib 3.0
 * This file contains definitions of 1D interpolation functions.
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

#include "diplib/library/types.h"
#include "diplib/library/sample_iterator.h"

namespace dip {
namespace interpolation {

/*
 * dip::interpolation::BSpline< TPI >()
 *    Uses a cubic B-spline interpolator. Each output sample depends on all input samples, but significantly
 *    only on 10 samples. a = 5.
 *
 * dip::interpolation::FourthOrderCubicSpline< TPI >()
 *    Uses a fourth order cubic spline convolution. Each output sample depends on 6 input samples. a = 3.
 *
 * dip::interpolation::ThirdOrderCubicSpline< TPI >()
 *    Uses a third order cubic spline convolution. Each output sample depends on 4 input samples. a = 2.
 *
 * dip::interpolation::Linear< TPI >()
 *    Uses a triangle convolution. Each output sample depends on 2 input samples. a = 1.
 *
 * dip::interpolation::NearestNeighbor< TPI >()
 *    Copies nearest pixel value. Each output sample depends on only 1 input sample. a = 0.
 *
 * dip::interpolation::Lanczos< TPI, a >
 *    Uses a sinc function windowed by a larger sinc function, directed by a parameter 'a'. Each output sample
 *    depends on 2a input samples. 2 <= a <= 8.
 *
 * All these functions have as parameters:
 *    TPI* input                      -- input buffer; because we need a boundary extension, it'll always be a
 *                                       copy and have stride 1.
 *    SampleIterator< TPI > output    -- output buffer; using sample iterator so we can write directly in output image.
 *    dip::uint outSize               -- size of output buffer, the number of interpolated samples to generate.
 *    dfloat zoom                     -- zoom factor for output w.r.t. input.
 *    dfloat shift                    -- shift for output w.r.t. input.
 * The algorithms will read the input buffer from `input[ floor( shift ) - a ]` to
 * `input[ floor( shift + outSize / zoom ) + a + 1 ]`, where `a` is the parameter of the Lanczos function, 1 for
 * linear interpolation, 2 and 3 for cubic interpolation, etc. This means that boundary extension is expected.
 *
 * TPI is expected to be a floating-point type or a complex type: sfloat, dfloat, scomplex, dcomplex.
 * NearestNeighbor can work with any type.
 */

// Computes the second derivative at each point, as required for B-spline interpolation.
template< typename TPI >
void SplineDerivative(
      TPI* input,
      TPI* spline1, // buffer will be filled with the estimated second derivative
      TPI* spline2, // buffer used for temporary storage
      dip::uint n   // length of input and buffers
) {
   using TPF = FloatType< TPI >;
   spline1[ 0 ] = -0.5;
   ++spline1;
   ++spline2;
   spline2[ 0 ] = TPF( 3.0 ) * ( input[ 1 ] - input[ 0 ] );
   for( dip::uint ii = 2; ii < n; ii++ ) {
      ++input;
      ++spline2;
      TPI p = TPF( 0.5 ) * spline1[ -1 ] + TPF( 2.0 );
      spline1[ 0 ] = TPF( -0.5 ) / p;
      spline2[ 0 ] = input[ 1 ] - TPF( 2.0 ) * input[ 0 ] + input[ -1 ];
      spline2[ 0 ] = ( TPF( 3.0 ) * spline2[ 0 ] - TPF( 0.5 ) * spline2[ -1 ] ) / p;
      ++spline1;
   }
   TPI qn = 0.5;
   TPI un = TPF( 3.0 ) * ( input[ 0 ] - input[ 1 ] );
   spline1[ 0 ] = ( un - qn * spline2[ 0 ] ) / ( qn * spline1[ -1 ] + TPF( 1.0 ));
   for( dip::uint ii = n - 1; ii > 0; ii-- ) {
      --spline1;
      spline1[ 0 ] = spline1[ 0 ] * spline1[ 1 ] + spline2[ 0 ];
      --spline2;
   }
}

template< typename TPI >
void BSpline(
      TPI* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift,
      TPI* spline1, // temporary buffer 1, size = size of input + border
      TPI* spline2  // temporary buffer 2, size = size of input + border
) {
   constexpr dip::uint boundary = 5;
   using TPF = FloatType< TPI >;
   dip::sint offset = static_cast< dip::sint >( std::floor( shift ));
   input += offset;
   SplineDerivative(
         input - boundary,
         spline1,
         spline2,
         static_cast< dip::uint >( std::floor( static_cast< dfloat >( outSize ) / zoom )) + 2 * boundary + 1 );
   spline1 += boundary;
   TPF pos = static_cast< TPF >( shift ) - static_cast< TPF >( offset );
   if( zoom == 1.0 ) {
      TPF a = 1 - pos;
      TPF as = ( a * a * a - a ) / 6;
      TPF bs = ( pos * pos * pos - pos ) / 6;
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = a * input[ 0 ] + pos * input[ 1 ] + as * spline1[ 0 ] + bs * spline1[ 1 ];
         ++output;
         ++input;
         ++spline1;
      }
   } else {
      TPF step = static_cast< TPF >( 1.0 / zoom );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         TPF a = 1 - pos;
         *output = a * input[ 0 ] + pos * input[ 1 ] +
                   (( a * a * a - a ) * spline1[ 0 ] + ( pos * pos * pos - pos ) * spline1[ 1 ] ) / static_cast< TPF >( 6 );
         ++output;
         pos += step;
         if( pos >= 1.0 ) {
            offset = static_cast< dip::sint >( std::floor( pos ));
            pos -= static_cast< TPF >( offset );
            input += offset;
            spline1 += offset;
         }
      }
   }
}

template< typename TPI >
void FourthOrderCubicSpline(
      TPI* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift
) {
   using TPF = FloatType< TPI >;
   dip::sint offset = static_cast< dip::sint >( std::floor( shift ));
   input += offset;
   TPF pos = static_cast< TPF >( shift ) - static_cast< TPF >( offset );
   if( zoom == 1.0 ) {
      TPF pos2 = pos * pos;
      TPF pos3 = pos2 * pos;
      TPF filter_m2 = ( pos3 - TPF( 2 ) * pos2 + pos ) / TPF( 12 );
      TPF filter_m1 = ( TPF( -7 ) * pos3 + TPF( 15 ) * pos2 - TPF( 8 ) * pos ) / TPF( 12 );
      TPF filter__0 = ( TPF( 16 ) * pos3 - TPF( 28 ) * pos2 + TPF( 12 )) / TPF( 12 );
      TPF filter__1 = ( TPF( -16 ) * pos3 + TPF( 20 ) * pos2 + TPF( 8 ) * pos ) / TPF( 12 );
      TPF filter__2 = ( TPF( 7 ) * pos3 - TPF( 6 ) * pos2 - pos ) / TPF( 12 );
      TPF filter__3 = ( -pos3 + pos2 ) / TPF( 12 );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = input[ -2 ] * filter_m2 +
                   input[ -1 ] * filter_m1 +
                   input[  0 ] * filter__0 +
                   input[  1 ] * filter__1 +
                   input[  2 ] * filter__2 +
                   input[  3 ] * filter__3;
         ++input;
         ++output;
      }
   } else {
      TPF step = static_cast< TPF >( 1.0 / zoom );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         TPF pos2 = pos * pos;
         TPF pos3 = pos2 * pos;
         *output = input[ -2 ] * ( pos3 - TPF( 2 ) * pos2 + pos ) / TPF( 12 ) +
                   input[ -1 ] * ( TPF( -7 ) * pos3 + TPF( 15 ) * pos2 - TPF( 8 ) * pos ) / TPF( 12 ) +
                   input[  0 ] * ( TPF( 16 ) * pos3 - TPF( 28 ) * pos2 + TPF( 12 )) / TPF( 12 ) +
                   input[  1 ] * ( TPF( -16 ) * pos3 + TPF( 20 ) * pos2 + TPF( 8 ) * pos ) / TPF( 12 ) +
                   input[  2 ] * ( TPF( 7 ) * pos3 - TPF( 6 ) * pos2 - pos ) / TPF( 12 ) +
                   input[  3 ] * ( -pos3 + pos2 ) / TPF( 12 );
         ++output;
         pos += step;
         if( pos >= 1.0 ) {
            offset = static_cast< dip::sint >( std::floor( pos ));
            pos -= static_cast< TPF >( offset );
            input += offset;
         }
      }
   }
}

template< typename TPI >
void ThirdOrderCubicSpline(
      TPI* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift
) {
   using TPF = FloatType< TPI >;
   dip::sint offset = static_cast< dip::sint >( std::floor( shift ));
   input += offset;
   TPF pos = static_cast< TPF >( shift ) - static_cast< TPF >( offset );
   if( zoom == 1.0 ) {
      TPF pos2 = pos * pos;
      TPF pos3 = pos2 * pos;
      TPF filter_m1 = ( -pos3 + TPF( 2 ) * pos2 - pos ) / TPF( 2 );
      TPF filter__0 = ( TPF( 3 ) * pos3 - TPF( 5 ) * pos2 + TPF( 2 )) / TPF( 2 );
      TPF filter__1 = ( TPF( -3 ) * pos3 + TPF( 4 ) * pos2 + pos ) / TPF( 2 );
      TPF filter__2 = ( pos3 - pos2 ) / TPF( 2 );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = input[ -1 ] * filter_m1 +
                   input[  0 ] * filter__0 +
                   input[  1 ] * filter__1 +
                   input[  2 ] * filter__2;
         ++input;
         ++output;
      }
   } else {
      TPF step = static_cast< TPF >( 1.0 / zoom );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         TPF pos2 = pos * pos;
         TPF pos3 = pos2 * pos;
         *output = TPF( 0.5 ) * ( input[ -1 ] * ( -pos3 + TPF( 2 ) * pos2 - pos ) +
                                  input[  0 ] * ( TPF( 3 ) * pos3 - TPF( 5 ) * pos2 + TPF( 2 )) +
                                  input[  1 ] * ( TPF( -3 ) * pos3 + TPF( 4 ) * pos2 + pos ) +
                                  input[  2 ] * ( pos3 - pos2 ));
         ++output;
         pos += step;
         if( pos >= 1.0 ) {
            offset = static_cast< dip::sint >( std::floor( pos ));
            pos -= static_cast< TPF >( offset );
            input += offset;
         }
      }
   }
}

template< typename TPI >
void Linear(
      TPI* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift
) {
   using TPF = FloatType< TPI >;
   dip::sint offset = static_cast< dip::sint >( std::floor( shift ));
   input += offset;
   TPF pos = static_cast< TPF >( shift ) - static_cast< TPF >( offset );
   if( zoom == 1.0 ) {
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = ( 1 - pos ) * input[ 0 ] + pos * input[ 1 ];
         ++input;
         ++output;
      }
   } else {
      TPF step = static_cast< TPF >( 1.0 / zoom );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = ( 1 - pos ) * input[ 0 ] + pos * input[ 1 ];
         ++output;
         pos += step;
         if( pos >= 1.0 ) {
            offset = static_cast< dip::sint >( std::floor( pos ));
            pos -= static_cast< TPF >( offset );
            input += offset;
         }
      }
   }
}

template< typename TPI, bool inverse = false >
void NearestNeighbor(
      TPI* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift
) {
   dip::sint offset = static_cast< dip::sint >( consistent_round< dfloat, inverse >( shift ));
   input += offset;
   if( zoom == 1.0 ) {
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = *input;
         ++input;
         ++output;
      }
   } else {
      dfloat pos = shift - static_cast< dfloat >( offset );
      dfloat step = 1.0 / zoom;
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = *input;
         ++output;
         pos += step;
         if( pos >= 0.5 ) {
            offset = static_cast< dip::sint >( consistent_round< dfloat, inverse >( pos ));
            pos -= static_cast< dfloat >( offset );
            input += offset;
         }
      }
   }
}

template< typename TPI, dip::uint a = 2 > // `a` is the filter parameter
void Lanczos(
      TPI* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift
) {
   static_assert(( a > 0 ) && ( a < 20 ), "Parameter out of range." );
   constexpr dip::sint sa = static_cast< dip::sint >( a );
   using TPF = FloatType< TPI >;
   dip::sint offset = static_cast< dip::sint >( std::floor( shift ));
   input += offset;
   TPF pos = static_cast< TPF >( shift ) - static_cast< TPF >( offset );
   if( zoom == 1.0 ) {
      if( pos > 1.0 - 1.0e-8 ) {
         // Assume integer shift.
         pos = 0.0; // (next `if` will test positive)
         ++input;
      }
      if( pos < 1.0e-8 ) {
         // Assume integer shift.
         // This avoids computing the sinc function at x=0.
         for( dip::uint ii = 0; ii < outSize; ii++ ) {
            *output = *input;
            ++input;
            ++output;
         }
      } else {
         TPF filter[ 2 * a ]; // buffer for the filter weights
         TPF sum = 0;         // sum of filter weights
         for( dip::uint jj = 0; jj < 2 * a; jj++ ) {
            long double x = pi * ( pos - ( static_cast< long double >( jj ) - static_cast< long double >( a - 1 )));
            sum += filter[ jj ] = static_cast< TPF >( a * std::sin( x ) * std::sin( x / a ) / ( x * x ));
         }
         for( dip::uint jj = 0; jj < 2 * a; jj++ ) {
            filter[ jj ] /= sum; // normalization avoids a large error
         }
         input -= a - 1;
         for( dip::uint ii = 0; ii < outSize; ii++ ) {
            TPI value = 0;
            for( dip::uint jj = 0; jj < 2 * a; jj++ ) {
               value += input[ jj ] * static_cast< TPF >( filter[ jj ] );
            }
            *output = value;
            ++input;
            ++output;
         }
      }
   } else {
      TPF step = static_cast< TPF >( 1.0 / zoom );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         if( pos < 1.0e-8 ) {
            *output = input[ 0 ];     // avoid computing the sinc function at x=0.
         } else if( pos > 1.0 - 1.0e-8 ) {
            *output = input[ 1 ];     // avoid computing the sinc function at x=0.
         } else {
            TPI value = 0;
            TPF weight = 0;
            for( dip::sint jj = -sa + 1; jj <= sa; jj++ ) {
               long double x = pi * ( pos - static_cast< long double >( jj ));
               TPF p = static_cast< TPF >( a * std::sin( x ) * std::sin( x / a ) / ( x * x ));
               value += input[ jj ] * p;
               weight += p;
               // TODO: It would be great to cache these values, but that's only viable if zoom is integer.
            }
            *output = value / weight; // normalization avoids a large error
         }
         ++output;
         pos += step;
         if( pos >= 1.0 ) {
            offset = static_cast< dip::sint >( std::floor( pos ));
            pos -= static_cast< TPF >( offset );
            input += offset;
         }
      }
   }
}

enum class Method {
      BSPLINE,
      CUBIC_ORDER_4,
      CUBIC_ORDER_3,
      LINEAR,
      NEAREST_NEIGHBOR,
      INVERSE_NEAREST_NEIGHBOR,
      LANCZOS8,
      LANCZOS6,
      LANCZOS4,
      LANCZOS3,
      LANCZOS2,
      FT
};

Method ParseMethod( String const& method ) {
   if( method.empty() || ( method == "3-cubic" )) {
      return Method::CUBIC_ORDER_3;
   } else if( method == "4-cubic" ) {
      return Method::CUBIC_ORDER_4;
   } else if( method == "linear" ) {
      return Method::LINEAR;
   } else if(( method == "nn" ) || ( method == "nearest" )) {
      return Method::NEAREST_NEIGHBOR;
   } else if(( method == "nn2" ) || ( method == "inverse nearest" )) {
      return Method::INVERSE_NEAREST_NEIGHBOR;
   } else if( method == "bspline" ) {
      return Method::BSPLINE;
   } else if( method == "ft" ) {
      return Method::FT;
   } else if( method == "lanczos8" ) {
      return Method::LANCZOS8;
   } else if( method == "lanczos6" ) {
      return Method::LANCZOS6;
   } else if( method == "lanczos4" ) {
      return Method::LANCZOS4;
   } else if( method == "lanczos3" ) {
      return Method::LANCZOS3;
   } else if( method == "lanczos2" ) {
      return Method::LANCZOS2;
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
}

dip::uint GetBorderSize( Method method ) {
   dip::uint border;
   switch( method ) {
      case Method::LANCZOS8:
         border = 8;
         break;
      case Method::LANCZOS6:
         border = 6;
         break;
      case Method::BSPLINE:
         border = 5;
         break;
      case Method::LANCZOS4:
         border = 4;
         break;
      case Method::LANCZOS3:
      case Method::CUBIC_ORDER_4:
         border = 3;
         break;
      case Method::LANCZOS2:
      case Method::CUBIC_ORDER_3:
         border = 2;
         break;
      case Method::LINEAR:
      case Method::INVERSE_NEAREST_NEIGHBOR:
      case Method::NEAREST_NEIGHBOR:
         border = 1;
         break;
      //case Method::FT:
      default:
         border = 0;
         break;
   }
   return border;
}

dip::uint GetNumberOfOperations( Method method, dip::uint lineLength, dfloat zoom ) {
   dip::uint outLength = static_cast< dip::uint >( std::ceil( static_cast< dfloat >( lineLength ) * zoom ));
   switch( method ) {
      case Method::BSPLINE:
         return ( lineLength + 10 ) * 40 + outLength * 12;
      case Method::CUBIC_ORDER_4:
         if( zoom == 1.0 ) {
            return 22 + 6 * lineLength;
         } else {
            return ( 22 + 6 ) * outLength;
         }
      case Method::CUBIC_ORDER_3:
         if( zoom == 1.0 ) {
            return 16 + 4 * lineLength;
         } else {
            return ( 16 + 4 ) * outLength;
         }
      case Method::LINEAR:
         return 3 * outLength;
      case Method::NEAREST_NEIGHBOR:
      case Method::INVERSE_NEAREST_NEIGHBOR:
         return outLength;
      case Method::LANCZOS8:
         if( zoom == 1.0 ) {
            return 16 * 50 + 17 * lineLength; // assuming sin = 20 cycles
         } else {
            return 17 * 50 * outLength;
         }
      case Method::LANCZOS6:
         if( zoom == 1.0 ) {
            return 12 * 50 + 13 * lineLength; // assuming sin = 20 cycles
         } else {
            return 13 * 50 * outLength;
         }
      case Method::LANCZOS4:
         if( zoom == 1.0 ) {
            return 8 * 50 + 9 * lineLength; // assuming sin = 20 cycles
         } else {
            return 9 * 50 * outLength;
         }
      case Method::LANCZOS3:
         if( zoom == 1.0 ) {
            return 6 * 50 + 7 * lineLength; // assuming sin = 20 cycles
         } else {
            return 7 * 50 * outLength;
         }
      case Method::LANCZOS2:
         if( zoom == 1.0 ) {
            return 4 * 50 + 5 * lineLength; // assuming sin = 20 cycles
         } else {
            return 5 * 50 * outLength;
         }
      //case Method::FT:
         //TODO: Implement FT interpolation method
      default:
         DIP_THROW( E::NOT_IMPLEMENTED );
   }
}

template< typename TPI >
void Dispatch(
      Method method,
      TPI* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift,
      TPI* spline1, // for BSpline only
      TPI* spline2  // for BSpline only
) {
   switch( method ) {
      case Method::BSPLINE:
         BSpline< TPI >( input, output, outSize, zoom, shift, spline1, spline2 );
         break;
      case Method::CUBIC_ORDER_4:
         FourthOrderCubicSpline< TPI >( input, output, outSize, zoom, shift );
         break;
      case Method::CUBIC_ORDER_3:
         ThirdOrderCubicSpline< TPI >( input, output, outSize, zoom, shift );
         break;
      case Method::LINEAR:
         Linear< TPI >( input, output, outSize, zoom, shift );
         break;
      case Method::NEAREST_NEIGHBOR:
         NearestNeighbor< TPI >( input, output, outSize, zoom, shift );
         break;
      case Method::INVERSE_NEAREST_NEIGHBOR:
         NearestNeighbor< TPI, true >( input, output, outSize, zoom, shift );
         break;
      case Method::LANCZOS2:
         Lanczos< TPI, 2 >( input, output, outSize, zoom, shift );
         break;
      case Method::LANCZOS3:
         Lanczos< TPI, 3 >( input, output, outSize, zoom, shift );
         break;
      case Method::LANCZOS4:
         Lanczos< TPI, 4 >( input, output, outSize, zoom, shift );
         break;
      case Method::LANCZOS6:
         Lanczos< TPI, 6 >( input, output, outSize, zoom, shift );
         break;
      case Method::LANCZOS8:
         Lanczos< TPI, 8 >( input, output, outSize, zoom, shift );
         break;
      //case Method::FT:
         //TODO: Implement FT interpolation method
         // Note that this will require us to cache the DFT data structures, we don't want to compute them anew
         // for each image line. We currently do not have the infrastructure in place to do so.
      default:
         DIP_THROW( E::NOT_IMPLEMENTED );
   }
}

} // namespace interpolation
} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include <numeric>

template< typename T >
T abs_diff( T a, T b ) {
   return std::abs( a - b );
}
template< typename T >
T abs_diff( std::complex< T > a, std::complex< T > b ) {
   return std::abs( a - b );
}

DOCTEST_TEST_CASE("[DIPlib] testing the interpolation functions") {

   // 1- Test all methods using sfloat, and unit zoom

   std::vector< dip::sfloat > buffer( 100 );
   std::vector< dip::sfloat > spline1( 100 );
   std::vector< dip::sfloat > spline2( 100 );
   std::iota( buffer.begin(), buffer.end(), 0 );
   dip::sfloat* input = buffer.data() + 20; // we use the elements 20-80, and presume 20 elements as boundary on either side

   dip::sfloat shift = 4.3f;
   dip::uint N = 60;
   std::vector< dip::sfloat > output( N, -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), N, 1.0, shift, spline1.data(), spline2.data() );
   bool error = false;
   dip::sfloat offset = 20.0f + shift;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 3e-4; // BSpline is not as precise, especially near the edges.
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), N, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), N, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), N, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), N, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ))) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), N, 1.0, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ));
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 3e-2; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 5 >( input, output.data(), N, 1.0, shift );
   error = false;
   //std::cout << "\nLanczos 5: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ));
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1.1e-2; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );


   // 2- Test all methods using sfloat, and zoom > 1

   shift = -3.4f;
   dip::sfloat scale = 3.3f;
   N = 200;
   output.resize( N );
   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), N, scale, shift, spline1.data(), spline2.data() );
   offset = 20.0f + shift;
   dip::sfloat step = 1.0f / scale;
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-3; // BSpline is not as precise, especially near the edges.
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ) * step )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), N, scale, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 3.2e-2; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 8 >( input, output.data(), N, scale, shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 7.2e-3; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 3- Test all methods using sfloat, and zoom < 1

   shift = 10.51f;
   scale = 0.41f;
   N = 20;
   output.resize( N );
   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), N, scale, shift, spline1.data(), spline2.data() );
   offset = 20.0f + shift;
   step = 1.0f / scale;
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-3; // BSpline is not as precise, especially near the edges.
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), N, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ) * step )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), N, scale, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 3.2e-2; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 8 >( input, output.data(), N, scale, shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 7.2e-3; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 4- Test all methods using dcomplex

   std::vector< dip::dcomplex > d_buffer( 100 );
   std::vector< dip::dcomplex > d_spline1( 100 );
   std::vector< dip::dcomplex > d_spline2( 100 );
   for( dip::uint ii = 0; ii < d_buffer.size(); ++ii ) {
      d_buffer[ ii ] = dip::dcomplex{ dip::dfloat( ii ), 200.0 - dip::dfloat( ii ) };
   }
   dip::dcomplex* d_input = d_buffer.data() + 20; // we use the elements 20-80, and presume 20 elements as boundary on either side

   dip::dfloat d_shift = -3.4;
   dip::dfloat d_scale = 5.23;
   N = 20;
   std::vector< dip::dcomplex > d_output( N, -1e6 );
   dip::interpolation::BSpline< dip::dcomplex >( d_input, d_output.data(), N, d_scale, d_shift, d_spline1.data(), d_spline2.data() );
   dip::dcomplex d_offset{ 20.0 + d_shift, 200.0 - 20.0 - d_shift };
   dip::dfloat d_step = 1.0 / d_scale;
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 2.2e-3; // BSpline is not as precise, especially near the edges.
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::dcomplex >( d_input, d_output.data(), N, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::dcomplex >( d_input, d_output.data(), N, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Linear< dip::dcomplex >( d_input, d_output.data(), N, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::dcomplex >( d_input, d_output.data(), N, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      dip::dcomplex res = d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step };
      error |= abs_diff( d_output[ ii ], { std::round( res.real() ), std::round( res.imag() ) } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::dcomplex, 2 >( d_input, d_output.data(), N, d_scale, d_shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } );
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 4.5e-2; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::dcomplex, 8 >( d_input, d_output.data(), N, d_scale, d_shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < N; ++ii ) {
      //std::cout << ", " << abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } );
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1.1e-2; // why such a large error?
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

}

#endif // DIP__ENABLE_DOCTEST
