/*
 * DIPlib 3.0
 * This file contains definitions for distance transforms
 *
 * (c)2018, Erik Wernersson and Cris Luengo.
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
#include "diplib/distance.h"
#include "diplib/framework.h"

#include "separable_dt.h"

namespace dip {

namespace {

template< typename TPI >
class DistanceTransformLineFilter : public Framework::SeparableLineFilter {
   public:
      // NOTE! This filter needs input and output buffers to be distinct only for the brute-force version (filterLength <= 3)
      DistanceTransformLineFilter( FloatArray const& spacing, dfloat maxDistance2, bool squareDistance )
            : spacing_( spacing ), maxDistance2_( static_cast< TPI >( maxDistance2 )), squareDistance_( squareDistance ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         if( spacing_.size() > 1 ) {
            buffers_.resize( threads ); // We don't need buffers for 1D thing.
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint procDim ) override {
         return lineLength * ( procDim == 0 ? 6 : 20 ); // TODO: how to estimate this one? It's not exactly linear...
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         DIP_ASSERT( params.inBuffer.stride == 1 ); // Guaranteed because we use an input buffer
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         const TPI spacing = static_cast< TPI >( spacing_[ params.dimension ] );
         const TPI spacing2 = spacing * spacing;
         dip::uint padding = params.inBuffer.border;  // inBuffer.border is the size of the border padding.
         bool border = padding == 0;                  // The border padding is either 0 or 1 pixel. If it's zero pixels,
         //                                              then the `border` boolean input argument was true (object).
         //                                              If it's one pixel, then the `border` argument was false (background).

         // If only one pass, avoid sqrt
         if(( params.nPasses == 1 ) && ( !squareDistance_ )) {

            // 1: Forward
            TPI d = border ? maxDistance2_ : 0;
            for( dip::uint ii = 0; ii < length; ++ii ) {
               d = ( *in == 0 ) ? 0 : d + spacing;
               *out = d;
               ++in;
               out += outStride;
            }

            // 2: Backward
            d = border ? maxDistance2_ : 0;
            for( dip::uint ii = length; ii-- > 0; ) {
               --in;
               out -= outStride;
               d = ( *in == 0 ) ? 0 : d + spacing;
               if( d < *out ) {
                  *out = d;
               }
            }
            return;
         }

         // Otherwise, accumulate square distances and take the sqrt after the last pass through the image
         if( params.pass == 0 ) { // First pass

            // 1: Forward
            TPI d = border ? maxDistance2_ : 0;
            for( dip::uint ii = 0; ii < length; ++ii ) {
               d = ( *in == 0 ) ? 0 : d + spacing;
               *out = d * d;
               ++in;
               out += outStride;
            }

            // 2: Backward
            d = border ? maxDistance2_ : 0;
            for( dip::uint ii = length; ii-- > 0; ) {
               --in;
               out -= outStride;
               d = ( *in == 0 ) ? 0 : d + spacing;
               if( d * d < *out ) {
                  *out = d * d;
               }
            }

         } else { // Subsequent passes

            // Do we have padding?
            dip::uint paddedLength = length + 2 * padding;
            in -= padding;

            // Buffers
            auto& buffer = buffers_[ params.thread ];
            buffer.resize( 2 * paddedLength ); // does nothing if already correct size
            dip::sint* S = buffer.data();
            dip::sint* T = S + paddedLength;

            dip::sint len = static_cast< dip::sint >( paddedLength );

            // 3: Forward
            dip::sint q = 0;
            S[ 0 ] = 0;
            T[ 0 ] = 0;
            for( dip::sint u = 1; u < len; ++u ) {
               // f(t[q],s[q]) > f(t[q], u)
               // f(x,i) = (x-i)^2 + g(i)^2
               while( q >= 0 ) {
                  TPI d1 = static_cast< TPI >( T[ q ] - S[ q ] );
                  TPI d2 = static_cast< TPI >( T[ q ] - u );
                  if(( spacing2 * d1 * d1 + in[ S[ q ]] ) < ( spacing2 * d2 * d2 + in[ u ] )) {
                     break;
                  }
                  --q;
               }

               if( q < 0 ) {
                  q = 0;
                  S[ 0 ] = u;
                  T[ 0 ] = 0;
               } else {
                  // w = 1 + Sep(s[q],u)
                  // Sep(i,u) = (u^2-i^2 +g(u)^2-g(i)^2) div (2(u-i))
                  // where division is rounded off towards zero
                  TPI d1 = static_cast< TPI >( u );
                  TPI d2 = static_cast< TPI >( S[ q ] );
                  dip::sint w = 1 + static_cast< dip::sint >( std::trunc(
                        ( spacing2 * d1 * d1 - spacing2 * d2 * d2 + in[ u ] - in[ S[ q ]] ) / ( spacing2 * 2 * ( d1 - d2 ))));
                  if( w < len ) {
                     ++q;
                     S[ q ] = u; // The point where the segment is a minimizer
                     T[ q ] = w; // The first pixel of the segment
                  }
               }
            }

            // 4: Backward
            dip::sint end = 0;
            if( padding > 0 ) { // This means that padding==1.
               --len;
               if( len == T[ q ] ) {
                  --q;
               }
               end = 1;
               out -= outStride;
            }
            for( dip::sint u = len; u-- > end; ) {
               //dt[u,y]:=f(u,s[q])
               TPI d1 = static_cast< TPI >( u - S[ q ] );
               out[ u * outStride ] = spacing2 * d1 * d1 + in[ S[ q ]];
               if( u == T[ q ] ) {
                  --q;
               }
            }

            // Final: square root
            if( !squareDistance_ && ( params.pass == params.nPasses - 1 )) {
               out = static_cast< TPI* >( params.outBuffer.buffer );
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  *out = std::sqrt( *out );
                  out += outStride;
               }
            }

         }
      }
   private:
      FloatArray const& spacing_;
      std::vector< std::vector< dip::sint >> buffers_; // one for each thread
      TPI const maxDistance2_; // maximum distance. Somehow, using dip::inf does not work here.
      bool squareDistance_;
};

} // namespace

// Implements `dip::EuclideanDistanceTransform(...,"separable")`
void SeparableDistanceTransform(
      Image const& in,
      Image& out,
      FloatArray const& spacing,
      bool border,
      bool squareDistance
) {
   dfloat maxDistance2 = 1;
   for( dip::uint ii = 0; ii < in.Dimensionality(); ++ii ) {
      dfloat d = static_cast< dfloat >( in.Size( ii )) * spacing[ ii ];
      maxDistance2 += d * d;
   }
   DistanceTransformLineFilter< sfloat > lineFilter( spacing, maxDistance2, squareDistance );
   if( border ) {
      DIP_STACK_TRACE_THIS( Framework::Separable( in, out, DT_SFLOAT, DT_SFLOAT,
            {}, {}, {}, lineFilter, Framework::SeparableOption::UseInputBuffer ));
   } else {
      DIP_STACK_TRACE_THIS( Framework::Separable( in, out, DT_SFLOAT, DT_SFLOAT,
            {}, { 1 }, { BoundaryCondition::ADD_ZEROS }, lineFilter, Framework::SeparableOption::UseInputBuffer ));
   }
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/generation.h"

DOCTEST_TEST_CASE("[DIPlib] testing the distance transform") {
   // 1D case
   {
      dip::Image gt{ dip::UnsignedArray{ 51 }, 1, dip::DT_SFLOAT };
      dip::FillRadiusCoordinate( gt );
      dip::Image in = gt != 0;
      dip::Image out;
      // border = "object"
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::SEPARABLE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::SQUARE );
      dip::Sqrt( out, out );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      // border = "background"
      dip::Infimum( gt, ( gt.Size( 0 ) / 2 + 1 ) - gt, gt );
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::SEPARABLE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::SQUARE );
      dip::Sqrt( out, out );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
   }
   // 2D case
   {
      dip::Image gt{ dip::UnsignedArray{ 31, 41 }, 1, dip::DT_SFLOAT };
      dip::FillRadiusCoordinate( gt );
      dip::Image in = gt != 0;
      dip::Image out;
      // border = "object"
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::SEPARABLE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::SQUARE );
      dip::Sqrt( out, out );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::TRUE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::TIES );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::FAST );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
      // border = "background"
      dip::Infimum( gt, ( gt.Size( 0 ) / 2 + 1 ) - dip::Abs( dip::CreateXCoordinate( gt.Sizes() )), gt );
      dip::Infimum( gt, ( gt.Size( 1 ) / 2 + 1 ) - dip::Abs( dip::CreateYCoordinate( gt.Sizes() )), gt );
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::SEPARABLE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::SQUARE );
      dip::Sqrt( out, out );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::TRUE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::TIES );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::FAST );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
      // We could be checking against the "brute force" method too, but it doesn't do the "background"
   }
   // 3D case
   {
      dip::Image gt{ dip::UnsignedArray{ 31, 21, 11 }, 1, dip::DT_SFLOAT };
      dip::FillRadiusCoordinate( gt );
      dip::Image in = gt != 0;
      dip::Image out;
      // border = "object"
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::SEPARABLE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::SQUARE );
      dip::Sqrt( out, out );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::TRUE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::TIES );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
      dip::EuclideanDistanceTransform( in, out, dip::S::OBJECT, dip::S::FAST );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
      // border = "background"
      dip::Infimum( gt, ( gt.Size( 0 ) / 2 + 1 ) - dip::Abs( dip::CreateXCoordinate( gt.Sizes() )), gt );
      dip::Infimum( gt, ( gt.Size( 1 ) / 2 + 1 ) - dip::Abs( dip::CreateYCoordinate( gt.Sizes() )), gt );
      dip::Infimum( gt, ( gt.Size( 2 ) / 2 + 1 ) - dip::Abs( dip::CreateZCoordinate( gt.Sizes() )), gt );
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::SEPARABLE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::SQUARE );
      dip::Sqrt( out, out );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::TRUE );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) == doctest::Approx( 0.0 ));
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::TIES );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
      dip::EuclideanDistanceTransform( in, out, dip::S::BACKGROUND, dip::S::FAST );
      DOCTEST_CHECK( dip::MaximumAbsoluteError( gt, out ) < 0.25 );
   }
}

#endif // DIP__ENABLE_DOCTEST
