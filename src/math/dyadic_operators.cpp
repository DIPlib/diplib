/*
 * (c)2017-2025, Cris Luengo.
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

#include "diplib/math.h"

#include <memory>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

constexpr char const* IMAGE_ARRAY_TOO_SMALL = "Need at least two input images";

} // namespace

void Atan2( Image const& y, Image const& x, Image& out ) {
   DIP_THROW_IF( !y.IsScalar() || !x.IsScalar(), E::IMAGE_NOT_SCALAR );
   DataType dt = DataType::SuggestArithmetic( y.DataType(), x.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return std::atan2( *its[ 0 ], *its[ 1 ] ); }, 20
   ), dt );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { y, x }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));
}

void Hypot( Image const& a, Image const& b, Image& out ) {
   DIP_THROW_IF( !a.IsScalar() || !b.IsScalar(), E::IMAGE_NOT_SCALAR );
   DataType dt = DataType::SuggestArithmetic( a.DataType(), b.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return std::hypot( *its[ 0 ], *its[ 1 ] ); }, 20 // rough guess at the cost
   ), dt );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { a, b }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));
}

namespace {

template< typename TPI, typename F >
class MultiScanLineFilter : public Framework::ScanLineFilter {
   public:
      MultiScanLineFilter( F const& func ) : func_( func ) {}
      dip::uint GetNumberOfOperations( dip::uint nInput, dip::uint /**/, dip::uint /**/ ) override { return nInput; } // assuming this is only used for Supremum and Infimum!
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint N = params.inBuffer.size();
         dip::uint const bufferLength = params.bufferLength;
         std::vector< TPI const* > in( N );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            in[ ii ] = static_cast< TPI const* >( params.inBuffer[ ii ].buffer );
         }
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            TPI res = *in[ 0 ];
            in[ 0 ] += params.inBuffer[ 0 ].stride;
            for( dip::uint ii = 1; ii < N; ++ii ) {
               res = func_( res, *in[ ii ] );
               in[ ii ] += params.inBuffer[ ii ].stride;
            }
            *out = res;
            out += outStride;
         }
      }
   private:
      F func_;
};

template< typename TPI, typename F >
inline std::unique_ptr< Framework::ScanLineFilter > NewMultiScanLineFilter( F const& func ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new MultiScanLineFilter< TPI, F >( func ));
}

} // namespace

void Supremum( ImageConstRefArray const& in, Image& out ) {
   DIP_THROW_IF( in.size() < 2, IMAGE_ARRAY_TOO_SMALL );
   DataType dt = in[ 0 ].get().DataType();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      dt = DataType::SuggestDyadicOperation( dt, in[ ii ].get().DataType() );
   }
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewMultiScanLineFilter, (
         []( auto a, auto b ) { return std::max( a, b ); }
   ), dt );
   ImageRefArray outar{ out };
   DataTypeArray buftypes( in.size(), dt );
   DIP_STACK_TRACE_THIS( Framework::Scan( in, outar, buftypes, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));
}

void Infimum( ImageConstRefArray const& in, Image& out ) {
   DIP_THROW_IF( in.size() < 2, IMAGE_ARRAY_TOO_SMALL );
   DataType dt = in[ 0 ].get().DataType();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      dt = DataType::SuggestDyadicOperation( dt, in[ ii ].get().DataType() );
   }
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewMultiScanLineFilter, (
         []( auto a, auto b ) { return std::min( a, b ); }
   ), dt );
   ImageRefArray outar{ out };
   DataTypeArray buftypes( in.size(), dt );
   DIP_STACK_TRACE_THIS( Framework::Scan( in, outar, buftypes, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));
}

void SignedInfimum( Image const& a, Image const& b, Image& out ) {
   DataType dt = DataType::SuggestSigned( a.DataType() );
   dt = DataType::SuggestDyadicOperation( dt, b.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_SIGNEDREAL( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return *its[ 0 ] > *its[ 1 ] ? static_cast< decltype( *its[ 1 ] ) >( -( *its[ 1 ] )) : *its[ 0 ]; }
   ), dt );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { a, b }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));
}

