/*
 * DIPlib 3.0
 * This file contains definitions for image drawing functions
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
#include "diplib/generation.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/saturated_arithmetic.h"
#include "draw_support.h"

namespace dip {

namespace {

template< typename TPI >
class DrawBandlimitedPointLineFilter : public Framework::ScanLineFilter {
   public:
      DrawBandlimitedPointLineFilter( FloatArray const& sigmas, FloatArray const& origin, Image::Pixel const& value, dip::uint nTensor, dfloat truncation ) {
         CopyPixelToVector( value, value_, nTensor );
         dip::uint nDims = sigmas.size();
         blob1d_.resize( nDims );
         origin_.resize( nDims );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            dfloat halfSize = truncation * sigmas[ ii ];
            dip::sint leftSide = ceil_cast( origin[ ii ] - halfSize ); // first pixel in box
            dip::sint rightSide = floor_cast( origin[ ii ] + halfSize ); // last pixel in box
            dfloat offset = origin[ ii ] - static_cast< dfloat >( leftSide ); // offset of origin w.r.t. first pixel in box
            origin_[ ii ] = leftSide;
            dip::uint size = static_cast< dip::uint >( rightSide - leftSide + 1 );
            blob1d_[ ii ].resize( size, 0 );
            dfloat factor = -1.0 / ( 2.0 * sigmas[ ii ] * sigmas[ ii ] );
            dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigmas[ ii ] );
            for( dip::uint jj = 0; jj < size; ++jj ) {
               dfloat rad = static_cast< dfloat >( jj ) - offset;
               blob1d_[ ii ][ jj ] = std::exp( factor * ( rad * rad )) * norm;
            }
         }
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint nTensor = params.outBuffer[ 0 ].tensorLength;
         dip::uint dim = params.dimension;
         // Find position w.r.t. box origin, and compute product of Gaussians in dimensions perpendicular to this one
         dfloat weight = 1.0;
         for( dip::uint ii = 0; ii < origin_.size(); ++ii ) {
            if( ii != dim ) {
               dip::sint p = static_cast< dip::sint >( params.position[ ii ] ) - origin_[ ii ];
               if(( p < 0 ) || ( p >= static_cast< dip::sint >( blob1d_[ ii ].size() ))) {
                  return; // we're outside of the blob's box
               }
               weight *= blob1d_[ ii ][ static_cast< dip::uint >( p ) ];
            }
         }
         // Move along this line to the start of the box
         dip::sint pos = origin_[ dim ];
         dip::uint end = std::min( blob1d_[ dim ].size(), params.bufferLength - static_cast< dip::uint >( pos )) - 1;
         dip::uint start = 0;
         if( pos < 0 ) {
            start = static_cast< dip::uint >( -pos );
            if( start > end ) {
               return;  // we're outside of the blob's box
            }
            pos = 0;
         }
         out += pos * stride;
         // Write Gaussian to image line
         for( dip::uint jj = start; jj <= end; ++jj, out += stride ) {
            dip::sint offset = 0;
            dfloat w = weight * blob1d_[ dim ][ jj ];
            for( dip::uint ii = 0; ii < nTensor; ++ii ) {
               FlexType< TPI > x = static_cast< FloatType< TPI >>( w ) * value_[ ii ];
               out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] ) + x );
               offset += tensorStride;
            }
         }
      }
   private:
      IntegerArray origin_;                        // top-left corner of box containing the blob
      std::vector< std::vector< dfloat >> blob1d_; // multiply 1D blobs to get the nD blob.
      std::vector< FlexType< TPI >> value_;        // scaling of the blob for each channel.
};

} // namespace

void DrawBandlimitedPoint(
      Image& out,
      FloatArray origin,
      Image::Pixel const& value,
      FloatArray sigmas,
      dfloat truncation
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( origin.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sigmas, nDims, 1.0 ));
   DIP_THROW_IF( truncation <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   FloatArray sizes = sigmas;
   for( auto& s: sizes ) {
      DIP_THROW_IF( s <= 0.0, E::INVALID_PARAMETER );
      s *= truncation * 2.0;
   }
   dip::Image tmp = out;
   if( !NarrowImageView( tmp, sizes, origin )) {
      return;
   }
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_NONBINARY( lineFilter, DrawBandlimitedPointLineFilter, ( sigmas, origin, value, tmp.TensorElements(), truncation ), tmp.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleOutput( tmp, tmp.DataType(), *lineFilter, Framework::ScanOption::NeedCoordinates ));
   // NOTE: because of the way we call the Scan framework, we know for sure that it won't use a temporary buffer for
   // the output samples, and thus we get to write directly in the output. We can modify only select pixels in the
   // output image.
}

namespace {

template< typename TPI >
class DrawBandlimitedLineLineFilter : public Framework::ScanLineFilter {
   public:
      DrawBandlimitedLineLineFilter( FloatArray const& start, FloatArray const& end, Image::Pixel const& value,
                                     dip::uint nTensor, dfloat sigma, dfloat margin ):
            A_( start ), B_( end ), sigma_( sigma ), scale_( -0.5 / ( sigma_ * sigma_ )), margin2_( margin * margin ) {
         CopyPixelToVector( value, value_, nTensor );
         FloatType< TPI > norm = static_cast< FloatType< TPI >>( 1.0 / ( std::sqrt( 2.0 * pi ) * sigma_ ));
         for( auto& v: value_ ) {
            v *= norm;
         }
         // Closest point on line AB to point P: C = A + t * BA, t = dot(PA,BA) / dot(BA,BA)
         // Distance of point P to line AB: norm(PC) = norm(PA - t * BA)
         // We pre-compute BA and dot(BA,BA)
         BA_ = B_;
         BA_ -= A_;
         dot_BA_BA_ = 0;
         for( dip::uint ii = 0; ii < A_.size(); ++ii ) {
            dot_BA_BA_ += BA_[ ii ] * BA_[ ii ];
         }
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint nTensor = params.outBuffer[ 0 ].tensorLength;
         dip::uint length = params.bufferLength;
         dip::uint dim = params.dimension;
         // For this line, we pre-compute all dimensions components except `dim` for PA, PB, dot(PA,BA)
         FloatArray PA = FloatArray( params.position );
         PA -= A_;
         FloatArray PB = FloatArray( params.position );
         PB -= B_;
         dfloat dot_PA_BA = 0;
         for( dip::uint ii = 0; ii < A_.size(); ++ii ) {
            if( ii != dim ) {
               dot_PA_BA += PA[ ii ] * BA_[ ii ];
            }
         }
         // Iterate over line and compute the rest...
         for( dip::uint jj = 0; jj < length; ++jj, ++PA[ dim ], ++PB[ dim ], out += stride ) {
            dfloat pdot_PA_BA = dot_PA_BA + PA[ dim ] * BA_[ dim ];
            dfloat t = pdot_PA_BA / dot_BA_BA_;
            dfloat distance2 = 0;
            if( t < 0 ) {
               // We're not projecting onto the line segment, use the distance to A directly.
               for( dip::uint ii = 0; ii < A_.size(); ++ii ) {
                  distance2 += PA[ ii ] * PA[ ii ];
               }
            } else if ( t > 1 ) {
               // We're not projecting onto the line segment, use the distance to B directly.
               for( dip::uint ii = 0; ii < A_.size(); ++ii ) {
                  distance2 += PB[ ii ] * PB[ ii ];
               }
            } else {
               for( dip::uint ii = 0; ii < A_.size(); ++ii ) {
                  dfloat v = PA[ ii ] - t * BA_[ ii ];
                  distance2 += v * v;
               }
            }
            if( distance2 <= margin2_ ) {
               dip::sint offset = 0;
               dfloat weight = std::exp( distance2 * scale_ );
               for( dip::uint ii = 0; ii < nTensor; ++ii ) {
                  FlexType< TPI > x = static_cast< FloatType< TPI >>( weight ) * value_[ ii ];
                  out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] ) + x );
                  offset += tensorStride;
               }
            }
         }
      }
   private:
      FloatArray const& A_;
      FloatArray const& B_;
      FloatArray BA_;
      dfloat dot_BA_BA_;
      std::vector< FlexType< TPI >> value_; // scaling of the blob for each channel.
      dfloat sigma_;
      dfloat scale_;
      dfloat margin2_; // = ( sigma * truncation )^2
};

} // namespace

void DrawBandlimitedLine(
      Image& out,
      FloatArray start,
      FloatArray end,
      Image::Pixel const& value,
      dfloat sigma,
      dfloat truncation
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   if( start == end ) {
      FloatArray sigmas( nDims, sigma );
      DrawBandlimitedPoint( out, start, value, sigmas, truncation );
      return;
   }
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( start.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( end.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( sigma <= 0.0, E::INVALID_PARAMETER );
   DIP_THROW_IF( truncation <= 0.0, E::INVALID_PARAMETER );
   dfloat margin = sigma * truncation;
   FloatArray sizes( nDims );
   FloatArray origin( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      sizes[ ii ] = std::abs( start[ ii ] - end[ ii ] ) + 2.0 * margin;
      origin[ ii ] = ( start[ ii ] + end[ ii ] ) / 2.0;
   }
   start -= origin;
   end -= origin;
   Image tmp = out;
   if( !NarrowImageView( tmp, sizes, origin )) {
      return;
   }
   start += origin;
   end += origin;
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_NONBINARY( lineFilter, DrawBandlimitedLineLineFilter, ( start, end, value, tmp.TensorElements(), sigma, margin ), tmp.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleOutput( tmp, tmp.DataType(), *lineFilter, Framework::ScanOption::NeedCoordinates ));
   // NOTE: because of the way we call the Scan framework, we know for sure that it won't use a temporary buffer for
   // the output samples, and thus we get to write directly in the output. We can modify only select pixels in the
   // output image.
}

namespace {

template< typename TPI >
void AddLine(
      TPI* out,         // points at the beginning of the line
      dip::sint start,
      dip::sint end,
      dip::sint length, // line length
      dip::sint stride,
      std::vector< FlexType< TPI >> value,
      dip::sint tensorStride
) {
   if(( start > length - 1 ) || ( end < 0 )) {
      return; // nothing to do here
   }
   start = std::max( start, dip::sint( 0 ));
   end = std::min( end, length - 1 );
   out += static_cast< dip::sint >( start ) * stride;
   for( dip::sint jj = start; jj <= end; ++jj, out += stride ) {
      dip::sint offset = 0;
      for( dip::uint ii = 0; ii < value.size(); ++ii ) {
         out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] ) + value[ ii ] );
         offset += tensorStride;
      }
   }
}

template< typename TPI >
void BallBlurredEdge(
      TPI* out,         // points at the beginning of the line
      dip::sint start,
      dip::sint end,
      dip::sint length, // line length
      dip::sint stride,
      std::vector< FlexType< TPI >> value,
      dip::sint tensorStride,
      dfloat distance2,
      dfloat origin,
      dfloat sigma,
      dfloat radius
) {
   if(( start > length - 1 ) || ( end < 0 )) {
      return; // nothing to do here
   }
   start = std::max( start, dip::sint( 0 ));
   end = std::min( end, length - 1 );
   dfloat norm = -1.0 / ( sigma * std::sqrt( 2.0 ));
   out += static_cast< dip::sint >( start ) * stride;
   for( dip::sint jj = start; jj <= end; ++jj, out += stride ) {
      dfloat d = static_cast< dfloat >( jj ) - origin;
      d = std::sqrt( distance2 + d * d ) - radius;
      dfloat weight = 0.5 + 0.5 * std::erf( d * norm );
      dip::sint offset = 0;
      for( dip::uint ii = 0; ii < value.size(); ++ii ) {
         out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] )
                                            + static_cast< FloatType< TPI >>( weight ) * value[ ii ] );
         offset += tensorStride;
      }
   }
}

template< typename TPI >
void BallBlurredLine(
      TPI* out,         // points at the beginning of the line
      dip::sint start,
      dip::sint end,
      dip::sint length, // line length
      dip::sint stride,
      std::vector< FlexType< TPI >> value,
      dip::sint tensorStride,
      dfloat distance2,
      dfloat origin,
      dfloat sigma,
      dfloat radius
) {
   if(( start > length - 1 ) || ( end < 0 )) {
      return; // nothing to do here
   }
   start = std::max( start, dip::sint( 0 ));
   end = std::min( end, length - 1 );
   dfloat scale = -0.5 / ( sigma * sigma );
   out += static_cast< dip::sint >( start ) * stride;
   for( dip::sint jj = start; jj <= end; ++jj, out += stride ) {
      dfloat d = static_cast< dfloat >( jj ) - origin;
      d = std::sqrt( distance2 + d * d ) - radius;
      dfloat weight = std::exp( d * d * scale );
      dip::sint offset = 0;
      for( dip::uint ii = 0; ii < value.size(); ++ii ) {
         out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] )
                                            + static_cast< FloatType< TPI >>( weight ) * value[ ii ] );
         offset += tensorStride;
      }
   }
}

template< typename TPI >
class DrawBandlimitedBallLineFilter : public Framework::ScanLineFilter {
   public:
      DrawBandlimitedBallLineFilter( dfloat diameter, FloatArray const& origin, Image::Pixel const& value,
                                     dip::uint nTensor, bool filled, dfloat sigma, dfloat margin ) :
            radius_( diameter / 2.0 ), origin_( origin ), filled_( filled ), sigma_( sigma ), margin_( margin ) {
         CopyPixelToVector( value, value_, nTensor );
         if( !filled_ ) {
            FloatType< TPI > norm = static_cast< FloatType< TPI >>( 1.0 / ( std::sqrt( 2.0 * pi ) * sigma_ ));
            for( auto& v: value_ ) {
               v *= norm;
            }
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint nTensorElements ) override {
         return ( filled_ ? 74 : 45 ) + nTensorElements; // This is not correct, we only do this for a subset of pixels on each line...
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::sint length = static_cast< dip::sint >( params.bufferLength );
         dip::uint dim = params.dimension;
         dip::uint nDims = origin_.size(); // or params.position.size()
         dfloat distance2 = 0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != dim ) {
               dfloat d = static_cast< dfloat >( params.position[ ii ] ) - origin_[ ii ];
               distance2 += d * d;
            }
         }
         dfloat outerRadius = radius_ + margin_;
         if( distance2 > outerRadius * outerRadius ) {
            return; // nothing to do on this line
         }
         dfloat outerWidth = std::sqrt( outerRadius * outerRadius - distance2 );
         dfloat innerRadius = std::max( 0.0, radius_ - margin_ );
         dfloat innerWidth = 0;
         if( distance2 <= innerRadius * innerRadius ) {
            // We cut through the core of the circle
            innerWidth = std::sqrt( innerRadius * innerRadius - distance2 );
         }
         // Now draw the blurry edges of the circle
         dip::sint start = ceil_cast( origin_[ dim ] - outerWidth );
         dip::sint end = ceil_cast( origin_[ dim ] - innerWidth ) - 1;
         if( filled_ ) {
            BallBlurredEdge( out, start, end, length, stride, value_, tensorStride, distance2, origin_[ dim ], sigma_, radius_ );
         } else {
            BallBlurredLine( out, start, end, length, stride, value_, tensorStride, distance2, origin_[ dim ], sigma_, radius_ );
         }
         if( innerWidth > 0 ) {
            start = end + 1;
            end = floor_cast( origin_[ dim ] + innerWidth );
            if( filled_ ) {
               AddLine( out, start, end, length, stride, value_, tensorStride );
            }
         }
         start = end + 1;
         end = floor_cast( origin_[ dim ] + outerWidth );
         if( filled_ ) {
            BallBlurredEdge( out, start, end, length, stride, value_, tensorStride, distance2, origin_[ dim ], sigma_, radius_ );
         } else {
            BallBlurredLine( out, start, end, length, stride, value_, tensorStride, distance2, origin_[ dim ], sigma_, radius_ );
         }
      }
   private:
      dfloat radius_;
      FloatArray const& origin_;
      std::vector< FlexType< TPI >> value_;
      bool filled_;
      dfloat sigma_;
      dfloat margin_; // = sigma * truncation
};

} // namespace

void DrawBandlimitedBall(
      Image& out,
      dfloat diameter,
      FloatArray origin,
      Image::Pixel const& value,
      String const& mode,
      dfloat sigma,
      dfloat truncation
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( origin.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( diameter <= 0.0, E::INVALID_PARAMETER );
   bool filled;
   DIP_STACK_TRACE_THIS( filled = BooleanFromString( mode, S::FILLED, S::EMPTY ));
   DIP_THROW_IF( sigma <= 0.0, E::INVALID_PARAMETER );
   DIP_THROW_IF( truncation <= 0.0, E::INVALID_PARAMETER );
   dfloat margin = sigma * truncation;
   FloatArray roiSizes( nDims, diameter + 2 * margin );
   dip::Image tmp = out;
   if( !NarrowImageView( tmp, roiSizes, origin )) {
      return;
   }
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_NONBINARY( lineFilter, DrawBandlimitedBallLineFilter, ( diameter, origin, value, tmp.TensorElements(), filled, sigma, margin ), tmp.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleOutput( tmp, tmp.DataType(), *lineFilter, Framework::ScanOption::NeedCoordinates ));
   // NOTE: because of the way we call the Scan framework, we know for sure that it won't use a temporary buffer for
   // the output samples, and thus we get to write directly in the output. We can modify only select pixels in the
   // output image.
}

namespace {

template< typename TPI >
void AddWeightedLine(
      TPI* out,         // points at the beginning of the line
      dip::sint start,
      dip::sint end,
      dip::sint length, // line length
      dip::sint stride,
      dfloat weight,
      std::vector< FlexType< TPI >> value,
      dip::sint tensorStride
) {
   if(( start > length - 1 ) || ( end < 0 )) {
      return; // nothing to do here
   }
   start = std::max( start, dip::sint( 0 ));
   end = std::min( end, length - 1 );
   out += static_cast< dip::sint >( start ) * stride;
   for( dip::sint jj = start; jj <= end; ++jj, out += stride ) {
      dip::sint offset = 0;
      for( dip::uint ii = 0; ii < value.size(); ++ii ) {
         out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] )
                                            + static_cast< FloatType< TPI >>( weight ) * value[ ii ] );
         offset += tensorStride;
      }
   }
}

template< typename TPI >
void BoxBlurredEdge(
      TPI* out,         // points at the beginning of the line
      dip::sint start,
      dip::sint end,
      dip::sint length, // line length
      dip::sint stride,
      dfloat distance,
      std::vector< FlexType< TPI >> value,
      dip::sint tensorStride,
      dfloat origin,
      dfloat sigma,
      dfloat radius
) {
   if(( start > length - 1 ) || ( end < 0 )) {
      return; // nothing to do here
   }
   start = std::max( start, dip::sint( 0 ));
   end = std::min( end, length - 1 );
   dfloat norm = -1.0 / ( sigma * std::sqrt( 2.0 ));
   out += static_cast< dip::sint >( start ) * stride;
   for( dip::sint jj = start; jj <= end; ++jj, out += stride ) {
      dfloat d = std::abs( static_cast< dfloat >( jj ) - origin ) - radius;
      d = std::max( d, distance );
      dfloat weight = 0.5 + 0.5 * std::erf( d * norm );
      dip::sint offset = 0;
      for( dip::uint ii = 0; ii < value.size(); ++ii ) {
         out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] )
                                            + static_cast< FloatType< TPI >>( weight ) * value[ ii ] );
         offset += tensorStride;
      }
   }
}

template< typename TPI >
void BoxBlurredLine(
      TPI* out,         // points at the beginning of the line
      dip::sint start,
      dip::sint end,
      dip::sint length, // line length
      dip::sint stride,
      dfloat distance,
      std::vector< FlexType< TPI >> value,
      dip::sint tensorStride,
      dfloat origin,
      dfloat sigma,
      dfloat radius
) {
   if(( start > length - 1 ) || ( end < 0 )) {
      return; // nothing to do here
   }
   start = std::max( start, dip::sint( 0 ));
   end = std::min( end, length - 1 );
   dfloat scale = -0.5 / ( sigma * sigma );
   out += static_cast< dip::sint >( start ) * stride;
   for( dip::sint jj = start; jj <= end; ++jj, out += stride ) {
      dfloat d = std::abs( static_cast< dfloat >( jj ) - origin ) - radius;
      d = std::max( d, distance );
      dfloat weight = std::exp( d * d * scale );
      dip::sint offset = 0;
      for( dip::uint ii = 0; ii < value.size(); ++ii ) {
         out[ offset ] = clamp_cast< TPI >( static_cast< FlexType< TPI >>( out[ offset ] )
                                            + static_cast< FloatType< TPI >>( weight ) * value[ ii ] );
         offset += tensorStride;
      }
   }
}

template< typename TPI >
class DrawBandlimitedBoxLineFilter : public Framework::ScanLineFilter {
   public:
      DrawBandlimitedBoxLineFilter( FloatArray const& halfSizes, FloatArray const& origin, Image::Pixel const& value,
                                    dip::uint nTensor, bool filled, dfloat sigma, dfloat margin ) :
            halfSizes_( halfSizes ), origin_( origin ), filled_( filled ), sigma_( sigma ), margin_( margin ) {
         CopyPixelToVector( value, value_, nTensor );
         if( !filled_ ) {
            FloatType< TPI > norm = static_cast< FloatType< TPI >>( 1.0 / ( std::sqrt( 2.0 * pi ) * sigma_ ));
            for( auto& v: value_ ) {
               v *= norm;
            }
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint nTensorElements ) override {
         return ( filled_ ? 55 : 25 ) + nTensorElements; // This is not correct, we only do this for a subset of pixels on each line...
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::sint length = static_cast< dip::sint >( params.bufferLength );
         dip::uint dim = params.dimension;
         dip::uint nDims = origin_.size(); // or params.position.size()
         dfloat distance = -margin_;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != dim ) {
               dfloat d = std::abs( static_cast< dfloat >( params.position[ ii ] ) - origin_[ ii ] ) - halfSizes_[ ii ];
               if( d > margin_ ) {
                  return; // we're outside the box
               }
               distance = std::max( d, distance );
            }
         }
         dfloat width = halfSizes_[ dim ];
         dfloat outerWidth = width + margin_;
         dfloat innerWidth = 0.0;
         // Draw the core of the box
         if( width > margin_ ) {
            innerWidth = width - margin_;
            dip::sint start = ceil_cast( origin_[ dim ] - innerWidth );
            dip::sint end = floor_cast( origin_[ dim ] + innerWidth );
            if( distance > -margin_ ) {
               // We go along an edge of the box
               dfloat weight = filled_
                               ? 0.5 + 0.5 * std::erf( -distance / ( sigma_ * std::sqrt( 2.0 )))
                               : std::exp( -0.5 * distance * distance / ( sigma_ * sigma_ ));
               AddWeightedLine( out, start, end, length, stride, weight, value_, tensorStride );
            } else {
               // We go through the middle of the box
               if( filled_ ) {
                  AddLine( out, start, end, length, stride, value_, tensorStride );
               }
            }
         }
         // Now draw the blurry edges of the box
         dip::sint start = ceil_cast( origin_[ dim ] - outerWidth );
         dip::sint end = ceil_cast( origin_[ dim ] - innerWidth ) - 1;
         if( filled_ ) {
            BoxBlurredEdge( out, start, end, length, stride, distance, value_, tensorStride, origin_[ dim ], sigma_, width );
         } else {
            BoxBlurredLine( out, start, end, length, stride, distance, value_, tensorStride, origin_[ dim ], sigma_, width );
         }
         if( innerWidth == 0 ) {
            // we don't have a "core", start where we left off
            start = end + 1;
         } else {
            start = floor_cast( origin_[ dim ] + innerWidth ) + 1;
         }
         end = floor_cast( origin_[ dim ] + outerWidth );
         if( filled_ ) {
            BoxBlurredEdge( out, start, end, length, stride, distance, value_, tensorStride, origin_[ dim ], sigma_, width );
         } else {
            BoxBlurredLine( out, start, end, length, stride, distance, value_, tensorStride, origin_[ dim ], sigma_, width );
         }
      }
   private:
      FloatArray const& halfSizes_;
      FloatArray const& origin_;
      std::vector< FlexType< TPI >> value_;
      bool filled_;
      dfloat sigma_;
      dfloat margin_; // = sigma * truncation
};

} // namespace

void DrawBandlimitedBox(
      Image& out,
      FloatArray sizes,
      FloatArray origin,
      Image::Pixel const& value,
      String const& mode,
      dfloat sigma,
      dfloat truncation
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sizes, nDims, 7.0 ));
   for( auto s: sizes ) {
      DIP_THROW_IF( s <= 0.0, E::INVALID_PARAMETER );
   }
   DIP_THROW_IF( origin.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   bool filled;
   DIP_STACK_TRACE_THIS( filled = BooleanFromString( mode, S::FILLED, S::EMPTY ));
   DIP_THROW_IF( sigma <= 0.0, E::INVALID_PARAMETER );
   DIP_THROW_IF( truncation <= 0.0, E::INVALID_PARAMETER );
   dfloat margin = sigma * truncation;
   FloatArray roiSizes = sizes;
   for( auto& s: roiSizes ) {
      s += 2 * margin; // compute half size
   }
   dip::Image tmp = out;
   if( !NarrowImageView( tmp, roiSizes, origin )) {
      return;
   }
   for( auto& s: sizes ) {
      s /= 2.0; // compute half size
   }
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_NONBINARY( lineFilter, DrawBandlimitedBoxLineFilter, ( sizes, origin, value, tmp.TensorElements(), filled, sigma, margin ), tmp.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleOutput( tmp, tmp.DataType(), *lineFilter, Framework::ScanOption::NeedCoordinates ));
   // NOTE: because of the way we call the Scan framework, we know for sure that it won't use a temporary buffer for
   // the output samples, and thus we get to write directly in the output. We can modify only select pixels in the
   // output image.
}


namespace {

template< typename TPI >
class GaussianEdgeClipLineFilter : public Framework::ScanLineFilter {
   public:
      GaussianEdgeClipLineFilter( Image::Pixel const& value, dfloat sigma, dfloat truncation ) :
            scale_( 1.0 / ( sigma * std::sqrt( 2.0 ))), margin_( sigma * truncation ) {
         CopyPixelToVector( value, value_, value.TensorElements() );
         for( auto& v: value_ ) {
            v *= FloatType< TPI >( 0.5 );
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint nTensorElements ) override {
         return 52 + nTensorElements; // but only on a subset of pixels...
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         DIP_ASSERT( params.outBuffer[ 0 ].tensorLength == value_.size() );
         for( dip::uint jj = 0; jj < params.bufferLength; ++jj ) {
            dfloat weight = *in;
            if( weight < -margin_ ) {
               weight = 0.0;
            } else if( weight > margin_ ) {
               weight = 2.0;
            } else {
               weight = ( 1.0 + std::erf( weight * scale_ )); // in [0.0,2.0]. Note that value_ is divided by 2.
            }
            dip::sint offset = 0;
            for( auto value: value_ ) {
               out[ offset ] = clamp_cast< TPI >( static_cast< FloatType< TPI >>( weight ) * value );
               offset += tensorStride;
            }
            in += inStride;
            out += outStride;
         }
      }
   private:
      std::vector< FlexType< TPI >> value_;
      dfloat scale_;
      dfloat margin_;
};

} // namespace

void GaussianEdgeClip(
      Image const& in,
      Image& out,
      Image::Pixel const& value,
      dfloat sigma,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( sigma <= 0.0, E::INVALID_PARAMETER );
   DIP_THROW_IF( truncation <= 0.0, E::INVALID_PARAMETER );
   DataType ovlDataType = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_FLOAT( lineFilter, GaussianEdgeClipLineFilter, ( value, sigma, truncation ), ovlDataType );
   DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, ovlDataType, ovlDataType, value.TensorElements(), *lineFilter ));
}

namespace {

template< typename TPI >
class GaussianLineClipLineFilter : public Framework::ScanLineFilter {
   public:
      GaussianLineClipLineFilter( Image::Pixel const& value, dfloat sigma, dfloat truncation ) :
            scale_( -0.5 / ( sigma * sigma )), margin_( sigma * truncation ) {
         CopyPixelToVector( value, value_, value.TensorElements() );
         FloatType< TPI > norm = static_cast< FloatType< TPI >>( 1.0 / ( std::sqrt( 2.0 * pi ) * sigma ));
         for( auto& v: value_ ) {
            v *= norm;
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint nTensorElements ) override {
         return 22 + nTensorElements; // but only on a subset of pixels...
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         DIP_ASSERT( params.outBuffer[ 0 ].tensorLength == value_.size() );
         for( dip::uint jj = 0; jj < params.bufferLength; ++jj ) {
            dfloat weight = *in;
            if( std::abs( weight ) > margin_ ) {
               weight = 0.0;
            } else {
               weight = std::exp( weight * weight * scale_ );
            }
            dip::sint offset = 0;
            for( auto value: value_ ) {
               out[ offset ] = clamp_cast< TPI >( static_cast< FloatType< TPI >>( weight ) * value );
               offset += tensorStride;
            }
            in += inStride;
            out += outStride;
         }
      }
   private:
      std::vector< FlexType< TPI >> value_;
      dfloat scale_;
      dfloat margin_;
};

} // namespace

void GaussianLineClip(
      Image const& in,
      Image& out,
      Image::Pixel const& value,
      dfloat sigma,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( sigma <= 0.0, E::INVALID_PARAMETER );
   DIP_THROW_IF( truncation <= 0.0, E::INVALID_PARAMETER );
   DataType ovlDataType = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_FLOAT( lineFilter, GaussianLineClipLineFilter, ( value, sigma, truncation ), ovlDataType );
   DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, ovlDataType, ovlDataType, value.TensorElements(), *lineFilter ));
}

} // namespace dip
