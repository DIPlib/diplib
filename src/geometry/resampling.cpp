/*
 * DIPlib 3.0
 * This file contains definitions for geometric transformations that use interpolation
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
#include "diplib/geometry.h"
#include "diplib/boundary.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"
#include "diplib/library/copy_buffer.h"

#include "interpolation.h"

namespace dip {

namespace {

template< typename TPI >
class WrapLineFilter : public Framework::SeparableLineFilter {
   public:
      WrapLineFilter( UnsignedArray const& wrap ) : wrap_( wrap ) {}
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         SampleIterator< TPI > in{ static_cast< TPI* >( params.inBuffer.buffer ), params.inBuffer.stride };
         SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
         dip::uint length = params.inBuffer.length;
         dip::uint wrap = wrap_[ params.dimension ]; // wrap > 0 && wrap < length
         std::copy( in, in + length - wrap, out + wrap );
         std::copy( in + length - wrap, in + length, out );
      }
   private:
      UnsignedArray const& wrap_;
};

} // namespace

void Wrap(
      Image const& in,
      Image& out,
      IntegerArray wrap
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims == 0, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      ArrayUseParameter( wrap, nDims, dip::sint( 0 ));
   DIP_END_STACK_TRACE

   // Determine processing parameters
   BooleanArray process( nDims, false );
   UnsignedArray uWrap( nDims, 0 );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dip::sint w = wrap[ ii ] % static_cast< dip::sint >( in.Size( ii ));
      if( w < 0 ) {
         w += static_cast< dip::sint >( in.Size( ii ));
      }
      process[ ii ] = w != 0;
      uWrap[ ii ] = static_cast< dip::uint >( w );
   }

   // Find line filter
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, WrapLineFilter, ( uWrap ), in.DataType() );

   // Call line filter through framework
   Framework::Separable( in, out, in.DataType(), in.DataType(), process, {}, {}, *lineFilter,
                         Framework::Separable_AsScalarImage );
}

namespace {

template< typename TPI >
class ResamplingLineFilter : public Framework::SeparableLineFilter {
   public:
      ResamplingLineFilter( interpolation::Method method, FloatArray const& zoom, FloatArray const& shift ) :
            method_( method ), zoom_( zoom ), shift_( shift ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         DIP_ASSERT( params.inBuffer.stride == 1 );
         SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
         dip::uint procDim = params.dimension;
         TPI* spline1 = nullptr;
         TPI* spline2 = nullptr;
         if( method_ == interpolation::Method::BSPLINE ) {
            dip::uint size = params.inBuffer.length + params.inBuffer.border;
            std::vector< TPI >& buffer = buffer_[ params.thread ];
            buffer.resize( 2 * size ); // NOP if already that size
            spline1 = buffer.data();
            spline2 = spline1 + size;
         }
         interpolation::Dispatch( method_, in, out, params.outBuffer.length, zoom_[ procDim ], -shift_[ procDim ], spline1, spline2 );
      }
   private:
      interpolation::Method method_;
      FloatArray const& zoom_;
      FloatArray const& shift_;
      std::vector< std::vector< TPI >> buffer_; // One per thread
};

} // namespace

void Resampling(
      Image const& c_in,
      Image& out,
      FloatArray zoom,
      FloatArray shift,
      String const& interpolationMethod,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims == 0, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      ArrayUseParameter( zoom, nDims, 1.0 );
      ArrayUseParameter( shift, nDims, 0.0 );
   DIP_END_STACK_TRACE
   for( auto z : zoom ) {
      DIP_THROW_IF( z <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   }
   interpolation::Method method;
   if( c_in.DataType().IsBinary() ) {
      method = interpolation::Method::NEAREST_NEIGHBOR;
   } else {
      DIP_START_STACK_TRACE
         method = interpolation::ParseMethod( interpolationMethod );
      DIP_END_STACK_TRACE
   }
   BoundaryConditionArray bc;
   DIP_START_STACK_TRACE
      bc = StringArrayToBoundaryConditionArray( boundaryCondition );
   DIP_END_STACK_TRACE

   // Preserve input
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // Calculate new output sizes and other processing parameters
   UnsignedArray outSizes = in.Sizes();
   BooleanArray process( nDims, false );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( zoom[ ii ] != 1.0 ) {
         process[ ii ] = true;
         outSizes[ ii ] = static_cast< dip::uint >( std::floor( static_cast< dfloat >( outSizes[ ii ] ) * zoom[ ii ] + 1e-6 ));
         // The 1e-6 is to avoid floating-point inaccuracies, ex: floor(49*(64/49))!=64
      } else if( shift[ ii ] != 0.0 ) {
         process[ ii ] = true;
      }
   }
   dip::uint border = interpolation::GetBorderSize( method );
   UnsignedArray borders( nDims, border );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      borders[ ii ] += static_cast< dip::uint >( std::ceil( std::abs( shift[ ii ] )));
   }

   // Create output
   out.ReForge( outSizes, in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( pixelSize );
   DataType bufferType = DataType::SuggestFlex( out.DataType() );

   // Find line filter
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_FLEX( lineFilter, ResamplingLineFilter, ( method, zoom, shift ), bufferType );

   // Call line filter through framework
   Framework::Separable( in, out, bufferType, out.DataType(), process, borders, bc, *lineFilter,
                         Framework::Separable_AsScalarImage + Framework::Separable_DontResizeOutput + Framework::Separable_UseInputBuffer );
}

namespace {

template< typename TPI >
class SkewLineFilter : public Framework::SeparableLineFilter {
   public:
      SkewLineFilter( interpolation::Method method, FloatArray const& tanShear, dip::uint axis, dip::uint origin, BoundaryConditionArray const& boundaryCondition ) :
            method_( method ), tanShear_( tanShear ), axis_( axis ), origin_( origin ), boundaryCondition_( boundaryCondition ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         DIP_ASSERT( params.inBuffer.stride == 1 );
         SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
         dip::uint length = params.inBuffer.length;
         dip::uint procDim = params.dimension;
         DIP_ASSERT( procDim != axis_ );
         DIP_ASSERT( tanShear_[ procDim ] != 0.0 );
         dfloat shift = tanShear_[ procDim ] * ( static_cast< dfloat >( origin_ ) - static_cast< dfloat >( params.position[ axis_ ] ));
         dip::sint offset = static_cast< dip::sint >( std::floor( shift ));
         shift -= static_cast< dfloat >( offset );
         if( shift > 0.5 ) {
            shift -= 1.0;
            ++offset;
         }
         TPI* spline1 = nullptr;
         TPI* spline2 = nullptr;
         if( method_ == interpolation::Method::BSPLINE ) {
            dip::uint size = length + params.inBuffer.border;
            std::vector< TPI >& buffer = buffer_[ params.thread ];
            buffer.resize( 2 * size ); // NOP if already that size
            spline1 = buffer.data();
            spline2 = spline1 + size;
         }
         if( boundaryCondition_[ procDim ] == BoundaryCondition::PERIODIC ) {
            if( offset >= 0 ) {
               dip::uint len = length - static_cast< dip::uint >( offset );
               auto outPtr = out + offset;
               interpolation::Dispatch( method_, in, outPtr, len, 1.0, -shift, spline1, spline2 );
               outPtr = out;
               in += len;
               len = static_cast< dip::uint >( offset );;
               interpolation::Dispatch( method_, in, outPtr, len, 1.0, -shift, spline1, spline2 );
            } else {
               dip::uint len = static_cast< dip::uint >( -offset );
               auto outPtr = out + ( length - len );
               interpolation::Dispatch( method_, in, outPtr, len, 1.0, -shift, spline1, spline2 );
               outPtr = out;
               in += len;
               len = length - len;
               interpolation::Dispatch( method_, in, outPtr, len, 1.0, -shift, spline1, spline2 );
            }
         } else {
            dip::uint skewSize = ( params.outBuffer.length - length ) / 2;
            offset += static_cast< dip::sint >( skewSize );
            DIP_ASSERT( offset >= 0 );
            out += offset;
            interpolation::Dispatch( method_, in, out, length, 1.0, -shift, spline1, spline2 );
            detail::ExpandBuffer( out.Pointer(), DataType( TPI( 0 )), out.Stride(), 1, length, 1, static_cast< dip::uint >( offset ),
                                  params.outBuffer.length - length - static_cast< dip::uint >( offset ), boundaryCondition_[ procDim ] );
         }
      }
   private:
      interpolation::Method method_;
      FloatArray const& tanShear_;
      dip::uint axis_;
      dip::uint origin_;
      BoundaryConditionArray const& boundaryCondition_;
      std::vector< std::vector< TPI >> buffer_; // One per thread
};

} // namespace

void Skew(
      Image const& c_in,
      Image& out,
      FloatArray const& shearArray,
      dip::uint axis,
      String const& interpolationMethod,
      BoundaryConditionArray boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( axis >= nDims , E::PARAMETER_OUT_OF_RANGE );
   interpolation::Method method;
   if( c_in.DataType().IsBinary() ) {
      method = interpolation::Method::NEAREST_NEIGHBOR;
   } else {
      DIP_START_STACK_TRACE
         method = interpolation::ParseMethod( interpolationMethod );
      DIP_END_STACK_TRACE
   }
   DIP_START_STACK_TRACE
      BoundaryArrayUseParameter( boundaryCondition, nDims );
   DIP_END_STACK_TRACE
   DIP_THROW_IF( method == interpolation::Method::FT, E::NOT_IMPLEMENTED );

   // Calculate new output sizes and other processing parameters
   UnsignedArray outSizes = c_in.Sizes();
   dip::uint origin = outSizes[ axis ] / 2;
   FloatArray tanShear( nDims, 0.0 );
   BooleanArray process( nDims, false );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( ii != axis ) && ( shearArray[ ii ] != 0.0 )) {
         DIP_THROW_IF(( shearArray[ ii ] <= -pi / 2.0 ) | ( shearArray[ ii ] >= pi / 2.0 ), E::PARAMETER_OUT_OF_RANGE );
         tanShear[ ii ] = std::tan( shearArray[ ii ] );
         process[ ii ] = true;
         if( boundaryCondition[ ii ] != BoundaryCondition::PERIODIC ) {
            dip::uint skewSize = static_cast< dip::uint >( std::ceil( std::abs( static_cast< dfloat >( origin ) * tanShear[ ii ] )));
            outSizes[ ii ] += 2 * skewSize;
         }
      }
   }
   UnsignedArray border( nDims, interpolation::GetBorderSize( method ));
   border[ axis ] = 0;

   // Preserve input
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // Create output
   out.ReForge( outSizes, in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( pixelSize );
   DataType bufferType = DataType::SuggestFlex( out.DataType() );

   // Find line filter
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_FLEX( lineFilter, SkewLineFilter, ( method, tanShear, axis, origin, boundaryCondition ), bufferType );

   // Call line filter through framework
   Framework::Separable( in, out, bufferType, out.DataType(), process, { border }, boundaryCondition, *lineFilter,
         Framework::Separable_AsScalarImage + Framework::Separable_DontResizeOutput + Framework::Separable_UseInputBuffer );
}

void Rotation(
      Image const& c_in,
      Image& out,
      dfloat angle,
      dip::uint dimension1,
      dip::uint dimension2,
      String const& method,
      String const& boundaryCondition
) {
   // Parse boundaryCondition
   BoundaryConditionArray bc;
   DIP_START_STACK_TRACE
      bc = BoundaryConditionArray( c_in.Dimensionality(), StringToBoundaryCondition( boundaryCondition ));
   DIP_END_STACK_TRACE
   // Preserve input
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   // Normalize angle to [0,180)
   angle = std::fmod( angle, 2.0 * pi );
   if( angle < 0.0 ) {
      angle += 2.0 * pi;
   }
   // Take care of multiples of 90 degrees
   dfloat n = std::round( 2.0 * angle / pi );
   angle -= n * pi / 2.0;
   DIP_START_STACK_TRACE
      // This tests for in being forged, and dim1 and dim2 being valid
      in.Rotation90( static_cast< dip::sint >( n ), dimension1, dimension2 );
   DIP_END_STACK_TRACE
   // Do the last rotation, in the range [-45,45], with three skews
   FloatArray skewArray1( in.Dimensionality(), 0.0 );
   skewArray1[ dimension1 ] = ( angle / 2.0 );
   FloatArray skewArray2( in.Dimensionality(), 0.0 );
   skewArray2[ dimension2 ] = std::atan( -std::sin( angle ));
   Skew( in, out, skewArray1, dimension2, method, bc );
   Skew( out, out, skewArray2, dimension1, method, bc );
   Skew( out, out, skewArray1, dimension2, method, bc );
   // Remove the useless borders of the image
   dfloat cos_angle = std::abs( std::cos( angle ));
   dfloat sin_angle = std::abs( std::sin( angle ));
   dfloat size1 = static_cast< dfloat >( in.Size( dimension1 ));
   dfloat size2 = static_cast< dfloat >( in.Size( dimension2 ));
   UnsignedArray newSize = out.Sizes();
   newSize[ dimension1 ] = std::min(
         out.Size( dimension1 ),
         2 * static_cast< dip::uint >( std::ceil(( size1 * cos_angle + size2 * sin_angle ) / 2.0 )) + ( in.Size( dimension1 ) & 1 ));
   newSize[ dimension2 ] = std::min(
         out.Size( dimension2 ),
         2 * static_cast< dip::uint >( std::ceil(( size1 * sin_angle + size2 * cos_angle ) / 2.0 )) + ( in.Size( dimension2 ) & 1 ));
   out = out.Crop( newSize );
   // Fix pixel sizes
   if( pixelSize.IsDefined() ) {
      if( pixelSize[ dimension1 ] != pixelSize[ dimension2 ] ) {
         pixelSize[ dimension1 ] = {};
         pixelSize[ dimension2 ] = {};
      }
      out.SetPixelSize( pixelSize );
   }
}

} // namespace dip
