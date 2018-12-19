/*
 * DIPlib 3.0
 * This file contains definitions for sub-pixel peak localization functions
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
#include "diplib/analysis.h"
#include "diplib/morphology.h" // Maxima, Minima
#include "diplib/generation.h" // SetBorder
#include "diplib/geometry.h" // ResampleAtUnchecked
#include "diplib/measurement.h" // MeasurementTool
#include "diplib/overload.h"

namespace dip {

namespace {

/*
Compute subpixel offset of local maximum of a 3x3 image patch around a peak
using 2D quadratic fit:
f = a0 + a1*x + a2*y + a3*x*x + a4*y*y + a5*x*y
formulae taken from "Estimating Mean Wind"
http://lidar.ssec.wisc.edu/papers/akp_thes/node19.htm

Shortcut for least quares solution is computed in Matlab as follows:
x = [-1 0 1 -1 0 1 -1 0 1]'; y = [-1 -1 -1 0 0 0 1 1 1]';
G = [ones([9 1]) x y x.*x y.*y x.*y];
=> a = inv(G'*G)*(G'*t) = (inv(G'*G)*G')*t  due to associativity

inv(G'*G)*G' * 6 =
    -0.667  1.333 -0.667  1.333  3.333  1.333 -0.667  1.333 -0.667
    -1      0      1     -1      0      1     -1      0      1
    -1     -1     -1      0      0      0      1      1      1
     1     -2      1      1     -2      1      1     -2      1
     1      1      1     -2     -2     -2      1      1      1
     1.5    0     -1.5    0      0      0     -1.5    0      1.5

Setting df/dx = df/dy = 0 => linear system
   | 2*a3   a5 |   | x |   | -a1 |
   |   a5 2*a4 | * | y | = | -a2 |
*/
bool quadratic_fit_3x3(
      dfloat const* t, // array of intensities of 9 neighborhood pixels (normal stride)
      dfloat* x,       // offset of subpixel maximum from center pix of 3x3 window
      dfloat* y,
      dfloat* val      // value at maximum
) {
   dfloat w[] = {
         -2/3,  4/3, -2/3,  4/3, 10/3,  4/3, -2/3,  4/3, -2/3,
         -1  ,  0  ,  1  , -1  ,  0  ,  1  , -1  ,  0  ,  1  ,
         -1  , -1  , -1  ,  0  ,  0  ,  0  ,  1  ,  1  ,  1  ,
          1  , -2  ,  1  ,  1  , -2  ,  1  ,  1  , -2  ,  1  ,
          1  ,  1  ,  1  , -2  , -2  , -2  ,  1  ,  1  ,  1  ,
          1.5,  0  , -1.5,  0  ,  0  ,  0  , -1.5,  0  ,  1.5
   };

   // Least squares solution of the 2D quadratic fit
   dfloat a[ 6 ];
   dip::uint kk = 0;
   for( dip::uint ii = 0; ii < 6; ++ii ) {
      a[ ii ] = 0;
      for( dip::uint jj = 0; jj < 9; ++jj ) {
         a[ ii ] += w[ kk++ ] * t[ jj ];
      }
      a[ ii ] = a[ ii ] / 6;
   }

   // Solution of the maximum offsets
   dfloat denom = a[ 5 ] * a[ 5 ] - 4 * a[ 3 ] * a[ 4 ];
   if( denom == 0 ) {
      return false;
   }
   *x = ( 2 * a[ 4 ] * a[ 1 ] - a[ 5 ] * a[ 2 ] ) / denom;
   *y = ( 2 * a[ 3 ] * a[ 2 ] - a[ 5 ] * a[ 1 ] ) / denom;

   // Offsets are supposed to be within +/-0.5, if not, the subpixel peak does not exist in the 3x3 input
   // neighborhood. However, if the real maximum is close to 0.5, a small numerical inaccuracy will invalidate
   // it, so we use +/-0.75 instead. */
   if(( *x < -0.75 ) || ( *x > 0.75 ) || ( *y < -0.75 ) || ( *y > 0.75 )) {
      return false;
   }

   // Value at the maximum
   *val = a[ 0 ] + a[ 1 ] * ( *x ) + a[ 2 ] * ( *y ) + a[ 3 ] * ( *x ) * ( *x ) + a[ 4 ] * ( *y ) * ( *y ) +
          a[ 5 ] * ( *x ) * ( *y );
   return true;
}

