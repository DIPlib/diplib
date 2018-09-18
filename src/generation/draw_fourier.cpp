/*
 * DIPlib 3.0
 * This file contains definitions for image drawing functions
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
#include "diplib/generation.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

void SetCenter( Image& out, dfloat amplitude ) {
   auto center = out.Sizes();
   center /= 2;
   out.At( center ) = amplitude;
}

dfloat ModifiedSinc( dfloat rr, dfloat scale, dfloat center ) {
   if( rr == 0 ) {
      return scale;
   }
   rr *= pi / center;
   return std::sin( rr * scale ) / rr;
}

class dip__FTBox : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return center_.size() * 20;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         dip::uint nDims = params.position.size();
         DIP_ASSERT( center_.size() == nDims );
         dfloat res = amplitude_;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != dim ) {
               res *= ModifiedSinc( static_cast< dfloat >( params.position[ ii ] ) - center_[ ii ], scale_[ ii ], center_[ ii ] );
            }
         }
         dfloat pp = static_cast< dfloat >( params.position[ dim ] ) - center_[ dim ];
         for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp, out += stride ) {
            *out = res * ModifiedSinc( pp, scale_[ dim ], center_[ dim ] );
         }
      }
      dip__FTBox( FloatArray const& center, FloatArray const& scale, dfloat amplitude )
            : center_( center ), scale_( scale ), amplitude_( amplitude ) {}
   private:
      FloatArray const& center_;
      FloatArray const& scale_;
      dfloat const amplitude_;
};

class dip__FTCross : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return center_.size() * 20;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         dip::uint nDims = params.position.size();
         DIP_ASSERT( center_.size() == nDims );
         dfloat res = 0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != dim ) {
               res += ModifiedSinc( static_cast< dfloat >( params.position[ ii ] ) - center_[ ii ], scale_[ ii ], center_[ ii ] );
            }
         }
         dfloat pp = static_cast< dfloat >( params.position[ dim ] ) - center_[ dim ];
         for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp, out += stride ) {
            *out = amplitude_ * ( res + ModifiedSinc( pp, scale_[ dim ], center_[ dim ] ));
         }
      }
      dip__FTCross( FloatArray const& center, FloatArray const& scale, dfloat amplitude )
            : center_( center ), scale_( scale ), amplitude_( amplitude ) {}
   private:
      FloatArray const& center_;
      FloatArray const& scale_;
      dfloat const amplitude_;
};

} // namespace

void FTEllipsoid(
      Image& out,
      FloatArray radius,
      dfloat amplitude
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( out.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( amplitude <= 0.0, E::INVALID_PARAMETER );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( !radius.empty() && ( radius <= 0.0 ).any(), E::INVALID_PARAMETER );
   DIP_STACK_TRACE_THIS( ArrayUseParameter( radius, nDims, 1.0 ));

   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      radius[ ii ] = radius[ ii ] * 2 * pi / static_cast< dfloat >( out.Size( ii ) );
   }
   Image rr = EuclideanDistanceToPoint( out.Sizes(), out.GetCenter(), radius );

   bool protect = out.Protect( true );
   PixelSize ps = out.PixelSize();
   switch( nDims ) {
      case 1:
         Sinc( rr, out );
         break;
      case 2:
         // 2 * amplitude * BesselJ1( rr ) / rr
         BesselJ1( rr, out );
         out /= rr;
         out *= 2 * amplitude;
         SetCenter( out, amplitude ); // We've divided by 0 at the origin.
         break;
      case 3:
      {
         // ( amplitude * 3 ) * ( Sin( rr ) - rr * Cos( rr )) / ( rr * rr * rr )
         Image divisor = Power( rr, -3 );
         Image rCosr = Cos( rr );
         rCosr *= rr;
         Sin( rr, out );
         out -= rCosr;
         out *= divisor;
         Multiply( out, amplitude * 3, out );
         SetCenter( out, amplitude ); // We've divided by 0 at the origin.
         break;
      }
      default:
         DIP_THROW( E::DIMENSIONALITY_NOT_SUPPORTED );
   }

   out.Protect( protect );
   out.SetPixelSize( ps );
}

void FTBox(
      Image& out,
      FloatArray length,
      dfloat amplitude
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( out.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( amplitude <= 0.0, E::INVALID_PARAMETER );
   dip::uint nDims = out.Dimensionality();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( length, nDims, 1.0 ));
   DIP_THROW_IF(( length <= 0.0 ).any(), E::INVALID_PARAMETER );

   FloatArray center = out.GetCenter();
   for( auto& c : center ) {
      c = std::max( c, 1.0 );
   }
   amplitude *= std::pow( 2.0, nDims );

   dip__FTBox scanLineFilter( center, length, amplitude );
   Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
}

void FTCross(
      Image& out,
      FloatArray length,
      dfloat amplitude
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( out.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( amplitude <= 0.0, E::INVALID_PARAMETER );
   dip::uint nDims = out.Dimensionality();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( length, nDims, 1.0 ));
   DIP_THROW_IF(( length <= 0.0 ).any(), E::INVALID_PARAMETER );

   FloatArray center = out.GetCenter();
   for( auto& c : center ) {
      c = std::max( c, 1.0 );
   }
   amplitude *= 2.0;

   dip__FTCross scanLineFilter( center, length, amplitude );
   Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
}

void FTGaussian(
      Image& out,
      FloatArray sigma,
      dfloat amplitude,
      dfloat truncation
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( out.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( amplitude <= 0.0, E::INVALID_PARAMETER );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( sigma.empty(), E::ARRAY_PARAMETER_EMPTY );
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sigma, nDims, 0.0 )); // default value never used
   DIP_THROW_IF(( sigma <= 0.0 ).any(), E::INVALID_PARAMETER );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      sigma[ ii ] = static_cast< dfloat >( out.Size( ii )) / ( sigma[ ii ] * pi );
      amplitude *= std::sqrt( 2.0 * pi ) * sigma[ ii ];
   }
   out.Fill( 0 );
   DrawBandlimitedPoint( out, out.GetCenter(), { amplitude }, sigma, truncation );
}

} // namespace
