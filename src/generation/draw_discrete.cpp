/*
 * (c)2017-2023, Cris Luengo.
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

#include "diplib/generation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/generic_iterators.h"
#include "diplib/iterators.h"
#include "diplib/overload.h"
#include "diplib/polygon.h"
#include "diplib/random.h"
#include "diplib/saturated_arithmetic.h"

#include "draw_support.h"

namespace dip {


//
// Bresenham lines
//


namespace {

// Cohen-Sutherland line clipping algorithm according to Wikipedia
// https://en.wikipedia.org/wiki/Cohen–Sutherland_algorithm

// outcodes
constexpr int INSIDE = 0b0000;
constexpr int LEFT = 0b0001;
constexpr int RIGHT = 0b0010;
constexpr int TOP = 0b0100;
constexpr int BOTTOM = 0b1000;

// Computes the outcode for one point, w.r.t. the image domain
int OutcodeForPoint( VertexFloat p, VertexFloat bottom_right ) {
   int code = INSIDE;
   if( p.x < 0 ) {
      code |= LEFT;
   } else if( p.x > bottom_right.x ) {
      code |= RIGHT;
   }
   if( p.y < 0 ) {
      code |= TOP;
   } else if( p.y > bottom_right.y ) {
      code |= BOTTOM;
   }
   return code;
}

// Moves points p0 and p1 to be within the image. If the line doesn't intersect the image at all, returns false.
bool ClipLineToImageDomain( VertexFloat& p0, VertexFloat& p1, UnsignedArray const& img_size ) {
   VertexFloat bottom_right{
         static_cast< double >( img_size[0] - 1 ),
         static_cast< double >( img_size[1] - 1 )
   };
   int outcode0 = OutcodeForPoint( p0, bottom_right );
   int outcode1 = OutcodeForPoint( p1, bottom_right );

   while( true ) {
      if(( outcode0 | outcode1 ) == 0 ) {
         // Both points are inside the image
         return true;
      }
      if(( outcode0 & outcode1 ) != 0 ) {
         // The line does not intersect the image
         return false;
      }
      // Find the point that is outside the image domain
      int outcode = std::max( outcode1, outcode0 );
      // Find the intersection point of the line with the given image edge
      VertexFloat p; // this is the new point
      if(( outcode & BOTTOM ) != 0 ) {
         p.x = p0.x + ( p1.x - p0.x ) * ( bottom_right.y - p0.y ) / ( p1.y - p0.y );
         p.y = bottom_right.y;
      } else if(( outcode & TOP ) != 0 ) {
         p.x = p0.x + ( p1.x - p0.x ) * ( 0 - p0.y ) / ( p1.y - p0.y );
         p.y = 0;
      } else if(( outcode & RIGHT ) != 0 ) {
         p.y = p0.y + ( p1.y - p0.y ) * ( bottom_right.x - p0.x ) / ( p1.x - p0.x );
         p.x = bottom_right.x;
      } else if(( outcode & LEFT ) != 0 ) {
         p.y = p0.y + ( p1.y - p0.y ) * ( 0 - p0.x ) / ( p1.x - p0.x );
         p.x = 0;
      }
      // Figure out which of the points to replace with the new p
      if( outcode == outcode0 ) {
         p0 = p;
         outcode0 = OutcodeForPoint( p0, bottom_right );
      } else {
         p1 = p;
         outcode1 = OutcodeForPoint( p1, bottom_right );
      }
   }
}

// Finds the two endpoints of the line as uint coordinates within the image.
// Returns false if the line does not intersect the image
bool VerticesToUnsignedArrays( VertexFloat p0, VertexFloat p1, UnsignedArray const& img_size, UnsignedArray& out_p0, UnsignedArray& out_p1 ) {
   if( !ClipLineToImageDomain( p0, p1, img_size ) ) {
      return false;
   }
   out_p0 = { static_cast< dip::uint >( std::round( p0.x )), static_cast< dip::uint >( std::round( p0.y )) };
   out_p1 = { static_cast< dip::uint >( std::round( p1.x )), static_cast< dip::uint >( std::round( p1.y )) };
   return true;
}

template< typename TPI, typename F >
void DrawOneLine(
      TPI* origin,
      dip::sint stride,
      BresenhamLineIterator& iterator,
      std::vector< TPI > const& value,
      F const& blend
) {
   do {
      dip::sint offset = *iterator;
      for( auto v: value ) {
         origin[ offset ] = blend( origin[ offset ], v );
         offset += stride;
      }
   } while( ++iterator );
}

template< typename TPI, typename F >
void DrawLineInternal(
      Image& out,
      BresenhamLineIterator& iterator,
      Image::Pixel const& value,
      F const& blend
) {
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   DrawOneLine( static_cast< TPI* >( out.Origin() ), out.TensorStride(), iterator, value_, blend );
}

template< typename TPI, typename F >
void DrawLinesInternal(
      Image& out,
      CoordinateArray const& points,
      Image::Pixel const& value,
      F const& blend
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
      DrawOneLine( origin, stride, iterator, value_, blend );
   }
}

} // namespace

void DrawLine(
      Image& out,
      UnsignedArray const& start,
      UnsignedArray const& end,
      Image::Pixel const& value,
      String const& blend
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( start.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( end.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( !( start < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   DIP_THROW_IF( !( end < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   BresenhamLineIterator iterator( out.Strides(), start, end );
   if( blend == S::ASSIGN ) {
      DIP_OVL_CALL_ALL( DrawLineInternal, ( out, iterator, value, []( auto, auto b ) { return b; } ), out.DataType() );
   } else if( blend == S::ADD ) {
      DIP_OVL_CALL_ALL( DrawLineInternal, ( out, iterator, value, []( auto a, auto b ) { return saturated_add( a, b ); } ), out.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( blend );
   }
}

void DrawLines(
      Image& out,
      CoordinateArray const& points,
      Image::Pixel const& value,
      String const& blend
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( points.size() < 2, E::ARRAY_PARAMETER_WRONG_LENGTH );
   for( auto const& point : points ) {
      DIP_THROW_IF( point.size() != out.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
      DIP_THROW_IF( !( point < out.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   }
   if( blend == S::ASSIGN ) {
      DIP_OVL_CALL_ALL( DrawLinesInternal, ( out, points, value, []( auto, auto b ) { return b; } ), out.DataType() );
   } else if( blend == S::ADD ) {
      DIP_OVL_CALL_ALL( DrawLinesInternal, ( out, points, value, []( auto a, auto b ) { return saturated_add( a, b ); } ), out.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( blend );
   }
}


//
// Polygons
//
// Filled polygon according to the algorithm described here:
// https://www.cs.rit.edu/~icss571/filling/how_to.html
// Except that the "special cases" (on the 2nd page) we handle differently.
// Also, we put in extra effort to handle floating-point vertices correctly.
//


namespace {

template< typename TPI >
void FillLine(
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
   out += start * stride;
   for( dip::sint jj = start; jj <= end; ++jj, out += stride ) {
      dip::sint offset = 0;
      for( dip::uint ii = 0; ii < value.size(); ++ii ) {
         out[ offset ] = value[ ii ];
         offset += tensorStride;
      }
   }
}

template< typename TPI >
void DrawPolygonInternal(
      Image& out,
      Polygon const& polygon,
      Image::Pixel const& value,
      bool open
) {
   std::vector< TPI > value_;
   CopyPixelToVector( value, value_, out.TensorElements() );
   dip::sint stride = out.TensorStride();
   TPI* origin = static_cast< TPI* >( out.Origin() );
   VertexFloat prev = polygon.vertices[ 0 ];
   UnsignedArray p0;
   UnsignedArray p1;
   for( dip::uint jj = 1; jj < polygon.vertices.size(); ++jj ) {
      VertexFloat cur = polygon.vertices[ jj ];
      if( VerticesToUnsignedArrays( prev, cur, out.Sizes(), p0, p1 )) {
         BresenhamLineIterator iterator( out.Strides(), p0, p1 );
         DrawOneLine( origin, stride, iterator, value_, []( auto, auto b ) { return b; } );
      }
      prev = cur;
   }
   if( !open ) {
      VertexFloat cur = polygon.vertices[ 0 ];
      if( VerticesToUnsignedArrays( prev, cur, out.Sizes(), p0, p1 )) {
         BresenhamLineIterator iterator( out.Strides(), p0, p1 );
         DrawOneLine( origin, stride, iterator, value_, []( auto, auto b ) { return b; } );
      }
   }
}

struct PolygonEdge {
   // y is the direction perpendicular to the scan lines
   dip::sint yMin;
   dip::sint yMax;
   dfloat x;         // initialized to value of x corresponding to yMin
   dfloat slope;     // increment x by this value for each unit increment of y
   PolygonEdge( VertexFloat pt1, VertexFloat pt2, bool horizontalScanLines ) {
      if( !horizontalScanLines ) {
         pt1 = pt1.Permute();
         pt2 = pt2.Permute();
      }
      if( pt1.y > pt2.y ) {
         std::swap( pt1, pt2 );
      }
      yMin = round_cast( pt1.y );
      yMax = round_cast( pt2.y );
      if( yMin == yMax ) {
         slope = dip::infinity;
      } else {
         // Note that use rounded y coordinates so the slope makes sense in computations below.
         slope = ( pt1.x - pt2.x ) / static_cast< dfloat >( yMin - yMax );
      }
      x = pt1.x;
   }
   bool operator<( PolygonEdge const& other ) const {
      return ( yMin == other.yMin ) ? ( x < other.x ) : yMin < other.yMin;
   }
   bool IsAlongScanLine() const {
      return yMin == yMax;
   }
};

struct ActiveEdge {
   // See above for description, this is identical to PolygonEdge but sorts differently and has different methods
   // Note that this will never be created with yMin = yMax
   dip::sint yMin;
   dip::sint yMax;
   dfloat x;         // initialized to value of x corresponding to yMin
   dfloat slope;     // increment x by this value for each unit increment of y
   explicit ActiveEdge( PolygonEdge const& edge ) :
         yMin( edge.yMin ), yMax( edge.yMax ), x( edge.x ), slope( edge.slope ) {
      DIP_ASSERT( yMin < yMax ); // one of the invariants
   }
   ActiveEdge& operator++() {
      x += slope;
      return *this;
   }
   bool operator<( ActiveEdge const& other ) const {
      return x < other.x;
   }
   bool FormsIVertex( ActiveEdge const& other, dip::sint y ) const {
      // True if the two edges form a vertex at y where the polygon "moves" through y ("I" shape).
      // Call this function only with two consecutive vertices (no other vertices in between).
      return ((( yMax == y ) && ( other.yMin == y )) || (( yMin == y ) && ( other.yMax == y )));
   }
   bool FormsVVertex( ActiveEdge const& other, dip::sint y ) const {
      // True if the two edges form a vertex at y where the polygon stays above or below y ("V" shape).
      // Call this function only with two consecutive vertices (no other vertices in between).
      return ((( yMax == y ) && ( other.yMax == y )) || (( yMin == y ) && ( other.yMin == y )));
   }
};

template< typename TPI >
void DrawFilledPolygon(
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
   // Initialize active edge list
   std::vector< ActiveEdge > active;
   dip::uint kk = 0;
   dip::sint y = edges[ kk ].yMin;
   if( y >= maxY ) {
      return; // Nothing to do!
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
   while( true ) {
      // Add new edges, if any
      while(( kk < edges.size() ) && ( edges[ kk ].yMin == y )) {
         active.emplace_back( edges[ kk ] );
         ++kk;
      }
      if( active.empty() ) {
         break;
      }
      if( y >= 0 ) {
         // Sort according to x values
         std::sort( active.begin(), active.end() );
         // Draw elements
         dip::uint first = 0;
         dip::uint second = 1;
         while( second < active.size() ) {
            // If this is a "V" vertex, skip all the other tests
            if( !active[ first ].FormsVVertex( active[ second ], y )) {
               // Eliminate "I" vertex on the left
               if( active[ first ].FormsIVertex( active[ second ], y )) {
                  ++second;
               }
               // If second is part of a "V" vertex, ignore it
               while( second + 1 < active.size() && active[ second ].FormsVVertex( active[ second + 1 ], y )) {
                  second += 2;
               }
               if( second >= active.size()) {
                  // We're done.
                  break;
               }
               // Eliminate "I" vertex on the right
               if(( second + 1 < active.size()) && active[ second ].FormsIVertex( active[ second + 1 ], y )) {
                  ++second;
               }
            }
            FillLine( line.Pointer(),
                      round_cast( active[ first ].x ),
                      round_cast( active[ second ].x ),
                      static_cast< dip::sint >( length ), stride, value_, tensorStride );
            // Next pair
            first = second + 1;
            second = first + 1;
         }
         // We exit the loop when `second` isn't a valid index;
         // if `first` is a valid index here, we have an odd number of edges
         DIP_ASSERT( first >= active.size());
         // Next scan line, don't increment if y < 0!
         ++line;
      }
      // Increment y, but not past the last scan line
      ++y;
      if( y >= maxY ) {
         return;
      }
      // Update active edges
      for( auto& a : active ) {
         ++a;
      }
      // Remove edges that are no longer active
      for( dip::uint ii = active.size(); ii > 0; ) {
         --ii;
         if( active[ ii ].yMax < y ) {
            active.erase( active.begin() + static_cast< dip::sint >( ii ));
         }
      }
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
   } else if( mode != S::CLOSED ) {
      DIP_THROW_INVALID_FLAG( mode );
   }
   DIP_THROW_IF( polygon.vertices.size() < ( open ? 2u : 3u ), E::ARRAY_PARAMETER_WRONG_LENGTH ); // need at least 2 points to draw an open polygon, otherwise 3 points
   if( filled ) {
      bool horizontalScanLines = Framework::OptimalProcessingDim( out ) == 0;
      std::vector< PolygonEdge > edges;
      edges.reserve( polygon.vertices.size() + 1 ); // max number of edges, could be fewer if some are along scan lines
      for( dip::uint ii = 1; ii < polygon.vertices.size(); ++ii ) {
         PolygonEdge e( polygon.vertices[ ii - 1 ], polygon.vertices[ ii ], horizontalScanLines );
         if( !e.IsAlongScanLine() ) {
            edges.push_back( e );
         }
      }
      PolygonEdge e( polygon.vertices.back(), polygon.vertices.front(), horizontalScanLines );
      if( !e.IsAlongScanLine() ) {
         edges.push_back( e );
      }
      DIP_THROW_IF( edges.size() < 2, "The polygon has a bad shape" );
      std::sort( edges.begin(), edges.end() );
      DIP_OVL_CALL_ALL( DrawFilledPolygon, ( out, edges, value, horizontalScanLines ), out.DataType() );
   } else {
      // Draw polygon as a set of Bresenham lines
      DIP_OVL_CALL_ALL( DrawPolygonInternal, ( out, polygon, value, open ), out.DataType() );
   }
}


//
// Other discrete shapes
//


namespace {

enum class EllipsoidNorm : uint8 { L1, L2, Lmax };

template< typename TPI >
class DrawEllipsoidLineFilter : public Framework::ScanLineFilter {
   public:
      DrawEllipsoidLineFilter( FloatArray const& scale, FloatArray const& origin, Image::Pixel const& value, dip::uint nTensor, EllipsoidNorm norm ) :
            scale_( scale ), origin_( origin ), norm_( norm ) {
         CopyPixelToVector( value, value_, nTensor );
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::sint length = static_cast< dip::sint >( params.bufferLength );
         dip::uint dim = params.dimension;
         dfloat width{};
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
         // Prevent rounding errors
         dfloat rounded_width = std::round( width );
         if( std::abs( width - rounded_width ) < 1e-7 ) {
            width = rounded_width;
         }
         // Draw the line
         dip::sint start = ceil_cast( origin_[ dim ] - width );
         dip::sint end = floor_cast( origin_[ dim ] + width );
         FillLine( out, start, end, length, stride, value_, tensorStride );
      }
   private:
      FloatArray const& scale_;
      FloatArray const& origin_;
      std::vector< TPI > value_;
      EllipsoidNorm norm_;
};

void DrawEllipsoidInternal(
      Image& out,
      FloatArray sizes,
      FloatArray origin,
      Image::Pixel const& value,
      EllipsoidNorm norm
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.TensorElements() != value.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sizes, nDims, 7.0 ));
   for( auto size: sizes ) {
      DIP_THROW_IF( size <= 0.0, E::INVALID_PARAMETER );
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
   DIP_OVL_NEW_ALL( lineFilter, DrawEllipsoidLineFilter, ( sizes, origin, value, tmp.TensorElements(), norm ), tmp.DataType() );
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
   DIP_STACK_TRACE_THIS( DrawEllipsoidInternal( out, sizes, origin, value, EllipsoidNorm::L2 ));
}

void DrawDiamond(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value
) {
   DIP_STACK_TRACE_THIS( DrawEllipsoidInternal( out, sizes, origin, value, EllipsoidNorm::L1 ));
}

void DrawBox(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value
) {
   DIP_STACK_TRACE_THIS( DrawEllipsoidInternal( out, sizes, origin, value, EllipsoidNorm::Lmax ));
}


//
// Discrete grids
//


namespace {

enum class GridType : uint8 { RECTANGULAR, HEXAGONAL, BCC, FCC };

GridType GetGridType( String const& type ) {
   if( type == S::RECTANGULAR ) {
      return GridType::RECTANGULAR;
   }
   if( type == S::HEXAGONAL ) {
      return GridType::HEXAGONAL;
   }
   if( type == S::BCC ) {
      return GridType::BCC;
   }
   if( type == S::FCC ) {
      return GridType::FCC;
   }
   DIP_THROW_INVALID_FLAG( type );
}

void MatrixMultiplyWithRound( std::vector< dfloat > const& M, FloatArray const& v, FloatArray& p ) {
   // Computes p = round(M * v)
   dip::uint nDims = v.size(); // p.size() == nDims; M.size() == nDims*nDims;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dfloat res = 0.0;
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         res += M[ ii + jj * nDims ] * v[ jj ];
      }
      p[ ii ] = std::round( res );
   }
}

// Generic grid in arbitrary dimensions

class FillRandomGridnDLineFilter : public Framework::ScanLineFilter {
   public:
      FillRandomGridnDLineFilter( std::vector< dfloat > const& M, FloatArray const& offset ) : M_( M ), offset_( offset ) {
         DIP_ASSERT( offset_.size() * offset_.size() == M_.size() );
         inv_M_.resize( M_.size() );
         Inverse( offset_.size(), M_.data(), inv_M_.data() );
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint length = params.bufferLength;
         dip::uint dim = params.dimension;
         dip::uint nDims = offset_.size();
         DIP_ASSERT( params.position.size() == nDims );
         FloatArray position{ params.position };
         position += offset_;
         FloatArray grid_index( nDims );
         FloatArray grid_position( nDims );
         for( dip::uint jj = 0; jj < length; ++jj, out += stride, position[ dim ] += 1.0 ) {
            // round( M * round( inv_M_ * position )) == position -> this is a grid point
            MatrixMultiplyWithRound( inv_M_, position, grid_index );
            MatrixMultiplyWithRound( M_, grid_index, grid_position );
            if( grid_position == position ) {
               *out = true;
            }
         }
      }
   private:
      std::vector< dfloat > const& M_;
      std::vector< dfloat > inv_M_;
      FloatArray const& offset_;
};

void FillRandomGridnD( Image& out, UniformRandomGenerator& uniform, dfloat distance, GridType type, bool isRotated ) {
   dip::uint nDims = out.Dimensionality();
   std::vector< dfloat > M;
   if( type == GridType::RECTANGULAR ) {
      M.resize( nDims * nDims, 0.0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         M[ ii * ( nDims + 1 ) ] = distance;
      }
   } else {
      DIP_ASSERT( nDims == 3 );
      if( type == GridType::FCC ) {
         M = { distance, distance,      0.0,
               distance,      0.0, distance,
                    0.0, distance, distance }; // Representation here is transposed, but M==M'.
      } else if( type == GridType::BCC ) {
         M = {  distance,  distance, -distance,
                distance, -distance,  distance,
               -distance,  distance,  distance }; // Representation here is transposed, but M==M'.
      }
   }
   if( isRotated ) {
      DIP_ASSERT( nDims == 3 );
      dfloat phi = uniform( 0, 2 * pi );
      dfloat theta = std::acos( uniform( -1.0, 1.0 ));
      dfloat psi = uniform( 0, pi );
      // Symbolic MATLAB code to generate the rotation matrix from our three angles:
      // syms phi theta psi
      // R1 = [ cos(phi),sin(phi),0 ; -sin(phi),cos(phi),0 ; 0,0,1 ]
      // R2 = [ cos(theta),0,-sin(theta) ; 0,1,0 ; sin(theta),0,cos(theta) ]
      // R3 = [ 1,0,0 ; 0,cos(psi),sin(psi) ; 0,-sin(psi),cos(psi) ]
      // R = R1 * R2 * R3
      //    [  cos(phi)*cos(theta), cos(psi)*sin(phi) + cos(phi)*sin(psi)*sin(theta), sin(phi)*sin(psi) - cos(phi)*cos(psi)*sin(theta)]
      //    [ -cos(theta)*sin(phi), cos(phi)*cos(psi) - sin(phi)*sin(psi)*sin(theta), cos(phi)*sin(psi) + cos(psi)*sin(phi)*sin(theta)]
      //    [           sin(theta),                             -cos(theta)*sin(psi),                              cos(psi)*cos(theta)]
      dfloat cos_phi = std::cos( phi );
      dfloat sin_phi = std::sin( phi );
      dfloat cos_theta = std::cos( theta );
      dfloat sin_theta = std::sin( theta );
      dfloat cos_psi = std::cos( psi );
      dfloat sin_psi = std::sin( psi );
      std::vector< dfloat > R = {
            // Column 1
            cos_phi * cos_theta,
            -cos_theta * sin_phi,
            sin_theta,
            // Column 2
            cos_psi * sin_phi + cos_phi * sin_psi * sin_theta,
            cos_phi * cos_psi - sin_phi * sin_psi * sin_theta,
            -cos_theta * sin_psi,
            // Column 3
            sin_phi * sin_psi - cos_phi * cos_psi * sin_theta,
            cos_phi * sin_psi + cos_psi * sin_phi * sin_theta,
            cos_psi * cos_theta,
      };
      std::vector< dfloat > RM( 9, 0.0 );
      for( dip::uint ii = 0; ii < 3; ++ii ) {
         for( dip::uint jj = 0; jj < 3; ++jj ) {
            for( dip::uint kk = 0; kk < 3; ++kk ) {
               RM[ ii + 3 * jj ] += R[ ii + 3 * kk ] * M[ kk + 3 * jj ];
            }
         }
      }
      M = std::move( RM );
   }
   // Offset (random within grid unit)
   FloatArray offset;
   if( nDims == 3 ) {
      dfloat x = uniform( 0, 1 );
      dfloat y = uniform( 0, 1 );
      dfloat z = uniform( 0, 1 );
      offset = {
            std::round( M[ 0 ] * x + M[ 3 ] * y + M[ 6 ] * z ),
            std::round( M[ 1 ] * x + M[ 4 ] * y + M[ 7 ] * z ),
            std::round( M[ 2 ] * x + M[ 5 ] * y + M[ 8 ] * z )
      };
   } else {
      // It's always a rectangular grid, ignore M
      offset.resize( nDims );
      for( auto& o : offset ) {
         o = std::round( uniform( 0, distance ));
      }
   }
   FillRandomGridnDLineFilter lineFilter( M, offset );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleOutput( out, DT_BIN, lineFilter, Framework::ScanOption::NeedCoordinates ));
}

// Specialization for 1D, for simplicity

void FillRandomGrid1D( Image& out, UniformRandomGenerator& uniform, dfloat distance ) {
   dfloat offset = uniform( 0, distance );
   dip::sint stride = out.Stride( 0 );
   bin* data = static_cast< bin* >( out.Origin() );
   dip::sint end = static_cast< dip::sint >( out.Size( 0 ));
   while( true ) {
      dip::sint index = round_cast( offset );
      if( index >= end ) {
         break;
      }
      data[ index * stride ] = true;
      offset += distance;
   }
}

// Specialization for 2D, for efficiency

class FillRandomGrid2DLineFilter : public Framework::ScanLineFilter {
   public:
      FillRandomGrid2DLineFilter( std::array< dfloat, 4 > const& M, VertexFloat offset ) // NOLINT(*-pro-type-member-init)
            : M_( M ), offset_( offset ) {
         Inverse( 2, M_.data(), inv_M_.data() );
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint length = params.bufferLength;
         dip::uint dim = params.dimension;
         DIP_ASSERT( params.position.size() == 2 );
         VertexFloat position = VertexFloat{ static_cast< dfloat >( params.position[ 0 ] ),
                                             static_cast< dfloat >( params.position[ 1 ] ) } + offset_;
         VertexFloat increment{ 1.0, 0.0 };
         if( dim == 1 ) {
            increment = { 0.0, 1.0 };
         }
         for( dip::uint jj = 0; jj < length; ++jj, out += stride, position += increment ) {
            // round( M * round( inv_M_ * position )) == position -> this is a grid point
            VertexFloat v = { std::round( inv_M_[ 0 ] * position.x + inv_M_[ 2 ] * position.y ),
                              std::round( inv_M_[ 1 ] * position.x + inv_M_[ 3 ] * position.y ) };
            v = { std::round( M_[ 0 ] * v.x + M_[ 2 ] * v.y ),
                  std::round( M_[ 1 ] * v.x + M_[ 3 ] * v.y ) };
            if( v == position ) {
               *out = true;
            }
         }
      }
   private:
      std::array< dfloat, 4 > const& M_;
      std::array< dfloat, 4 > inv_M_;
      VertexFloat offset_;
};

void FillRandomGrid2D( Image& out, UniformRandomGenerator& uniform, dfloat distance, bool isRectangular, bool isRotated ) {
   std::array< dfloat, 4 > M{};
   dfloat x = 1;
   dfloat y = 0;
   if( isRotated ) {
      dfloat angle = uniform( 0, pi );
      x = std::cos( angle );
      y = std::sin( angle );
   }
   M[ 0 ] = x * distance;
   M[ 1 ] = y * distance;
   if( isRectangular ) {
      M[ 2 ] = -y * distance;
      M[ 3 ] =  x * distance;
   } else {
      M[ 2 ] = ( 0.5 * x - std::sqrt( 3 ) * 0.5 * y ) * distance;
      M[ 3 ] = ( 0.5 * y + std::sqrt( 3 ) * 0.5 * x ) * distance;
   }
   x = uniform( 0, 1 );
   y = uniform( 0, 1 );
   VertexFloat offset = {
      std::round( M[ 0 ] * x + M[ 2 ] * y ),
      std::round( M[ 1 ] * x + M[ 3 ] * y )
   };
   FillRandomGrid2DLineFilter lineFilter( M, offset );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleOutput( out, DT_BIN, lineFilter, Framework::ScanOption::NeedCoordinates ));
}

} // namespace

void FillRandomGrid(
      Image& out,
      Random& random,
      dfloat density,
      String const& type_s,
      String const& mode
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !out.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   GridType type{};
   DIP_STACK_TRACE_THIS( type = GetGridType( type_s ));
   DIP_THROW_IF(( type == GridType::HEXAGONAL ) && ( nDims != 2 ), "Hexagonal grid requires a 2D image" );
   DIP_THROW_IF((( type == GridType::FCC ) || ( type == GridType::BCC )) && ( nDims != 3 ), "FCC and BCC grids require a 3D image" );
   bool isRotated = false;
   if(( nDims == 2 ) || ( nDims == 3 )) {
      DIP_STACK_TRACE_THIS( isRotated = BooleanFromString( mode, S::ROTATION, S::TRANSLATION ));
   }
   // Grid point distances
   dfloat distance = std::pow( 1.0 / density, 1.0 / static_cast< dfloat >( nDims ));
   if( type == GridType::HEXAGONAL ) {
      distance *= std::sqrt( 2.0 / std::sqrt( 3.0 ));
   } else if( type == GridType::FCC ) {
      distance *= 1.0 / std::cbrt( 2.0 );
   } else if( type == GridType::BCC ) {
      distance *= std::cbrt( 2.0 ) / 2.0;
   }
   DIP_THROW_IF( distance < 2.0, E::PARAMETER_OUT_OF_RANGE );
   // Initialize output to zeros
   out.Fill( 0 );
   // Draw grid
   UniformRandomGenerator uniform( random );
   switch( nDims ) {
      case 1:
         FillRandomGrid1D( out, uniform, distance );
         break;
      case 2:
         FillRandomGrid2D( out, uniform, distance, type == GridType::RECTANGULAR, isRotated );
         break;
      default:
         FillRandomGridnD( out, uniform, distance, type, isRotated );
         break;
   }
}

} // namespace dip
