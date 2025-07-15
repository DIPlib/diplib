/*
 * (c)2016-2024, Cris Luengo.
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

#include "diplib/regions.h"

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <sys/stat.h>

#include "diplib.h"
#include "diplib/binary.h"
#include "diplib/framework.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"
#include "diplib/label_map.h"
#include "diplib/measurement.h"
#include "diplib/neighborlist.h"
#include "diplib/overload.h"
#include "diplib/polygon.h"
#include "diplib/private/robin_map.h"
#include "diplib/private/robin_set.h"

namespace dip {

using LabelSet = tsl::robin_set< LabelType >;

namespace {

template< typename TPI, bool edgesOnly_ = false >
class GetLabelsLineFilter : public Framework::ScanLineFilter {
   public:
      // not defining GetNumberOfOperations(), always called in a single thread
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* data = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint stride = params.inBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         bool isOnEdge = !edgesOnly_ || IsOnEdge( params.position, sizes_, params.dimension );
         // If isOneEdge, this line goes along the image edge, include the whole line.
         // Otherwise, use only the first and last pixels of this line.
         // But: the test is only done when we only want edges. If we want the whole image,
         // then isOnEdge is always true, meaning we always use the whole line.
         if( params.inBuffer.size() > 1 ) {
            bin* mask = static_cast< bin* >( params.inBuffer[ 1 ].buffer );
            dip::sint mask_stride = params.inBuffer[ 1 ].stride;
            if( isOnEdge ) {
               LabelType prevID = 0;
               bool setPrevID = false;
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask ) {
                     if( !setPrevID || ( *data != prevID ) ) {
                        prevID = CastLabelType( *data );
                        setPrevID = true;
                        objectIDs_.insert( prevID );
                     }
                  }
                  data += stride;
                  mask += mask_stride;
               }
            } else {
               if( *mask ) {
                  objectIDs_.insert( CastLabelType( *data ));
               }
               dip::sint n = static_cast< dip::sint >( bufferLength ) - 1;
               if( *( mask + n * mask_stride )) {
                  objectIDs_.insert( CastLabelType( *( data + n * stride )));
               }
            }
         } else {
            if( isOnEdge ) {
               LabelType prevID = CastLabelType( *data ) + 1; // something that's different from the first pixel value
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *data != prevID ) {
                     prevID = CastLabelType( *data );
                     objectIDs_.insert( prevID );
                  }
                  data += stride;
               }
            } else {
               objectIDs_.insert( CastLabelType( *data ));
               dip::sint n = static_cast< dip::sint >( bufferLength ) - 1;
               objectIDs_.insert( CastLabelType( *( data + n * stride )));
            }
         }
      }
      GetLabelsLineFilter( LabelSet& objectIDs, UnsignedArray const& sizes ) : objectIDs_( objectIDs ), sizes_( sizes ) {}
   private:
      LabelSet& objectIDs_;
      UnsignedArray const& sizes_;
};

template< typename TPI >
using GetEdgeLabelsLineFilter = GetLabelsLineFilter< TPI, true >;

} // namespace

std::vector< LabelType > ListObjectLabels(
      Image const& label,
      Image const& mask,
      String const& background,
      String const& region
) {
   // Check input
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( label.Sizes(), Option::AllowSingletonExpansion::DONT_ALLOW, Option::ThrowException::DO_THROW ));
   }
   bool nullIsObject{};
   DIP_STACK_TRACE_THIS( nullIsObject = BooleanFromString( background, S::INCLUDE, S::EXCLUDE ));
   bool edgesOnly{};
   DIP_STACK_TRACE_THIS( edgesOnly = BooleanFromString( region, "edges", "" ));

   LabelSet objectIDs; // output

   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   Framework::ScanOptions opts = Framework::ScanOption::NoMultiThreading;
   if( edgesOnly ) {
      // Scan the image edges only
      DIP_OVL_NEW_UINT( scanLineFilter, GetEdgeLabelsLineFilter, ( objectIDs, label.Sizes() ), label.DataType() );
      opts += Framework::ScanOption::NeedCoordinates;
   } else {
      // Scan the whole image
      DIP_OVL_NEW_UINT( scanLineFilter, GetLabelsLineFilter, ( objectIDs, label.Sizes() ), label.DataType() );
   }
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( label, mask, label.DataType(), *scanLineFilter, opts ));

   // Should we ignore the 0 label?
   if( !nullIsObject ) {
      objectIDs.erase( 0 );
   }

   // Copy the labels to output array
   std::vector< LabelType > out( objectIDs.size() );
   std::copy( objectIDs.begin(), objectIDs.end(), out.begin() );

   // Our set is unordered, we now need to sort the list of objects
   std::sort( out.begin(), out.end() );

   return out;
}

namespace {

template< typename TPI >
class RelabelLineFilter : public Framework::ScanLineFilter {
   public:
      // not defining GetNumberOfOperations(), always called in a single thread
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         TPI inLabel = 0;       // last label seen, initialize to background label
         TPI outLabel = 0;      // new label assigned to prevID
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            if( *in == 0 ) {
               // The background label is processed differently
               *out = 0;
            } else if( *in == inLabel ) {
               *out = outLabel;
            } else {
               inLabel = *in;
               auto it = objectIDs_.find( inLabel );
               if( it == objectIDs_.end() ) {
                  // It's a new label
                  outLabel = ++lastLabel_;
                  objectIDs_.emplace( inLabel, outLabel );
               } else {
                  outLabel = it.value();
               }
               *out = outLabel;
            }
            in += inStride;
            out += outStride;
         }
      }
   private:
      tsl::robin_map< TPI, TPI > objectIDs_;
      TPI lastLabel_ = 0;
};

} // namespace

void Relabel( Image const& label, Image& out ) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );

   // Get pointer to overloaded scan function
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_UINT( scanLineFilter, RelabelLineFilter, (), label.DataType() );

   // Do the scan
   DIP_STACK_TRACE_THIS( Framework::ScanMonadic( label, out, label.DataType(), label.DataType(), 1, *scanLineFilter, Framework::ScanOption::NoMultiThreading ));
}

void SmallObjectsRemove(
      Image const& in,
      Image& out,
      dip::uint threshold,
      dip::uint connectivity
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   if( in.DataType().IsBinary() ) {
      Image tmp = Label( in, connectivity, threshold, 0 );
      NotEqual( tmp, Image( 0, tmp.DataType() ), out );
   } else if( in.DataType().IsUnsigned() ) {
      MeasurementTool msr;
      Measurement sizes = msr.Measure( in, {}, { "Size" }, {}, 1 );
      if( !sizes.IsForged() ) {
         return;
      }
      LabelMap selection = sizes[ "Size" ] >= static_cast< Measurement::ValueType >( threshold );
      selection.Apply( in, out );
   } else {
      DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}

void EdgeObjectsRemove( Image const& in, Image& out, dip::uint connectivity ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   if( in.DataType().IsBinary() ) {
      DIP_START_STACK_TRACE
         Image tmp_in = in;
         if( out.Aliases( tmp_in )) { // make sure we don't overwrite in
            DIP_STACK_TRACE_THIS( out.Strip() );
         }
         // Propagate with empty seed mask, iteration until done and treating outside the image as object
         BinaryPropagation( Image(), tmp_in, out, static_cast< dip::sint >( connectivity ), 0, S::OBJECT );
         // The out-image now contains the edge objects
         // Remove them by toggling these bits in the in-image and writing the result in out
         out ^= tmp_in;
      DIP_END_STACK_TRACE
   } else if( in.DataType().IsUInt() ) {
      DIP_START_STACK_TRACE
         auto edgeObjects = ListObjectLabels( in, {}, S::EXCLUDE, "edges" );
         dip::LabelMap map; // Note that labels not in the map will be preserved, we only need to insert the zero mappings.
         for( auto obj : edgeObjects ) {
            map[ obj ] = 0;
         }
         map.Apply( in, out );
      DIP_END_STACK_TRACE
   } else {
      DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}


namespace {

template< typename TPI >
void ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck(
      TPI* ptr,
      NeighborList const& neighbors,
      IntegerArray const& offsets,
      UnsignedArray const& coords,
      UnsignedArray const& sizes
) {
   if( *ptr ) {
      // Examine all neighbors to see if any has a different label
      auto nit = neighbors.begin();
      for( auto o : offsets ) {
         if( nit.IsInImage( coords, sizes )) {
            TPI n = ptr[ o ];
            if( n > *ptr ) {
               *ptr = 0;
               break;
            }
         }
         ++nit;
      }
   }
}

template< typename TPI >
void ZeroPixelIfHasDifferentNeighbor(
      TPI* ptr,
      IntegerArray const& offsets
) {
   if( *ptr ) {
      // Examine all neighbors to see if any has a different label
      for( auto o : offsets ) {
         TPI n = ptr[ o ];
         if( n > *ptr ) {
            *ptr = 0;
            break;
         }
      }
   }
}

template< typename TPI >
void SplitRegions(
   Image& img,
   NeighborList const& neighbors,
   IntegerArray const& offsets
) {
   dip::uint procDim = Framework::OptimalProcessingDim( img );
   // Iterate over image lines
   ImageIterator< TPI > it( img, procDim );
   do {
      auto coords = it.Coordinates();
      auto lit = it.GetLineIterator();
      if( it.IsOnEdge()) {
         // We test all the pixels along the line
         do {
            ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck( lit.Pointer(), neighbors, offsets, coords, img.Sizes() );
            ++coords[ procDim ];
         } while( ++lit );
      } else {
         // We test the first pixel along the line
         ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck( lit.Pointer(), neighbors, offsets, coords, img.Sizes() );
         ++lit;
         // The bulk of the pixels we don't need to worry about testing
         for( dip::uint ii = 1; ii < lit.Length() - 1; ++ii, ++lit ) {
            ZeroPixelIfHasDifferentNeighbor( lit.Pointer(), offsets );
         }
         // The last pixel we test again
         coords[ procDim ] = lit.Coordinate();
         ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck( lit.Pointer(), neighbors, offsets, coords, img.Sizes() );
      }
   } while( ++it );
}

} // namespace

void SplitRegions(
      Image const& label,
      Image& out,
      dip::uint connectivity
) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );

   // We work in-place in the output image. Copy input data there.
   if( &out != &label ) {
      out.Copy( label );
      DIP_THROW_IF( !out.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   }

   // Create a copy with optimally sorted strides and no singleton dimensions
   Image img = out.QuickCopy();
   img.StandardizeStrides();
   dip::uint nDims = img.Dimensionality();
   NeighborList neighbors(Metric( Metric::TypeCode::CONNECTED, connectivity ), nDims);
   IntegerArray offsets = neighbors.ComputeOffsets( img.Strides() );

   // Process
   DIP_OVL_CALL_UINT( SplitRegions, ( img, neighbors, offsets ), img.DataType() );

}


namespace {

struct MinMax {
   dip::uint min;
   dip::uint max;
};
using Contour = std::map< dip::uint, MinMax >; // Storing minimum and maximum x coordinate for each y coordinate. Must be iterable in order.
using ObjectContours = tsl::robin_map< dip::uint, Contour >; // Stores a contour for each label

template< typename TPI > // TPI is an unsigned integer type
ObjectContours GetObjectContours( Image const& label ) {
   // Find first and last pixel for each label on each image line.
   // See Cadenas JO, Megson GM, and Luengo Hendriks CL, "Preconditioning 2D Integer Data for
   //    Fast Convex Hull Computations", PLOS ONE 11(3):e0149860, 2016.
   ImageIterator< TPI > it( label, 0 );
   ObjectContours out;
   do { // loops over rows
      dip::uint row = it.Coordinates()[ 1 ];
      auto lit = it.GetLineIterator();
      do { // loops over pixels in this row
         TPI lab = *lit;
         if( lab ) {
            dip::uint start = lit.Coordinate();
            dip::uint stop = start;
            while( ++lit && *lit == lab ) {
               stop = lit.Coordinate();
            }
            auto& contour = out[ static_cast< dip::uint >( lab ) ];
            auto minMax = contour.find( row );
            if( minMax == contour.end() ) {
               contour[ row ] = MinMax{ start, stop };
            } else {
               contour[ row ].max = stop;
            }
         } else {
            ++lit;
         }
      } while( lit ); // note: no increment!
   } while( ++it );
   return out;
}

using ObjectConvexHulls = tsl::robin_map< dip::uint, Polygon >; // Stores a convex hull for each label

template< typename TPI > // TPI is an unsigned integer type
ObjectConvexHulls GetObjectConvexHulls( ObjectContours const& objectContours ) {
   // Combine first and last pixels for each image line into a polygon, then compute the convex hull.
   // We add or subtract 0.1 from the x coordinate to ensure that the polygon is "nice", without two vertices
   // at apposite sides of the polygon being on top of each other.
   ObjectConvexHulls out;
   for( auto obj_it = objectContours.begin(); obj_it != objectContours.end(); ++obj_it ) {
      Polygon polygon;
      polygon.vertices.reserve( obj_it.value().size() * 2 );
      for( auto it = obj_it.value().begin(); it != obj_it.value().end(); ++it ) {
         polygon.vertices.emplace_back( static_cast< dip::sfloat >( it->second.max ) + 0.1,  static_cast< dip::sfloat >( it->first ));
      }
      for( auto it = obj_it.value().rbegin(); it != obj_it.value().rend(); ++it ) {
         polygon.vertices.emplace_back( static_cast< dip::sfloat >( it->second.min ) - 0.1,  static_cast< dip::sfloat >( it->first ));
      }
      out[ obj_it.key() ] = std::move( polygon.ConvexHull().Polygon() );
   }
   return out;
}

template< typename TPI > // TPI is an unsigned integer type
void DrawObjectConvexHulls( Image& label, ObjectConvexHulls const& objectConvexHulls, bool filled ) {
   // For hollow polygons, clear the image first
   String mode = S::FILLED;
   if( !filled ) {
      label.Fill( 0 );
      mode = S::CLOSED;
   }
   // Sort object IDs so we draw the convex objects in the right order
   std::set< dip::uint > objects;
   for( auto obj_it = objectConvexHulls.begin(); obj_it != objectConvexHulls.end(); ++obj_it ) {
      objects.insert( obj_it.key() );
   }
   // Draw the polygons
   for( auto id : objects ) {
      auto const& polygon = objectConvexHulls.at( id );
      if( polygon.vertices.size() == 1 ) {
         // We draw this pixel in case a previous polygon overlaps it.
         auto pt = polygon.vertices[ 0 ].Round();
         UnsignedArray ptu{ static_cast< dip::uint >( pt.x ), static_cast< dip::uint >( pt.y ) };
         label.At( ptu ) = id;
      } else if( polygon.vertices.size() == 2 ) {
         // Idem, draw in case it was overwritten.
         auto pt1 = polygon.vertices[ 0 ].Round();
         auto pt2 = polygon.vertices[ 1 ].Round();
         UnsignedArray pt1u{ static_cast< dip::uint >( pt1.x ), static_cast< dip::uint >( pt1.y ) };
         UnsignedArray pt2u{ static_cast< dip::uint >( pt2.x ), static_cast< dip::uint >( pt2.y ) };
         DrawLine( label, pt1u, pt2u, { id }, S::ASSIGN );
      } else {
         DrawPolygon2D( label, polygon, { id }, mode );
      }
   }
}

template< typename TPI > // TPI is an unsigned integer type
void MakeRegionsConvex2DInternal( Image& label, bool filled ) {
   ObjectContours objectContours = GetObjectContours< TPI >( label );
   ObjectConvexHulls objectConvexHulls = GetObjectConvexHulls< TPI >( objectContours );
   DrawObjectConvexHulls< TPI >( label, objectConvexHulls, filled );
}

} // namespace

void MakeRegionsConvex2D(
      Image const& label,
      Image& out,
      String const& mode
) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !label.DataType().IsUnsigned(), E::DATA_TYPE_NOT_SUPPORTED );
   bool filled = BooleanFromString( mode, S::FILLED, S::HOLLOW );
   out.Copy( label );
   dip::Image tmp = out.QuickCopy();
   if( tmp.DataType().IsBinary() ) {
      tmp.Convert( DT_UINT8 ); // This doesn't change the data, which is shared with the output.
   }
   DIP_OVL_CALL_UINT( MakeRegionsConvex2DInternal, ( tmp, filled ), tmp.DataType() );
   // We wrote into `tmp`, a UINT image, but it shares data with `out`, which is either UINT or BIN.
}


namespace {

template< typename TPI >
RangeArray GetLabelBoundingBoxInternal(
      Image const& label,
      LabelType objectID
) {
   DIP_ASSERT( label.DataType() == DataType( TPI( 0 )));
   dip::uint nDims = label.Dimensionality();
   RangeArray bb;
   bool needInit = true; // bb needs initialization
   ImageIterator< TPI > it( label );
   do {
      if( *it == objectID ) {
         if( needInit ) {
            // The first pixel with this value: initialize the output RangeArray
            bb.resize( nDims );
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               bb[ ii ] = Range{ static_cast< dip::sint >( it.Coordinates()[ ii ] ) };
            }
            needInit = false;
         } else {
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               bb[ ii ].start = std::min( bb[ ii ].start, static_cast< dip::sint >( it.Coordinates()[ ii ] ));
               bb[ ii ].stop = std::max( bb[ ii ].stop, static_cast< dip::sint >( it.Coordinates()[ ii ] ));
            }
         }
      }
   } while( ++it );
   return bb;
}

} // namespace

RangeArray GetLabelBoundingBox(
      Image const& label,
      LabelType objectID
) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( label.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   RangeArray bb;
   DIP_OVL_CALL_ASSIGN_UNSIGNED( bb, GetLabelBoundingBoxInternal, ( label, objectID ), label.DataType() );
   return bb;
}

} // namespace dip
