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
#include "diplib/border.h"
#include "diplib/generic_iterators.h"
#include "diplib/chain_code.h" // Polygon
#include "diplib/overload.h"
#include "draw_support.h"

namespace dip {

namespace {

template< typename TPI >
void dip__SetBorder( Image& out, Image::Pixel const& value, dip::uint size ) {
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   detail::ProcessBorders< TPI >(
         out,
         [ &value_ ]( auto* ptr, dip::sint tStride ) {
            for( auto v : value_ ) {
               *ptr = v;
               ptr += tStride;
            }
         }, size );
}

} // namespace

void SetBorder( Image& out, Image::Pixel const& value, dip::uint size ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_OVL_CALL_ALL( dip__SetBorder, ( out, value, size ), out.DataType() );
}


//
// Bresenham lines
//


namespace {

template< typename TPI >
void dip__DrawOneLine(
      TPI* origin,
      dip::sint stride,
      BresenhamLineIterator& iterator,
      std::vector< TPI > const& value
) {
   do {
      dip::sint offset = *iterator;
      for( auto v : value ) {
         origin[ offset ] = v;
         offset += stride;
      }
   } while( ++iterator );
}

template< typename TPI >
void dip__DrawLine(
      Image& out,
      BresenhamLineIterator& iterator,
      Image::Pixel const& value
) {
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   dip__DrawOneLine( static_cast< TPI* >( out.Origin() ), out.TensorStride(), iterator, value_ );
}

template< typename TPI >
void dip__DrawLines(
      Image& out,
      CoordinateArray const& points,
      Image::Pixel const& value
) {
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   dip::sint stride = out.TensorStride();
   TPI* origin = static_cast< TPI* >( out.Origin() );
   for( dip::uint jj = 1; jj < points.size(); ++jj ) {
      BresenhamLineIterator iterator( out.Strides(), points[ jj - 1 ], points[ jj ] );
      if( jj != 1 ) {
         ++iterator; // skip the first point, it's already drawn
      }
      dip__DrawOneLine( origin, stride, iterator, value_ );
   }
}

} // namespace

void DrawLine(
      Image& out,
      UnsignedArray const& start,
      UnsignedArray const& end,
      Image::Pixel const& value
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( start.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( end.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( !( start < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   DIP_THROW_IF( !( end < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   BresenhamLineIterator iterator( out.Strides(), start, end );
   DIP_OVL_CALL_ALL( dip__DrawLine, ( out, iterator, value ), out.DataType() );
}

void DrawLines(
      Image& out,
      CoordinateArray const& points,
      Image::Pixel const& value
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( points.size() < 2, E::ARRAY_ILLEGAL_SIZE );
   for( auto& point : points ) {
      DIP_THROW_IF( point.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
      DIP_THROW_IF( !( point < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   }
   DIP_OVL_CALL_ALL( dip__DrawLines, ( out, points, value ), out.DataType() );
}


//
// Polygons
//
// Filled polygon according to the algorithm described here:
// https://www.cs.rit.edu/~icss571/filling/how_to.html
//


namespace {

UnsignedArray VertexToUnsignedArray( VertexFloat p ) {
   p = p.Round();
   return { static_cast< dip::uint >( p.x ), static_cast< dip::uint >( p.y ) };
}

template< typename TPI >
void dip__FillLine(
      TPI* out,         // points at the beginning of the line
      dip::sint start,
      dip::sint end,
      dip::sint length, // line length
      dip::sint stride,
      std::vector< TPI > value,
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
         out[ offset ] = value[ ii ];
         offset += tensorStride;
      }
   }
}

template< typename TPI >
void dip__DrawPolygon(
      Image& out,
      Polygon const& polygon,
      Image::Pixel const& value,
      bool open
) {
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   dip::sint stride = out.TensorStride();
   TPI* origin = static_cast< TPI* >( out.Origin() );
   UnsignedArray prev = VertexToUnsignedArray( polygon.vertices[ 0 ] );
   for( dip::uint jj = 1; jj < polygon.vertices.size(); ++jj ) {
      UnsignedArray cur = VertexToUnsignedArray( polygon.vertices[ jj ] );
      BresenhamLineIterator iterator( out.Strides(), prev, cur );
      if( jj != 1 ) {
         ++iterator; // skip the first point, it's already drawn
      }
      dip__DrawOneLine( origin, stride, iterator, value_ );
      prev = cur;
   }
   if( !open ) {
      UnsignedArray cur = VertexToUnsignedArray( polygon.vertices[ 0 ] );
      BresenhamLineIterator iterator( out.Strides(), prev, cur );
      ++iterator; // skip the first point, it's already drawn
      dip__DrawOneLine( origin, stride, iterator, value_ );
   }
}

struct PolygonEdge {
   // y is the direction perpendicular to the scan lines
   dip::sint yMin;
   dip::sint yMax;
   dfloat x;         // initialized to value of x corresponding to yMin
   dfloat slope;     // increment x by this value for each unit increment of y
   PolygonEdge( VertexFloat pt1, VertexFloat pt2, bool horizontalScanLines ) {
      if( horizontalScanLines ) {
         if( pt1.y > pt2.y ) {
            std::swap( pt1, pt2 );
         }
         slope = ( pt1.x - pt2.x ) / ( pt1.y - pt2.y );
         yMin = round_cast( pt1.y );
         yMax = round_cast( pt2.y );
         x = pt1.x;
      } else {
         if( pt1.x > pt2.x ) {
            std::swap( pt1, pt2 );
         }
         slope = ( pt1.y - pt2.y ) / ( pt1.x - pt2.x );
         yMin = round_cast( pt1.x );
         yMax = round_cast( pt2.x );
         x = pt1.y;
      }
   }
   bool operator<( PolygonEdge const& other) const {
      return ( yMin == other.yMin ) ? ( x < other.x ) : yMin < other.yMin;
   }
   bool IsAlongScanLine() const {
      return !std::isfinite( slope );
   }
};

struct ActiveEdge {
   // y is the direction perpendicular to the scan lines
   dip::sint yMax;
   dfloat x;         // initialized to value of x corresponding to yMin
   dfloat slope;     // increment x by this value for each unit increment of y
   ActiveEdge( PolygonEdge const& edge ) {
      yMax = edge.yMax;
      x = edge.x;
      slope = edge.slope;
   }
   ActiveEdge& operator++() {
      x += slope;
      return *this;
   }
   bool operator<( ActiveEdge const& other) const {
      return x < other.x;
   }
};

template< typename TPI >
void dip__DrawFilledPolygon(
      Image& out,
      std::vector< PolygonEdge > const& edges,
      Image::Pixel const& value,
      bool horizontalScanLines
) {
   // Prepare pixel values
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   // Prepare some other constants
   dip::uint procDim = horizontalScanLines ? 0 : 1;
   dip::uint length = out.Size( procDim );
   dip::sint maxY = static_cast< dip::sint >( out.Size( 1 - procDim ));
   dip::sint stride = out.Stride( procDim );
   dip::sint tensorStride = out.TensorStride();
   // Initialize active edge list (will automatically be sorted)
   std::vector< ActiveEdge > active;
   dip::uint kk = 0;
   dip::sint y = edges[ kk ].yMin;
   if( y >= maxY ) {
      return; // Nothing to do!
   }
   while(( kk < edges.size()) && ( edges[ kk ].yMin == y )) {
      active.emplace_back( edges[ kk ] );
      ++kk;
   }
   // Create an iterator
   ImageIterator< TPI > line( out, procDim );
   // Move to the first scan line we need to process
   if( y > 0 ) {
      UnsignedArray startpos( 2, 0 );
      startpos[ 1 - procDim ] = static_cast< dip::uint >( y );
      line.SetCoordinates( startpos );
   }
   // Process a scan line at a time
   while( !active.empty() ) {
      if( y >= 0 ) {
         // Draw elements
         //DIP_ASSERT( !( active.size() & 1 )); // It's odd!? No matter, we ignore the last edge.
         for( dip::uint jj = 0; jj < active.size() - 1; jj += 2 ) {
            dip__FillLine( line.Pointer(),
                           round_cast( active[ jj ].x ),
                           round_cast( active[ jj + 1 ].x ),
                           static_cast< dip::sint >( length ), stride, value_, tensorStride );
         }
         // Next scan line, don't increment if y < 0!
         ++line;
      }
      // Increment y, but not past the last scan line.
      ++y;
      if( y >= maxY ) {
         return;
      }
      // Remove edges that are no longer active
      // NOTE: Removing edges with maxY == y means the bottom row of pixels on the polygon is not drawn.
      //       This is a simplification, let's hope it doesn't matter.
      for( dip::sint ii = static_cast< dip::sint >( active.size() - 1 ); ii >= 0; --ii ) {
         if( active[ static_cast< dip::uint >( ii ) ].yMax == y ) {
            active.erase( active.begin() + ii );
         }
      }
      // Update active edges
      for( auto& a : active ) {
         ++a;
      }
      // Add new edges, if any
      while(( kk < edges.size()) && ( edges[ kk ].yMin == y )) {
         active.emplace_back( edges[ kk ] );
         ++kk;
      }
      // Sort according to x values
      std::sort( active.begin(), active.end() );
   }
}

} // namespace

void DrawPolygon2D(
      Image& out,
      Polygon const& polygon,
      Image::Pixel const& value,
      String const& mode
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   bool filled = false;
   bool open = false;
   if( mode == S::FILLED ) {
      filled = true;
   } else if( mode == S::OPEN ) {
      open = true;
   } else {
      DIP_THROW_IF( mode != S::CLOSED, E::INVALID_FLAG );
   }
   DIP_THROW_IF( polygon.vertices.size() < ( open ? 2u : 3u ), E::ARRAY_ILLEGAL_SIZE ); // need at least 2 points to draw an open polygon, otherwise 3 points
   if( filled ) {
      bool horizontalScanLines = Framework::OptimalProcessingDim( out ) == 0;
      std::vector< PolygonEdge > edges;
      edges.reserve( polygon.vertices.size() + 1 ); // max number of edges, could be fewer if some are along scan lines
      for( dip::uint ii = 1; ii < polygon.vertices.size(); ++ii ) {
         PolygonEdge e( polygon.vertices[ ii - 1 ], polygon.vertices[ ii ], horizontalScanLines );
         if( !e.IsAlongScanLine() ) {
            edges.push_back( std::move( e ));
         }
      }
      PolygonEdge e( polygon.vertices.back(), polygon.vertices.front(), horizontalScanLines );
      if( !e.IsAlongScanLine() ) {
         edges.push_back( std::move( e ));
      }
      DIP_THROW_IF( edges.size() < 2, "The polygon has a bad shape" );
      std::sort( edges.begin(), edges.end() );
      DIP_OVL_CALL_ALL( dip__DrawFilledPolygon, ( out, edges, value, horizontalScanLines ), out.DataType());
   } else {
      // Test all points to be within the image
      BoundingBox bb{ VertexInteger{ 0, 0 },
                      VertexInteger{ static_cast< dip::sint >( out.Size( 0 ) - 1 ),
                                     static_cast< dip::sint >( out.Size( 1 ) - 1 ) }};
      for( auto point : polygon.vertices ) {
         point = point.Round();
         DIP_THROW_IF( !bb.Contains( point ), E::COORDINATES_OUT_OF_RANGE );
      }
      // Draw polygon as a set of Bresenham lines
      DIP_OVL_CALL_ALL( dip__DrawPolygon, ( out, polygon, value, open ), out.DataType());
   }
}


//
// Other discrete shapes
//


namespace {

enum class EllipsoidNorm{ L1, L2, Lmax };

template< typename TPI >
class dip__DrawEllipsoidLineFilter : public Framework::ScanLineFilter {
   public:
      dip__DrawEllipsoidLineFilter( FloatArray const& scale, FloatArray const& origin, Image::Pixel const& value, dip::uint nTensor, EllipsoidNorm norm ) :
            scale_( scale ), origin_( origin ), norm_( norm ) {
         CopyPixelToVector( value, value_, nTensor );
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::sint length = static_cast< dip::sint >( params.bufferLength );
         dip::uint dim = params.dimension;
         dfloat width;
         switch( norm_ ) {
            case EllipsoidNorm::L1: {
               dfloat distance = 0;
               for( dip::uint ii = 0; ii < scale_.size(); ++ii ) {
                  if( ii != dim ) {
                     dfloat d = ( static_cast< dfloat >( params.position[ ii ] ) - origin_[ ii ] ) * scale_[ ii ];
                     distance += std::abs( d );
                  }
               }
               if( distance > 1.0 ) {
                  return; // nothing to do on this line
               }
               width = ( 1.0 - distance ) / scale_[ dim ];
               break;
            }
            case EllipsoidNorm::L2: {
               dfloat distance2 = 0;
               for( dip::uint ii = 0; ii < scale_.size(); ++ii ) {
                  if( ii != dim ) {
                     dfloat d = ( static_cast< dfloat >( params.position[ ii ] ) - origin_[ ii ] ) * scale_[ ii ];
                     distance2 += d * d;
                  }
               }
               if( distance2 > 1.0 ) {
                  return; // nothing to do on this line
               }
               width = std::sqrt( 1.0 - distance2 ) / scale_[ dim ];
               break;
            }
            default: { // to prevent compilers complaining
            //case EllipsoidNorm::Lmax: {
               for( dip::uint ii = 0; ii < scale_.size(); ++ii ) {
                  if( ii != dim ) {
                     dfloat d = ( static_cast< dfloat >( params.position[ ii ] ) - origin_[ ii ] ) * scale_[ ii ];
                     if( std::abs( d ) > 1.0 ) {
                        return; // nothing to do on this line
                     }
                  }
               }
               width = 1.0 / scale_[ dim ];
               break;
            }
         }
         dip::sint start = ceil_cast( origin_[ dim ] - width );
         dip::sint end = floor_cast( origin_[ dim ] + width );
         dip__FillLine( out, start, end, length, stride, value_, tensorStride );
      }
   private:
      FloatArray const& scale_;
      FloatArray const& origin_;
      std::vector< TPI > value_;
      EllipsoidNorm norm_;
};

void dip__DrawEllipsoid(
      Image& out,
      FloatArray sizes,
      FloatArray origin,
      Image::Pixel const& value,
      EllipsoidNorm norm
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sizes, nDims, 7.0 ));
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      DIP_THROW_IF( sizes[ ii ] <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   }
   DIP_THROW_IF( origin.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   dip::Image tmp = out;
   if( !NarrowImageView( tmp, sizes, origin )) {
      return;
   }
   for( auto& s: sizes ) {
      s = 2.0 / s; // modify `sizes` to be `scale`.
   }
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, dip__DrawEllipsoidLineFilter, ( sizes, origin, value, tmp.TensorElements(), norm ), tmp.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleOutput( tmp, tmp.DataType(), *lineFilter, Framework::ScanOption::NeedCoordinates ));
   // NOTE: because of the way we call the Scan framework, we know for sure that it won't use a temporary buffer for
   // the output samples, and thus we get to write directly in the output. We can modify only select pixels in the
   // output image.
}

} // namespace

void DrawEllipsoid(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value
) {
   DIP_STACK_TRACE_THIS( dip__DrawEllipsoid( out, sizes, origin, value, EllipsoidNorm::L2 ));
}

void DrawDiamond(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value
) {
   DIP_STACK_TRACE_THIS( dip__DrawEllipsoid( out, sizes, origin, value, EllipsoidNorm::L1 ));
}

void DrawBox(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value
) {
   DIP_STACK_TRACE_THIS( dip__DrawEllipsoid( out, sizes, origin, value, EllipsoidNorm::Lmax ));
}

} // namespace dip
