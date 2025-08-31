/*
 * (c)2017-2024, Cris Luengo.
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

#include "diplib/linear.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/dft.h"
#include "diplib/framework.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"
#include "diplib/overload.h"
#include "diplib/transform.h"

namespace dip {

namespace {

inline dip::uint HalfGaussianSize(
      dfloat sigma,
      dip::uint derivativeOrder,
      dfloat truncation,
      DataType dt // either DT_SFLOAT or DT_DFLOAT, not tested for
) {
   if( truncation <= 0 ) {
      truncation = 3; // The default value
   }
   double max_trunc = dt == DT_DFLOAT ? maximum_gauss_truncation< dfloat >() : maximum_gauss_truncation< sfloat >();
   truncation = std::min( truncation, max_trunc );
   truncation += 0.5 * static_cast< dfloat >( derivativeOrder );
   return clamp_cast< dip::uint >( std::ceil( truncation * sigma ));
}

std::vector< dfloat > MakeHalfGaussianInternal(
      dfloat sigma,
      dip::uint derivativeOrder,
      dfloat truncation,
      DataType dt // either DT_SFLOAT or DT_DFLOAT, not tested for
) {
   dip::uint halfFilterSize = 1 + HalfGaussianSize( sigma, derivativeOrder, truncation, dt );
   if( derivativeOrder > 2 && halfFilterSize < 2 ) {
      halfFilterSize = 2;
   }
   std::vector< dfloat > filter( halfFilterSize );
   dip::uint r0 = halfFilterSize - 1;
   dfloat sigma2 = sigma * sigma;
   switch( derivativeOrder ) {
      case 0: {
         dfloat factor = -0.5 / sigma2;
         dfloat normalization = 0;
         filter[ r0 ] = 1.0;
         for( dip::uint rr = 1; rr < halfFilterSize; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat g = std::exp( factor * ( rad * rad ));
            filter[ r0 - rr ] = g;
            normalization += g;
         }
         normalization = 1.0 / ( normalization * 2 + 1 );
         for( dip::uint rr = 0; rr < halfFilterSize; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 1: {
         dfloat factor = -0.5 / sigma2;
         dfloat moment = 0.0;
         filter[ r0 ] = 0.0;
         for( dip::uint rr = 1; rr < halfFilterSize; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat g = rad * std::exp( factor * ( rad * rad ));
            filter[ r0 - rr ] = g;
            moment += rad * g;
         }
         dfloat normalization = 1.0 / ( 2.0 * moment );
         for( dip::uint rr = 0; rr < halfFilterSize - 1; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 2: {
         dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigma * sigma2 );
         dfloat mean = 0.0;
         filter[ r0 ] = -norm;
         for( dip::uint rr = 1; rr < halfFilterSize; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat sr2 = rad * rad / sigma2;
            dfloat g = ( sr2 - 1.0 ) * norm * std::exp( -0.5 * sr2 );
            filter[ r0 - rr ] = g;
            mean += g;
         }
         mean = ( mean * 2.0 + filter[ r0 ] ) / ( static_cast< dfloat >( r0 ) * 2.0 + 1.0 );
         filter[ r0 ] -= mean;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < halfFilterSize; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            filter[ r0 - rr ] -= mean;
            moment += rad * rad * filter[ r0 - rr ];
         }
         dfloat normalization = 1.0 / moment;
         for( dip::uint rr = 0; rr < halfFilterSize; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 3: {
         dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigma * sigma2 * sigma2 );
         filter[ r0 ] = 0.0;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < halfFilterSize; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat rr2 = rad * rad;
            dfloat sr2 = rr2 / sigma2;
            dfloat g = norm * std::exp( -0.5 * sr2 ) * ( rad * ( 3.0 - sr2 ));
            filter[ r0 - rr ] = g;
            moment += g * rr2 * rad;
         }
         dfloat normalization = 3.0 / moment;
         for( dip::uint rr = 0; rr < halfFilterSize; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      default:
         DIP_THROW( E::NOT_IMPLEMENTED );
   }
   return filter;
}

} // namespace

// Creates a half Gaussian kernel, with the x=0 at the right end (last element) of the output array.
std::vector< dfloat > MakeHalfGaussian(
      dfloat sigma,
      dip::uint derivativeOrder,
      dfloat truncation,
      DataType dt
) {
   // Handle sigma == 0.0
   if( sigma == 0.0 ) {
      return { 1.0 };
   }
   // Create half Gaussian
   std::vector< dfloat > gaussian;
   DIP_STACK_TRACE_THIS( gaussian = MakeHalfGaussianInternal( sigma, derivativeOrder, truncation, dt ));
   return gaussian;
}

// Create 1D full Gaussian
std::vector< dfloat > MakeGaussian(
      dfloat sigma,
      dip::uint derivativeOrder,
      dfloat truncation,
      DataType dt
) {
   // Handle sigma == 0.0
   if( sigma == 0.0 ) {
      return { 1.0 };
   }
   // Create half Gaussian
   std::vector< dfloat > gaussian;
   DIP_STACK_TRACE_THIS( gaussian = MakeHalfGaussianInternal( sigma, derivativeOrder, truncation, dt ));
   dip::uint halfFilterSize = gaussian.size() - 1;
   // Complete the Gaussian
   gaussian.resize( halfFilterSize * 2 + 1 );
   dfloat symmetrySign = ( derivativeOrder & 1 ) ? -1.0 : 1.0;
   for( dip::uint ii = 1; ii <= halfFilterSize; ++ii ) {
      gaussian[ halfFilterSize + ii ] = symmetrySign * gaussian[ halfFilterSize - ii ];
   }
   return gaussian;
}

void CreateGauss(
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray orders,
      dfloat truncation,
      UnsignedArray exponents,
      String const& extent
) {
   // Verify dimensionality
   dip::uint nDims = sigmas.size();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( orders, nDims, dip::uint( 0 )));
   DIP_STACK_TRACE_THIS( ArrayUseParameter( exponents, nDims, dip::uint( 0 )));

   bool full{};
   DIP_STACK_TRACE_THIS( full = BooleanFromString( extent, "full", "half" ));

   // Create 1D gaussian for each dimension
   std::vector< std::vector< dfloat >> gaussians( nDims );
   UnsignedArray outSizes( nDims );
   UnsignedArray centers( nDims );

   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_STACK_TRACE_THIS( gaussians[ ii ] = full ? MakeGaussian( sigmas[ ii ], orders[ ii ], truncation, DT_DFLOAT ) : MakeHalfGaussian( sigmas[ ii ], orders[ ii ], truncation, DT_DFLOAT ));
      dip::uint gaussianLength = gaussians[ ii ].size();
      outSizes[ ii ] = gaussianLength;
      centers[ ii ] = full ? ( gaussianLength - 1 ) / 2 : gaussianLength - 1;
   }

   // Create output image
   out.ReForge( outSizes, 1, DT_DFLOAT );
   ImageIterator< dfloat > itOut( out );
   do {
      UnsignedArray const& coords = itOut.Coordinates();
      // Multiply Gaussians
      dfloat value = 1.0;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         value *= gaussians[ ii ][ coords[ ii ]];
         // Add moments
         if( exponents[ ii ] > 0 ) {
            dfloat v = static_cast< dfloat >( coords[ ii ] ) - static_cast< dfloat >( centers[ ii ] );
            value *= exponents[ ii ] > 1 ? std::pow( v, exponents[ ii ] ) : v;
         }
      }
      *itOut = value;
   } while( ++itOut );
}

void GaussFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
   DIP_END_STACK_TRACE
   OneDimensionalFilterArray filter( nDims );
   BooleanArray process( nDims, true );
   DataType computeType = in.DataType().IsA( DataType::Class_DFloat + DataType::Class_DComplex ) ? DT_DFLOAT : DT_SFLOAT;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( sigmas[ ii ] > 0.0 ) && ( in.Size( ii ) > 1 )) {
         bool found = false;
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            if( process[ jj ] && ( sigmas[ jj ] == sigmas[ ii ] ) && ( order[ jj ] == order[ ii ] )) {
               filter[ ii ] = filter[ jj ];
               found = true;
               break;
            }
         }
         if( !found ) {
            switch( order[ ii ] ) {
               case 0:
               case 2:
                  filter[ ii ].symmetry = S::EVEN;
                  break;
               case 1:
               case 3:
                  filter[ ii ].symmetry = S::ODD;
                  break;
               default:
                  DIP_THROW( "Gaussian FIR filter not implemented for order > 3" );
            }
            filter[ ii ].filter = MakeHalfGaussian( sigmas[ ii ], order[ ii ], truncation, computeType );
            // NOTE: origin defaults to the middle of the filter, so we don't need to set it explicitly here.
         }
      } else {
         process[ ii ] = false;
      }
   }
   SeparableConvolution( in, out, filter, boundaryCondition, process );
}


namespace {

template< typename TPI > // TPI is always complex (either scomplex or dcomplex)
class GaussFTLineFilter : public Framework::ScanLineFilter {
   public:
      using TPIf = FloatType< TPI >;
      GaussFTLineFilter( UnsignedArray const& sizes, FloatArray const& sigmas, UnsignedArray const& order, dfloat truncation ) {
         dip::uint nDims = sizes.size();
         gaussLUTs_.resize( nDims );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            bool found = false;
            for( dip::uint jj = 0; jj < ii; ++jj ) {
               if(( sizes[ jj ] == sizes[ ii ] ) && ( sigmas[ jj ] == sigmas[ ii ] ) && ( order[ jj ] == order[ ii ] )) {
                  gaussLUTs_[ ii ] = gaussLUTs_[ jj ];
                  found = true;
                  break;
               }
            }
            if( !found ) {
               gaussLUTs_[ ii ].resize( sizes[ ii ], TPI( 0 ));
               TPI* lut = gaussLUTs_[ ii ].data();
               // (( i*2*pi ) * x / size )^o * exp( -0.5 * (( 2*pi * sigma ) * x / size )^2 ) == a * x^o * exp( b * x^2 )
               dip::sint origin = static_cast< dip::sint >( sizes[ ii ] ) / 2;
               TPIf b = static_cast< TPIf >( 2.0 * pi * sigmas[ ii ] ) / static_cast< TPIf >( sizes[ ii ] );
               b = -TPIf( 0.5 ) * b * b;
               dip::uint N = ( b == 0 )
                             ? sizes[ ii ]
                             : HalfGaussianSize( static_cast< dfloat >( sizes[ ii ] ) / ( 2.0 * pi * sigmas[ ii ] ), order[ ii ], truncation, DataType( TPIf{} ));
               dip::sint begin = std::max( dip::sint( 0 ), origin - static_cast< dip::sint >( N ));
               dip::sint end = std::min( static_cast< dip::sint >( sizes[ ii ] ), origin + static_cast< dip::sint >( N ) + 1 );
               if( order[ ii ] > 0 ) {
                  TPIf o = static_cast< TPIf >( order[ ii ] );
                  TPI a = { 0, static_cast< TPIf >( 2.0 * pi ) / static_cast< TPIf >( sizes[ ii ] ) };
                  a = std::pow( a, o );
                  if( b != 0 ) {
                     lut += begin;
                     for( dip::sint jj = begin; jj < end; ++jj ) {
                        TPIf x = static_cast< TPIf >( jj - origin ); // narrowing conversion!
                        *lut = a * std::pow( x, o ) * std::exp( b * x * x );
                        ++lut;
                     }
                  } else {
                     lut += begin;
                     for( dip::sint jj = begin; jj < end; ++jj ) {
                        TPIf x = static_cast< TPIf >( jj - origin ); // narrowing conversion!
                        *lut = a * std::pow( x, o );
                        ++lut;
                     }
                  }
               } else {
                  if( b != 0 ) {
                     lut += begin;
                     for( dip::sint jj = begin; jj < end; ++jj ) {
                        TPIf x = static_cast< TPIf >( jj - origin ); // narrowing conversion!
                        *lut = std::exp( b * x * x );
                        ++lut;
                     }
                  } else {
                     std::fill( lut, lut + sizes[ ii ], TPI( 1 ));
                  }
               }
            }
         }
      }
      dip::uint GetNumberOfOperations( dip::uint /*nInput*/, dip::uint /*nOutput*/, dip::uint /*nTensorElements*/ ) override { return 3; } // not counting initialization
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         auto bufferLength = params.bufferLength;
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         TPI weight = 1;
         dip::uint procDim = params.dimension;
         for( dip::uint ii = 0; ii < gaussLUTs_.size(); ++ii ) {
            if( ii != procDim ) {
               weight *= gaussLUTs_[ ii ][ params.position[ ii ] ];
            }
         }
         TPI const* lut = gaussLUTs_[ procDim ].data();
         lut += params.position[ procDim ];
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            *out = *in * weight * *lut;
            in += inStride;
            out += outStride;
            ++lut;
         }
      }
   private:
      std::vector< std::vector< TPI >> gaussLUTs_;
};

