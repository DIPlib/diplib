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
#include "diplib/dft.h"

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
 * dip::interpolation::Fourier< TPI >()
 *    Interpolates by manipulating the Fourier transform. Each output depends on all input samples. Imposes
 *    a periodic boundary condition. a = 0.
 *
 * All these functions have as parameters:
 *    TPI const* input                -- input buffer; because we need a boundary extension, it'll always be a
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
 * Fourier works only with complex types: scomplex, dcomplex, and expects the output to be contiguous (stride==1).
 */

// Computes the second derivative at each point, as required for B-spline interpolation.
template< typename TPI >
void SplineDerivative(
      TPI const* input,
      TPI* buffer,  // buffer will be filled with the estimated second derivative, second half of buffer for temp data
      dip::uint n   // length of input, buffer has 2n elements
) {
   TPI* spline1 = buffer;
   TPI* spline2 = buffer + n;
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
      TPI const* input,
      SampleIterator< TPI > output,
      dip::uint outSize,
      dfloat zoom,
      dfloat shift,
      TPI* buffer    // temporary buffer, size = 2 * ( size of input + border )
) {
   constexpr dip::uint boundary = 5;
   using TPF = FloatType< TPI >;
   dip::sint offset = static_cast< dip::sint >( std::floor( shift ));
   input += offset;
   SplineDerivative(
         input - boundary,
         buffer,
         static_cast< dip::uint >( std::floor( static_cast< dfloat >( outSize ) / zoom )) + 2 * boundary + 1 );
   buffer += boundary;
   TPF pos = static_cast< TPF >( shift ) - static_cast< TPF >( offset );
   if( zoom == 1.0 ) {
      TPF a = 1 - pos;
      TPF as = ( a * a * a - a ) / 6;
      TPF bs = ( pos * pos * pos - pos ) / 6;
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         *output = a * input[ 0 ] + pos * input[ 1 ] + as * buffer[ 0 ] + bs * buffer[ 1 ];
         ++output;
         ++input;
         ++buffer;
      }
   } else {
      TPF step = static_cast< TPF >( 1.0 / zoom );
      for( dip::uint ii = 0; ii < outSize; ii++ ) {
         TPF a = 1 - pos;
         *output = a * input[ 0 ] + pos * input[ 1 ] +
                   (( a * a * a - a ) * buffer[ 0 ] + ( pos * pos * pos - pos ) * buffer[ 1 ] ) / static_cast< TPF >( 6 );
         ++output;
         pos += step;
         if( pos >= 1.0 ) {
            offset = static_cast< dip::sint >( std::floor( pos ));
            pos -= static_cast< TPF >( offset );
            input += offset;
            buffer += offset;
         }
      }
   }
}