/*
Compute subpixel offset of local maximum of a 3x3x3 image patch around a peak
using 3D quadratic fit:
f = a0 + a1*x + a2*y + a3*z + a4*x*x + a5*y*y + a6*z*z + a7*y*z + a8*z*x + a9*x*y

Shortcut for least quares solution is computed in Matlab as follows:
[y,x,z] = meshgrid([-1:1],[-1:1],[-1:1]);
x = x(:); y = y(:); z = z(:);
G = [ones([27 1]) x y z x.*x y.*y z.*z y.*z z.*x x.*y];
=> a = inv(G'*G)*(G'*t) = (inv(G'*G)*G')*t  due to associativity

inv(G'*G)*G' * 18 =
 -1.33  0.67 -1.33  0.67  2.67  0.67 -1.33  0.67 -1.33  0.67  2.67  0.67  2.67  4.67  2.67  0.67  2.67  0.67 -1.33  0.67 -1.33  0.67  2.67  0.67 -1.33  0.67 -1.33
 -1     0     1    -1     0     1    -1     0     1    -1     0     1    -1     0     1    -1     0     1    -1     0     1    -1     0     1    -1     0     1
 -1    -1    -1     0     0     0     1     1     1    -1    -1    -1     0     0     0     1     1     1    -1    -1    -1     0     0     0     1     1     1
 -1    -1    -1    -1    -1    -1    -1    -1    -1     0     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1
  1    -2     1     1    -2     1     1    -2     1     1    -2     1     1    -2     1     1    -2     1     1    -2     1     1    -2     1     1    -2     1
  1     1     1    -2    -2    -2     1     1     1     1     1     1    -2    -2    -2     1     1     1     1     1     1    -2    -2    -2     1     1     1
  1     1     1     1     1     1     1     1     1    -2    -2    -2    -2    -2    -2    -2    -2    -2     1     1     1     1     1     1     1     1     1
  1.5   1.5   1.5   0     0     0    -1.5  -1.5  -1.5   0     0     0     0     0     0     0     0     0    -1.5  -1.5  -1.5   0     0     0     1.5   1.5   1.5
  1.5   0    -1.5   1.5   0    -1.5   1.5   0    -1.5   0     0     0     0     0     0     0     0     0    -1.5   0     1.5  -1.5   0     1.5  -1.5   0     1.5
  1.5   0    -1.5   0     0     0    -1.5   0     1.5   1.5   0    -1.5   0     0     0    -1.5   0     1.5   1.5   0    -1.5   0     0     0    -1.5   0     1.5


Setting df/dx = df/dy = df/dz = 0 => linear system
   | 2*a4   a9   a8 |   | x |   | -a1 |
   |   a9 2*a5   a7 | * | y | = | -a2 |
   |   a8   a7 2*a6 |   | z |   | -a3 |
*/
bool quadratic_fit_3x3x3(
      dfloat const* t, // array of intensities of 27 neighborhood pixels (normal stride)
      dfloat* x,       // offset of subpixel maximum from center pix of 3x3x3 window
      dfloat* y,
      dfloat* z,
      dfloat* val      // value at maximum
) {
   dfloat w[] = {
         -4./3,  2./3, -4./3,  2./3,  8./3,  2./3, -4./3,  2./3, -4./3,  2./3,  8./3,  2./3,  8./3, 14./3,  8./3,  2./3,  8./3,  2./3, -4./3,  2./3, -4./3,  2./3,  8./3,  2./3, -4./3,  2./3, -4./3,
         -1   ,  0   ,  1   , -1   ,  0   ,  1   , -1   ,  0   ,  1   , -1   ,  0   ,  1   , -1   ,  0   ,  1   , -1   ,  0   ,  1   , -1   ,  0   ,  1   , -1   ,  0   ,  1   , -1   ,  0   ,  1   ,
         -1   , -1   , -1   ,  0   ,  0   ,  0   ,  1   ,  1   ,  1   , -1   , -1   , -1   ,  0   ,  0   ,  0   ,  1   ,  1   ,  1   , -1   , -1   , -1   ,  0   ,  0   ,  0   ,  1   ,  1   ,  1   ,
         -1   , -1   , -1   , -1   , -1   , -1   , -1   , -1   , -1   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,
          1   , -2   ,  1   ,  1   , -2   ,  1   ,  1   , -2   ,  1   ,  1   , -2   ,  1   ,  1   , -2   ,  1   ,  1   , -2   ,  1   ,  1   , -2   ,  1   ,  1   , -2   ,  1   ,  1   , -2   ,  1   ,
          1   ,  1   ,  1   , -2   , -2   , -2   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   , -2   , -2   , -2   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   , -2   , -2   , -2   ,  1   ,  1   ,  1   ,
          1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   , -2   , -2   , -2   , -2   , -2   , -2   , -2   , -2   , -2   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,  1   ,
          1.5 ,  1.5 ,  1.5 ,  0   ,  0   ,  0   , -1.5 , -1.5 , -1.5 ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   , -1.5 , -1.5 , -1.5 ,  0   ,  0   ,  0   ,  1.5 ,  1.5 ,  1.5 ,
          1.5 ,  0   , -1.5 ,  1.5 ,  0   , -1.5 ,  1.5 ,  0   , -1.5 ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  0   , -1.5 ,  0   ,  1.5 , -1.5 ,  0   ,  1.5 , -1.5 ,  0   ,  1.5 ,
          1.5 ,  0   , -1.5 ,  0   ,  0   ,  0   , -1.5 ,  0   ,  1.5 ,  1.5 ,  0   , -1.5 ,  0   ,  0   ,  0   , -1.5 ,  0   ,  1.5 ,  1.5 ,  0   , -1.5 ,  0   ,  0   ,  0   , -1.5 ,  0   ,  1.5
   };

   // Least squares solution of the 3D quadratic fit
   dfloat a[ 10 ];
   dip::uint kk = 0;
   for( dip::uint ii = 0; ii < 10; ++ii ) {
      a[ ii ] = 0;
      for( dip::uint jj = 0; jj < 27; ++jj ) {
         a[ ii ] += w[ kk++ ] * t[ jj ];
      }
      a[ ii ] = a[ ii ] / 18;
   }

   // Solution of the maximum offsets
   dfloat b[ 9 ];
   b[ 0 ] = 2 * a[ 4 ];
   b[ 1 ] = a[ 9 ];
   b[ 2 ] = a[ 8 ];
   b[ 3 ] = a[ 9 ];
   b[ 4 ] = 2 * a[ 5 ];
   b[ 5 ] = a[ 7 ];
   b[ 6 ] = a[ 8 ];
   b[ 7 ] = a[ 7 ];
   b[ 8 ] = 2 * a[ 6 ];
   dfloat c[ 3 ];
   c[ 0 ] = -a[ 1 ];
   c[ 1 ] = -a[ 2 ];
   c[ 2 ] = -a[ 3 ];
   Solve( 3, 3, b, c, c );
   *x = c[ 0 ];
   *y = c[ 1 ];
   *z = c[ 2 ];

   // Offsets are supposed to be within +/-0.5, if not, the subpixel peak does not exist in the 3x3 input
   // neighborhood. However, if the real maximum is close to 0.5, a small numerical inaccuracy will invalidate
   // it, so we use +/-0.75 instead.
   if( *x < -0.75 || *x > 0.75 || *y < -0.75 || *y > 0.75 || *z < -0.75 || *z > 0.75 ) {
      return false;
   }

   // Value at the maximum
   *val = a[ 0 ] + a[ 1 ] * ( *x ) + a[ 2 ] * ( *y ) + a[ 3 ] * ( *z ) + a[ 4 ] * ( *x ) * ( *x ) +
          a[ 5 ] * ( *y ) * ( *y ) +
          a[ 6 ] * ( *z ) * ( *z ) + a[ 7 ] * ( *y ) * ( *z ) + a[ 8 ] * ( *z ) * ( *x ) + a[ 9 ] * ( *x ) * ( *y );

   return true;
}

