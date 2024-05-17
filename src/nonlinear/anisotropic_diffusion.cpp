/*
 * (c)2018, Cris Luengo.
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

#include "diplib/nonlinear.h"

#include <cmath>
#include <memory>
#include <vector>

#include "diplib.h"
#include "diplib/analysis.h"
#include "diplib/boundary.h"
#include "diplib/framework.h"
#include "diplib/geometry.h"
#include "diplib/kernel.h"
#include "diplib/linear.h"
#include "diplib/math.h"
#include "diplib/pixel_table.h"
#include "diplib/statistics.h"

namespace dip {

namespace {

template< typename F >
class PeronaMalikLineFilter : public Framework::FullLineFilter {
   public:
      PeronaMalikLineFilter( F g, dip::uint cost, sfloat lambda ) : g_( std::move( g )), cost_( cost ), lambda_( lambda ) {}
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override {
         return cost_ * lineLength;
      }
      void Filter( Framework::FullLineFilterParameters const& params ) override {
         sfloat* in = static_cast< sfloat* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         sfloat* out = static_cast< sfloat* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         dip::uint nDims = params.pixelTable.Dimensionality();
         std::vector< PixelTableOffsets::PixelRun > pixelTableRuns = params.pixelTable.Runs(); // copy!
         // The pixel table has `(nDims-1)*2+1` runs of length 1, and 1 run of length 3:
         DIP_ASSERT( pixelTableRuns.size() == ( nDims - 1 ) * 2 + 1 );
         for( dip::uint ii = 0; ii < ( nDims - 1 ) * 2 + 1; ++ii ) {
            if( pixelTableRuns[ ii ].length == 3 ) {
               //DIP_ASSERT( pixelTableRuns[ ii ].offset == -inStride );
               pixelTableRuns[ ii ].length = 1; // the run should have a length of 1
               pixelTableRuns.push_back( { -pixelTableRuns[ ii ].offset, 1 } ); // add another run of length one to the other side
            //} else {
               //DIP_ASSERT( pixelTableRuns[ ii ].length == 1 );
            }
         }
         //DIP_ASSERT( pixelTableRuns.size() == nDims * 2 );
         // Now the pixel table has `2*nDims` runs, one for each neighbor. The `offset` is the address of the neighbor.
         for( dip::uint ii = 0; ii < length; ++ii ) {
            sfloat delta = 0;
            for( auto run : pixelTableRuns ) {
               sfloat diff = in[ run.offset ] - in[ 0 ];
               delta += g_( diff ) * diff;
            }
            *out = *in + lambda_ * delta;
            in += inStride;
            out += outStride;
         }
      }
   private:
      F g_;
      dip::uint cost_;
      sfloat lambda_;
};

template< typename F >
inline std::unique_ptr< Framework::FullLineFilter > NewPeronaMalikLineFilter( F g, dip::uint cost, sfloat lambda ) {
   return static_cast< std::unique_ptr< Framework::FullLineFilter >>( new PeronaMalikLineFilter< F >( std::move( g ), cost, lambda ));
}

} // namespace

void PeronaMalikDiffusion(
      Image const& in,
      Image& out,
      dip::uint iterations,
      dfloat K,
      dfloat lambda,
      String const& g
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( iterations < 1, E::INVALID_PARAMETER );
   DIP_THROW_IF( K <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF(( lambda <= 0.0 ) || ( lambda > 1.0 ), E::PARAMETER_OUT_OF_RANGE );

   // Create a line filter for the scan framework that applies `g`.
   std::unique_ptr< Framework::FullLineFilter > lineFilter;
   sfloat fK = static_cast< sfloat >( K );
   if( g == "Gauss" ) {
      lineFilter = NewPeronaMalikLineFilter(
            [ fK ]( sfloat v ) { v /= fK; return std::exp( -v * v ); },
            20, static_cast< sfloat >( lambda ));
   } else if( g == "quadratic") {
      lineFilter = NewPeronaMalikLineFilter(
            [ fK ]( sfloat v ) { v /= fK; return 1.0f / ( 1.0f + ( v * v )); },
            4, static_cast< sfloat >( lambda ));
   } else if( g == "exponential") {
      lineFilter = NewPeronaMalikLineFilter(
            [ fK ]( sfloat v ) { v /= fK; return std::exp( -std::abs( v )); },
            20, static_cast< sfloat >( lambda ));
   } else if( g == "Tukey") {
      lineFilter = NewPeronaMalikLineFilter(
            [ fK ]( sfloat v ) { v /= fK; return std::abs( v ) < 1.0f ? ( 1 - ( v * v )) * ( 1 - ( v * v )) : 0.0f; },
            6, static_cast< sfloat >( lambda ));
   } else {
      DIP_THROW_INVALID_FLAG( g );
   }

   // Each iteration is applied to `out`.
   BoundaryConditionArray bc( in.Dimensionality(), BoundaryCondition::ADD_ZEROS );
   Kernel kernel( Kernel::ShapeCode::DIAMOND, { 3 } );
   for( dip::uint ii = 0; ii < iterations; ++ii ) {
      Framework::Full( ii == 0 ? in : out, out, DT_SFLOAT, DT_SFLOAT, DT_SFLOAT, 1, bc, kernel, *lineFilter, Framework::FullOption::AsScalarImage );
   }
}

namespace {

template< typename F >
class GaussianAnisotropicDiffusionLineFilter : public Framework::ScanLineFilter {
   public:
      GaussianAnisotropicDiffusionLineFilter( F g, dip::uint cost ) : g_( std::move( g )), cost_( cost ) {}
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint nTensorElements ) override {
         return cost_ + nTensorElements + 20;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         sfloat* in = static_cast< sfloat* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::sint inTStride = params.inBuffer[ 0 ].tensorStride;
         dip::uint nTensorElems = params.inBuffer[ 0 ].tensorLength;
         sfloat* out = static_cast< sfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint outTStride = params.outBuffer[ 0 ].tensorStride;
         DIP_ASSERT( nTensorElems == params.outBuffer[ 0 ].tensorLength );
         dip::uint length = params.bufferLength;
         for( dip::uint ii = 0; ii < length; ++ii ) {
            ConstSampleIterator< sfloat > inIt( in, inTStride );
            sfloat n2 = SumAbsSquare( nTensorElems, inIt );
            sfloat c = g_( n2 );
            SampleIterator< sfloat > outIt( out, outTStride );
            auto endIt = outIt + nTensorElems;
            for( ; outIt != endIt; ++inIt, ++outIt ) {
               *outIt = *inIt * c;
            }
            in += inStride;
            out += outStride;
         }
      }
   private:
      F g_;
      dip::uint cost_;
};

template< typename F >
inline std::unique_ptr< Framework::ScanLineFilter > NewGaussianAnisotropicDiffusionLineFilter( F g, dip::uint cost ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new GaussianAnisotropicDiffusionLineFilter< F >( std::move( g ), cost ));
}

} // namespace

void GaussianAnisotropicDiffusion(
      Image const& in,
      Image& out,
      dip::uint iterations,
      dfloat K,
      dfloat lambda,
      String const& g
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( iterations < 1, E::INVALID_PARAMETER );
   DIP_THROW_IF( K <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF(( lambda <= 0.0 ) || ( lambda > 1.0 ), E::PARAMETER_OUT_OF_RANGE );

   // Create a line filter for the scan framework that applies `g`.
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   sfloat fK = 1.0f / static_cast< sfloat >( K * K );
   sfloat fL = static_cast< sfloat >( lambda );
   if( g == "Gauss" ) {
      lineFilter = NewGaussianAnisotropicDiffusionLineFilter(
            [ fK, fL ]( sfloat n2 ) { n2 *= fK; return fL * std::exp( -n2 ); }, // In these functions, `n2` is the square of the norm
            20 );
   } else if( g == "quadratic") {
      lineFilter = NewGaussianAnisotropicDiffusionLineFilter(
            [ fK, fL ]( sfloat n2 ) { n2 *= fK; return fL / ( 1.0f + ( n2 )); },
            3 );
   } else if( g == "exponential") {
      lineFilter = NewGaussianAnisotropicDiffusionLineFilter(
            [ fK, fL ]( sfloat n2 ) { n2 *= fK; return fL * std::exp( -std::sqrt( n2 )); },
            30 );
   } else {
      DIP_THROW_INVALID_FLAG( g );
   }

   // Each iteration is applied to `out`.
   Image nabla, inc;
   nabla.SetDataType( DT_SFLOAT );
   nabla.Protect( true ); // force Gradient in the first loop iteration to produce an SFLOAT out, even if in is DFLOAT.
   for( dip::uint ii = 0; ii < iterations; ++ii ) {
      Gradient( ii == 0 ? in : out, nabla, { 0.8 }, "gaussFIR" );
      Framework::ScanMonadic( nabla, nabla, DT_SFLOAT, DT_SFLOAT, nabla.TensorElements(), *lineFilter );
      Divergence( nabla, inc, { 0.8 }, "gaussFIR" );
      Add( ii == 0 ? in : out, inc, out, DT_SFLOAT );
   }
}

namespace {
   inline void EigenComposition(
      Image const& eigenvalues,
      Image const& eigenvectors,
      Image& out
   ) {
      Multiply( eigenvectors, eigenvalues, out, eigenvalues.DataType() );
      Multiply( out, Transpose( eigenvectors ), out, out.DataType() );
   }
}

void CoherenceEnhancingDiffusion(
      Image const& in,
      Image& out,
      dfloat derivativeSigma,
      dfloat regularizationSigma,
      dip::uint iterations,
      StringSet const& flags
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( iterations < 1, E::INVALID_PARAMETER );
   DIP_THROW_IF(( derivativeSigma < 0.4 ) || ( regularizationSigma < 1.0 ), E::PARAMETER_OUT_OF_RANGE );
   bool variable = true;
   bool first = true;
   bool resample = false;
   for( auto& flag : flags ) {
      if( flag == "const" ) {
         variable = false;
      } else if( flag == "variable" ) {
         variable = true;
      } else if( flag == "all" ) {
         first = false;
      } else if( flag == "first" ) {
         first = true;
      } else if( flag == "resample" ) {
         resample = true;
      } else {
         DIP_THROW_INVALID_FLAG( flag );
      }
   }
   if( nDims > 2 ) {
      first = false; // we cannot do this yet.
   }

   // Resample the input image, and copy to `out`
   if( !out.IsProtected() ) {
      out.Strip();
      out.SetDataType( DataType::SuggestFloat( in.DataType() ));
      out.Protect( true );
   }
   Resampling( in, out, { 2.0 }, { 0.0 }, S::LINEAR );
   derivativeSigma *= 2;
   regularizationSigma *= 2;

   Image S;            // structure tensor, 2x2 symmetric matrix
   Image D;            // diffusion tensor, 2x2 matrix (symmetric, but the way we construct it it's stored as a full matrix)
   Image eigenvalues;  // 2x2 diagonal matrix
   Image eigenvectors; // 2x2 matrix
   Image diff;         // scalar
   Image anisotropy;   // scalar
   Image mask;         // scalar
   Image gradient;     // 2-vector
   Image hessian;      // 2x2 symmetric matrix
   Image delta;        // scalar
   for( dip::uint ii = 0; ii < iterations; ++ii ) {

      StructureTensor( out, {}, S, { derivativeSigma }, { regularizationSigma }, "gaussFIR" );
      EigenDecomposition( S, eigenvalues, eigenvectors );
      if( first ) {
         // Parameters
         constexpr dfloat alpha = 0.01;
         dfloat c_par = Percentile( eigenvalues[ 1 ], {}, 50.0 ).As< dfloat >();
         c_par *= c_par; // 50th percentile of square of 2nd eigenvalue.
         // TODO: stuff below here can be a `D = NewDyadicScanLineFilter( eigenvalues, eigenvectors )`
         // Anisotropy = ( mu_1 - mu_2 ) / ( mu_1 + mu_2 )
         Subtract( eigenvalues[ 0 ], eigenvalues[ 1 ], diff, eigenvalues.DataType() );
         SafeDivide( diff, eigenvalues[ 0 ] + eigenvalues[ 1 ], anisotropy, eigenvalues.DataType() );
         // l_2 = ( conf > 0.01 ) * ( alpha + ( 1.0 - alpha ) * exp( -c_par / ( mu_1 - mu_2 ) ^ ( 2 * m_par ))) + ~( conf > 0.01 ) * ( alpha );
         Greater( anisotropy, alpha, mask );
         Image tmp = diff.At( mask );
         tmp *= tmp;
         Divide( -c_par, tmp, tmp, tmp.DataType() );
         Exp( tmp, tmp );
         tmp *= 1.0 - alpha;
         tmp += alpha;
         eigenvalues.Fill( alpha );
         eigenvalues.At( mask )[ 1 ] = tmp;
         EigenComposition( eigenvalues, eigenvectors, D );
      } else { // all
         // This is not exactly the same as the code in DIPimage 2. That was specifically 2D. It does:
         //  - normalize D by its trace
         //  - swap D[0] and D[1] (the two diagonal elements)
         //  - negate D[2] (the two off-diagonal elements)
         // The last two steps is equivalent to swapping the two eigenvalues. Here, instead, we compute the
         // reciprocal of each eigenvalue, then normalize the trace one to 1 (that is, we divide by the trace).
         // Swapping eigenvectors does not generalize to higher dimensionalities.
         // TODO: All of this can be a `D = NewMonadicScanLineFilter( D )`
         Divide( 1, eigenvalues, eigenvalues, eigenvalues.DataType() );
         Divide( eigenvalues, Trace( eigenvalues ), eigenvalues, eigenvalues.DataType() );
         EigenComposition( eigenvalues, eigenvectors, D );
         //
         //DataType dt = D.DataType();
         //dip::uint nTElem = D.TensorElements(); // == ((nDims+1)*nDims)/2
         //std::unique_ptr< dip::Framework::ScanLineFilter > scanLineFilter;
         //DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewMonadicScanLineFilter, (
         //   [ nDims, nTElem ]( auto its ) {
         //
         //   }, 1 ), dt );
         //dip::Framework::ScanMonadic( D, D, dt, dt, nTElem, *scanLineFilter );
      }

      if( variable ) {
         Gradient( out, gradient, { 1 }, "gaussFIR" );
         Multiply( D, gradient, gradient, gradient.DataType() );
         Divergence( gradient, delta, { 1 }, "gaussFIR" );
      } else { // const
         Hessian( out, hessian, { 1 }, "gaussFIR" );
         MultiplySampleWise( D, hessian, D, D.DataType() );
         DIP_ASSERT( D.TensorElements() == nDims * nDims ); // make sure D is a full matrix
         SumTensorElements( D, delta );
      }
      out += delta;
   }

   // Subsample the output image
   out.Protect( false );
   if( !resample ) {
      Subsampling( out, out, { 2 } );
   }
}

} // namespace dip