dip::Image ExpandInput( dip::Image const& in, FloatArray const& sigmas, UnsignedArray const& order, dfloat truncation, StringArray const& boundaryCondition ) {
   BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
   UnsignedArray sizes = in.Sizes();
   DIP_ASSERT( sigmas.size() == sizes.size() );  // Our caller has already expanded these arrays to be the right size.
   DIP_ASSERT( order.size() == sizes.size() );
   dip::uint maxFactor = MaxFactor( in.DataType().IsComplex() );
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      sizes[ ii ] += 2 * HalfGaussianSize( sigmas[ ii ], order[ ii ], truncation, in.DataType() );
      sizes[ ii ] = GetOptimalDFTSize( sizes[ ii ], true, maxFactor );
   }
   dip::Image out;
   ExtendImageToSize( in, out, sizes, Option::CropLocation::CENTER, bc );
   return out;
}

} // namespace

void GaussFT(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      dfloat truncation,
      String const& inRepresentation,
      String const& outRepresentation,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   bool inSpatial{};
   DIP_STACK_TRACE_THIS( inSpatial = BooleanFromString( inRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool outSpatial{};
   DIP_STACK_TRACE_THIS( outSpatial = BooleanFromString( outRepresentation, S::SPATIAL, S::FREQUENCY ));
   if( !inSpatial ) {
      DIP_THROW_IF( !in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   }
   bool expanded = false;
   UnsignedArray originalSizes = in.Sizes(); // NOLINT(*-unnecessary-copy-initialization)
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
   DIP_END_STACK_TRACE
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( in.Size( ii ) == 1 ) {
         sigmas[ ii ] = 0;
         order[ ii ] = 0;
      } else {
         if( sigmas[ ii ] < 0 ) {
            sigmas[ ii ] = 0; // no smoothing
         }
      }
   }
   if( !sigmas.any() && !order.any() ) {
      DIP_START_STACK_TRACE
      if( inSpatial == outSpatial ) {
         out = in;
      } else if( inSpatial ) {
         FourierTransform( in, out );
      } else {
         FourierTransform( in, out, { S::INVERSE } );
      }
      return;
      DIP_END_STACK_TRACE
   }
   bool real = false;
   Image inFT;
   bool reuseInFT = false;
   if( inSpatial ) {
      real = !in.DataType().IsComplex();
      expanded = !boundaryCondition.empty();
      dip::Image const& tmp = expanded ? ExpandInput( in, sigmas, order, truncation, boundaryCondition ) : in;
      DIP_STACK_TRACE_THIS( inFT = FourierTransform( tmp ));
      reuseInFT = true;
   } else {
      inFT = in.QuickCopy();
      inFT.SetPixelSize( in.PixelSize() );
   }
   DataType dtype = inFT.DataType(); // a complex type
   Image outFT;
   if( !outSpatial || !real ) { // write directly into out if out is not real-valued
      DIP_STACK_TRACE_THIS( out.ReForge( inFT, dtype ));
      outFT = out.QuickCopy();
   } else {
      if( reuseInFT ) {
         outFT = inFT.QuickCopy();
      } // else outFT will be a new temporary
   }
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_NEW_COMPLEX( scanLineFilter, GaussFTLineFilter, ( inFT.Sizes(), sigmas, order, truncation ), dtype );
   Framework::ScanMonadic(
         inFT, outFT, dtype, dtype, 1, *scanLineFilter,
         Framework::ScanOption::TensorAsSpatialDim + Framework::ScanOption::NeedCoordinates );
   if( outSpatial ) {
      StringSet options{ S::INVERSE };
      if( real ) {
         options.insert( S::REAL );
      }
      if( expanded ) {
         dip::Image tmp;
         DIP_STACK_TRACE_THIS( FourierTransform( outFT, tmp, options ));
         out = tmp.Crop( originalSizes );
      } else {
         DIP_STACK_TRACE_THIS( FourierTransform( outFT, out, options ));
      }
   } else {
      out.SetPixelSize( inFT.PixelSize() );
   }
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE("[DIPlib] testing the Gaussian filters") {

   // Test smoothing for the 3 filters
   dip::Image img{ dip::UnsignedArray{ 256 }, 1, dip::DT_DFLOAT };
   img.Fill( 0.0 );
   img.At( 128 ) = 1.0;
   dip::dfloat sigma = 5.0;
   dip::dfloat amplitude = 1.0 / ( std::sqrt( 2.0 * dip::pi ) * sigma );
   dip::Image ft = dip::GaussFT( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( ft.At( 128 ).As< dip::dfloat >() - amplitude ) < 0.00015 );
   DOCTEST_CHECK( dip::Sum( ft ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   dip::Image fir = dip::GaussFIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( dip::testing::CompareImages( fir, ft, 0.0006 ));
   dip::Image iir = dip::GaussIIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( dip::testing::CompareImages( iir, ft, 0.0015 ));

   // Test first derivative for the 3 filters
   dip::ImageIterator< dip::dfloat > it( img );
   for( dip::dfloat x = -128; it; ++it, ++x ) {
      *it = x;
   }
   ft = dip::GaussFT( img, { sigma }, { 1 } );
   DOCTEST_CHECK( std::abs( ft.At( 128 ).As< dip::dfloat >() - 1.0 ) < 0.0015 ); // Affected by edge effects?
   fir = dip::GaussFIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( fir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( iir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));

   // Test second derivative for the 3 filters
   img = img * img;
   ft = dip::GaussFT( img, { sigma }, { 2 } );
   DOCTEST_CHECK( std::abs( ft.At( 128 ).As< dip::dfloat >() - 2.0 ) < 0.0005 );
   fir = dip::GaussFIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( fir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( iir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));

   // Test we can protect the output image (issue #170)
   dip::Image out = img.Similar();
   out.Protect();
   DOCTEST_CHECK_NOTHROW( dip::GaussFT( img, out, { sigma } ));
   DOCTEST_CHECK_NOTHROW( dip::GaussFIR( img, out, { sigma } ));
   DOCTEST_CHECK_NOTHROW( dip::GaussIIR( img, out, { sigma } ));
   DOCTEST_CHECK_NOTHROW( dip::Gradient( img, out, { .79 } ));
}

#ifdef _OPENMP

#include "diplib/multithreading.h"
#include "diplib/random.h"

DOCTEST_TEST_CASE("[DIPlib] testing the separable framework under multithreading") {

   // Compute using one thread
   dip::SetNumberOfThreads( 1 );

   // Generate test image
   dip::Image img{ dip::UnsignedArray{ 256, 192, 59 }, 1, dip::DT_DFLOAT };
   img.Fill( 0 );
   dip::Random random( 0 );
   dip::GaussianNoise( img, img, random );

   // Apply separable filter using one thread
   dip::Image out1 = dip::Gauss( img, { 2 } );

   // Reset number of threads
   dip::SetNumberOfThreads( 0 );

   // Apply separable filter using all threads
   dip::Image out2 = dip::Gauss( img, { 2 } );

   // Compare
   DOCTEST_CHECK( dip::testing::CompareImages( out1, out2, dip::Option::CompareImagesMode::EXACT ));
}

#endif // _OPENMP

#endif // DIP_CONFIG_ENABLE_DOCTEST
