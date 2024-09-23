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

#include "diplib/generation.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/geometry.h"
#include "diplib/linear.h"
#include "diplib/mapping.h"
#include "diplib/math.h"
#include "diplib/microscopy.h"
#include "diplib/random.h"
#include "diplib/statistics.h"
#include "diplib/transform.h"

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

class FTBoxLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override {
         return center_.size() * 20;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      FTBoxLineFilter( FloatArray const& center, FloatArray const& scale, dfloat amplitude )
            : center_( center ), scale_( scale ), amplitude_( amplitude ) {}
   private:
      FloatArray const& center_;
      FloatArray const& scale_;
      dfloat const amplitude_;
};

class FTCrossLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override {
         return center_.size() * 20;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      FTCrossLineFilter( FloatArray const& center, FloatArray const& scale, dfloat amplitude )
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

   switch( nDims ) {
      case 1:
         amplitude *= 2 * radius[ 0 ];
         break;
      case 2:
         amplitude *= pi * radius[ 0 ] * radius[ 1 ];
         break;
      case 3:
         amplitude *= 4.0 / 3.0 * pi * radius[ 0 ] * radius[ 1 ] * radius[ 2 ];
         break;
      default:
         DIP_THROW( E::DIMENSIONALITY_NOT_SUPPORTED );
   }

   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      radius[ ii ] = radius[ ii ] * 2 * pi / static_cast< dfloat >( out.Size( ii ) );
   }
   Image rr = EuclideanDistanceToPoint( out.Sizes(), out.GetCenter(), radius );

   bool protect = out.Protect( true );
   PixelSize ps = out.PixelSize();
   switch( nDims ) {
      case 1:
         Sinc( rr, out );
         out *= amplitude;
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
   out.SetPixelSize( std::move( ps ));
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

   FTBoxLineFilter scanLineFilter( center, length, amplitude );
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

   FTCrossLineFilter scanLineFilter( center, length, amplitude );
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

namespace {

void ToSpatial(
      Image& out,
      Image& outFT,
      bool& isFT
){
   if( isFT ) {
      if( outFT.IsForged() ) {
         DIP_STACK_TRACE_THIS( FourierTransform( outFT, out, { S::INVERSE, S::REAL } ));
      } else {
         DIP_STACK_TRACE_THIS( FourierTransform( out, out, { S::INVERSE, S::REAL } ));
      }
      isFT = false;
   }
}

}

void TestObject(
      Image& out,
      TestObjectParams const& params,
      Random& random
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( out.TensorElements() != 1, E::IMAGE_NOT_SCALAR );

   bool protect = out.Protect( true );

   // -- Stage 1: generate shape --

   DIP_THROW_IF( params.objectSizes.empty(), E::ARRAY_PARAMETER_EMPTY );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF(( params.objectSizes.size() != 1 ) && ( params.objectSizes.size() != nDims ), E::ARRAY_PARAMETER_WRONG_LENGTH );
   bool isFT{};
   DIP_STACK_TRACE_THIS( isFT = BooleanFromString( params.generationMethod, S::FOURIER, S::GAUSSIAN ));

   // Origin for "gauss" method
   FloatArray origin = out.GetCenter();
   if( !isFT && params.randomShift ) {
      UniformRandomGenerator rng( random );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin[ ii ] += rng( -0.5, 0.5 );
      }
   }

   // Generate shape
   if( params.objectShape == S::ELLIPSOID ) {
      if( isFT ) {
         auto sizes = params.objectSizes;
         sizes /= 2;
         DIP_STACK_TRACE_THIS( FTEllipsoid( out, sizes, params.objectAmplitude ));
      } else {
         for( dip::uint ii = 1; ii < params.objectSizes.size(); ++ii ) {
            DIP_THROW_IF( params.objectSizes[ ii ] != params.objectSizes[ 0 ], "The combination of \"ellipsoid\" and \"gaussian\" generation requires isotropic object sizes" );
         }
         out.Fill( 0 );
         DIP_STACK_TRACE_THIS( DrawBandlimitedBall( out, params.objectSizes[ 0 ], origin, { params.objectAmplitude }, S::FILLED, 0.9 ));
      }
   } else if( params.objectShape == S::ELLIPSOID_SHELL ) {
      if( isFT ) {
         auto sizes = params.objectSizes;
         sizes /= 2;
         DIP_STACK_TRACE_THIS( FTEllipsoid( out, sizes, params.objectAmplitude ));
         Image inner = out.Similar();
         sizes -= 2;
         DIP_STACK_TRACE_THIS( FTEllipsoid( inner, sizes, params.objectAmplitude ));
         out -= inner;
      } else {
         for( dip::uint ii = 1; ii < params.objectSizes.size(); ++ii ) {
            DIP_THROW_IF( params.objectSizes[ ii ] != params.objectSizes[ 0 ], "The combination of \"ellipsoid shell\" and \"gaussian\" generation requires isotropic object sizes" );
         }
         out.Fill( 0 );
         DIP_STACK_TRACE_THIS( DrawBandlimitedBall( out, params.objectSizes[ 0 ], origin, { params.objectAmplitude }, S::EMPTY, 0.9 ));
      }
   } else if( params.objectShape == S::BOX ) {
      if( isFT ) {
         auto sizes = params.objectSizes;
         sizes /= 2;
         DIP_STACK_TRACE_THIS( FTBox( out, sizes, params.objectAmplitude ));
      } else {
         out.Fill( 0 );
         DIP_STACK_TRACE_THIS( DrawBandlimitedBox( out, params.objectSizes, origin, { params.objectAmplitude }, S::FILLED, 0.9 ));
      }
   } else if( params.objectShape == S::BOX_SHELL ) {
      if( isFT ) {
         auto sizes = params.objectSizes;
         sizes /= 2;
         DIP_STACK_TRACE_THIS( FTBox( out, sizes, params.objectAmplitude ));
         Image inner = out.Similar();
         sizes -= 2;
         DIP_STACK_TRACE_THIS( FTBox( inner, sizes, params.objectAmplitude ));
         out -= inner;
      } else {
         out.Fill( 0 );
         DIP_STACK_TRACE_THIS( DrawBandlimitedBox( out, params.objectSizes, origin, { params.objectAmplitude }, S::EMPTY, 0.9 ));
      }
   } else if( params.objectShape == S::CUSTOM ) {
      // Don't do anything, `out` already contains the shape
      // TODO: call Shift if params.randomShift and !isFT
   } else {
      DIP_THROW_INVALID_FLAG( params.objectShape );
   }

   // Random shift to origin for "ft" method
   Image outFT;
   if( isFT && params.randomShift ) {
      UniformRandomGenerator rng( random );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin[ ii ] = rng( -0.5, 0.5 );
      }
      DIP_STACK_TRACE_THIS( ShiftFT( out, outFT, origin ));
   }

   // -- Stage 2: modulate --

   if( params.modulationDepth != 0 ) {

      DIP_THROW_IF( params.modulationFrequency.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );

      // We modulate in the spatial domain
      ToSpatial( out, outFT, isFT );

      // Modulate along one dimension at the time
      dfloat amplitude = params.modulationDepth / static_cast< dfloat >( params.modulationFrequency.count() );
      auto m = MaximumAndMinimum( out );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         // TODO: This is an inefficient way of computing the modulation, but I presume this is not going to be used a whole lot.
         //       That this is related to ShiftFT, and in the original code both were handled using the same functions.
         if( params.modulationFrequency[ ii ] != 0 ) {
            Image tmp = CreateRamp( out.Sizes(), ii );
            tmp *= 2.0 * pi * params.modulationFrequency[ ii ];
            Cos( tmp, tmp );
            m = MaximumAndMinimum( tmp );
            LinearCombination( out, tmp, out, 1.0, amplitude );
            m = MaximumAndMinimum( out );
         }
      }

   }

   // -- Stage 3: blur --

   if( params.pointSpreadFunction != S::NONE ) {
      DIP_THROW_IF( params.oversampling <= 0, E::INVALID_PARAMETER );

      if( params.pointSpreadFunction == S::GAUSSIAN ) {

         ToSpatial( out, outFT, isFT );
         DIP_STACK_TRACE_THIS( GaussFIR( out, out, { 0.9 * params.oversampling } ));
         // TODO: If isFT, we can do GaussFT().

      } else if( params.pointSpreadFunction == S::INCOHERENT ) {

         if( !isFT ) {
            DIP_STACK_TRACE_THIS( FourierTransform( out, outFT ));
            isFT = true; // to mark we're in the frequency domain now
         }
         Image otf = out.Similar( DT_SFLOAT );
         DIP_STACK_TRACE_THIS( IncoherentOTF( otf, 0.0, params.oversampling, 1.0 ));
         if( outFT.IsForged() ) {
            outFT *= otf;
         } else {
            out *= otf;
         }

      } else {
         DIP_THROW_INVALID_FLAG( params.pointSpreadFunction );
      }
   }

   // We're done with the frequency domain now, transform to spatial domain if we're not there yet
   ToSpatial( out, outFT, isFT );

   // -- Stage 4: noise --

   DIP_THROW_IF( params.backgroundValue < 0.0, E::PARAMETER_OUT_OF_RANGE );

   if( params.signalNoiseRatio > 0 ) {
      DIP_THROW_IF( params.gaussianNoise < 0.0, E::INVALID_PARAMETER );
      DIP_THROW_IF( params.poissonNoise < 0.0, E::PARAMETER_OUT_OF_RANGE );

      // Determine image energy and mean intensity
      dfloat objEnergy = MeanSquareModulus( out ).As< dfloat >();

      // Add background
      if( params.backgroundValue != 0.0 ) {
         out += params.backgroundValue;
      }

      // Add Poisson Noise
      if( params.poissonNoise != 0.0 ) {
         dfloat pn = params.signalNoiseRatio / params.poissonNoise * ( params.gaussianNoise + params.poissonNoise );
         DIP_STACK_TRACE_THIS( ClipLow( out, out, 0.0 ));
         dfloat objIntensity = MeanModulus( out ).As< dfloat >();
         dfloat cnv = pn * objIntensity / objEnergy;
         DIP_STACK_TRACE_THIS( PoissonNoise( out, out, random, cnv ));
      }

      // Add Gaussian Noise
      if( params.gaussianNoise != 0.0 ) {
         dfloat gn = params.signalNoiseRatio / params.gaussianNoise * ( params.gaussianNoise + params.poissonNoise );
         dfloat var = objEnergy / gn;
         DIP_STACK_TRACE_THIS( GaussianNoise( out, out, random, var ));
      }

   } else {
      // We're not adding noise, but we still want to add the background
      if( params.backgroundValue != 0.0 ) {
         out += params.backgroundValue;
      }
   }

   out.Protect( protect );
}

} // namespace
