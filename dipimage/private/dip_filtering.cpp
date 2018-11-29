/*
 * DIPimage 3.0
 *
 * (c)2017-2018, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#include "dip_matlab_interface.h"

#include "diplib/linear.h"
#include "diplib/nonlinear.h"
#include "diplib/transform.h"

#include "diplib/generation.h"
#include "diplib/analysis.h"

namespace {

char const* wrongFilter = "Wrong filter definition";

// Convert a real or complex floating-point array for `mxArray` to `std::vector<dip::dfloat>` by copy.
// If the array was complex, the output contains real and imaginary elements interleaved.
inline std::vector< dip::dfloat > GetRealOrComplexArray( mxArray const* mx ) {
   if( mxIsDouble( mx ) && dml::IsVector( mx )) {
      bool isComplex = mxIsComplex( mx );
      dip::uint k = isComplex ? 2 : 1;
      dip::uint n = mxGetNumberOfElements( mx );
      std::vector< dip::dfloat > out( k * n );
      double* data = mxGetPr( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ k * ii ] = data[ ii ];
      }
      if( isComplex ) {
         data = mxGetPi( mx );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            out[ k * ii + 1 ] = data[ ii ];
         }
      }
      return out;
   }
   DIP_THROW( "Real- or complex-valued floating-point array expected" );
}

dip::OneDimensionalFilter GetFilter(
      mxArray const* mxFilter, // A struct array
      dip::uint ii
) {
   dip::OneDimensionalFilter out;
   mxArray const* elem = mxGetField( mxFilter, ii, "filter" );
   DIP_THROW_IF( !elem, "" );
   out.filter = GetRealOrComplexArray( elem );
   out.isComplex = mxIsComplex( elem );

   elem = mxGetField( mxFilter, ii, "origin" );
   if( elem ) {
      out.origin = dml::GetInteger( elem );
   }

   elem = mxGetField( mxFilter, ii, "flags" );
   if( elem ) {
      out.symmetry = dml::GetString( elem );
   }

   return out;
}

void convolve( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 2 );
   dip::StringArray bc = nrhs > 1 ? dml::GetStringArray( prhs[ 1 ] ) : dip::StringArray{};
   dip::OneDimensionalFilterArray filterArray;
   mxArray const* mxFilter = prhs[ 0 ];
   if( mxIsNumeric( mxFilter ) || mxIsClass( mxFilter, "dip_image" )) {
      dip::Image const filter = dml::GetImage( mxFilter );
      filterArray = dip::SeparateFilter( filter );
      if( filterArray.empty() ) {
         if( filter.NumberOfPixels() > 7 * 7 ) { // note that this is an arbitrary threshold, and should probably depend also on log2(image size)
            dip::ConvolveFT( in, filter, out );
         } else {
            dip::GeneralConvolution( in, filter, out, bc );
         }
      } else {
         dip::SeparableConvolution( in, out, filterArray, bc );
      }
   } else {
      if( mxIsCell( mxFilter )) {
         DIP_THROW_IF( !dml::IsVector( mxFilter ), wrongFilter );
         dip::uint n = mxGetNumberOfElements( mxFilter );
         filterArray.resize( n );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            mxArray const* elem = mxGetCell( mxFilter, ii );
            try {
               filterArray[ ii ].filter = GetRealOrComplexArray( elem );
               filterArray[ ii ].isComplex = mxIsComplex( elem );
            } catch( dip::Error& ) {
               DIP_THROW( wrongFilter );
            }
         }
      } else if( mxIsStruct( mxFilter )) {
         dip::uint n = mxGetNumberOfElements( mxFilter );
         filterArray.resize( n );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            try {
               filterArray[ ii ] = GetFilter( mxFilter, ii );
            } catch( dip::Error& ) {
               DIP_THROW( wrongFilter );
            }
         }
      }
      dip::SeparableConvolution( in, out, filterArray, bc );
   }
}


void derivative( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::UnsignedArray order = nrhs > 0 ? dml::GetUnsignedArray( prhs[ 0 ] ) : dip::UnsignedArray{ 0 };
   dip::FloatArray sigmas = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 1 };
   dip::String method = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : dip::S::BEST;
   dip::StringArray bc = nrhs > 3 ? dml::GetStringArray( prhs[ 3 ] ) : dip::StringArray{};
   dip::dfloat truncation = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 3.0;
   if( method == "kernel" ) {
      dip::CreateGauss( out, sigmas, order, truncation );
   } else {
      dip::Derivative( in, out, order, sigmas, method, bc, truncation );
   }
}


struct DerivativeArguments {
   dip::FloatArray sigmas = { 1 };
   dip::String method = dip::S::BEST;
   dip::StringArray bc = {};
   dip::BooleanArray process = {};
   dip::dfloat truncation = 3;
};

DerivativeArguments GetDerivativeArguments(
      int nrhs,
      const mxArray* prhs[],
      dip::uint nDims
) {
   DML_MAX_ARGS( 5 );
   DerivativeArguments args;
   if( nrhs > 0 ) { args.sigmas = dml::GetFloatArray( prhs[ 0 ] ); }
   if( nrhs > 1 ) { args.method = dml::GetString( prhs[ 1 ] ); }
   if( nrhs > 2 ) { args.bc = dml::GetStringArray( prhs[ 2 ] ); }
   if( nrhs > 3 ) { args.process = dml::GetProcessArray( prhs[ 3 ], nDims ); }
   if( nrhs > 4 ) { args.truncation = dml::GetFloat( prhs[ 4 ] ); }
   return args;
}

void curl( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::Curl( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void divergence( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::Divergence( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void gradient( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::Gradient( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void gradmag( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::GradientMagnitude( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void hessian( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::Hessian( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void laplace( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::Laplace( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void dgg( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::Dgg( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void laplace_min_dgg( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::LaplaceMinusDgg( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}
void laplace_plus_dgg( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   auto args = GetDerivativeArguments( nrhs, prhs, in.Dimensionality() );
   dip::LaplacePlusDgg( in, out, args.sigmas, args.method, args.bc, args.process, args.truncation );
}


void gabor( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   dip::FloatArray sigmas = nrhs > 0 ? dml::GetFloatArray( prhs[ 0 ] ) : dip::FloatArray{ 5.0 };
   dip::FloatArray frequencies = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 0.15 };
   int index = 2;
   if(( in.Dimensionality() == 2 ) && ( frequencies.size() == 1 )) {
      dip::dfloat frequency = frequencies[ 0 ];
      dip::dfloat direction = dip::pi;
      if( nrhs > index ) {
         direction = dml::GetFloat( prhs[ index ] );
         ++index;
      }
      frequencies = { frequency * std::cos( direction ), frequency * std::sin( direction ) };
   }
   DML_MAX_ARGS( index + 4 );
   dip::String method = nrhs > index ? dml::GetString( prhs[ index ] ) : "iir";
   dip::StringArray bc = nrhs > index + 1 ? dml::GetStringArray( prhs[ index + 1 ] ) : dip::StringArray{};
   dip::BooleanArray process = nrhs > index + 2 ? dml::GetProcessArray( prhs[ index + 2 ], in.Dimensionality() ) : dip::BooleanArray{};
   dip::dfloat truncation = nrhs > index + 3 ? dml::GetFloat( prhs[ index + 3 ] ) : 3.0;
   if( method == "iir" ) {
      dip::GaborIIR( in, out, sigmas, frequencies, bc, process, {}, truncation );
   } else if( method == "fir" ) {
      dip::GaborFIR( in, out, sigmas, frequencies, bc, process, truncation );
   } else if( method == "kernel" ) {
      if( sigmas.size() == 1 ) {
         sigmas.resize( in.Dimensionality(), sigmas[ 0 ] );
      }
      dip::CreateGabor( out, sigmas, frequencies, truncation );
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
}

void loggabor( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::FloatArray wavelengths = nrhs > 0 ? dml::GetFloatArray( prhs[ 0 ] ) : dip::FloatArray{ 3.0, 6.0, 12.0, 24.0 };
   dip::dfloat bandwidth = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 0.75;
   dip::uint nOrientations = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 6;
   dip::String inRepresentation = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::String{ dip::S::SPATIAL };
   dip::String outRepresentation = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::String{ dip::S::SPATIAL };
   dip::LogGaborFilterBank( in, out, wavelengths, bandwidth, nOrientations, inRepresentation, outRepresentation );
}

void normconv( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 6 );
   dip::Image const mask = dml::GetImage( prhs[ 0 ] );
   bool computeDerivative = false;
   dip::uint dimension = 0;
   if( nrhs > 1 ) {
      if( !mxIsEmpty( prhs[ 1 ] )) {
         dimension = dml::GetUnsigned( prhs[ 1 ] );
         DIP_THROW_IF( ( dimension < 1 ) || ( dimension > in.Dimensionality() ), "Dimension index out of range" );
         --dimension;
         computeDerivative = true;
      }
   }
   dip::FloatArray sigmas = nrhs > 2 ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 1.0 };
   dip::String method = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::S::BEST;
   dip::StringArray boundaryCondition = nrhs > 4 ? dml::GetStringArray( prhs[ 4 ] ) : dip::StringArray{ dip::S::ADD_ZEROS };
   dip::dfloat truncation = nrhs > 5 ? dml::GetFloat( prhs[ 5 ] ) : 3.0;
   if( computeDerivative ) {
      dip::NormalizedDifferentialConvolution( in, mask, out, dimension, sigmas, method, boundaryCondition, truncation );
   } else {
      dip::NormalizedConvolution( in, mask, out, sigmas, method, boundaryCondition, truncation );
   }
}

void unif( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   int index = 0;
   auto kernel = dml::GetKernel< dip::Kernel >( nrhs, prhs, index, in.Dimensionality() );
   DML_MAX_ARGS( index + 1 );
   dip::StringArray bc = nrhs > index ? dml::GetStringArray( prhs[ index ] ) : dip::StringArray{};
   dip::Uniform( in, out, kernel, bc );
}

void bilateralf( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::FloatArray spatialSigmas = nrhs > 0 ? dml::GetFloatArray( prhs[ 0 ] ) : dip::FloatArray{ 2.0 };
   dip::dfloat tonalSigma = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 30.0;
   dip::dfloat truncation = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 2.0;
   dip::String method = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::String( "xysep" );
   dip::StringArray bc = nrhs > 4 ? dml::GetStringArray( prhs[ 4 ] ) : dip::StringArray{};
   dip::BilateralFilter( in, {}, out, spatialSigmas, tonalSigma, truncation, method, bc );
}

void ced( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 6 );
   dip::dfloat derivativeSigma = nrhs > 0 ? dml::GetFloat( prhs[ 0 ] ) : 1.0;
   dip::dfloat regularizationSigma = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 3.0;
   dip::uint iterations = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 5;
   dip::StringSet flags = {};
   if( nrhs > 3 ) {
      dip::String coef = dml::GetString( prhs[ 3 ] );
      flags.insert( std::move( coef ));
   }
   if( nrhs > 4 ) {
      dip::String flavour = dml::GetString( prhs[ 4 ] );
      flags.insert( std::move( flavour ));
   }
   if( nrhs > 5 ) {
      if( !dml::GetBoolean( prhs[ 5 ] )) {
         flags.emplace( "resample" );
      }
   }
   CoherenceEnhancingDiffusion( in, out, derivativeSigma, regularizationSigma, iterations, flags );
}

void gaussf_adap( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 2 || nDims > 3, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::ImageArray params;
   if(( nrhs < 1 ) || mxIsEmpty( prhs[ 0 ] )) {
      // Compute orientation
      params = StructureTensorAnalysis( dip::StructureTensor( in ),
                                        nDims == 2 ? dip::StringArray{ "orientation" }
                                                   : dip::StringArray{ "phi3", "theta3" } );
   } else {
      params = dml::GetImageArray( prhs[ 0 ] );
   }
   dip::FloatArray sigmas;
   if( nrhs > 1 ) {
      sigmas = dml::GetFloatArray( prhs[ 1 ] );
   } else {
      sigmas.resize( nDims, 0 );
      if( nDims == 2 ) {
         sigmas.front() = 2.0;
      } else {
         sigmas.back() = 2.0;
      }
   }
   dip::UnsignedArray order = nrhs > 2 ? dml::GetUnsignedArray( prhs[ 2 ] ) : dip::UnsignedArray{ 0 };
   dip::UnsignedArray exponents =  nrhs > 3 ? dml::GetUnsignedArray( prhs[ 3 ] ) : dip::UnsignedArray{ 0 };
   dip::dfloat truncation = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 2.0;
   dip::AdaptiveGauss( in, dip::CreateImageConstRefArray( params ), out, sigmas, order, truncation, exponents );
}

void gaussf_adap_banana( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims != 2, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
   int index = 0;
   dip::ImageArray params;
   if(( nrhs < 2 ) || mxIsEmpty( prhs[ 1 ] )) {
      // Compute orientation
      params = StructureTensorAnalysis( dip::StructureTensor( in ), { "orientation", "curvature" } );
      ++index;
   } else if( mxIsCell( prhs[ 1 ] )) {
      // It's a params_im
      params = dml::GetImageArray( prhs[ 1 ] );
      ++index;
   } else {
      // It's orien_im, curv_im, {scale_im}
      DML_MIN_ARGS( 3 );
      params.resize( 2 );
      params[ 0 ] = dml::GetImage( prhs[ 1 ] );
      params[ 1 ] = dml::GetImage( prhs[ 2 ] );
      index += 2;
      // Is there a scaling image?
      if(( nrhs > index ) && !( mxIsDouble( prhs[ index ] ) && ( mxGetNumberOfElements( prhs[ index ] ) <= 2 ))) {
         params.emplace_back( dml::GetImage( prhs[ 3 ] ));
         ++index;
      }
   }
   DML_MAX_ARGS( index + 4 );
   dip::FloatArray sigmas = nrhs > index ? dml::GetFloatArray( prhs[ index ] ) : dip::FloatArray{ 2.0, 0.0 };
   dip::UnsignedArray order = nrhs > index + 1 ? dml::GetUnsignedArray( prhs[ index + 1 ] ) : dip::UnsignedArray{ 0 };
   dip::UnsignedArray exponents = nrhs > index + 2 ? dml::GetUnsignedArray( prhs[ index + 2 ] ) : dip::UnsignedArray{ 0 };
   dip::dfloat truncation = nrhs > index + 3 ? dml::GetFloat( prhs[ index + 3 ] ) : 2.0;
   dip::AdaptiveBanana( in, dip::CreateImageConstRefArray( params ), out, sigmas, order, truncation, exponents );}

void kuwahara( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   int index = 0;
   auto kernel = dml::GetKernel< dip::Kernel >( nrhs, prhs, index, in.Dimensionality() );
   dip::dfloat threshold = nrhs > index ? dml::GetFloat( prhs[ index ] ) : 0.0;
   dip::StringArray bc = nrhs > index + 1 ? dml::GetStringArray( prhs[ index + 1 ] ) : dip::StringArray{};
   dip::Kuwahara( in, out, kernel, threshold, bc );
}

void nonmaximumsuppression( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 3 );
   dip::Image const gradient = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::String mode = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : dip::String{ "interpolate" };
   dip::NonMaximumSuppression( in, gradient, mask, out, mode );
}

void percf( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   dip::dfloat percentile = nrhs > 0 ? dml::GetFloat( prhs[ 0 ] ) : 50.0;
   int index = 1;
   auto kernel = dml::GetKernel< dip::Kernel >( nrhs, prhs, index, in.Dimensionality() );
   dip::StringArray bc = nrhs > index ? dml::GetStringArray( prhs[ index ] ) : dip::StringArray{};
   dip::PercentileFilter( in, out, percentile, kernel, bc );
}

void pmd( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::uint iterations = nrhs > 0 ? dml::GetUnsigned( prhs[ 0 ] ) : 5;
   dip::dfloat K = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 10.0;
   dip::dfloat lambda = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.25;
   dip::String g = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : "Gauss";
   dip::PeronaMalikDiffusion( in, out, iterations, K, lambda, g );
}

void pmd_gaussian( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::uint iterations = nrhs > 0 ? dml::GetUnsigned( prhs[ 0 ] ) : 5;
   dip::dfloat K = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 10.0;
   dip::dfloat lambda = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.25;
   dip::String g = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : "Gauss";
   dip::GaussianAnisotropicDiffusion( in, out, iterations, K, lambda, g );
}

void selectionf( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 6 );
   dip::Image const control = dml::GetImage( prhs[ 0 ] );
   int index = 1;
   auto kernel = dml::GetKernel< dip::Kernel >( nrhs, prhs, index, in.Dimensionality() );
   dip::dfloat threshold = nrhs > index ? dml::GetFloat( prhs[ index ] ) : 0.0;
   dip::String mode = nrhs > index + 1 ? dml::GetString( prhs[ index + 1 ] ) : dip::S::MINIMUM;
   dip::StringArray bc = nrhs > index + 2 ? dml::GetStringArray( prhs[ index + 2 ] ) : dip::StringArray{};
   dip::SelectionFilter( in, control, out, kernel, threshold, mode, bc );
}

void varif( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   int index = 0;
   auto kernel = dml::GetKernel< dip::Kernel >( nrhs, prhs, index, in.Dimensionality() );
   dip::StringArray bc = nrhs > index ? dml::GetStringArray( prhs[ index ] ) : dip::StringArray{};
   dip::VarianceFilter( in, out, kernel, bc );
}


void ft( dip::Image const& in, dip::Image& out, int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 2 );
   dip::StringSet options = nrhs > 0 ? dml::GetStringSet( prhs[ 0 ] ) : dip::StringSet{};
   dip::BooleanArray process = nrhs > 1 ? dml::GetProcessArray( prhs[ 1 ], in.Dimensionality() ) : dip::BooleanArray{};
   dip::FourierTransform( in, out, options, process );
}

void riesz( dip::Image const& in, dip::Image& out, int nrhs ) {
   DML_MAX_ARGS( 0 );
   dip::RieszTransform( in, out );
}

} // namespace

// Gateway function

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      dip::Image in;
      if(( function == "loggabor" ) && mxIsNumeric( prhs[ 1 ] ) && dml::IsVector( prhs[ 1 ] )) {
         in.SetSizes( dml::GetUnsignedArray( prhs[ 1 ] ));
      } else {
         in = dml::GetImage( prhs[ 1 ] );
      }
      prhs += 2;
      nrhs -= 2;

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      if( function == "convolve" ) {
         convolve( in, out, nrhs, prhs );
      } else if( function == "derivative" ) {
         derivative( in, out, nrhs, prhs );
      } else if( function == "curl" ) {
         curl( in, out, nrhs, prhs );
      } else if( function == "divergence" ) {
         divergence( in, out, nrhs, prhs );
      } else if( function == "gradient" ) {
         gradient( in, out, nrhs, prhs );
      } else if( function == "gradmag" ) {
         gradmag( in, out, nrhs, prhs );
      } else if( function == "hessian" ) {
         hessian( in, out, nrhs, prhs );
      } else if( function == "laplace" ) {
         laplace( in, out, nrhs, prhs );
      } else if( function == "dgg" ) {
         dgg( in, out, nrhs, prhs );
      } else if( function == "laplace_min_dgg" ) {
         laplace_min_dgg( in, out, nrhs, prhs );
      } else if( function == "laplace_plus_dgg" ) {
         laplace_plus_dgg( in, out, nrhs, prhs );
      } else if( function == "gabor" ) {
         gabor( in, out, nrhs, prhs );
      } else if( function == "loggabor" ) {
         loggabor( in, out, nrhs, prhs );
      } else if( function == "normconv" ) {
         normconv( in, out, nrhs, prhs );
      } else if( function == "unif" ) {
         unif( in, out, nrhs, prhs );

      } else if( function == "bilateralf" ) {
         bilateralf( in, out, nrhs, prhs );
      } else if( function == "ced" ) {
         ced( in, out, nrhs, prhs );
      } else if( function == "gaussf_adap" ) {
         gaussf_adap( in, out, nrhs, prhs );
      } else if( function == "gaussf_adap_banana" ) {
         gaussf_adap_banana( in, out, nrhs, prhs );
      } else if( function == "kuwahara" ) {
         kuwahara( in, out, nrhs, prhs );
      } else if( function == "nonmaximumsuppression" ) {
         nonmaximumsuppression( in, out, nrhs, prhs );
      } else if( function == "percf" ) {
         percf( in, out, nrhs, prhs );
      } else if( function == "pmd" ) {
         pmd( in, out, nrhs, prhs );
      } else if( function == "pmd_gaussian" ) {
         pmd_gaussian( in, out, nrhs, prhs );
      } else if( function == "selectionf" ) {
         selectionf( in, out, nrhs, prhs );
      } else if( function == "varif" ) {
         varif( in, out, nrhs, prhs );

      } else if( function == "ft" ) {
         ft( in, out, nrhs, prhs );
      } else if( function == "riesz" ) {
         riesz( in, out, nrhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