// Computes the natural logarithm of the n values pointed to by t.
// If `invert`, it computes the log of (-t).
void log_transform(
      dfloat* t,
      dip::uint n,
      bool invert
) {
   if( invert ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         t[ ii ] = std::log( -t[ ii ] );
      }
   } else {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         t[ ii ] = std::log( t[ ii ] );
      }
   }
}

enum class SubpixelExtremumMethod {
      LINEAR,
      PARABOLIC_SEPARABLE,
      GAUSSIAN_SEPARABLE,
      PARABOLIC,
      GAUSSIAN,
      INTEGER
};

SubpixelExtremumMethod ParseMethod( String const& s_method, dip::uint nDims ) {
   SubpixelExtremumMethod method;
   if( s_method == dip::S::LINEAR ) {
      method = SubpixelExtremumMethod::LINEAR;
   } else if( s_method == dip::S::PARABOLIC ) {
      method = SubpixelExtremumMethod::PARABOLIC;
   } else if( s_method == dip::S::PARABOLIC_SEPARABLE ) {
      method = SubpixelExtremumMethod::PARABOLIC_SEPARABLE;
   } else if( s_method == dip::S::GAUSSIAN ) {
      method = SubpixelExtremumMethod::GAUSSIAN;
   } else if( s_method == dip::S::GAUSSIAN_SEPARABLE ) {
      method = SubpixelExtremumMethod::GAUSSIAN_SEPARABLE;
   } else if( s_method == dip::S::INTEGER ) {
      method = SubpixelExtremumMethod::INTEGER;
   } else {
      DIP_THROW_INVALID_FLAG( s_method );
   }
   if( nDims == 1 ) {
      // The non-separable and separable methods are identical for 1D images.
      if( method == SubpixelExtremumMethod::PARABOLIC ) {
         method = SubpixelExtremumMethod::PARABOLIC_SEPARABLE;
      }
      if( method == SubpixelExtremumMethod::GAUSSIAN ) {
         method = SubpixelExtremumMethod::GAUSSIAN_SEPARABLE;
      }
   }
   return method;
}

