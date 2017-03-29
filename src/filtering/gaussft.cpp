/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the FIR and FT Gaussian filter.
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
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/transform.h"
#include "gauss.h"

namespace dip {

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

} // namespace dip
