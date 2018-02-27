/*
 * DIPlib 3.0
 * This file contains definitions for geometric transformations that use interpolation
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/geometry.h"
#include "diplib/generic_iterators.h"
#include "diplib/overload.h"

namespace dip {

namespace {

enum class Method {
      NEAREST_NEIGHBOR,
      LINEAR,
      CUBIC_ORDER_3,
};

Method ParseMethod( String const& method ) {
   if( method.empty() || ( method == S::LINEAR )) {
      return Method::LINEAR;
   } else if(( method == "cubic" ) || ( method == S::CUBIC_ORDER_3 )) {
      return Method::CUBIC_ORDER_3;
   } else if(( method == "nn" ) || ( method == S::NEAREST )) {
      return Method::NEAREST_NEIGHBOR;
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
}

// Takes `coords` and subtracts the integer value. The base pixel offset is returned
UnsignedArray GetIntegerCoordinates( Image const& in, FloatArray& coords ) {
   UnsignedArray intCoord{ coords };
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      if( intCoord[ ii ] == in.Size( ii ) - 1 ) {
         --intCoord[ ii ];
      }
      coords[ ii ] -= static_cast< dfloat >( intCoord[ ii ] );
   }
   return intCoord;
}

//
// 1D interpolation functions. TPD is dfloat or dcomplex
//

template< typename TPD >
TPD Linear1D( TPD a, TPD b, dfloat pos ) {
   return a * ( 1.0 - pos ) + b * pos;
}

template< typename TPD >
TPD ThirdOrderCubicSpline1D( TPD a, TPD b, TPD c, TPD d, dfloat pos ) {
   dfloat pos2 = pos * pos;
   dfloat pos3 = pos2 * pos;
   dfloat filter_m1 = ( -pos3 + 2.0 * pos2 - pos ) / 2.0;
   dfloat filter__0 = ( 3.0 * pos3 - 5.0 * pos2 + 2.0) / 2.0;
   dfloat filter__1 = ( -3.0 * pos3 + 4.0 * pos2 + pos ) / 2.0;
   dfloat filter__2 = ( pos3 - pos2 ) / 2.0;
   return a * filter_m1 + b * filter__0 + c * filter__1 + d * filter__2;
}

//
// Recursive nD interpolation functions, call the 1D versions. TPI is the input image type.
//

template< typename TPI >
DoubleType< TPI > LinearND( TPI* src, IntegerArray const& srcStride,
               UnsignedArray const& coords, FloatArray const& subpos, dip::uint nDims ) {
   using TPD = DoubleType< TPI >;
   --nDims;
   dip::sint stride = srcStride[ nDims ];
   src += static_cast< dip::sint >( coords[ nDims ] ) * srcStride[ nDims ];
   // If there's only one dimension, directly compute and be done
   if( nDims == 0 ) {
      TPD a = static_cast< TPD >( src[ 0 ] );
      TPD b = static_cast< TPD >( src[ stride] );
      return Linear1D( a, b, subpos[ 0 ] );
   }
   // Otherwise, recursively compute result at two points along the last dimension and linearly interpolate them
   TPD a = LinearND( src, srcStride, coords, subpos, nDims );
   TPD b = LinearND( src + srcStride[ nDims ], srcStride, coords, subpos, nDims );
   return Linear1D( a, b, subpos[ nDims ] );
}

template< typename TPI >
DoubleType< TPI > ThirdOrderCubicSplineND( TPI* src, UnsignedArray const& srcSizes, IntegerArray const& srcStride,
                              UnsignedArray const& coords, FloatArray const& subpos, dip::uint nDims ) {
   using TPD = DoubleType< TPI >;
   --nDims;
   bool start = coords[ nDims ] == 0;
   bool end = coords[ nDims ] == srcSizes[ nDims ] - 2; // can never be srcSizes[ 0 ] - 1
   dip::sint stride = srcStride[ nDims ];
   src += static_cast< dip::sint >( coords[ nDims ] ) * stride;
   // If there's only one dimension, directly compute and be done
   if( nDims == 0 ) {
      TPD b = static_cast< TPD >( src[ 0 ] );
      TPD c = static_cast< TPD >( src[ stride ] );
      TPD a = start ? b : static_cast< TPD >( src[ -stride ] );
      TPD d = end ? c : static_cast< TPD >( src[ 2 * stride ] );
      return ThirdOrderCubicSpline1D( a, b, c, d, subpos[ 0 ] );
   }
   // Otherwise, recursively compute result at four points along the last dimension and linearly interpolate them
   TPD b = ThirdOrderCubicSplineND( src, srcSizes, srcStride, coords, subpos, nDims );
   TPD c = ThirdOrderCubicSplineND( src + stride, srcSizes, srcStride, coords, subpos, nDims );
   TPD a = start ? b : ThirdOrderCubicSplineND( src - stride, srcSizes, srcStride, coords, subpos, nDims );
   TPD d = end ? c : ThirdOrderCubicSplineND( src + 2 * stride, srcSizes, srcStride, coords, subpos, nDims );
   return ThirdOrderCubicSpline1D( a, b, c, d, subpos[ nDims ] );
}

//
// Driver interpolation functions, call the recursive functions
//

template< typename TPI >
void NearestNeighborInterpolationFunction( Image const& in, Image::Pixel const& out, FloatArray pos ) {
   UnsignedArray coords = GetIntegerCoordinates( in, pos );
   DIP_ASSERT( in.DataType() == DataType( TPI( 0 )));
   dip::uint nDims = in.Dimensionality();
   TPI* src = static_cast< TPI* >( in.Pointer( coords ));
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      src += pos[ ii ] > 0.5 ? in.Stride( ii ) : 0;
   }
   for( auto it = out.begin(); it != out.end(); ++it ) {
      *it = static_cast< FlexType< TPI >>( *src );
      src += in.TensorStride();
   }
}

template< typename TPI >
void LinearInterpolationFunction( Image const& in, Image::Pixel const& out, FloatArray pos ) {
   UnsignedArray coords = GetIntegerCoordinates( in, pos );
   DIP_ASSERT( in.DataType() == DataType( TPI( 0 )));
   TPI* src = static_cast< TPI* >( in.Origin() );
   for( auto it = out.begin(); it != out.end(); ++it ) {
      *it = LinearND( src, in.Strides(), coords, pos, in.Dimensionality() );
      src += in.TensorStride();
   }
}

template< typename TPI >
void ThirdOrderCubicSplineInterpolationFunction( Image const& in, Image::Pixel const& out, FloatArray pos ) {
   UnsignedArray coords = GetIntegerCoordinates( in, pos );
   DIP_ASSERT( in.DataType() == DataType( TPI( 0 )));
   TPI* src = static_cast< TPI* >( in.Origin() );
   for( auto it = out.begin(); it != out.end(); ++it ) {
      *it = ThirdOrderCubicSplineND( src, in.Sizes(), in.Strides(), coords, pos, in.Dimensionality() );
      src += in.TensorStride();
   }
}

using InterpolationFunctionPointer = void ( * )( Image const&, Image::Pixel const&, FloatArray );

InterpolationFunctionPointer GetInterpFunctionPtr( String const& method, DataType dt ) {
   InterpolationFunctionPointer function;
   switch( ParseMethod( method ) ) {
      case Method::NEAREST_NEIGHBOR:
         DIP_OVL_ASSIGN_NONBINARY( function, NearestNeighborInterpolationFunction, dt );
         break;
      default:
      //case Method::LINEAR:
         DIP_OVL_ASSIGN_NONBINARY( function, LinearInterpolationFunction, dt );
         break;
      case Method::CUBIC_ORDER_3:
         DIP_OVL_ASSIGN_NONBINARY( function, ThirdOrderCubicSplineInterpolationFunction, dt );
         break;
   }
   return function;
}

} // namespace

void ResampleAt(
      Image const& c_in,
      Image& out,
      FloatCoordinateArray const& coordinates,
      String const& method
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( c_in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims == 0, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( coordinates.empty(), E::ARRAY_PARAMETER_EMPTY );
   for( auto& c : coordinates ) {
      DIP_THROW_IF( c.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   }

   // Preserve input
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();

   // Create output
   UnsignedArray outSize( 1, coordinates.size() );
   out.ReForge( outSize, in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( pixelSize );
   out.SetColorSpace( colorSpace );

   // Find interpolator
   InterpolationFunctionPointer function;
   DIP_STACK_TRACE_THIS( function = GetInterpFunctionPtr( method, in.DataType() ));

   // Iterate over coordinates and out
   GenericImageIterator<> outIt( out );
   for( auto cIt = coordinates.begin(); cIt != coordinates.end(); ++cIt, ++outIt ) {
      if( in.IsInside( *cIt ) ) {
         function( in, *outIt, *cIt );
      } else {
         *outIt = 0;
      }
   }
}

Image::Pixel ResampleAt(
      Image const& in,
      FloatArray const& coordinates,
      String const& method
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims == 0, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( coordinates.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );

   // Create output
   Image::Pixel out( in.DataType(), in.TensorElements() );
   out.ReshapeTensor( in.Tensor() );

   // Find interpolator
   InterpolationFunctionPointer function;
   DIP_STACK_TRACE_THIS( function = GetInterpFunctionPtr( method, in.DataType() ));

   // Call interpolator
   if( in.IsInside( coordinates ) ) {
      function( in, out, coordinates );
   } else {
      out = 0;
   }

   return out;
}

} // namespace dip
