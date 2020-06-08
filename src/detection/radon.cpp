/*
 * DIPlib 3.0
 * This file contains definitions of various Hough Transform functions.
 *
 * (c)2018-2019, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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
#include "diplib/detection.h"
#include "diplib/generation.h"
#include "diplib/transform.h"
#include "diplib/mapping.h"
#include "diplib/math.h"
#include "diplib/morphology.h"
#include "diplib/measurement.h"
#include "diplib/generic_iterators.h"

namespace dip {

namespace {

enum class RadonTransformCirclesMode { full, projection, subpixelProjection };

enum class RadonTransformCirclesOption { normalize, correct, hollow, filled, detectMaxima, saveParamSpace };
DIP_DECLARE_OPTIONS( RadonTransformCirclesOption, RadonTransformCirclesOptions )

void CreateSphere(
      Image& sphere, // Already forged, should be of type SFLOAT or DFLOAT
      dfloat radius,
      dfloat sigma,
      RadonTransformCirclesOptions options
) {
   dip::uint nDims = sphere.Dimensionality();
   if( options.Contains( RadonTransformCirclesOption::correct )) {
      dfloat tmp = 0.25 * radius * radius - static_cast< dfloat >( nDims - 1 ) * sigma * sigma;
      tmp = std::max( tmp, 0.0 ); // prevent square root of negative value
      radius = 0.5 * radius + std::sqrt( tmp );
   }
   dfloat sphereValue = std::sqrt( 2.0 * pi ) * sigma;
   dfloat innerRadius = radius - 3.0 * sigma;
   dfloat innerValue = options.Contains( RadonTransformCirclesOption::hollow ) ? sphereValue : 1.0;
   if( options.Contains( RadonTransformCirclesOption::normalize )) {
      sphereValue /= HypersphereSurface( nDims, radius );
      if( options.Contains( RadonTransformCirclesOption::filled )) {
         innerValue /= std::max( 1.0, HypersphereVolume( nDims, innerRadius )); // TODO!
      } else if( options.Contains( RadonTransformCirclesOption::hollow )) {
         innerValue /= std::max( 1.0, HypersphereSurface( nDims, innerRadius ));
      }
   }
   auto sz = static_cast< dip::uint >( 1 + 2 * ( std::ceil( radius ) + std::ceil( 3 * sigma )));
   UnsignedArray sizes( nDims, sz );
   sphere.Fill( 0 );
   FloatArray origin = sphere.GetCenter();
   DrawBandlimitedBall( sphere, 2 * radius, origin, { sphereValue }, S::EMPTY, sigma );
   if( innerRadius >= 1 ) {
      if( options.Contains( RadonTransformCirclesOption::filled )) {
         DrawBandlimitedBall( sphere, 2 * innerRadius, origin, { -innerValue }, S::FILLED, sigma );
      } else if( options.Contains( RadonTransformCirclesOption::hollow )) {
         DrawBandlimitedBall( sphere, 2 * innerRadius, origin, { -innerValue }, S::EMPTY, sigma );
      }
   }
}

void ComputeParameterSpaceSlice(
      Image const& inFT,
      Image& sphere,       // Storage to be re-used (needs to be forged and of the right sizes)
      Image& sphereFT,     // Storage to be re-used
      Image& dest,
      dfloat radius,
      dfloat sigma,
      RadonTransformCirclesOptions options
) {
   CreateSphere( sphere, radius, sigma, options );
   FourierTransform( sphere, sphereFT );
   sphereFT *= inFT;
   FourierTransform( sphereFT, dest, { S::INVERSE, S::REAL } );
}

void ComputeFullParameterSpace(
      Image const& inFT,
      Image& paramSpace,
      Range const& radii,
      dfloat sigma,
      RadonTransformCirclesOptions options
) {
   UnsignedArray outSize = inFT.Sizes();
   Image sphere( outSize, 1, DT_SFLOAT );
   Image sphereFT;
   outSize.push_back( radii.Size() );
   paramSpace.ReForge( outSize, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   ImageSliceIterator dest( paramSpace, outSize.size() - 1 );
   for( auto radius = radii.begin(); radius != radii.end(); ++radius, ++dest ) {
      ComputeParameterSpaceSlice( inFT, sphere, sphereFT, *dest, static_cast< dfloat >( *radius ), sigma, options );
   }
   ClipLow( paramSpace, paramSpace, 0 );
}

void ComputeProjectedParameterSpace(
      Image const& inFT,
      Image& paramSpace,
      Range const& radii,
      dfloat sigma,
      RadonTransformCirclesOptions options
) {
   UnsignedArray const& outSize = inFT.Sizes();
   Image sphere( outSize, 1, DT_SFLOAT );
   Image sphereFT;
   paramSpace.ReForge( outSize, 2, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   paramSpace.Fill( 0 );
   Image max = paramSpace[ 0 ];
   max.Protect();
   Image argmax = paramSpace[ 1 ];
   argmax.Protect();
   Image tmp, mask;
   for( auto radius : radii ) {
      ComputeParameterSpaceSlice( inFT, sphere, sphereFT, tmp, static_cast< dfloat >( radius ), sigma, options );
      Greater( tmp, max, mask );
      Select( tmp, max, mask, max );
      argmax.At( std::move( mask )) = radius;
   }
}

void UpdateMaxima(
      Image& max,
      Image& argmax,
      Image const& tmp1,
      Image const& tmp2,
      Image const& tmp3,
      dfloat radius // the radius at tmp2
) {
   DIP_ASSERT( max.DataType() == DT_SFLOAT );
   DIP_ASSERT( argmax.DataType() == DT_SFLOAT );
   DIP_ASSERT( tmp1.DataType() == DT_SFLOAT );
   DIP_ASSERT( tmp2.DataType() == DT_SFLOAT );
   DIP_ASSERT( tmp3.DataType() == DT_SFLOAT );
   JointImageIterator< sfloat, sfloat, sfloat, sfloat, sfloat > it( { tmp1, tmp2, tmp3, max, argmax } );
   it.Optimize();
   do {
      dfloat v0 = it.Sample< 0 >();
      dfloat v1 = it.Sample< 1 >();
      dfloat v2 = it.Sample< 2 >();
      if(( v1 > v2 ) && ( v1 > v0 )) {
         dfloat m = v0 - 2 * v1 + v2;
         dfloat d = v0 - v2;
         dfloat v = v1 - d * d / ( 8 * m );
         v = std::max( v, v1 );
         if( it.Sample< 3 >() < v ) {
            dfloat r = radius + d / ( 2 * m );
            it.Sample< 3 >() = static_cast< sfloat >( v );
            it.Sample< 4 >() = static_cast< sfloat >( r );
         }
      }
   } while( ++it );
}

void ComputeProjectedParameterSpace_SubPixel(
      Image const& inFT,
      Image& paramSpace,
      Range const& radii,  // We've already made sure this has at least 3 elements.
      dfloat sigma,
      RadonTransformCirclesOptions options
) {
   DIP_ASSERT( radii.Size() >= 3 );
   UnsignedArray const& outSize = inFT.Sizes();
   Image sphere( outSize, 1, DT_SFLOAT );
   Image sphereFT;
   paramSpace.ReForge( outSize, 2, DT_SFLOAT, Option::AcceptDataTypeChange::DONT_ALLOW );
   paramSpace.Fill( 0 );
   Image max = paramSpace[ 0 ];
   max.Protect();
   Image argmax = paramSpace[ 1 ];
   argmax.Protect();
   Image tmp1, tmp2, tmp3;
   auto radius = radii.begin();
   ComputeParameterSpaceSlice( inFT, sphere, sphereFT, tmp1, static_cast< dfloat >( *radius ), sigma, options );
   ++radius;
   ComputeParameterSpaceSlice( inFT, sphere, sphereFT, tmp2, static_cast< dfloat >( *radius ), sigma, options );
   ++radius;
   dfloat step = static_cast< dfloat >( radii.Step() );
   for( ; radius != radii.end(); ++radius ) {
      dfloat r = static_cast< dfloat >( *radius );
      ComputeParameterSpaceSlice( inFT, sphere, sphereFT, tmp3, r, sigma, options );
      DIP_ASSERT( tmp3.DataType() == DT_SFLOAT );
      r -= step;
      UpdateMaxima( max, argmax, tmp1, tmp2, tmp3, r );
      tmp1 = std::move( tmp2 );
      tmp2 = std::move( tmp3 );
   }
}

// Much like `SubpixelExtrema`, but for maxima only, using `WatershedMaxima` instead of `Maxima`,
// with a fixed choice of separable parabolic fit, and for DT_SFLOAT images only.
RadonCircleParametersArray RadonCircleSubpixelMaxima(
      Image const& in,
      double threshold
) {
   // Check input
   DIP_ASSERT( in.IsForged() );
   DIP_ASSERT( in.IsScalar() );
   DIP_ASSERT( in.DataType() == DT_SFLOAT );
   dip::uint nDims = in.Dimensionality();

   // Find local maxima
   Image localMaxima;
   DIP_STACK_TRACE_THIS( localMaxima = WatershedMaxima( in, {}, 1, threshold, 0, S::LABELS ));
   // But not on the edge of the image
   SetBorder( localMaxima );

   // Get CoG of local maxima
   MeasurementTool msrTool;
   localMaxima.ResetPixelSize(); // Make sure the measurement tool uses pixels, not physical units.
   Measurement measurement;
   DIP_STACK_TRACE_THIS( measurement = msrTool.Measure( localMaxima, in, { "Center", "Size" } ));
   if( !measurement.IsForged() ) {
      return {};
   }

   // Allocate output
   RadonCircleParametersArray out( measurement.NumberOfObjects() );

   // For each extremum: find sub-pixel location and write to output
   FloatArray coords( nDims );
   auto objIterator = measurement.FirstObject();
   for( dip::uint ii = 0; ii < out.size(); ++ii, ++objIterator ) {
      auto it = objIterator[ "Center" ];
      std::copy( it.begin(), it.end(), coords.begin() );
      if( *objIterator[ "Size" ] > 1 ) {
         // The local extremum is a plateau
         out[ ii ].origin = coords;
      } else {
         UnsignedArray position( nDims );
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            position[ jj ] = static_cast< dip::uint >( round_cast( coords[ jj ] ));
         }
         sfloat* ptr = static_cast< sfloat* >( in.Pointer( position ));
         out[ ii ].origin.resize( nDims, 0.0 );
         for( dip::uint kk = 0; kk < nDims; ++kk ) {
            dfloat t[ 3 ];
            t[ 0 ] = static_cast< dfloat >( *( ptr - in.Stride( kk )));
            t[ 1 ] = static_cast< dfloat >( *( ptr ));
            t[ 2 ] = static_cast< dfloat >( *( ptr + in.Stride( kk )));
            out[ ii ].origin[ kk ] = static_cast< dfloat >( position[ kk ] );
            dfloat m = t[ 0 ] - 2 * t[ 1 ] + t[ 2 ];
            if( m != 0 ) {
               out[ ii ].origin[ kk ] += ( t[ 0 ] - t[ 2 ] ) / ( 2 * m );
            }
         }
      }
   }

   // Done!
   return out;
}

} // namespace

RadonCircleParametersArray RadonTransformCircles(
      Image const& in,
      Image& out,
      Range radii,
      dfloat sigma,
      dfloat threshold,
      String const& s_mode,
      StringSet const& s_options
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( sigma < 0.8, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( radii.start < static_cast< dip::sint >( std::ceil( 3 * sigma )) - 1, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( radii.stop > static_cast< dip::sint >( in.Sizes().minimum_value() ), E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( radii.step < 1, E::PARAMETER_OUT_OF_RANGE );

   // Modes
   RadonTransformCirclesMode mode;
   if( s_mode == S::FULL ) {
      mode = RadonTransformCirclesMode::full;
   } else if( s_mode == S::PROJECTION ) {
      mode = RadonTransformCirclesMode::projection;
   } else if( s_mode == S::SUBPIXEL_PROJECTION ) {
      mode = RadonTransformCirclesMode::subpixelProjection;
   } else {
      DIP_THROW_INVALID_FLAG( s_mode );
   }

   // Options
   RadonTransformCirclesOptions options = RadonTransformCirclesOption::detectMaxima + RadonTransformCirclesOption::saveParamSpace;
   for( auto& s : s_options ) {
      if( s == S::NORMALIZE ) {
         options += RadonTransformCirclesOption::normalize;
      } else if( s == S::CORRECT ) {
         options += RadonTransformCirclesOption::correct;
      } else if( s == S::HOLLOW ) {
         options += RadonTransformCirclesOption::hollow;
      } else if( s == S::FILLED ) {
         options += RadonTransformCirclesOption::filled;
      } else if( s == S::NO_MAXIMA_DETECTION ) {
         options -= RadonTransformCirclesOption::detectMaxima;
      } else if( s == S::NO_PARAMETER_SPACE ) {
         options -= RadonTransformCirclesOption::saveParamSpace;
      } else {
         DIP_THROW_INVALID_FLAG( s );
      }
   }
   DIP_THROW_IF( !options.Contains( RadonTransformCirclesOption::saveParamSpace ) &&
                 !options.Contains( RadonTransformCirclesOption::detectMaxima ),
                 "Both \"no maxima detection\" and \"no parameter space\" options were given -- nothing to do" );
   if( !options.Contains( RadonTransformCirclesOption::normalize )) {
      options -= RadonTransformCirclesOption::correct; // Never correct if we don't normalize
   }

   // Prepare
   Image inFT = FourierTransform( in ); // TODO: We could try using the "fast" option, leading to a slightly larger parameter space.
   Image tmp_paramSpace;
   Image& parameterSpace = options.Contains( RadonTransformCirclesOption::saveParamSpace ) ? out : tmp_paramSpace;
   RadonCircleParametersArray out_params;

   // Compute parameter space
   if(( mode == RadonTransformCirclesMode::subpixelProjection ) && ( radii.Size() < 3 )) {
      mode = RadonTransformCirclesMode::projection;
   }
   switch( mode ) {
      case RadonTransformCirclesMode::full:
         //if ( options.Contains( RadonTransformCirclesOption::saveParamSpace ) || ( radii.Size() < 3 )) {
            DIP_STACK_TRACE_THIS( ComputeFullParameterSpace( inFT, parameterSpace, radii, sigma, options ));
         //} else {
            // TODO: Compute `parameterSpace` in chunks and fill `out_params`
            //       This will change the maxima found, as we use the watershed method, which will be affected
            //return out_params;
         //}
         break;
      case RadonTransformCirclesMode::projection:
         DIP_STACK_TRACE_THIS( ComputeProjectedParameterSpace( inFT, parameterSpace, radii, sigma, options ));
         break;
      case RadonTransformCirclesMode::subpixelProjection:
         DIP_STACK_TRACE_THIS( ComputeProjectedParameterSpace_SubPixel( inFT, parameterSpace, radii, sigma, options ));
         break;
   }

   // Find circle parameters
   if( options.Contains( RadonTransformCirclesOption::detectMaxima )) {
      DIP_STACK_TRACE_THIS( out_params = RadonCircleSubpixelMaxima( parameterSpace[ 0 ], threshold ));
      dfloat radiusStep = static_cast< dfloat >( radii.Step() );
      if( parameterSpace.IsScalar() ) {
         // Full parameter space
         for( auto& p : out_params ) {
            p.radius = p.origin.back() * radiusStep + static_cast< dfloat >( radii.start );
            p.origin.pop_back();
         }
      } else {
         // Max and ArgMax projection of parameter space along radius axis
         Image radImg = parameterSpace[ 1 ];
         DIP_ASSERT( radImg.DataType() == DT_SFLOAT );
         sfloat const* radPtr = static_cast< sfloat const* >( radImg.Origin() );
         for( auto& p : out_params ) {
            dip::sint offset = 0;
            for( dip::uint ii = 0; ii < p.origin.size(); ++ii ) {
               offset += static_cast< dip::sint >( std::round( p.origin[ ii ] )) * radImg.Stride( ii );
            }
            p.radius = radPtr[ offset ];  // We don't interpolate here, I don't think that is a good idea.
         }
      }
   }
   return out_params;
}

} // namespace
