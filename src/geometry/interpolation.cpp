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
      DIP_THROW_IF( z <= 0.0, E::INVALID_PARAMETER );
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
   pixelSize.Scale( zoom );
   String colorSpace = c_in.ColorSpace();

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
   out.SetPixelSize( pixelSize );
   out.SetColorSpace( colorSpace );
}

namespace {

template< typename TPI > // TPI is a complex type
class ShiftFTLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint nTensorElements ) override {
         return 40 + nTensorElements; // TODO: is this OK?
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer[ 0 ].buffer );
         dip::sint nTensor = static_cast< dip::sint >( params.inBuffer[ 0 ].tensorLength );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::sint inTStride = params.inBuffer[ 0 ].tensorStride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint outTStride = params.outBuffer[ 0 ].tensorStride;

         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         dip::uint nDims = params.position.size();
         DIP_ASSERT( shift_.size() == nDims );

         dfloat phase = 0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != dim ) {
               phase -= shift_[ ii ] * ( static_cast< dfloat >( params.position[ ii ] ) - static_cast< dfloat >( sizes_[ ii ] / 2 )) * 2.0 * pi / static_cast< dfloat >( sizes_[ ii ] );
            }
         }
         dfloat offset = static_cast< dfloat >( sizes_[ dim ] / 2 );
         dfloat scale = shift_[ dim ] *  2.0 * pi / static_cast< dfloat >( sizes_[ dim ] );

         dfloat pp = static_cast< dfloat >( params.position[ dim ] ) - offset;
         for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp, in += inStride, out += outStride ) {
            dfloat ph = phase - pp * scale;
            TPI mul{ static_cast< FloatType< TPI >>( std::cos( ph )),
                     static_cast< FloatType< TPI >>( std::sin( ph )) };
            for( dip::sint jj = 0; jj < nTensor; ++jj ) {
               out[ jj * outTStride ] = in[ jj * inTStride ] * mul;
            }
         }
         // TODO: We can speed up this code
         // by computing the cos and sin once outside the loop, and tabulating cos and sin values to be used inside
         // the loop. To create the tables, we need a function like SetNumberOfThreads to receive also the processing
         // dimension. Mike's description of this:
         //
         //  >   Below we need the quantities:
         //  >
         //  >   cos( Fx X + Fy Y + ... ) and sin( Fx X + ... )
         //  >
         //  >   Using exp( ja ) exp( jb ) = exp( j ( a + b ))    ->
         //  >
         //  >   ( cos( a ) + j sin( a ) ) * ( cos( b ) + j sin( b ) ) =
         //  >   cos( a + b ) + j sin( a + b )    ->
         //  >
         //  >   cos( a + b ) = cos( a ) cos( b ) - sin( a ) sin( b )
         //  >   sin( a + b ) = sin( a ) cos( b ) + cos( a ) sin( b )
         //  >
         //  >   Therefore:
         //  >
         //  >   cos( Fx X + Fy Y ... ) =
         //  >   cos( Fx X ) cos( Fy Y + Fz Z + ... ) -
         //  >   sin( Fx X ) sin( Fy Y + Fz Z + ... )
         //  >
         //  >   These formulas are separable in the X and the other dimensions. The
         //  >   cos( Fx X ) and sin( Fx X ) can be tabulated, while the other terms
         //  >   are evaluated on the fly. This is sort of an optimal balance between
         //  >   speed and memory usage.
      }
      ShiftFTLineFilter( FloatArray const& shift, UnsignedArray const& sizes )
            : shift_( shift ), sizes_( sizes ) {}
   private:
      FloatArray const& shift_;
      UnsignedArray const& sizes_;
};

} // namespace

