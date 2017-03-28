/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the Gaussian filter.
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
#include "diplib/linear.h"
#include <diplib/framework.h>
#include <diplib/overload.h>
#include <diplib/transform.h>

namespace dip {

namespace {

inline dip::uint HalfGaussianSize(
      dfloat sigma,
      dip::uint order,
      dfloat truncation
) {
   return static_cast< dip::uint >( std::ceil( ( truncation + 0.5 * order ) * sigma ));
}

// Creates a half Gaussian kernel, with the x=0 at the right end (last element) of the output array.
FloatArray MakeHalfGaussian(
      dfloat sigma,
      dip::uint order,
      dip::uint length // the length of the array
) {
   FloatArray filter( length );
   dip::uint r0 = length - 1;
   switch( order ) {
      case 0: {
         dfloat factor = -1.0 / ( 2.0 * sigma * sigma );
         //dfloat norm = 1.0 / ( sqrt( 2.0 * pi ) * sigma ); // There's no point in computing this if we normalize later!
         dfloat normalization = 0;
         filter[ r0 ] = 1.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat g = exp( factor * ( rr * rr ) );
            filter[ r0 - rr ] = g;
            normalization += g;
         }
         normalization = 1.0 / ( normalization * 2 + 1 );
         for( dip::uint rr = 0; rr < length; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 1: {
         dfloat factor = -1.0 / ( 2.0 * sigma * sigma );
         dfloat moment = 0.0;
         filter[ r0 ] = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat g = rr * exp( factor * ( rr * rr ) );
            filter[ r0 - rr ] = g;
            moment += rr * g;
         }
         dfloat normalization = 1.0 / ( 2.0 * moment );
         for( dip::uint rr = 0; rr < length - 1; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 2: {
         dfloat sigma2 = sigma * sigma;
         dfloat sigma4 = sigma2 * sigma2;
         dfloat norm = 1.0 / ( sqrt( 2.0 * pi ) * sigma );
         dfloat mean = 0.0;
         filter[ r0 ] = ( -1.0 / sigma2 ) * norm;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rr2 = rr * rr;
            dfloat g = ( ( -1.0 / sigma2 ) + ( rr2 ) / sigma4 ) * norm * exp( -( rr2 ) / ( 2.0 * sigma2 ) );
            filter[ r0 - rr ] = g;
            mean += g;
         }
         mean = ( mean * 2.0 + filter[ r0 ] ) / ( r0 * 2.0 + 1.0 );
         filter[ r0 ] -= mean;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            filter[ r0 - rr ] -= mean;
            moment += rr * rr * filter[ r0 - rr ];
         }
         dfloat normalization = 1.0 / moment;
         for( dip::uint rr = 0; rr < length; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 3: {
         dfloat sigma2 = sigma * sigma;
         dfloat sigma4 = sigma2 * sigma2;
         dfloat sigma6 = sigma4 * sigma2;
         dfloat norm = 1.0 / ( sqrt( 2.0 * pi ) * sigma );
         filter[ r0 ] = 0.0;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat r2 = rr * rr;
            dfloat g = norm * exp( -r2 / ( 2.0 * sigma2 ) ) * ( rr * ( 3.0 * sigma2 - r2 ) / sigma6 );
            filter[ r0 - rr ] = g;
            moment += g * r2 * rr;
         }
         dfloat normalization = 3.0 / moment;
         for( dip::uint rr = 0; rr < length; rr++ ) {
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

void GaussFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      StringArray const& boundaryCondition,
      BooleanArray process,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
      ArrayUseParameter( process, nDims, true );
   DIP_END_STACK_TRACE
   OneDimensionalFilterArray filter( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         bool found = false;
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            if( process[ jj ] && ( sigmas[ jj ] == sigmas[ ii ] ) && ( order[ jj ] == order[ ii ] )) {
               filter[ ii ] = filter[ jj ];
               found = true;
               break;
            }
         }
         if( !found ) {
            if( sigmas[ ii ] <= 0 ) {
               process[ ii ] = false;
            } else {
               switch( order[ ii ] ) {
                  case 0:
                  case 2:
                     filter[ ii ].symmetry = "even";
                     break;
                  case 1:
                  case 3:
                     filter[ ii ].symmetry = "odd";
                     break;
                  default:
                     DIP_THROW( "Gaussian FIR filter not implemented for order > 3" );
               }
               filter[ ii ].filter = MakeHalfGaussian(
                     sigmas[ ii ], order[ ii ],
                     HalfGaussianSize( sigmas[ ii ], order[ ii ], truncation ) + 1 );
               // NOTE: origin defaults to the middle of the filter, so we don't need to set it explicitly here.
            }
         }
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
         gaussLUTs.resize( nDims );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            bool found = false;
            for( dip::uint jj = 0; jj < ii; ++jj ) {
               if(( sizes[ jj ] == sizes[ ii ] ) && ( sigmas[ jj ] == sigmas[ ii ] ) && ( order[ jj ] == order[ ii ] )) {
                  gaussLUTs[ ii ] = gaussLUTs[ jj ];
                  found = true;
                  break;
               }
            }
            if( !found ) {
               gaussLUTs[ ii ].resize( sizes[ ii ], TPI( 0 ));
               TPI* lut = gaussLUTs[ ii ].data();
               // ( (i*2*pi) * x / size )^o * exp( -0.5 * ( ( 2*pi * sigma ) * x / size )^2 ) == a * x^o * exp( b * x^2 )
               dip::sint origin = sizes[ ii ] / 2;
               TPIf b = static_cast< TPIf >( 2.0 * pi * sigmas[ ii ] / sizes[ ii ] );
               b = -TPIf( 0.5 ) * b * b;
               dip::uint N = b == 0 ? sizes[ ii ] : HalfGaussianSize( sizes[ ii ] / ( 2.0 * pi * sigmas[ ii ] ), order[ ii ], truncation );
               dip::sint begin = std::max< dip::sint >( 0, origin - N );
               dip::sint end = std::min< dip::sint >( sizes[ ii ], origin + N + 1 );
               if( order[ ii ] > 0 ) {
                  TPIf o = static_cast< TPIf >( order[ ii ] );
                  TPI a = { 0, static_cast< TPIf >( 2.0 * pi / sizes[ ii ] ) };
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
                     std::fill( lut, lut + sizes[ ii ], TPI( 1 ) );
                  }
               }
            }
         }
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         auto bufferLength = params.bufferLength;
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         TPI weight = 1;
         dip::uint offset = params.tensorToSpatial ? 1 : 0;
         for( dip::uint ii = offset; ii < params.position.size(); ++ii ) {
            if( ii != params.dimension ) {
               weight *= gaussLUTs[ ii - offset ][ params.position[ ii ] ];
            }
         }
         TPI const* lut = gaussLUTs[ params.dimension - offset ].data();
         lut += params.position[ params.dimension ];
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            *out = *in * weight * *lut;
            in += inStride;
            out += outStride;
            ++lut;
         }
      }
   private:
      std::vector< std::vector< TPI >> gaussLUTs;
};

} // namespace

void GaussFT(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      BooleanArray process,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
      ArrayUseParameter( process, nDims, true );
   DIP_END_STACK_TRACE
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( !process[ ii ] || ( in.Size( ii ) == 1 )) {
         sigmas[ ii ] = 0;
         order[ ii ] = 0;
      } else {
         if( sigmas[ ii ] < 0 ) {
            sigmas[ ii ] = 0;
         }
      }
   }
   if( sigmas.any() || order.any() ) {
      bool isreal = !in.DataType().IsComplex();
      Image ft = FourierTransform( in );
      DataType dtype = DataType::SuggestComplex( ft.DataType() );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_NEW_COMPLEX( scanLineFilter, GaussFTLineFilter, ( in.Sizes(), sigmas, order, truncation ), dtype );
      Framework::ScanMonadic(
            ft, ft, dtype, dtype, 1, *scanLineFilter,
            Framework::Scan_TensorAsSpatialDim + Framework::Scan_NeedCoordinates );
      StringSet opts = { "inverse" };
      if( isreal ) {
         opts.emplace( "real" );
      }
      FourierTransform( ft, out, opts );
   }
}

