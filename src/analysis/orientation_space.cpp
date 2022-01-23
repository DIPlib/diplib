/*
 * (c)2019, Cris Luengo.
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
#include "diplib/transform.h"
#include "diplib/math.h"
#include "diplib/generation.h"
#include "diplib/generic_iterators.h"
#include "diplib/framework.h"

namespace dip {

void OrientationSpace(
      Image const& in,
      Image& out,
      dip::uint order,
      dfloat radCenter,
      dfloat radSigma,
      dip::uint orientations
) {
   // Test input image
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );

   // Fix parameters
   dip::uint slices = orientations == 0 ? order * 2 + 1 : orientations;
   radCenter *= 2.0 * pi;
   radSigma *= radCenter;
   dfloat dPhi = pi / static_cast< dfloat >( slices );

   // Fourier transform of input image
   Image ftIn = FourierTransform( in );
   PixelSize pixelSize = in.PixelSize();
   pixelSize.Set( 2, dPhi * Units::Radian() );

   // Forge output image (we can overwrite in at this point)
   UnsignedArray outSizes = in.Sizes();
   outSizes.push_back( slices );
   DataType cedType = DataType::SuggestComplex( in.DataType() );
   out.ReForge( outSizes, 1, cedType, Option::AcceptDataTypeChange::DO_ALLOW );

   // Compute radial component of filter
   Image rad = CreateRadiusCoordinate( ftIn.Sizes(), { S::FREQUENCY, S::RADIAL } );
   dfloat power = radCenter / radSigma;
   power *= power;
   dfloat sq = radSigma * radSigma;
   dfloat tv = 1.0 / std::sqrt( power * sq );
   rad = Exp( 0.5 * power - rad * rad * ( 0.5 / sq ) + power * Ln( rad * tv )); // TODO: this would be more efficient if we create a scan filter
   UnsignedArray origin = ftIn.Sizes();
   origin /= 2;
   rad.At( origin ) = 0.0;
   ftIn *= rad;
   rad.Strip();

   // Compute angular component of filter
   Image ang = CreatePhiCoordinate( ftIn.Sizes() );
   dfloat sigmaScale = pi / ( 1.0 + 2.0 * static_cast< dfloat >( order ));
   sigmaScale = -0.5 / ( sigmaScale * sigmaScale );

   // Loop over angles
   ImageSliceIterator it( out, 2 );
   for ( dip::uint ii = 0; ii < slices; ++ii, ++it ) {
      Image mask = ( ang <= -0.5 * pi ) | ( ang >= 0.5 * pi );
      Image filter = 2.0 * Exp( ang * ang * sigmaScale ); // TODO: this would be more efficient if we create a scan filter
      filter.At( mask ) = 0;
      *it = ftIn * filter;
      ang -= dPhi;
      Lesser( ang, -pi, mask );
      ang.At( mask ) = ang.At( mask ) + 2.0 * pi;
   }

   FourierTransform( out, out, { S::INVERSE }, { true, true, false } );
   out.SetPixelSize( std::move( pixelSize ));
}

namespace {

class SeparableConvolutionLineFilter : public Framework::SeparableLineFilter {
   public:
      SeparableConvolutionLineFilter( dip::uint numberOfGaussians, dip::uint maxIter, Option::Periodicity periodicity, dfloat scale )
            : numberOfGaussians_( numberOfGaussians ), maxIter_( maxIter ), periodicity_( periodicity ), scale_( scale ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength * numberOfGaussians_ * maxIter_ * 100; // the 100 is a totally wild guess...
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         ConstSampleIterator< dfloat > in{
               static_cast< dfloat const* >( params.inBuffer.buffer ),
               params.inBuffer.stride
         };
         SampleIterator< dfloat > out{
               static_cast< dfloat* >( params.outBuffer.buffer ),
               params.outBuffer.stride
         };
         dip::uint length = params.inBuffer.length;
         std::vector< dfloat >& buffer = buffer_[ params.thread ];
         if( buffer.empty() ) {
            buffer.resize( length * numberOfGaussians_ ); // doesn't need to be initialized
         }
         std::vector< GaussianParameters > gmm = GaussianMixtureModel( in, buffer.data(), length, numberOfGaussians_, maxIter_, periodicity_ );
         for( auto const& g : gmm ) {
            *out++ = g.position * scale_;
            *out++ = g.amplitude;
            *out++ = g.sigma * scale_;
         }
      }
   private:
      dip::uint numberOfGaussians_;
      dip::uint maxIter_;
      Option::Periodicity periodicity_;
      dfloat scale_;
      std::vector< std::vector< dfloat >> buffer_; // one per thread

};

} // namespace

// This function here because it's useful applied to the output of OrientationSpace().
void GaussianMixtureModel(
      Image const& in,
      Image& out,
      dip::uint dimension,
      dip::uint numberOfGaussians,
      dip::uint maxIter,
      StringSet const& flags
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( dimension >= nDims, E::ILLEGAL_DIMENSION );
   DIP_THROW_IF( numberOfGaussians < 1, E::INVALID_PARAMETER );
   Option::Periodicity periodicity = Option::Periodicity::NOT_PERIODIC;
   dfloat scale = 1.0;
   for( auto const& f : flags ) {
      if( f == S::PERIODIC ) {
         periodicity = Option::Periodicity::PERIODIC;
      } else if( f == "pixel size" ) {
         scale = in.PixelSize( dimension ).magnitude;
      } else {
         DIP_THROW_INVALID_FLAG( f );
      }
   }
   UnsignedArray outSizes = in.Sizes();
   outSizes[ dimension ] = 3 * numberOfGaussians;
   out.ReForge( outSizes, 1, DT_DFLOAT );
   BooleanArray process( nDims, false );
   process[ dimension ] = true;
   SeparableConvolutionLineFilter lineFilter( numberOfGaussians, maxIter, periodicity, scale );
   Framework::Separable( in, out, DT_DFLOAT, DT_DFLOAT, process, {}, {}, lineFilter,
                         Framework::SeparableOption::DontResizeOutput );
   out.SetPixelSize( dimension, {} );
   out.SplitDimension( dimension, 3 );
   out.SpatialToTensor( dimension );
}

} // namespace dip
