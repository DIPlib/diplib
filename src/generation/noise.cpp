/*
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

#include "diplib/generation.h"

#include <cmath>
#include <memory>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/random.h"
#include "diplib/statistics.h"
#include "diplib/transform.h"

namespace dip {

// TODO: Is there false sharing in these line filters?

namespace {
class UniformScanLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 40; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      void SetNumberOfThreads( dip::uint threads ) override  {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< UniformRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1, Random( 0 ));
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< UniformRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      UniformScanLineFilter( Random& random, dfloat lowerBound, dfloat upperBound ) :
            random_( random ), lowerBound_( lowerBound ), upperBound_( upperBound ) {}
      // Make the class non copyable (generatorArray_ cannot be copied)
      UniformScanLineFilter( const UniformScanLineFilter& ) = delete;
      UniformScanLineFilter& operator=( const UniformScanLineFilter& ) = delete;
      // Complete the set of special functions
      UniformScanLineFilter( UniformScanLineFilter&& ) = delete;
      UniformScanLineFilter& operator=( UniformScanLineFilter&& ) = delete;
      ~UniformScanLineFilter() override = default;
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
   Framework::ScanMonadic( in, out, DT_DFLOAT, dt, 1, filter, Framework::ScanOption::TensorAsSpatialDim );
}

namespace {
class GaussianScanLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 150; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         GaussianRandomGenerator& generator = *( generatorArray_[ params.thread ] );
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = generator( *in, std_ );
            in += inStride;
            out += outStride;
         }
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< GaussianRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1, Random( 0 ));
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< GaussianRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      GaussianScanLineFilter( Random& random, dfloat std ) : random_( random ), std_( std ) {}
      // Make the class non copyable (generatorArray_ cannot be copied)
      GaussianScanLineFilter( const GaussianScanLineFilter& ) = delete;
      GaussianScanLineFilter& operator=( const GaussianScanLineFilter& ) = delete;
      // Complete the set of special functions
      GaussianScanLineFilter( GaussianScanLineFilter&& ) = delete;
      GaussianScanLineFilter& operator=( GaussianScanLineFilter&& ) = delete;
      ~GaussianScanLineFilter() override = default;
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
   Framework::ScanMonadic( in, out, DT_DFLOAT, dt, 1, filter, Framework::ScanOption::TensorAsSpatialDim );
}

namespace {
class PoissonScanLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 800; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      void SetNumberOfThreads( dip::uint threads ) override {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< PoissonRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1, Random( 0 ));
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< PoissonRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      PoissonScanLineFilter( Random& random, dfloat conversion ) : random_( random ), conversion_( conversion ) {}
      // Make the class non copyable (generatorArray_ cannot be copied)
      PoissonScanLineFilter( const PoissonScanLineFilter& ) = delete;
      PoissonScanLineFilter& operator=( const PoissonScanLineFilter& ) = delete;
      // Complete the set of special functions
      PoissonScanLineFilter( PoissonScanLineFilter&& ) = delete;
      PoissonScanLineFilter& operator=( PoissonScanLineFilter&& ) = delete;
      ~PoissonScanLineFilter() override = default;
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
   Framework::ScanMonadic( in, out, DT_DFLOAT, dt, 1, filter, Framework::ScanOption::TensorAsSpatialDim );
}

namespace {
class BinaryScanLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 40; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         bin const* in = static_cast< bin const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         BinaryRandomGenerator& generator = *generatorArray_[ params.thread ];
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = generator( *in ? pForeground_ : pBackground_ );
            in += inStride;
            out += outStride;
         }
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< BinaryRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1, Random( 0 ));
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< BinaryRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      BinaryScanLineFilter( Random& random, dfloat p10, dfloat p01 ) :
            random_( random ), pForeground_( 1.0 - p10 ), pBackground_( p01 ) {}
      // Make the class non copyable (generatorArray_ cannot be copied)
      BinaryScanLineFilter( const BinaryScanLineFilter& ) = delete;
      BinaryScanLineFilter& operator=( const BinaryScanLineFilter& ) = delete;
      // Complete the set of special functions
      BinaryScanLineFilter( BinaryScanLineFilter&& ) = delete;
      BinaryScanLineFilter& operator=( BinaryScanLineFilter&& ) = delete;
      ~BinaryScanLineFilter() override = default;
   private:
      Random& random_;
      std::vector< Random > randomArray_;
      std::vector< std::unique_ptr< BinaryRandomGenerator >> generatorArray_;
      dfloat pForeground_;
      dfloat pBackground_;
};
} // namespace

void BinaryNoise( Image const& in, Image& out, Random& random, dfloat p10, dfloat p01 ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   BinaryScanLineFilter filter( random, p10, p01 );
   Framework::ScanMonadic( in, out, DT_BIN, DT_BIN, 1, filter, Framework::ScanOption::TensorAsSpatialDim );
}

namespace {
class SaltPepperScanLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 40; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         UniformRandomGenerator& generator = *generatorArray_[ params.thread ];
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            dfloat p = generator( 0.0, 1.0 );
            if( p < p0_ ) {
               *out = 0;
            } else if( p >= p1_ ) {
               *out = white_;
            } else {
               *out = *in;
            }
            in += inStride;
            out += outStride;
         }
      }
      void SetNumberOfThreads( dip::uint threads ) override  {
         generatorArray_.resize( threads );
         generatorArray_[ 0 ] = std::make_unique< UniformRandomGenerator >( random_ );
         if( threads > 1 ) {
            randomArray_.resize( threads - 1, Random( 0 ));
            for( dip::uint ii = 1; ii < threads; ++ii ) {
               randomArray_[ ii - 1 ] = random_.Split();
               generatorArray_[ ii ] = std::make_unique< UniformRandomGenerator >( randomArray_[ ii - 1 ] );
            }
         }
      }
      SaltPepperScanLineFilter( Random& random, dfloat p0, dfloat p1, dfloat white ) :
            random_( random ), p0_( p0 ), p1_( 1.0 - p1 ), white_( white ) {}
      // Make the class non copyable (generatorArray_ cannot be copied)
      SaltPepperScanLineFilter( const SaltPepperScanLineFilter& ) = delete;
      SaltPepperScanLineFilter& operator=( const SaltPepperScanLineFilter& ) = delete;
      // Complete the set of special functions
      SaltPepperScanLineFilter( SaltPepperScanLineFilter&& ) = delete;
      SaltPepperScanLineFilter& operator=( SaltPepperScanLineFilter&& ) = delete;
      ~SaltPepperScanLineFilter() override = default;
   private:
      Random& random_;
      std::vector< Random > randomArray_;
      std::vector< std::unique_ptr< UniformRandomGenerator >> generatorArray_;
      dfloat p0_;
      dfloat p1_;
      dfloat white_;
};
} // namespace

void SaltPepperNoise( Image const& in, Image& out, Random& random, dfloat p0, dfloat p1, dfloat white ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( p0 < 0.0 || p1 < 0.0, E::INVALID_PARAMETER );
   dfloat s = p0 + p1;
   if( s > 1.0 ) {
      p0 /= s;
      p1 /= s; // This means the whole image will be black and white noise!
   }
   SaltPepperScanLineFilter filter( random, p0, p1, white );
   DataType dt = in.DataType();
   Framework::ScanMonadic( in, out, DT_DFLOAT, dt, 1, filter, Framework::ScanOption::TensorAsSpatialDim );
}

void FillColoredNoise( Image& out, Random& random, dfloat variance, dfloat color ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   // White noise in the Fourier Domain
   // Note: Ideally we'd generate an image with conjugate symmetry in the Fourier domain. Instead, we'll generate twice the
   // number of random values, and throw away the imaginary component after inverse transform.
   Image fd( out.Sizes(), out.TensorElements(), DT_DCOMPLEX );
   fd.SplitComplex();
   fd.Fill( 0.0 );
   GaussianNoise( fd, fd, random, 1.0 );
   fd.MergeComplex();
   // Frequency spectrum modulation function
   Image modulation = CreateRadiusSquareCoordinate( out.Sizes(), { S::RADFREQ } );
   dfloat power = color / 2.0; // divide by 2.0 because we haven't done the square root in the "radfreq" image yet.
   // TODO: According to wikipedia, pink noise in 2D is 1/f^2, not 1/f as it is in 1D. This doesn't look right to me...
   Power( modulation, power, modulation );
   // The value at the origin must be 0 for zero mean in spatial domain
   auto origin = modulation.Sizes();
   for( auto& o : origin ) {
      o /= 2;
   }
   modulation.At( origin ) = 0;
   // Normalize modulation function
   dfloat w = SumSquare( modulation ).As< dfloat >();
   w = std::sqrt( variance / w ) * static_cast< dfloat >( modulation.Sizes().product() );
   modulation *= w;
   // Modulate and inverse transform
   fd *= modulation;
   bool prot = out.Protect();
   FourierTransform( fd, out, { S::INVERSE, S::REAL } ); // We set "real" so only the real component is written to `out`.
   out.Protect( prot );
}

} // namespace dip