void ShiftFT(
      Image const& in,
      Image& out,
      FloatArray shift
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims == 0, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      ArrayUseParameter( shift, nDims, 0.0 );
   DIP_END_STACK_TRACE
   DataType dt = DataType::SuggestComplex( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_COMPLEX( lineFilter, ShiftFTLineFilter, ( shift, in.Sizes() ), dt );
   Framework::ScanMonadic( in, out, dt, dt, in.TensorElements(), *lineFilter, Framework::ScanOption::NeedCoordinates );
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
   String colorSpace = c_in.ColorSpace();

   // Create output
   out.ReForge( outSizes, in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
   DataType bufferType = DataType::SuggestFlex( out.DataType() );

   // Find line filter
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_FLEX( lineFilter, SkewLineFilter, ( method, shearArray, offset, axis, boundaryCondition ), bufferType );

   // Call line filter through framework
   Framework::Separable( in, out, bufferType, out.DataType(), process, { border }, boundaryCondition, *lineFilter,
         Framework::SeparableOption::AsScalarImage + Framework::SeparableOption::DontResizeOutput + Framework::SeparableOption::UseInputBuffer );
   out.SetPixelSize( pixelSize );
   out.SetColorSpace( colorSpace );

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
   String colorSpace = c_in.ColorSpace();
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
   bool shiftOrigin1 = (( in.Size( dimension1 ) & 1 ) == 0 ) && ( in.Stride( dimension1 ) < 0 );
   bool shiftOrigin2 = (( in.Size( dimension2 ) & 1 ) == 0 ) && ( in.Stride( dimension2 ) < 0 );
   dip::uint origin1 = in.Size( dimension1 ) / 2 - ( shiftOrigin1 ? 1 : 0 );
   dip::uint origin2 = in.Size( dimension2 ) / 2 - ( shiftOrigin2 ? 1 : 0 );
   dfloat maxDisplacement = std::abs( std::sin( angle )) * static_cast< dfloat >( std::max( origin1, origin2 ));
   if( maxDisplacement < 1e-3 ) {
      // For very small rotations, let's not bother interpolating
      //    But we do need to take care of the correct location of the origin, see the more complex case below
      //    for details on why and how this works.
      RangeArray region( nDims );
      UnsignedArray newSize = in.Sizes();
      if( shiftOrigin1 ) {
         newSize[ dimension1 ] += 2;
         region[ dimension1 ].start = 2;
      }
      if( shiftOrigin2 ) {
         newSize[ dimension2 ] += 2;
         region[ dimension2 ].start = 2;
      }
      if( shiftOrigin1 || shiftOrigin2 ) {
         out.ReForge( newSize, in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
         out.CopyNonDataProperties( in );
         DIP_STACK_TRACE_THIS( out.At( region ).Copy( in ));
         ExtendRegion( out, region, bc );
      } else {
         out.Copy( in );
      }
      return;
   }
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
   newSize[ dimension2 ] = std::min(
         out.Size( dimension2 ),
         2 * static_cast< dip::uint >( std::ceil(( size1 * sin_angle + size2 * cos_angle ) / 2.0 )) + ( in.Size( dimension2 ) & 1 ));
   // Next, check for the case where we shifted the origin, which means we need to adjust the size of `out`
   // so that the pixel at the origin of `in` is also at the origin of `out`.
   // If `newSize` is too large to allow this shift by cropping alone, we need to add two pixels to the
   // left and/or top. This happens when rotating very close to 90, 180 or 270 degrees, when `newSize`
   // is the same as `in.Sizes()`, but the origin shifted.
   bool extend1 = false;
   bool extend2 = false;
   RangeArray region( nDims );
   if( origin1 < ( newSize[ dimension1 ] / 2 )) {
      DIP_ASSERT( shiftOrigin1 );   // This can happen only if the origin was shifted
      if( newSize[ dimension1 ] < out.Size( dimension1 )) {
         // We can solve this case by adding an extra pixel, the skewing already gave that to us.
         newSize[ dimension1 ] = origin1 * 2;
         DIP_ASSERT( newSize[ dimension1 ] <= out.Size( dimension1 ));
      } else {
         // We need to extend `out`
         extend1 = true;
         newSize[ dimension1 ] += 2;
         region[ dimension1 ].start = 2;
         ++origin1;
      }
   }
   if( origin2 < ( newSize[ dimension2 ] / 2 )) {
      DIP_ASSERT( shiftOrigin2 );   // This can happen only if the origin was shifted
      if( newSize[ dimension2 ] < out.Size( dimension2 )) {
         // We can solve this case by adding an extra pixel, the skewing already gave that to us.
         newSize[ dimension2 ] = origin2 * 2;
         DIP_ASSERT( newSize[ dimension2 ] <= out.Size( dimension2 ));
      } else {
         // We need to extend `out`
         extend2 = true;
         newSize[ dimension2 ] += 2;
         region[ dimension2 ].start = 2;
         ++origin2;
      }
   }
   // First cut the dimensions we don't need to extend
   // The section below is similar to out.Crop( newSize ), except we use `origin1` and `origin2` to determine where to cut.
   UnsignedArray origin( nDims, 0 );
   UnsignedArray cropSize = out.Sizes();
   if( !extend1 ) {
      cropSize[ dimension1 ] = newSize[ dimension1 ];
      origin[ dimension1 ] = origin1 - ( newSize[ dimension1 ] / 2 );
      DIP_ASSERT( origin[ dimension1 ] <= out.Size( dimension1 ) - newSize[ dimension1 ] );
   }
   if( !extend2 ) {
      cropSize[ dimension2 ] = newSize[ dimension2 ];
      origin[ dimension2 ] = origin2 - ( newSize[ dimension2 ] / 2 );
      DIP_ASSERT( origin[ dimension2 ] <= out.Size( dimension2 ) - newSize[ dimension2 ] );
   }
   out.SetOriginUnsafe( out.Pointer( origin ));
   out.SetSizesUnsafe( cropSize );
   // Next, extend as needed
   if( extend1 || extend2 ) {
      Image newOut;
      newOut.CopyProperties( out );
      newOut.SetSizes( newSize );
      newOut.Forge();
      DIP_STACK_TRACE_THIS( newOut.At( region ).Copy( out )); // This will throw if sizes don't match -- it means our assumption is wrong!
      ExtendRegion( newOut, region, bc );
      out.swap( newOut );
   }
   // Fix pixel sizes
   if( pixelSize.IsDefined() ) {
      if( pixelSize[ dimension1 ] != pixelSize[ dimension2 ] ) {
         dip::uint k = out.Dimensionality() - 1;
         pixelSize.Set( k, pixelSize[ k ] ); // This ensures that all elements of pixelSize are defined, so that the commands below only change a single dimension
         pixelSize.Set( dimension1, Units::Pixel() );
         pixelSize.Set( dimension2, Units::Pixel() );
      }
      out.SetPixelSize( pixelSize );
   }
   out.SetColorSpace( colorSpace );
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"
#include "diplib/transform.h"
#include "diplib/testing.h"
#include <numeric>

template< typename T >
T abs_diff( T a, T b ) {
   return std::abs( a - b );
}
template< typename T >
T abs_diff( std::complex< T > a, std::complex< T > b ) {
   return std::abs( a - b );
}


DOCTEST_TEST_CASE("[DIPlib] testing the interpolation functions (except Fourier)") {

   // 1- Test all methods (except FOURIER) using sfloat, and unit zoom

   std::vector< dip::sfloat > buffer( 100 );
   std::vector< dip::sfloat > tmpSpline( 200 );
   std::iota( buffer.begin(), buffer.end(), 0.0f );
   dip::sfloat* input = buffer.data() + 20; // we use the elements 20-80, and presume 20 elements as boundary on either side
   dip::uint inSize = 60;

   dip::sfloat shift = 4.3f;
   dip::uint outSize = inSize;
   std::vector< dip::sfloat > output( outSize, -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), outSize, 1.0, shift, tmpSpline.data() );
   bool error = false;
   dip::sfloat offset = 20.0f + shift;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), outSize, 1.0, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ))) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), outSize, 1.0, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ));
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 3e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 5 >( input, output.data(), outSize, 1.0, shift );
   error = false;
   //std::cout << "\nLanczos 5: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ));
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii )) > 1.1e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 2- Test all methods (except FOURIER) using sfloat, and zoom > 1

   shift = -3.4f;
   dip::sfloat scale = 3.3f;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), outSize, scale, shift, tmpSpline.data() );
   offset = 20.0f + shift;
   dip::sfloat step = 1.0f / scale;
   error = false;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ) * step )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 3.2e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 8 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 7.2e-3; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 3- Test all methods (except FOURIER) using sfloat, and zoom < 1

   shift = 10.51f;
   scale = 0.41f;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::BSpline< dip::sfloat >( input, output.data(), outSize, scale, shift, tmpSpline.data() );
   offset = 20.0f + shift;
   step = 1.0f / scale;
   error = false;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Linear< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::sfloat >( input, output.data(), outSize, scale, shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( output[ ii ], std::round( offset + static_cast< dip::sfloat >( ii ) * step )) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 2 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 3.2e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::sfloat, 8 >( input, output.data(), outSize, scale, shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step );
      error |= abs_diff( output[ ii ], offset + static_cast< dip::sfloat >( ii ) * step ) > 7.2e-3; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   // 4- Test all methods (except FOURIER) using dcomplex

   std::vector< dip::dcomplex > d_buffer( 100 );
   std::vector< dip::dcomplex > d_tmpSpline( 200 );
   for( dip::uint ii = 0; ii < d_buffer.size(); ++ii ) {
      d_buffer[ ii ] = dip::dcomplex{ dip::dfloat( ii ), 200.0 - dip::dfloat( ii ) };
   }
   dip::dcomplex* d_input = d_buffer.data() + 20; // we use the elements 20-80, and presume 20 elements as boundary on either side

   dip::dfloat d_shift = -3.4;
   dip::dfloat d_scale = 5.23;
   outSize = dip::interpolation::ComputeOutputSize( inSize, d_scale );
   std::vector< dip::dcomplex > d_output( outSize, -1e6 );
   dip::interpolation::BSpline< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift, d_tmpSpline.data() );
   dip::dcomplex d_offset{ 20.0 + d_shift, 200.0 - 20.0 - d_shift };
   dip::dfloat d_step = 1.0 / d_scale;
   error = false;
   for( dip::uint ii = 2; ii < outSize - 11; ++ii ) { // BSpline is not as precise near the edges. Do we need a larger boundary?
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::FourthOrderCubicSpline< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::ThirdOrderCubicSpline< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Linear< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::NearestNeighbor< dip::dcomplex >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      dip::dcomplex res = d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step };
      error |= abs_diff( d_output[ ii ], { std::round( res.real() ), std::round( res.imag() ) } ) > 1e-4;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::dcomplex, 2 >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   //std::cout << "\nLanczos 2: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } );
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 4.5e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

   std::fill( d_output.begin(), d_output.end(), -1e6f );
   dip::interpolation::Lanczos< dip::dcomplex, 8 >( d_input, d_output.data(), outSize, d_scale, d_shift );
   error = false;
   //std::cout << "\nLanczos 8: ";
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      //std::cout << ", " << abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } );
      error |= abs_diff( d_output[ ii ], d_offset + dip::dcomplex{ dip::dfloat( ii ) * d_step, -dip::dfloat( ii ) * d_step } ) > 1.1e-2; // input data is worst-case for Lanczos interpolation...
   }
   //std::cout << "\n\n";
   DOCTEST_CHECK_FALSE( error );

}