template< typename TPI >
void FourthOrderCubicSpline(
      TPI const* input,
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
      TPI const* input,
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
      TPI const* input,
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
      TPI const* input,
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
      TPI const* input,
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

template< typename TPI >
void Fourier(
      std::complex< TPI > const* input,
      std::complex< TPI >* output,
      dfloat shift,
      DFT< TPI > const& ft,               // DFT object configured for a transform of <size of input>
      DFT< TPI > const& ift,              // DFT object configured for an inverse transform of <size of output>
      std::complex< TPI > const* weights, // weights to apply a shift in the FT, <size of input> elements; if nullptr, use <shift>
      std::complex< TPI >* buffer         // temporary buffer, size = FourierBufferSize()
) {
   std::complex< TPI >* intermediate = buffer;
   dip::uint inSize = ft.TransformSize();
   dip::uint outSize = ift.TransformSize();
   buffer += std::max( inSize, outSize );
   TPI invScale = static_cast< TPI >( 1.0 / static_cast< dip::dfloat >( inSize ));
   // FT of input
   ft.Apply( input, intermediate, buffer, 1.0 );
   // Shift
   if( weights ) {
      // Use given weights
      for( auto ptr = intermediate; ptr < intermediate + inSize; ++ptr, ++weights ) {
         *ptr *= *weights;
      }
   } else if( shift != 0.0 ) {
      // Compute weights
      dfloat inc = -2 * pi / static_cast< dfloat >( inSize ) * shift;
      dfloat theta = inc;
      for( dip::uint ii = 1; ii < inSize / 2; ++ii ) {
         std::complex< TPI > w ( static_cast< TPI >( std::cos( theta )), static_cast< TPI >( std::sin( theta )));
         intermediate[ ii ] *= w;
         intermediate[ inSize - ii ] *= std::conj( w );
         theta += inc;
      }
   }
   // Scale
   if( outSize < inSize ) {
      // Crop: we keep (outSize+1)/2 on the left side, and outSize/2 on the right.
      std::move( intermediate + inSize - outSize / 2, intermediate + inSize, intermediate + ( outSize + 1 ) / 2 );
   } else if( outSize > inSize ) {
      // Expand: we keep (inSize+1)/2 on the left side, and inSize/2 on the right; the space in between we fill with 0.
      std::move_backward( intermediate + inSize - inSize / 2, intermediate + inSize, intermediate + outSize );
      std::fill( intermediate + inSize - inSize / 2, intermediate + outSize - inSize / 2, std::complex< TPI >( 0 ));
   }
   // Inverse FT
   ift.Apply( intermediate, output, buffer, invScale );
}

template< typename TPI >
void FourierShiftWeights( std::vector< TPI >&, dfloat ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

// Compute weights to apply a shift in the Fourier Domain, input argument to Fourier()
template< typename TPI >
void FourierShiftWeights(
      std::vector< std::complex< TPI >>& weights,
      dfloat shift
) {
   dip::uint inSize = weights.size();
   dfloat inc = -2 * pi / static_cast< dfloat >( inSize ) * shift;
   weights[ 0 ] = 1;
   weights[ inSize / 2 ] = 1; // In case it's an odd-sized array
   dfloat theta = inc;
   for( dip::uint ii = 1; ii < inSize / 2; ++ii ) {
      std::complex< TPI > w ( static_cast< TPI >( std::cos( theta )), static_cast< TPI >( std::sin( theta )));
      weights[ ii ] = w;
      weights[ inSize - ii ] = std::conj( w );
      theta += inc;
   }
}

// Returns the size of the buffer expected by Fourier()
template< typename TPI >
inline dip::uint FourierBufferSize(
      DFT< TPI > const& ft,         // DFT object configured for a transform of <size of input>
      DFT< TPI > const& ift         // DFT object configured for an inverse transform of <size of output>
) {
   return std::max( ft.TransformSize(), ift.TransformSize() ) + std::max( ft.BufferSize(), ift.BufferSize() );
}


// Returns the output size of an image line after the zooom
inline dip::uint ComputeOutputSize( dip::uint inSize, dfloat zoom ) {
   return static_cast< dip::uint >( std::floor( static_cast< dfloat >( inSize ) * zoom + 1e-6 ));
   // The 1e-6 is to avoid floating-point inaccuracies, ex: floor(49*(64/49))!=64
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
      FOURIER
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
   } else if( method == "ft" ) {
      return Method::FOURIER;
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
      //case Method::FOURIER:
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
      //case Method::FOURIER:
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
      TPI* buffer = nullptr        // for BSpline only
) {
   switch( method ) {
      case Method::BSPLINE:
         BSpline< TPI >( input, output, outSize, zoom, shift, buffer );
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
      //case Method::FOURIER:
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

DOCTEST_TEST_CASE("[DIPlib] testing the interpolation functions (except Fourier)") {

   // 1- Test all methods (except FOURIER) using sfloat, and unit zoom

   std::vector< dip::sfloat > buffer( 100 );
   std::vector< dip::sfloat > tmpSpline( 200 );
   std::iota( buffer.begin(), buffer.end(), 0 );
   dip::sfloat* input = buffer.data() + 20; // we use the elements 20-80, and presume 20 elements as boundary on either side
   dip::uint inSize = 60;

   dip::sfloat shift = 4.3f;
   dip::uint outSize = inSize;
   std::vector< dip::sfloat > output( outSize, -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), outSize, 1.0, shift, tmpSpline.data() );
   bool error = false;
   dip::sfloat offset = 20.0f + shift;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ))) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), outSize, 1.0, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ));
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 3e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 5 >( input, output.data(), outSize, 1.0, shift );
   error = false;
   //std::cout << "\nLanczos 5: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ));
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1.1e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 2- Test all methods (except FOURIER) using sfloat, and zoom > 1

   shift = -3.4f;
   dip::sfloat scale = 3.3f;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), outSize, scale, shift, tmpSpline.data() );
   offset = 20.0f + shift;
   dip::sfloat step = 1.0f / scale;
   error = false;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ) * step )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 3.2e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 8 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 7.2e-3; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 3- Test all methods (except FOURIER) using sfloat, and zoom < 1

   shift = 10.51f;
   scale = 0.41f;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), outSize, scale, shift, tmpSpline.data() );
   offset = 20.0f + shift;
   step = 1.0f / scale;
   error = false;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ) * step )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 3.2e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 8 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 7.2e-3; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 4- Test all methods (except FOURIER) using dcomplex

   std::vector< dip::dcomplex > d_buffer( 100 );
   std::vector< dip::dcomplex > d_tmpSpline( 200 );
   for( dip::uint ii = 0; ii < d_buffer.size(); ++ii ) {
      d_buffer[ ii ] = dip::dcomplex{ dip::dfloat( ii ), 200.0 - dip::dfloat( ii ) };
   }
   dip::dcomplex* d_input = d_buffer.data() + 20; // we use the elements 20-80, and presume 20 elements as boundary on either side

   dip::dfloat d_shift = -3.4;
   dip::dfloat d_scale = 5.23;
   outSize = dip::interpolation::ComputeOutputSize( inSize, d_scale );
   std::vector< dip::dcomplex > d_output( outSize, -1e6 );
   dip::interpolation::BSpline< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift, d_tmpSpline.data() );
   dip::dcomplex d_offset{ 20.0 + d_shift, 200.0 - 20.0 - d_shift };
   dip::dfloat d_step = 1.0 / d_scale;
   error = false;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Linear< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      dip::dcomplex res = d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step };
      error |= abs_diff( d_output[ ii ], { std::round( res.real() ), std::round( res.imag() ) } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::dcomplex, 2 >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } );
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 4.5e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::dcomplex, 8 >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } );
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1.1e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

}