namespace {

template< typename TPI >
class LinearCombinationScanLineFilter: public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 2; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint const bufferLength = params.bufferLength;
         TPI const* a = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint const aStride = params.inBuffer[ 0 ].stride;
         TPI const* b = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         dip::sint const bStride = params.inBuffer[ 1 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = *a * a_weight_ + *b * b_weight_;
            a += aStride;
            b += bStride;
            out += outStride;
         }
      }
      LinearCombinationScanLineFilter( dfloat aWeight, dfloat bWeight ) :
            a_weight_( static_cast< FloatType< TPI >>( aWeight )),
            b_weight_( static_cast< FloatType< TPI >>( bWeight )) {}
      LinearCombinationScanLineFilter( dcomplex aWeight, dcomplex bWeight ):
            a_weight_( static_cast< TPI >( aWeight )),
            b_weight_( static_cast< TPI >( bWeight )) {} // When we use complex weights, TPI is a complex type
   private:
      TPI a_weight_;
      TPI b_weight_;
};

}

void LinearCombination( Image const& a, Image const& b, Image& out, dfloat aWeight, dfloat bWeight ) {
   DataType dt = DataType::SuggestArithmetic( a.DataType(), b.DataType() );
   if( dt.IsBinary() ) {
      dt = DT_SFLOAT;   // Suggest arithmetic will return DT_BIN in both inputs are binary, but we don't want to do binary arithmetic here.
   }
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_FLEX( scanLineFilter, LinearCombinationScanLineFilter, ( aWeight, bWeight ), dt );
   DIP_STACK_TRACE_THIS( Framework::ScanDyadic( a, b, out, dt, dt, dt, *scanLineFilter ));
}

void LinearCombination( Image const& a, Image const& b, Image& out, dcomplex aWeight, dcomplex bWeight ) {
   DataType dt = DataType::SuggestArithmetic( DataType::SuggestComplex( a.DataType() ), DataType::SuggestComplex( b.DataType() ));
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_COMPLEX( scanLineFilter, LinearCombinationScanLineFilter, ( aWeight, bWeight ), dt );
   DIP_STACK_TRACE_THIS( Framework::ScanDyadic( a, b, out, dt, dt, dt, *scanLineFilter ));
}

void AlphaBlend(
      Image const& c_in,
      Image const& c_overlay,
      Image const& c_alpha,
      Image& out
) {
   DIP_THROW_IF( !c_in.IsForged() || !c_overlay.IsForged() || !c_alpha.IsForged(), E::IMAGE_NOT_FORGED );
   dip::UnsignedArray sizes;
   DIP_STACK_TRACE_THIS( sizes = Framework::SingletonExpandedSize( ImageConstRefArray{ c_in, c_overlay, c_alpha } ));
   DIP_THROW_IF( !c_alpha.IsScalar(), E::MASK_NOT_SCALAR );
   auto tensor = c_in.Tensor();
   auto otherTensor = c_overlay.Tensor();
   if( tensor.Elements() == 1 ) {
      tensor = otherTensor;
   } else {
      DIP_THROW_IF( otherTensor.Elements() != 1 && otherTensor.Elements() != tensor.Elements(), E::NTENSORELEM_DONT_MATCH );
   }
   if( out.IsForged() && out.IsSingletonExpanded() ) {
      // This could happen if &out == &c_in.
      DIP_STACK_TRACE_THIS( out.Strip() );
   }
   Image in = c_in;
   Image overlay = c_overlay.QuickCopy();
   Image alpha = c_alpha.QuickCopy();
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, tensor.Elements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW ));
   out.Copy( in.ExpandSingletonDimensions( sizes ));
   out *= ( 1 - alpha );
   dip::Image tmp = overlay * alpha;
   out += tmp;
   out.ReshapeTensor( tensor );
   out.SetPixelSize( in.PixelSize() );
   out.SetColorSpace( in.ColorSpace() );
}

} // namespace dip