DOCTEST_TEST_CASE("[DIPlib] testing the Fourier interpolation function") {

   // 1- Test using unit zoom (shift only)

   std::vector< dip::scomplex > input( 100 );
   dip::uint inSize = input.size();
   for( dip::uint ii = 0; ii < inSize; ++ii ) {
      auto x = static_cast< dip::dfloat >( ii ) / static_cast< dip::dfloat >( inSize );
      input[ ii ] = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
   }
   dip::dfloat shift = 4.3;
   dip::uint outSize = inSize;
   std::vector< dip::scomplex > output( outSize, -1e6f );

   dip::DFT< dip::sfloat > ft( inSize, false );
   dip::DFT< dip::sfloat > ift( outSize, true );
   std::vector< dip::scomplex > buffer( dip::interpolation::FourierBufferSize( ft, ift ));
   dip::interpolation::Fourier< dip::sfloat >( input.data(), output.data(), shift, ft, ift, nullptr, buffer.data() );

   bool error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      auto x = ( static_cast< dip::dfloat >( ii ) - shift ) / static_cast< dip::dfloat >( outSize );
      dip::scomplex expected = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
      error |= abs_diff( output[ ii ], expected ) > 1e-6;
      //std::cout << x << ": " << output[ ii ] << " == " << expected << std::endl;
   }
   DOCTEST_CHECK_FALSE( error );

   // 2- Test using zoom > 1

   shift = -3.4;
   dip::dfloat scale = 3.3;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   scale = static_cast< dip::dfloat >( outSize ) / static_cast< dip::dfloat >( inSize );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );
   ft.Initialize( inSize, false );
   ift.Initialize( outSize, true );
   buffer.resize( dip::interpolation::FourierBufferSize( ft, ift ));
   dip::interpolation::Fourier< dip::sfloat >( input.data(), output.data(), shift, ft, ift, nullptr, buffer.data() );

   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      auto x = ( static_cast< dip::dfloat >( ii ) - shift * scale ) / static_cast< dip::dfloat >( outSize );
      dip::scomplex expected = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
      error |= abs_diff( output[ ii ], expected ) > 1e-6;
      //std::cout << x << ": " << output[ ii ] << " == " << expected << std::endl;
   }
   DOCTEST_CHECK_FALSE( error );

   // 3- Test using zoom < 1

   shift = 10.51;
   scale = 0.41;
   outSize = dip::interpolation::ComputeOutputSize( inSize, scale );
   scale = static_cast< dip::dfloat >( outSize ) / static_cast< dip::dfloat >( inSize );
   output.resize( outSize );
   std::fill( output.begin(), output.end(), -1e6f );

   ft.Initialize( inSize, false );
   ift.Initialize( outSize, true );
   buffer.resize( dip::interpolation::FourierBufferSize( ft, ift ));
   dip::interpolation::Fourier< dip::sfloat >( input.data(), output.data(), shift, ft, ift, nullptr, buffer.data() );

   error = false;
   for( dip::uint ii = 0; ii < outSize; ++ii ) {
      auto x = ( static_cast< dip::dfloat >( ii ) - shift * scale ) / static_cast< dip::dfloat >( outSize );
      dip::scomplex expected = static_cast< dip::sfloat >( std::cos( 4.0 * dip::pi * x ));
      error |= abs_diff( output[ ii ], expected ) > 1e-6;
      //std::cout << x << ": " << output[ ii ] << " == " << expected << std::endl;
   }
   DOCTEST_CHECK_FALSE( error );

}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::Rotation() preserves the origin location") {
   dip::Image out;
   // 1- Even sized image
   dip::Image img( { 64, 64 }, 1, dip::DT_SFLOAT );
   img.Fill( 0 );
   dip::DrawBandlimitedPoint( img, { 32, 32 } );
   // Rotations of 0, 90, 180 and 270 degrees -- no interpolation
   dip::Rotation2D( img, out, 0 ); // Note that dip::Rotation2D() calls dip::Rotation()
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out ));
   dip::Rotation2D( img, out, dip::pi / 2 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1e-6 ));
   dip::Rotation2D( img, out, dip::pi );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1e-6 ));
   dip::Rotation2D( img, out, 3 * dip::pi / 2 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1e-6 ));
   // Rotations close to 0, 90, 180 and 270 -- yes interpolation
   dip::Rotation2D( img, out, 0 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   dip::Rotation2D( img, out, dip::pi / 2 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   dip::Rotation2D( img, out, dip::pi + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   dip::Rotation2D( img, out, 3 * dip::pi / 2 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   // Larger rotations
   dip::Rotation2D( img, out, dip::pi/4 - 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 0.008 ));
   dip::Rotation2D( img, out, dip::pi/4 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 0.008 ));

   // 2- Odd sized image
   img.Crop( { 63, 63 } );
   // Rotations of 0, 90, 180 and 270 degrees -- no interpolation -- an no need to crop
   dip::Rotation2D( img, out, 0 );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out ));
   dip::Rotation2D( img, out, dip::pi / 2 );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1e-6 ));
   dip::Rotation2D( img, out, dip::pi );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1e-6 ));
   dip::Rotation2D( img, out, 3 * dip::pi / 2 );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1e-6 ));
   // Rotations close to 0, 90, 180 and 270 -- yes interpolation
   dip::Rotation2D( img, out, 0 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   dip::Rotation2D( img, out, dip::pi / 2 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   dip::Rotation2D( img, out, dip::pi + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   dip::Rotation2D( img, out, 3 * dip::pi / 2 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 1.1e-5 ));
   // Larger rotations
   dip::Rotation2D( img, out, dip::pi/4 - 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 0.008 ));
   dip::Rotation2D( img, out, dip::pi/4 + 1e-3 );
   out.Crop( img.Sizes() );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out, 0.008 ));
}

DOCTEST_TEST_CASE("[DIPlib] testing dip::ShiftFT() against dip::Shift()") {
   dip::Image img( { 83, 120 }, 1, dip::DT_SFLOAT );
   img.Fill( 0 );
   dip::DrawBandlimitedBall( img, 30, img.GetCenter(), { 255 }, dip::S::FILLED, 5 );
   dip::Image ft = dip::FourierTransform( img );
   dip::ShiftFT( ft, ft, { -10.3, 15.2 } );
   dip::Image shifted = dip::FourierTransform( ft, { "inverse", "real" } );
   dip::Image shifted2 = dip::Shift( img, { -10.3, 15.2 }, "fourier" );
   DOCTEST_CHECK( dip::testing::CompareImages( shifted, shifted2, 0.015 ));
}

#endif // DIP__ENABLE_DOCTEST