DOCTEST_TEST_CASE("[DIPlib] testing the Fourier interpolation function") {

   // 1- Test using unit zoom (shift only)

   std::vector< dip::scomplex > input( 100 );
   dip::uint inSize = input.size();
   for( dip::uint ii = 0; ii < inSize; ++ii ) {
      auto x = static_cast< dip::dfloat >( ii ) / static_cast< dip::dfloat >( inSize );
      input[ ii ] = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
   }
   dip::dfloat shift = 4.3;
   dip::uint outSize = inSize;
   std::vector< dip::scomplex > output( outSize, -1e6f );

   dip::DFT< dip::sfloat > ft( inSize, false );
   dip::DFT< dip::sfloat > ift( outSize, true );
   std::vector< dip::scomplex > buffer( dip::interpolation::FourierBufferSize( ft, ift ));
   dip::interpolation::Fourier< dip::sfloat >( input.data(), output.data(), shift, ft, ift, nullptr, buffer.data() );

   bool error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      auto x = ( static_cast< dip::dfloat >( ii ) - shift ) / static_cast< dip::dfloat >( outSize );
      dip::scomplex expected = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
      error |= abs_diff( output[ ii ], expected ) > 1e-6;
      //std::cout << x << ": " << output[ ii ] << " == " << expected << std::endl;
   }
   DOCTEST_CHECK_FALSE( error );

   // 2- Test using zoom > 1

   shift = -3.4;
   dip::dfloat scale = 3.3;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   scale = static_cast< dip::dfloat >( outSize ) / static_cast< dip::dfloat >( inSize );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );
   ft.Initialize( inSize, false );
   ift.Initialize( outSize, true );
   buffer.resize( dip::interpolation::FourierBufferSize( ft, ift ));
   dip::interpolation::Fourier< dip::sfloat >( input.data(), output.data(), shift, ft, ift, nullptr, buffer.data() );

   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      auto x = ( static_cast< dip::dfloat >( ii ) - shift * scale ) / static_cast< dip::dfloat >( outSize );
      dip::scomplex expected = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
      error |= abs_diff( output[ ii ], expected ) > 1e-6;
      //std::cout << x << ": " << output[ ii ] << " == " << expected << std::endl;
   }
   DOCTEST_CHECK_FALSE( error );

   // 3- Test using zoom < 1

   shift = 10.51;
   scale = 0.41;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   scale = static_cast< dip::dfloat >( outSize ) / static_cast< dip::dfloat >( inSize );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );

   ft.Initialize( inSize, false );
   ift.Initialize( outSize, true );
   buffer.resize( dip::interpolation::FourierBufferSize( ft, ift ));
   dip::interpolation::Fourier< dip::sfloat >( input.data(), output.data(), shift, ft, ift, nullptr, buffer.data() );

   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      auto x = ( static_cast< dip::dfloat >( ii ) - shift * scale ) / static_cast< dip::dfloat >( outSize );
      dip::scomplex expected = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
      error |= abs_diff( output[ ii ], expected ) > 1e-6;
      //std::cout << x << ": " << output[ ii ] << " == " << expected << std::endl;
   }
   DOCTEST_CHECK_FALSE( error );

}

#endif // DIP__ENABLE_DOCTEST