template< typename TPI >
SubpixelLocationResult dip__SubpixelLocation(
      Image const& input,
      UnsignedArray const& position,
      SubpixelExtremumMethod method,
      bool invert
) {
   TPI* in = static_cast< TPI* >( input.Pointer( position ));
   dip::uint nd = input.Dimensionality();
   SubpixelLocationResult out;
   out.value = static_cast< dfloat >( *in );
   out.coordinates.resize( nd, 0.0 );

   switch( method ) {

      // Linear == CoG
      case SubpixelExtremumMethod::LINEAR:
         for( dip::uint ii = 0; ii < nd; ++ii ) {
            dfloat t[ 3 ];
            t[ 0 ] = static_cast< dfloat >( *( in - input.Stride( ii )));
            t[ 1 ] = static_cast< dfloat >( *( in ));
            t[ 2 ] = static_cast< dfloat >( *( in + input.Stride( ii )));
            if( invert ) { // invert data
               t[ 0 ] = -t[ 0 ];
               t[ 1 ] = -t[ 1 ];
               t[ 2 ] = -t[ 2 ];
            }
            dfloat b = std::min( t[ 0 ], t[ 1 ] );
            b = std::min( b, t[ 2 ] );
            t[ 0 ] -= b;
            t[ 1 ] -= b;
            t[ 2 ] -= b;
            dfloat m = t[ 0 ] + t[ 1 ] + t[ 2 ];
            out.coordinates[ ii ] = static_cast< dfloat >( position[ ii ] );
            if( m != 0 ) {
               out.coordinates[ ii ] += ( t[ 2 ] - t[ 0 ] ) / m;
            }
         }
         // Max value is value of center pixel -- linear interpolation will only make this value lower.
         break;

      // Parabolic fit, separable
      default:
      //case SubpixelExtremumMethod::PARABOLIC_SEPARABLE:
      //case SubpixelExtremumMethod::GAUSSIAN_SEPARABLE:
         for( dip::uint ii = 0; ii < nd; ++ii ) {
            dfloat t[ 3 ];
            t[ 0 ] = static_cast< dfloat >( *( in - input.Stride( ii )));
            t[ 1 ] = static_cast< dfloat >( *( in ));
            t[ 2 ] = static_cast< dfloat >( *( in + input.Stride( ii )));
            bool inverted = false;
            if( method == SubpixelExtremumMethod::GAUSSIAN_SEPARABLE ) {
               inverted = t[ 1 ] < 0;
               log_transform( t, 3, inverted );
            }
            out.coordinates[ ii ] = static_cast< dfloat >( position[ ii ] );
            dfloat m = t[ 0 ] - 2 * t[ 1 ] + t[ 2 ];
            if( m != 0 ) {
               out.coordinates[ ii ] += ( t[ 0 ] - t[ 2 ] ) / ( 2 * m );
               dfloat b = t[ 1 ] - ( t[ 0 ] - t[ 2 ] ) * ( t[ 0 ] - t[ 2 ] ) / ( 8 * m );
               if( method == SubpixelExtremumMethod::GAUSSIAN_SEPARABLE ) {
                  b = std::exp( b );
                  if( inverted ) {
                     b = -b;
                  }
               }
               if( invert ) {
                  out.value = std::min( out.value, b );
               } else {
                  out.value = std::max( out.value, b );
               }
               // This is sort-of a cup-out: we're taking the max of all the 1D interpolated values, not the value
               // at the actual location of the max. Does this matter all that much? If so, we'd need to call
               // ResampleAt().
            }
         }
         break;

      // Parabolic fit, non-separable
      case SubpixelExtremumMethod::PARABOLIC:
      case SubpixelExtremumMethod::GAUSSIAN: {
         dfloat val;
         switch( nd ) {
            case 2: {
               dfloat t[ 9 ]; // 3x3 neighborhood around integer local maximum
               dip::uint count = 0;
               for( dip::sint jj = -1; jj <= 1; ++jj ) {
                  for( dip::sint ii = -1; ii <= 1; ++ii ) {
                     t[ count++ ] = *( in + ii * input.Stride( 0 ) + jj * input.Stride( 1 ));
                  }
               }
               bool inverted = false;
               if( method == SubpixelExtremumMethod::GAUSSIAN ) {
                  inverted = t[ 1 + 3 ] < 0;
                  log_transform( t, 3 * 3, inverted );
               }
               out.coordinates[ 0 ] = static_cast< dfloat >( position[ 0 ] );
               out.coordinates[ 1 ] = static_cast< dfloat >( position[ 1 ] );
               dfloat x, y;
               if( quadratic_fit_3x3( t, &x, &y, &val )) {
                  out.coordinates[ 0 ] += x;
                  out.coordinates[ 1 ] += y;
                  if( method == SubpixelExtremumMethod::GAUSSIAN ) {
                     val = std::exp( val );
                     if( inverted ) {
                        val = -( val );
                     }
                  }
               }
               break;
            }
            case 3: {
               dfloat t[ 28 ]; // 3x3x3 neighborhood around integer local maximum
               dip::uint count = 0;
               for( dip::sint kk = -1; kk <= 1; ++kk ) {
                  for( dip::sint jj = -1; jj <= 1; ++jj ) {
                     for( dip::sint ii = -1; ii <= 1; ++ii ) {
                        t[ count++ ] = *( in + ii * input.Stride( 0 ) + jj * input.Stride( 1 ) + kk * input.Stride( 2 ));
                     }
                  }
               }
               bool inverted = false;
               if( method == SubpixelExtremumMethod::GAUSSIAN ) {
                  inverted = t[ 1 + 3 + 3 * 3 ] < 0;
                  log_transform( t, 3 * 3 * 3, inverted );
               }
               out.coordinates[ 0 ] = static_cast< dfloat >( position[ 0 ] );
               out.coordinates[ 1 ] = static_cast< dfloat >( position[ 1 ] );
               out.coordinates[ 2 ] = static_cast< dfloat >( position[ 2 ] );
               dfloat x, y, z;
               if( quadratic_fit_3x3x3( t, &x, &y, &z, &val )) {
                  out.coordinates[ 0 ] += x;
                  out.coordinates[ 1 ] += y;
                  out.coordinates[ 2 ] += z;
                  if( method == SubpixelExtremumMethod::GAUSSIAN ) {
                     val = std::exp( val );
                     if( inverted ) {
                        val = -( val );
                     }
                  }
               }
               break;
            }
            default:
               DIP_THROW( E::ILLEGAL_DIMENSIONALITY );
               break;
         }
         // Don't allow the value to be lower than that of the maximal pixel's value.
         if( invert ) {
            out.value = std::min( out.value, val );
         } else {
            out.value = std::max( out.value, val );
         }
         break;
      }

      // Integer: don't do anything
      case SubpixelExtremumMethod::INTEGER:
         for( dip::uint ii = 0; ii < nd; ++ii ) {
            out.coordinates[ ii ] = static_cast< dfloat >( position[ ii ] );
         }
         // Max value is already given.
         break;
   }

   return out;
}

} // namespace

