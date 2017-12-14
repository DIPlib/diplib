/*
 * DIPlib 3.0
 * This file contains the definition the arithmetic, bitwise and comparison operators for Pixel
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
#include "diplib/overload.h"

namespace dip {

namespace {

DataType SuggestArithmetic( DataType type1, DataType type2 ) {
   if( type1.IsComplex() || type2.IsComplex() ) {
      return DT_DCOMPLEX;
   } else {
      return DT_DFLOAT;
   }
}

enum class ComputationType {
      Class_Flex,    // dfloat, dcomplex
      Class_Float,   // dfloat
      Class_Integer, // all integer types
      Class_IntOrBin // bin + all integer types
};

template< typename TPI, typename F >
void ApplyMonadicOperator( DataType in1Type, void* in1Ptr, DataType outType, void* outPtr, F const& function ) {
   TPI res = static_cast< TPI >( function( detail::CastSample< TPI >( in1Type, in1Ptr )));
   detail::CastSample( DataType( TPI( 0 )), &res, outType, outPtr );
}

template< ComputationType classes, typename F >
struct MonadicOperatorDispatch {
   static void Call( DataType in1Type, void* in1Ptr, DataType outType, void* outPtr, DataType computeType, F const& function ) {
      DIP_THROW( E::NOT_IMPLEMENTED );
   }
};
template< typename F >
struct MonadicOperatorDispatch< ComputationType::Class_Flex, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType outType, void* outPtr, DataType computeType, F const& function ) {
      switch( computeType ) {
         case dip::DT_DFLOAT: ApplyMonadicOperator< dip::dfloat >( in1Type, in1Ptr, outType, outPtr, function ); break;
         case dip::DT_DCOMPLEX: ApplyMonadicOperator< dip::dcomplex >( in1Type, in1Ptr, outType, outPtr, function ); break;
         default: DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
      }
   }
};
template< typename F >
struct MonadicOperatorDispatch< ComputationType::Class_Float, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType outType, void* outPtr, DataType computeType, F const& function ) {
      switch( computeType ) {
         case dip::DT_DFLOAT: ApplyMonadicOperator< dip::dfloat >( in1Type, in1Ptr, outType, outPtr, function ); break;
         default: DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
      }
   }
};
template< typename F >
struct MonadicOperatorDispatch< ComputationType::Class_Integer, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType outType, void* outPtr, DataType computeType, F const& function ) {
      DIP_OVL_CALL_INTEGER( ApplyMonadicOperator, ( in1Type, in1Ptr, outType, outPtr, function ), computeType );
   }
};
template< typename F >
struct MonadicOperatorDispatch< ComputationType::Class_IntOrBin, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType outType, void* outPtr, DataType computeType, F const& function ) {
      DIP_OVL_CALL_INT_OR_BIN( ApplyMonadicOperator, ( in1Type, in1Ptr, outType, outPtr, function ), computeType );
   }
};

template< ComputationType classes, typename F >
void CallMonadicOperator( DataType in1Type, void* in1Ptr, DataType outType, void* outPtr, DataType computeType, F const& function ) {
   MonadicOperatorDispatch< classes, F >::Call( in1Type, in1Ptr, outType, outPtr, computeType, function );
};

template< ComputationType classes, typename F >
Image::Pixel MonadicOperator(
      Image::Pixel const& in1,   // must have N or 1 tensor elements
      DataType computeType,      // one of the types in classes, otherwise throws
      DataType outType,
      F const& function
) {
   dip::uint N = in1.TensorElements();
   Image::Pixel out( outType, N ); // note: out.TensorStride() == 1
   out.ReshapeTensor( in1.Tensor() );
   dip::sint in1Sz = static_cast< dip::sint >( in1.DataType().SizeOf() );
   dip::sint outSz = static_cast< dip::sint >( out.DataType().SizeOf() );
   uint8* in1Ptr = static_cast< uint8* >( in1.Origin());
   uint8* outPtr = static_cast< uint8* >( out.Origin());
   for( dip::uint ii = 0; ii < N; ++ii ) {
      CallMonadicOperator< classes >(
            in1.DataType(), in1Ptr, outType, outPtr, computeType, function );
      in1Ptr += in1Sz;
      outPtr += outSz;
   }
   return out;
}

template< typename TPI, typename F >
void ApplyDyadicOperator( DataType in1Type, void* in1Ptr, DataType in2Type, void* in2Ptr,
                          DataType outType, void* outPtr, F const& function ) {
   TPI res = static_cast< TPI >( function(
         detail::CastSample< TPI >( in1Type, in1Ptr ),
         detail::CastSample< TPI >( in2Type, in2Ptr )));
   detail::CastSample( DataType( TPI( 0 )), &res, outType, outPtr );
}

template< ComputationType classes, typename F >
struct DyadicOperatorDispatch {
   static void Call( DataType in1Type, void* in1Ptr, DataType in2Type, void* in2Ptr,
                     DataType outType, void* outPtr, DataType computeType, F const& function ) {
      DIP_THROW( E::NOT_IMPLEMENTED );
   }
};
template< typename F >
struct DyadicOperatorDispatch< ComputationType::Class_Flex, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType in2Type, void* in2Ptr,
                     DataType outType, void* outPtr, DataType computeType, F const& function ) {
      switch( computeType ) {
         case dip::DT_DFLOAT: ApplyDyadicOperator< dip::dfloat >( in1Type, in1Ptr, in2Type, in2Ptr, outType, outPtr, function ); break;
         case dip::DT_DCOMPLEX: ApplyDyadicOperator< dip::dcomplex >( in1Type, in1Ptr, in2Type, in2Ptr, outType, outPtr, function ); break;
         default: DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
      }
   }
};
template< typename F >
struct DyadicOperatorDispatch< ComputationType::Class_Float, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType in2Type, void* in2Ptr,
                     DataType outType, void* outPtr, DataType computeType, F const& function ) {
      switch( computeType ) {
         case dip::DT_DFLOAT: ApplyDyadicOperator< dip::dfloat >( in1Type, in1Ptr, in2Type, in2Ptr, outType, outPtr, function ); break;
         default: DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
      }
   }
};
template< typename F >
struct DyadicOperatorDispatch< ComputationType::Class_Integer, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType in2Type, void* in2Ptr,
                     DataType outType, void* outPtr, DataType computeType, F const& function ) {
      DIP_OVL_CALL_INTEGER( ApplyDyadicOperator, ( in1Type, in1Ptr, in2Type, in2Ptr, outType, outPtr, function ), computeType );
   }
};
template< typename F >
struct DyadicOperatorDispatch< ComputationType::Class_IntOrBin, F > {
   static void Call( DataType in1Type, void* in1Ptr, DataType in2Type, void* in2Ptr,
                     DataType outType, void* outPtr, DataType computeType, F const& function ) {
      DIP_OVL_CALL_INT_OR_BIN( ApplyDyadicOperator, ( in1Type, in1Ptr, in2Type, in2Ptr, outType, outPtr, function ), computeType );
   }
};

template< ComputationType classes, typename F >
void CallDyadicOperator( DataType in1Type, void* in1Ptr, DataType in2Type, void* in2Ptr,
                         DataType outType, void* outPtr, DataType computeType, F const& function ) {
   DyadicOperatorDispatch< classes, F >::Call( in1Type, in1Ptr, in2Type, in2Ptr, outType, outPtr, computeType, function );
};

template< ComputationType classes, typename F >
Image::Pixel DyadicOperator(
      Image::Pixel const& in1,   // must have N or 1 tensor elements
      Image::Pixel const& in2,   // must have N or 1 tensor elements
      DataType computeType,      // one of the types in classes, otherwise throws
      DataType outType,
      F const& function
) {
   dip::uint N1 = in1.TensorElements();
   dip::uint N2 = in2.TensorElements();
   dip::uint N = std::max( N1, N2 );
   DIP_THROW_IF(( N1 != 1 ) && ( N1 != N ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF(( N2 != 1 ) && ( N2 != N ), E::NTENSORELEM_DONT_MATCH );
   Image::Pixel out( outType, N ); // note: out.TensorStride() == 1
   if( N1 == N ) {
      out.ReshapeTensor( in1.Tensor() );
   } else {
      out.ReshapeTensor( in2.Tensor() );
   }
   dip::sint in1Sz = static_cast< dip::sint >( in1.DataType().SizeOf() );
   dip::sint in2Sz = static_cast< dip::sint >( in2.DataType().SizeOf() );
   dip::sint outSz = static_cast< dip::sint >( out.DataType().SizeOf() );
   uint8* in1Ptr = static_cast< uint8* >( in1.Origin());
   uint8* in2Ptr = static_cast< uint8* >( in2.Origin());
   uint8* outPtr = static_cast< uint8* >( out.Origin());
   for( dip::uint ii = 0; ii < N; ++ii ) {
      CallDyadicOperator< classes >(
            in1.DataType(), in1Ptr, in2.DataType(), in2Ptr, outType, outPtr, computeType, function );
      in1Ptr += ( N1 > 1 ) ? ( in1Sz * in1.TensorStride() ) : ( 0 );
      in2Ptr += ( N2 > 1 ) ? ( in2Sz * in2.TensorStride() ) : ( 0 );
      outPtr += outSz;
   }
   return out;
}

} // namespace

//
// Arithmetic
//

Image::Pixel operator+( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   dip::DataType dt = SuggestArithmetic( lhs.DataType(), rhs.DataType() );
   return DyadicOperator< ComputationType::Class_Flex >(
         lhs, rhs, dt, dt,
         [ = ]( auto in1, auto in2 ) { return in1 + in2; }
   );
}

Image::Pixel operator-( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   dip::DataType dt = SuggestArithmetic( lhs.DataType(), rhs.DataType() );
   return DyadicOperator< ComputationType::Class_Flex >(
         lhs, rhs, dt, dt,
         [ = ]( auto in1, auto in2 ) { return in1 - in2; }
   );
}

Image::Pixel operator*( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   if(( lhs.TensorElements() == 1 ) || ( rhs.TensorElements() == 1 )) {
      dip::DataType dt = SuggestArithmetic( lhs.DataType(), rhs.DataType() );
      return DyadicOperator< ComputationType::Class_Flex >(
            lhs, rhs, dt, dt,
            [ = ]( auto in1, auto in2 ) { return in1 * in2; }
      );
   } else {
      // This one is complicated to implement, I don't really want to replicate what is already done for images.
      Image tmp;
      Multiply( Image{ lhs }, Image{ rhs }, tmp, SuggestArithmetic( lhs.DataType(), rhs.DataType() ));
      Image::Pixel out( tmp.DataType(), tmp.TensorElements() );
      out = tmp.At( 0 );
      out.ReshapeTensor( tmp.Tensor() );
      return out;
   }
}

Image::Pixel operator/( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   dip::DataType dt = SuggestArithmetic( lhs.DataType(), rhs.DataType() );
   return DyadicOperator< ComputationType::Class_Flex >(
         lhs, rhs, dt, dt,
         [ = ]( auto in1, auto in2 ) { return in1 / in2; }
   );
}

Image::Pixel operator%( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   if( lhs.DataType().IsFloat() ) {
      return DyadicOperator< ComputationType::Class_Float >(
            lhs, rhs, lhs.DataType(), lhs.DataType(),
            [ = ]( auto in1, auto in2 ) { return std::fmod( in1, in2 ); }
      );
   } else {
      return DyadicOperator< ComputationType::Class_Integer >(
            lhs, rhs, lhs.DataType(), lhs.DataType(),
            [ = ]( auto in1, auto in2 ) { return in1 % in2; }
      );
   }
}

Image::Pixel operator-( Image::Pixel const& in ) {
   dip::DataType dt = dip::DataType::SuggestFlex( in.DataType() );
   return MonadicOperator< ComputationType::Class_Flex >(
         in, dt, dt,
         [ = ]( auto in1 ) { return -in1; }
   );
}

//
// Bit-wise / Boolean
// To compute these correctly, we need to perform the operation in the given type size.
//

Image::Pixel operator&( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   dip::DataType dt = lhs.DataType();
   return DyadicOperator< ComputationType::Class_IntOrBin >(
         lhs, rhs, dt, dt,
         [ = ]( auto in1, auto in2 ) { return in1 & in2; }
   );
}

Image::Pixel operator|( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   dip::DataType dt = lhs.DataType();
   return DyadicOperator< ComputationType::Class_IntOrBin >(
         lhs, rhs, dt, dt,
         [ = ]( auto in1, auto in2 ) { return in1 | in2; }
   );
}

Image::Pixel operator^( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   dip::DataType dt = lhs.DataType();
   return DyadicOperator< ComputationType::Class_IntOrBin >(
         lhs, rhs, dt, dt,
         [ = ]( auto in1, auto in2 ) { return in1 ^ in2; }
   );
}

namespace {
Image::Pixel Not( Image::Pixel const& in ) {
   dip::DataType dt = in.DataType();
   return MonadicOperator< ComputationType::Class_IntOrBin >(
         in, dt, dt,
         [ = ]( auto in1 ) { return ~in1; }
   );
}
} // namespace

Image::Pixel operator~( Image::Pixel const& in ) {
   DIP_THROW_IF( !in.DataType().IsInteger(), "Bit-wise unary not operator only applicable to integer pixels" );
   return Not( in );
}
Image::Pixel operator!( Image::Pixel const& in ) {
   DIP_THROW_IF( !in.DataType().IsBinary(), "Boolean unary not operator only applicable to binary pixels" );
   return Not( in );
}


//
// Comparison
//

bool operator==( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   dip::uint lhsN = lhs.TensorElements();
   dip::uint rhsN = rhs.TensorElements();
   if(( lhsN > 1 ) && ( rhsN > 1 ) && ( lhsN != rhsN )) {
      return false; // tests false if different number of tensor elements
   }
   // Compare element-wise
   dip::DataType dt = SuggestArithmetic( lhs.DataType(), rhs.DataType());
   Image::Pixel res = DyadicOperator< ComputationType::Class_Flex >( lhs, rhs, dt, DT_BIN,
                                                                     [ = ]( auto in1, auto in2 ) { return in1 == in2; }
   );
   // Tests true only if all elements test true
   return res.All();
}

namespace {

template< typename F >
bool ComparisonOperator(
      Image::Pixel const& lhs,
      Image::Pixel const& rhs,
      F const& function
) {
   DIP_THROW_IF( lhs.DataType().IsComplex() || rhs.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint lhsN = lhs.TensorElements();
   dip::uint rhsN = rhs.TensorElements();
   if(( lhsN > 1 ) && ( rhsN > 1 ) && ( lhsN != rhsN )) {
      return false; // tests false if different number of tensor elements
   }
   // Compare element-wise
   Image::Pixel res = DyadicOperator< ComputationType::Class_Float >( lhs, rhs, DT_DFLOAT, DT_BIN, function );
   // Tests true only if all elements test true
   return res.All();
}

} // namespace

bool operator<( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   return ComparisonOperator( lhs, rhs,
                              [ = ]( auto in1, auto in2 ) { return in1 < in2; }
   );
}

bool operator>( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   return ComparisonOperator( lhs, rhs,
                              [ = ]( auto in1, auto in2 ) { return in1 > in2; }
   );
}

bool operator<=( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   return ComparisonOperator( lhs, rhs,
                              [ = ]( auto in1, auto in2 ) { return in1 <= in2; }
   );
}

bool operator>=( Image::Pixel const& lhs, Image::Pixel const& rhs ) {
   return ComparisonOperator( lhs, rhs,
                              [ = ]( auto in1, auto in2 ) { return in1 >= in2; }
   );
}

} // namespace dip