namespace {

void GaussDispatch(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray const& derivativeOrder,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   // If any( sigmas < 0.8 ) || any( derivativeOrder > 3 )  ==>  FT
   // Else if any( sigmas > 10 )  ==>  IIR
   // Else ==>  FIR
   for( dip::uint ii = 0; ii < derivativeOrder.size(); ++ii ) { // We can't fold this loop in with the next one, the two arrays might be of different size
      if( derivativeOrder[ ii ] > 3 ) {
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
         return;
      }
   }
   for( dip::uint ii = 0; ii < sigmas.size(); ++ii ) {
      if( sigmas[ ii ] < 0.8 ) {
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
         return;
      }
   }
   for( dip::uint ii = 0; ii < sigmas.size(); ++ii ) {
      if( sigmas[ ii ] > 10 ) {
         GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, {}, "", truncation );
         return;
      }
   }
   GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
}

} // namespace

void Gauss(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray const& derivativeOrder,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      String method,
      dfloat truncation
) {
   if( method == "best" ) {
      DIP_START_STACK_TRACE
         GaussDispatch( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "FIR" ) || ( method == "fir" ) ) {
      DIP_START_STACK_TRACE
         GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "FT" ) || ( method == "ft" ) ) {
      DIP_START_STACK_TRACE
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
      DIP_END_STACK_TRACE
   } else if( ( method == "IIR" ) || ( method == "iir" ) ) {
      DIP_START_STACK_TRACE
         GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, {}, "", truncation );
      DIP_END_STACK_TRACE
   } else {
      DIP_THROW( "Unknown Gauss filter method" );
   }
}