SubpixelLocationResult SubpixelLocation(
      Image const& in,
      UnsignedArray const& position,
      String const& polarity,
      String const& s_method
) {
   // Check input
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( position.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      // We can't determine sub-pixel locations if the maximum pixel is on the image border!
      if(( position[ ii ] < 1 ) || ( position[ ii ] >= in.Size( ii ) - 1 )) {
         DIP_THROW_IF( position[ ii ] >= in.Size( ii ), "Initial coordinates out of image bounds" );
         SubpixelLocationResult out;
         out.coordinates = FloatArray{ position };
         out.value = in.At< dfloat >( position );
         return out;
      }
   }
   bool invert;
   SubpixelExtremumMethod method;
   DIP_START_STACK_TRACE
      invert = BooleanFromString( polarity, S::MINIMUM, S::MAXIMUM );
      method = ParseMethod( s_method, nDims );
   DIP_END_STACK_TRACE
   // Data-type dependent stuff
   SubpixelLocationResult out;
   DIP_OVL_CALL_ASSIGN_REAL( out, dip__SubpixelLocation, ( in, position, method, invert ), in.DataType() );
   return out;
}

namespace {

SubpixelLocationArray SubpixelExtrema(
      Image const& in,
      Image const& mask,
      String const& s_method,
      bool invert // true for local minima
) {
   // Check input
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   SubpixelExtremumMethod method;
   DIP_STACK_TRACE_THIS( method = ParseMethod( s_method, nDims ));

   // Find local extrema
   Image localExtrema;
   if( invert ) {
      DIP_STACK_TRACE_THIS( Minima( in, localExtrema, nDims, "labels" ));
   } else {
      DIP_STACK_TRACE_THIS( Maxima( in, localExtrema, nDims, "labels" ));
   }

   // Mask local extrema
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( localExtrema.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW ));
      And( localExtrema, mask, localExtrema );
   }

   // Remove local extrema on the edge of the image
   SetBorder( localExtrema );

   // Get CoG of local extrema
   MeasurementTool msrTool;
   localExtrema.ResetPixelSize(); // Make sure the measurement tool uses pixels, not physical units.
   Measurement measurement;
   DIP_STACK_TRACE_THIS( measurement = msrTool.Measure( localExtrema, in, { "Center", "Size", "Mean" } ));

   // Allocate output
   dip::uint nExtrema = measurement.NumberOfObjects();
   SubpixelLocationArray out( nExtrema );

   // Find the version of `dip__SubpixelLocation` to call, depending on input data type.
   auto SubpixelLocationFunction = dip__SubpixelLocation< sfloat >; // any of them, for the sake of finding out the type.
   DIP_OVL_ASSIGN_REAL( SubpixelLocationFunction, dip__SubpixelLocation, in.DataType() );

   // For each extremum: find sub-pixel location and write to output
   FloatArray coords( nDims );
   auto objIterator = measurement.FirstObject();
   for( dip::uint ii = 0; ii < nExtrema; ++ii ) {
      auto it = objIterator[ "Center" ];
      std::copy( it.begin(), it.end(), coords.begin() );
      if(( method == SubpixelExtremumMethod::INTEGER ) || ( *objIterator[ "Size" ] > 1 )) {
         // The local extremum is a plateau (or we're not interested in sub-pixel locations)
         out[ ii ].coordinates = coords;
         out[ ii ].value = *objIterator[ "Mean" ];
      } else {
         UnsignedArray position( nDims );
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            position[ jj ] = static_cast< dip::uint >( round_cast( coords[ jj ] ));
         }
         out[ ii ] = SubpixelLocationFunction( in, position, method, invert );
      }
      ++objIterator;
   }

   // Done!
   return out;
}

} // namespace

