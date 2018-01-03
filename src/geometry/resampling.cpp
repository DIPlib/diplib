/*
 * DIPlib 3.0
 * This file contains definitions for geometric transformations that use interpolation
 *
 * (c)2017-2018, Cris Luengo.
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
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength;
      }
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
   DIP_STACK_TRACE_THIS( ArrayUseParameter( wrap, nDims, dip::sint( 0 )));

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
                         Framework::SeparableOption::AsScalarImage );
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
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint procDim ) override {
         return interpolation::GetNumberOfOperations( method_, lineLength, zoom_[ procDim ] );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         DIP_ASSERT( params.inBuffer.stride == 1 );
         dip::uint procDim = params.dimension;
         SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
         TPI* buffer = nullptr;
         if( method_ == interpolation::Method::BSPLINE ) {
            dip::uint size = params.inBuffer.length + 2 * params.inBuffer.border;
            buffer_[ params.thread ].resize( 2 * size ); // NOP if already that size
            buffer = buffer_[ params.thread ].data();
         }
         interpolation::Dispatch( method_, in, out, params.outBuffer.length, zoom_[ procDim ], -shift_[ procDim ], buffer );
      }
   private:
      interpolation::Method method_;
      FloatArray const& zoom_;                  // One per dimension
      FloatArray const& shift_;                 // One per dimension
      std::vector< std::vector< TPI >> buffer_; // One per thread
};

template< typename TPI >
TPI CastToOutType( std::complex< TPI > in ) {
   return std::real( in );
}
template< typename TPI >
TPI CastToOutType( TPI in ) {
   return in;
}

template< typename TPI >
class FourierResamplingLineFilter : public Framework::SeparableLineFilter {
      using TPF = FloatType< TPI >;
      using TPC = ComplexType< TPI >;
   public:
      FourierResamplingLineFilter( FloatArray const& zoom, FloatArray const& shift, UnsignedArray const& sizes ) {
         dip::uint nDims = sizes.size();
         ft_.resize( nDims );
         ift_.resize( nDims );
         weights_.resize( nDims );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            bool foundInSize = false;
            bool foundOutSize = false;
            bool foundShift = false;
            dip::uint outSize = interpolation::ComputeOutputSize( sizes[ ii ], zoom[ ii ] );
            for( dip::uint jj = 0; jj < ii; ++jj ) {
               if( sizes[ jj ] == sizes[ ii ] ) {
                  if( !foundInSize ) {
                     ft_[ ii ] = ft_[ jj ];
                     foundInSize = true;
                  }
                  if( !foundShift && ( shift[ jj ] == shift[ ii ] )) {
                     weights_[ ii ] = weights_[ jj ];
                     foundShift = true; // note that foundShift implies foundInSize.
                  }
               }
               if( !foundOutSize && ( ift_[ jj ].TransformSize() == outSize )) {
                  ift_[ ii ] = ift_[ jj ];
                  foundOutSize = true;
               }
               if( foundOutSize && foundShift ) {
                  break;
               }
            }
            if( !foundInSize ) {
               ft_[ ii ].Initialize( sizes[ ii ], false );
            }
            if( !foundOutSize ) {
               ift_[ ii ].Initialize( outSize, true );
            }
            if( !foundShift ) {
               weights_[ ii ].resize( sizes[ ii ] );
               interpolation::FourierShiftWeights( weights_[ ii ], shift[ ii ] );
            }
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint procDim ) override {
         dip::uint outLength = ift_[ procDim ].TransformSize();
         return 10 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )))
              + 10 * outLength  * static_cast< dip::uint >( std::round( std::log2( outLength  )));
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         constexpr bool complexInput = std::is_same< TPI, TPC >::value;
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint procDim = params.dimension;
         dip::uint bufferSize = interpolation::FourierBufferSize( ft_[ procDim ], ift_[ procDim ] );
         dip::uint inOutSize = 0;
         bool useOutBuffer = true;
         if( complexInput ) {
            useOutBuffer = params.outBuffer.stride != 1;
            if( useOutBuffer ) {
               bufferSize += params.outBuffer.length;
            }
         } else {
            inOutSize = std::max( params.inBuffer.length, params.outBuffer.length );
            bufferSize += inOutSize;
         }
         buffer_[ params.thread ].resize( bufferSize ); // NOP if already that size
         TPC* buffer = buffer_[ params.thread ].data();
         // Copy input to `data`
         TPC* tmpIn;
         TPC* tmpOut;
         if( complexInput ) {
            tmpIn = reinterpret_cast< TPC* >( in ); // Cast does nothing if `complexInput`, this is here so we can compile.
            if( useOutBuffer ) {
               tmpOut = buffer;
               buffer += params.outBuffer.length;
            } else {
               tmpOut = static_cast< TPC* >( params.outBuffer.buffer );
            }
         } else {
            tmpIn = buffer;
            for( dip::uint ii = 0; ii < params.inBuffer.length; ++ii ) {
               *tmpIn = *in;
               ++tmpIn;
               ++in;
            }
            tmpOut = tmpIn = buffer;
            buffer += inOutSize;
         }
         // Interpolate
         interpolation::Fourier< TPF >( tmpIn, tmpOut, 0.0, ft_[ procDim ], ift_[ procDim ], weights_[ procDim ].data(), buffer );
         // Copy `data` to output
         if( useOutBuffer ) {
            SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
            for( dip::uint ii = 0; ii < params.outBuffer.length; ++ii ) {
               *out = CastToOutType< TPI >( *tmpIn );
               ++out;
               ++tmpIn;
            }
         }
         // TODO: For FOURIER method, add a border to: improve results, use boundary condition, use an optimal transform size.
         //       This will be easy only for the zoom==1.0 case, for other cases, you cannot pick one of the two FT sizes.
      }

   private:
      std::vector< DFT< TPF >> ft_;                // One per dimension
      std::vector< DFT< TPF >> ift_;               // One per dimension
      std::vector< std::vector< TPC >> weights_;   // One per dimension
      std::vector< std::vector< TPC >> buffer_;    // One per thread
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
      DIP_STACK_TRACE_THIS( method = interpolation::ParseMethod( interpolationMethod ));
   }
   BoundaryConditionArray bc;
   DIP_STACK_TRACE_THIS( bc = StringArrayToBoundaryConditionArray( boundaryCondition ));

   // Preserve input
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // Calculate new output sizes and other processing parameters
   UnsignedArray outSizes = in.Sizes();
   BooleanArray process( nDims, false );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( zoom[ ii ] != 1.0 ) {
         process[ ii ] = true;
         outSizes[ ii ] = interpolation::ComputeOutputSize( outSizes[ ii ], zoom[ ii ] );
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
   if( method == interpolation::Method::FOURIER ) {
      DIP_OVL_NEW_FLEX( lineFilter, FourierResamplingLineFilter, ( zoom, shift, in.Sizes() ), bufferType );
   } else {
      DIP_OVL_NEW_FLEX( lineFilter, ResamplingLineFilter, ( method, zoom, shift ), bufferType );
   }

   // Call line filter through framework
   Framework::Separable( in, out, bufferType, out.DataType(), process, borders, bc, *lineFilter,
                         Framework::SeparableOption::AsScalarImage + Framework::SeparableOption::DontResizeOutput + Framework::SeparableOption::UseInputBuffer );
}

namespace {

template< typename TPI >
class SkewLineFilter : public Framework::SeparableLineFilter {
   public:
      SkewLineFilter( interpolation::Method method, FloatArray const& tanShear, FloatArray const& offset, dip::uint axis, BoundaryConditionArray const& boundaryCondition ) :
            method_( method ), tanShear_( tanShear ), offset_( offset ), axis_( axis ), boundaryCondition_( boundaryCondition ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return interpolation::GetNumberOfOperations( method_, lineLength, 1.0 );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         DIP_ASSERT( params.inBuffer.stride == 1 );
         SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
         dip::uint length = params.inBuffer.length;
         dip::uint procDim = params.dimension;
         DIP_ASSERT( procDim != axis_ );
         DIP_ASSERT( tanShear_[ procDim ] != 0.0 );
         TPI* buffer = nullptr;
         if( method_ == interpolation::Method::BSPLINE ) {
            dip::uint size = length + 2 * params.inBuffer.border;
            buffer_[ params.thread ].resize( 2 * size ); // NOP if already that size
            buffer = buffer_[ params.thread ].data();
         }
         dfloat fullShift = tanShear_[ procDim ] * static_cast< dfloat >( params.position[ axis_ ] ) + offset_[ procDim ];
         dip::sint offset = floor_cast( fullShift );
         dfloat shift = -( fullShift - static_cast< dfloat >( offset ));
         if( boundaryCondition_[ procDim ] == BoundaryCondition::PERIODIC ) {
            offset %= static_cast< dip::sint >( length );
            if( offset < 0 ) {
               offset += static_cast< dip::sint >( length );
            }
            dip::uint len = length - static_cast< dip::uint >( offset );
            auto outPtr = out + offset;
            interpolation::Dispatch( method_, in, outPtr, len, 1.0, shift, buffer );
            outPtr = out;
            in += len;
            len = static_cast< dip::uint >( offset );
            interpolation::Dispatch( method_, in, outPtr, len, 1.0, shift, buffer );
         } else {
            DIP_ASSERT( offset >= 0 );
            out += offset;
            if( shift < 0.0 ) {
               ++length; // Fill in one sample more than we have in the input, so we interpolate properly.
            }
            interpolation::Dispatch( method_, in, out, length, 1.0, shift, buffer );
            detail::ExpandBuffer( out.Pointer(), DataType( TPI( 0 )), out.Stride(), 1, length, 1, static_cast< dip::uint >( offset ),
                                  params.outBuffer.length - length - static_cast< dip::uint >( offset ), boundaryCondition_[ procDim ] );
         }
      }
   private:
      interpolation::Method method_;
      FloatArray const& tanShear_;
      FloatArray const& offset_;
      dip::uint axis_;
      BoundaryConditionArray const& boundaryCondition_;
      std::vector< std::vector< TPI >> buffer_; // One per thread
};

} // namespace

dip::UnsignedArray Skew(
      Image const& c_in,
      Image& out,
      FloatArray const& shearArray,
      dip::uint axis,
      dip::uint origin,
      String const& interpolationMethod,
      BoundaryConditionArray boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( axis >= nDims , E::ILLEGAL_DIMENSION );
   interpolation::Method method;
   if( c_in.DataType().IsBinary() ) {
      method = interpolation::Method::NEAREST_NEIGHBOR;
   } else {
      DIP_STACK_TRACE_THIS( method = interpolation::ParseMethod( interpolationMethod ));
   }
   DIP_STACK_TRACE_THIS( BoundaryArrayUseParameter( boundaryCondition, nDims ));
   DIP_THROW_IF( method == interpolation::Method::FOURIER, E::NOT_IMPLEMENTED ); // TODO: implement Fourier interpolation

   // Calculate new output sizes and other processing parameters
   UnsignedArray outSizes = c_in.Sizes();
   DIP_THROW_IF( origin > outSizes[ axis ], E::PARAMETER_OUT_OF_RANGE );
   FloatArray offset( nDims, 0.0 );
   BooleanArray process( nDims, false );
   UnsignedArray outArray( nDims, 0 );
   outArray[ axis ] = origin;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( ii != axis ) && ( shearArray[ ii ] != 0.0 )) {
         process[ ii ] = true;
         // On the line indicated by `origin` we want to do an integer shift. Adding `offset` makes the shift an integer value
         dfloat originShift = static_cast< dfloat >( origin ) * shearArray[ ii ];
         offset[ ii ] = static_cast< dfloat >( originShift > 0.0 ? ceil_cast( originShift ) : floor_cast( originShift )) - originShift;
         if( boundaryCondition[ ii ] != BoundaryCondition::PERIODIC ) {
            // We need to increase the size of the output image to accommodate all the data
            dip::uint skewSize = static_cast< dip::uint >(
                  std::ceil( std::abs( static_cast< dfloat >( outSizes[ axis ] - 1 ) * shearArray[ ii ] + offset[ ii ] ))
            );
            outSizes[ ii ] += skewSize;
            // Add to `offset` an integer number such that the computed output start locations are always positive.
            if( shearArray[ ii ] < 0.0 ) {
               offset[ ii ] += static_cast< dfloat >( skewSize );
            }
         }
         outArray[ ii ] = static_cast< dip::uint >( round_cast( originShift + offset[ ii ] ));
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
   DIP_OVL_NEW_FLEX( lineFilter, SkewLineFilter, ( method, shearArray, offset, axis, boundaryCondition ), bufferType );

   // Call line filter through framework
   Framework::Separable( in, out, bufferType, out.DataType(), process, { border }, boundaryCondition, *lineFilter,
         Framework::SeparableOption::AsScalarImage + Framework::SeparableOption::DontResizeOutput + Framework::SeparableOption::UseInputBuffer );

   return outArray;
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
   dip::uint nDims = c_in.Dimensionality();
   BoundaryConditionArray bc;
   DIP_STACK_TRACE_THIS( bc = BoundaryConditionArray( nDims, StringToBoundaryCondition( boundaryCondition )));
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
   // This tests for in being forged, and dim1 and dim2 being valid
   DIP_STACK_TRACE_THIS( in.Rotation90( static_cast< dip::sint >( n ), dimension1, dimension2 ));
   // NOTE: The rotation above swaps and flips dimensions, it doesn't keep the origin pixel in its place.
   //       This means that even-sized dimensions with a negative stride now need to be shifted up by 1 pixel
   //       `origin1` and `origin2` are the location of the pixel that shouldn't move in the rotation.
   dip::uint origin1 = in.Size( dimension1 ) / 2 - ( !( in.Size( dimension1 ) & 1 ) && ( in.Stride( dimension1 ) < 0 ));
   dip::uint origin2 = in.Size( dimension2 ) / 2 - ( !( in.Size( dimension2 ) & 1 ) && ( in.Stride( dimension2 ) < 0 ));
   // Do the last rotation, in the range [-45,45], with three skews
   //    As origin we take the pixel that was at the origin *before* the `Rotation90` call.
   FloatArray skewArray1( nDims, 0.0 );
   skewArray1[ dimension1 ] = -std::tan( angle / 2.0 );
   FloatArray skewArray2( nDims, 0.0 );
   skewArray2[ dimension2 ] = std::sin( angle );
   UnsignedArray ret = Skew( in, out, skewArray1, dimension2, origin2, method, bc );
   origin1 += ret[ dimension1 ];
   ret = Skew( out, out, skewArray2, dimension1, origin1, method, bc );
   origin2 += ret[ dimension2 ];
   ret = Skew( out, out, skewArray1, dimension2, origin2, method, bc );
   origin1 += ret[ dimension1 ];
   // Remove the useless borders of the image
   //    This is where we adjust such that the pixel at the input's origin is also at the output's origin.
   dfloat cos_angle = std::abs( std::cos( angle ));
   dfloat sin_angle = std::abs( std::sin( angle ));
   dfloat size1 = static_cast< dfloat >( in.Size( dimension1 ));
   dfloat size2 = static_cast< dfloat >( in.Size( dimension2 ));
   UnsignedArray newSize = out.Sizes();
   newSize[ dimension1 ] = std::min(
         out.Size( dimension1 ),
         2 * static_cast< dip::uint >( std::ceil(( size1 * cos_angle + size2 * sin_angle ) / 2.0 )) + ( in.Size( dimension1 ) & 1 ));
   if ( origin1 < ( newSize[ dimension1 ] / 2 )) {
      // if `newSize` is too large to be symmetric around the origin, make it smaller.
      newSize[ dimension1 ] = origin1 * 2 + ( newSize[ dimension1 ] & 1 ); // keep odd if it was odd.
   }
   newSize[ dimension2 ] = std::min(
         out.Size( dimension2 ),
         2 * static_cast< dip::uint >( std::ceil(( size1 * sin_angle + size2 * cos_angle ) / 2.0 )) + ( in.Size( dimension2 ) & 1 ));
   if ( origin2 < ( newSize[ dimension2 ] / 2 )) {
      // if `newSize` is too large to be symmetric around the origin, make it smaller.
      newSize[ dimension2 ] = origin2 * 2 + ( newSize[ dimension2 ] & 1 ); // keep odd if it was odd.
   }
   // The section below is similar to out.Crop( newSize ), except we use `origin1` and `origin2` to determine where to cut.
   UnsignedArray origin( nDims, 0 );
   origin[ dimension1 ] = origin1 - ( newSize[ dimension1 ] / 2 );
   DIP_ASSERT( origin[ dimension1 ] <= out.Size( dimension1 ) - newSize[ dimension1 ] );
   origin[ dimension2 ] = origin2 - ( newSize[ dimension2 ] / 2 );
   DIP_ASSERT( origin[ dimension2 ] <= out.Size( dimension2 ) - newSize[ dimension2 ] );
   out.dip__SetOrigin( out.Pointer( origin ));
   out.dip__SetSizes( newSize );
   // Fix pixel sizes
   if( pixelSize.IsDefined() ) {
      if( pixelSize[ dimension1 ] != pixelSize[ dimension2 ] ) {
         pixelSize.Set( dimension1, {} );
         pixelSize.Set( dimension2, {} );
      }
      out.SetPixelSize( pixelSize );
   }
}

} // namespace dip