void Derivative(
      Image const& in,
      Image& out,
      UnsignedArray const& derivativeOrder,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   if( method == "finitediff" ) {
      // Set process to false where sigma <= 0 and derivativeOrder == 0;
      BooleanArray ps = process;
      FloatArray ss = sigmas;
      UnsignedArray order = derivativeOrder;
      dip::uint nDims = in.Dimensionality();
      DIP_START_STACK_TRACE
         ArrayUseParameter( ps, nDims, true );
         ArrayUseParameter( ss, nDims, 1.0 );
         ArrayUseParameter( order, nDims, dip::uint( 0 ));
      DIP_END_STACK_TRACE
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if((( ss[ ii ] <= 0.0 ) && ( order[ ii ] == 0 )) || ( in.Size( ii ) == 1 )) {
            ps[ ii ] = 0;
         }
      }
      DIP_START_STACK_TRACE
         FiniteDifference( in, out, order, "smooth", boundaryCondition, ps );
      DIP_END_STACK_TRACE
   } else if( ( method == "best" ) || ( method == "gauss" ) ) {
      DIP_START_STACK_TRACE
         GaussDispatch( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "gaussFIR" ) || ( method == "gaussfir" ) ) {
      DIP_START_STACK_TRACE
         GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "gaussFT" ) || ( method == "gaussft" ) ) {
      DIP_START_STACK_TRACE
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
      DIP_END_STACK_TRACE
   } else if( ( method == "gaussIIR" ) || ( method == "gaussiir" ) ) {
      DIP_START_STACK_TRACE
         GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, {}, "", truncation );
      DIP_END_STACK_TRACE
   } else {
      DIP_THROW( "Unknown derivative method" );
   }
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <random>
#include "diplib/math.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the Gaussian filter") {
   dip::Image img{ dip::UnsignedArray{ 256 }, 1, dip::DT_DFLOAT };
   img.Fill( 0.0 );
   img.At( 128 ) = 1.0;
   dip::dfloat sigma = 5.0;
   dip::dfloat amplitude = 1.0 / ( std::sqrt( 2.0 * dip::pi ) * sigma );
   dip::Image ft = dip::GaussFT( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( static_cast< dip::dfloat >( ft.At( 128 )) - amplitude ) < 0.00015 );
   DOCTEST_CHECK( static_cast< dip::dfloat >( dip::Sum( ft )) == doctest::Approx( 1.0 ));
   dip::Image fir = dip::GaussFIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( static_cast< dip::dfloat >( dip::Maximum( fir - ft ))) < 0.0003 );
   dip::Image iir = dip::GaussIIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( static_cast< dip::dfloat >( dip::Maximum( iir - ft ))) < 0.0015 );
   dip::ImageIterator< dip::dfloat > it( img );
   for( dip::dfloat x = -128; it; ++it, ++x ) {
      *it = x;
   }
   ft = dip::GaussFT( img, { sigma }, { 1 } );
   DOCTEST_CHECK( std::abs( static_cast< dip::dfloat >( ft.At( 128 )) - 1.0 ) < 0.0015 ); // Affected by edge effects?
   fir = dip::GaussFIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( static_cast< dip::dfloat >( fir.At( 128 )) == doctest::Approx( 1.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( static_cast< dip::dfloat >( iir.At( 128 )) == doctest::Approx( 1.0 ));
   img = img * img;
   ft = dip::GaussFT( img, { sigma }, { 2 } );
   DOCTEST_CHECK( std::abs( static_cast< dip::dfloat >( ft.At( 128 )) - 2.0 ) < 0.0005 );
   fir = dip::GaussFIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( static_cast< dip::dfloat >( fir.At( 128 )) == doctest::Approx( 2.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( static_cast< dip::dfloat >( iir.At( 128 )) == doctest::Approx( 2.0 ));
}

#endif // DIP__ENABLE_DOCTEST