SubpixelLocationArray SubpixelMaxima(
      Image const& in,
      Image const& mask,
      String const& method
) {
   return SubpixelExtrema( in, mask, method, false );
}

SubpixelLocationArray SubpixelMinima(
      Image const& in,
      Image const& mask,
      String const& method
) {
   return SubpixelExtrema( in, mask, method, true );
}

FloatArray MeanShift(
      Image const& meanShiftVectorResult,
      FloatArray const& start,
      dfloat epsilon
) {
   DIP_THROW_IF( !meanShiftVectorResult.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = meanShiftVectorResult.Dimensionality();
   DIP_THROW_IF( meanShiftVectorResult.TensorElements() != nDims, E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( !meanShiftVectorResult.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( start.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( epsilon <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   epsilon *= epsilon; // epsilon square
   auto interpFunc = PrepareResampleAtUnchecked( meanShiftVectorResult, S::CUBIC_ORDER_3 );
   auto pt = start;
   dfloat distance;
   std::cout << "pt = " << pt << ":\n";
   do {
      auto meanShift = static_cast< FloatArray >( ResampleAtUnchecked( meanShiftVectorResult, pt, interpFunc ));
      std::cout << "     " << meanShift << '\n';
      pt += meanShift;
      distance = meanShift.norm_square();
   } while( distance > epsilon );
   std::cout << '\n';
   return pt;
}

} // namespace dip
