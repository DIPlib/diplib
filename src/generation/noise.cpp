/*
 * DIPlib 3.0
 * This file contains definitions for coordinate image generation
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
#include "diplib/random.h"
#include "diplib/generation.h"
#include "diplib/framework.h"

namespace dip {

namespace {
class DIP_EXPORT UniformScanLineFilter : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         UniformRandomGenerator& generator = *generatorArray_[ params.thread ];
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = *in + generator( lowerBound_, upperBound_ );
            in += inStride;
            out += outStride;
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override  {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< UniformRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1 );
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< UniformRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      UniformScanLineFilter( Random& random, dfloat lowerBound, dfloat upperBound ) :
            random_( random ), lowerBound_( lowerBound ), upperBound_( upperBound ) {}
   private:
      Random& random_;
      std::vector< Random > randomArray_;
      std::vector< std::unique_ptr< UniformRandomGenerator >> generatorArray_;
      dfloat lowerBound_;
      dfloat upperBound_;
};
} // namespace

void UniformNoise( Image const& in, Image& out, Random& random, dfloat lowerBound, dfloat upperBound ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   UniformScanLineFilter filter( random, lowerBound, upperBound );
   DataType dt = in.DataType();
   Framework::ScanMonadic( in, out, DataType::SuggestFloat( dt ), dt, 1, filter, Framework::Scan_TensorAsSpatialDim );
}

namespace {
class DIP_EXPORT GaussianScanLineFilter : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         GaussianRandomGenerator& generator = *generatorArray_[ params.thread ];
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = generator( *in, std_ );
            in += inStride;
            out += outStride;
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override  {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< GaussianRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1 );
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< GaussianRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      GaussianScanLineFilter( Random& random, dfloat std ) :
            random_( random ), std_( std ) {}
   private:
      Random& random_;
      std::vector< Random > randomArray_;
      std::vector< std::unique_ptr< GaussianRandomGenerator >> generatorArray_;
      dfloat std_;
};
} // namespace

void GaussianNoise( Image const& in, Image& out, Random& random, dfloat variance ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   GaussianScanLineFilter filter( random, std::sqrt( variance ));
   DataType dt = in.DataType();
   Framework::ScanMonadic( in, out, DataType::SuggestFloat( dt ), dt, 1, filter, Framework::Scan_TensorAsSpatialDim );
}

namespace {
class DIP_EXPORT PoissonScanLineFilter : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         PoissonRandomGenerator& generator = *generatorArray_[ params.thread ];
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = static_cast< dfloat >( generator( *in * conversion_ )) / conversion_;
            in += inStride;
            out += outStride;
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override  {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< PoissonRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1 );
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< PoissonRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      PoissonScanLineFilter( Random& random, dfloat conversion ) :
            random_( random ), conversion_( conversion ) {}
   private:
      Random& random_;
      std::vector< Random > randomArray_;
      std::vector< std::unique_ptr< PoissonRandomGenerator >> generatorArray_;
      dfloat conversion_;
};
} // namespace

void PoissonNoise( Image const& in, Image& out, Random& random, dfloat conversion ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   PoissonScanLineFilter filter( random, conversion );
   DataType dt = in.DataType();
   Framework::ScanMonadic( in, out, DataType::SuggestFloat( dt ), dt, 1, filter, Framework::Scan_TensorAsSpatialDim );
}

namespace {
class DIP_EXPORT BinaryScanLineFilter : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         bin const* in = static_cast< bin const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         BinaryRandomGenerator& generator = *generatorArray_[ params.thread ];
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = generator( *in ? p10_ : p01_ );
            in += inStride;
            out += outStride;
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override  {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< BinaryRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1 );
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< BinaryRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      BinaryScanLineFilter( Random& random, dfloat p10, dfloat p01 ) :
            random_( random ), p10_( p10 ), p01_( p01 ) {}
   private:
      Random& random_;
      std::vector< Random > randomArray_;
      std::vector< std::unique_ptr< BinaryRandomGenerator >> generatorArray_;
      dfloat p10_;
      dfloat p01_;
};
} // namespace

void BinaryNoise( Image const& in, Image& out, Random& random, dfloat p10, dfloat p01 ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   BinaryScanLineFilter filter( random, p10, p01 );
   Framework::ScanMonadic( in, out, DT_BIN, DT_BIN, 1, filter, Framework::Scan_TensorAsSpatialDim );
}

} // namespace dip
