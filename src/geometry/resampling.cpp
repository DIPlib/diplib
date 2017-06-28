/*
 * DIPlib 3.0
 * This file contains definitions for coordinate image generation
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
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"
#include "diplib/library/copy_buffer.h"

#include "interpolation.h"

namespace dip {

void Resampling(
      Image const& in,
      Image& out,
      FloatArray const& /*zoom*/,
      FloatArray const& /*shift*/,
      String const& /*method*/,
      StringArray const& /*boundaryCondition*/
) {
   // TODO: Resampling
}

namespace {

template< typename TPI >
class SkewLineFilter : public Framework::SeparableLineFilter {
   public:
      SkewLineFilter( interpolation::Method method, dfloat tanShear, dip::uint axis, dip::uint origin, BoundaryCondition boundaryCondition ) :
            method_( method ), tanShear_( tanShear ), axis_( axis ), origin_( origin ), boundaryCondition_( boundaryCondition ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         DIP_ASSERT( params.inBuffer.stride == 1 );
         SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
         dip::uint length = params.inBuffer.length;
         // params.dimension == skew;
         //params.thread;
         dfloat skew = tanShear_ * ( static_cast< dfloat >( origin_ ) - static_cast< dfloat >( params.position[ axis_ ] ));
         dip::sint offset = static_cast< dip::sint >( std::floor( skew ));
         dfloat shift = skew - static_cast< dfloat >( offset );
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
         if( boundaryCondition_ == BoundaryCondition::PERIODIC ) {
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
            detail::ExpandBuffer( out.Pointer(), DataType( TPI( 0 )), out.Stride(), 1, length, 1, static_cast< dip::uint >( offset ), params.outBuffer.length - length - static_cast< dip::uint >( offset ), boundaryCondition_ );
         }
      }
   private:
      interpolation::Method method_;
      dfloat tanShear_;
      dip::uint axis_;
      dip::uint origin_;
      BoundaryCondition boundaryCondition_;
      std::vector< std::vector< TPI >> buffer_; // One per thread
};

} // namespace

void Skew(
      Image const& c_in,
      Image& out,
      dfloat shear,
      dip::uint skew,
      dip::uint axis,
      String const& s_method,
      String const& boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( axis == skew, E::INVALID_PARAMETER );
   DIP_THROW_IF(( axis >= nDims ) || ( skew >= nDims ), E::PARAMETER_OUT_OF_RANGE );
   interpolation::Method method;
   BoundaryCondition bc;
   DIP_START_STACK_TRACE
      method = interpolation::ParseMethod( s_method );
      bc = StringToBoundaryCondition( boundaryCondition );
   DIP_END_STACK_TRACE
   bool periodicSkew = bc == BoundaryCondition::PERIODIC;
   DIP_THROW_IF( method == interpolation::Method::FT, E::NOT_IMPLEMENTED );
   DIP_THROW_IF(( shear <= -pi / 2.0) | ( shear >= pi / 2.0), E::PARAMETER_OUT_OF_RANGE );

   // Preserve input
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // Calculate new output sizes
   UnsignedArray outSizes = in.Sizes();
   dip::uint origin = outSizes[ axis ] / 2;
   dfloat tanShear = std::tan( shear );
   if( !periodicSkew ) {
      dip::uint skewSize = static_cast< dip::uint >( std::ceil( std::abs( static_cast<dfloat>( origin ) * tanShear )));
      outSizes[ skew ] += 2 * skewSize;
   }

   // Determine further processing parameters
   dip::uint border = interpolation::GetBorderSize( method );
   BooleanArray process( nDims, false );
   process[ skew ] = true;

   // Create output
   out.ReForge( outSizes, in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( pixelSize );
   DataType bufferType = DataType::SuggestFlex( out.DataType() );

   // Find line filter
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_FLEX( lineFilter, SkewLineFilter, ( method, tanShear, axis, origin, bc ), bufferType );

   // Call line filter through framework
   Framework::Separable( in, out, bufferType, out.DataType(), process, { border }, { bc }, *lineFilter,
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
   Skew( in, out, ( angle / 2.0 ), dimension1, dimension2, method, boundaryCondition );
   Skew( out, out, std::atan( -std::sin( angle )), dimension2, dimension1, method, boundaryCondition );
   Skew( out, out, ( angle / 2.0 ), dimension1, dimension2, method, boundaryCondition );
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
