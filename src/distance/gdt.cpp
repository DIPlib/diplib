/*
 * DIPlib 3.0
 * This file contains definitions for distance transforms
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
#include "diplib/distance.h"
#include "diplib/statistics.h"
#include "diplib/overload.h"

namespace dip {

namespace {

constexpr dip::sint QSORT_MAGIC_SIZE = 10;
constexpr dip::uint SMALL_STACK_SIZE = 32;

struct GDTNode {
   dip::sint offset;
   sfloat value;
};

enum class SortType { VALUE, OFFSET };
inline void Sort3ByValue( GDTNode& n1, GDTNode& n2, GDTNode& n3 ) {
   if(( n1.value ) > ( n2.value )) { std::swap( n1, n2 ); }
   if(( n2.value ) > ( n3.value )) { std::swap( n2, n3 ); }
   if(( n1.value ) > ( n2.value )) { std::swap( n1, n2 ); }
}
inline void Sort3ByOffset( GDTNode& n1, GDTNode& n2, GDTNode& n3 ) {
   if(( n1.offset ) > ( n2.offset )) { std::swap( n1, n2 ); }
   if(( n2.offset ) > ( n3.offset )) { std::swap( n2, n3 ); }
   if(( n1.offset ) > ( n2.offset )) { std::swap( n1, n2 ); }
}

inline dip::uint GetCeilingLog2( dip::uint number ) {
   dip::uint ii, jj, ln2 = number;
   for( jj = 0, ii = number; ii > 0; ii >>= 1, jj++ ) {
      if( ii & 1 ) {
         ln2 = jj;
      }
   }
   if( number != ( 1u << ( jj - 1 ))) {
      ln2++;
   }
   return ln2;
}

void GDTQuickSort(
      GDTNode* out,
      dip::uint size,
      SortType sortType
) {
   if( size <= 1 ) {
      return;
   }

   dip::uint maxSp = GetCeilingLog2( size ) << 1;
   dip::sint* stack;
   dip::sint smallStack[ SMALL_STACK_SIZE ];
   std::vector< dip::sint > memStack;
   if( maxSp <= SMALL_STACK_SIZE ) {
      stack = smallStack;
   } else {
      memStack.resize( maxSp );
      stack = memStack.data();
   }

   GDTNode* data = out;
   GDTNode key;
   dip::sint ii, jj, mi;
   dip::sint li = 0;
   dip::sint ri = static_cast< dip::sint >( size - 1 );
   dip::uint sp = 0;

   if( sortType == SortType::VALUE ) {
      for( ;; ) {
         if(( ri - li ) < QSORT_MAGIC_SIZE ) {
            for( ii = li + 1; ii <= ri; ii++ ) {
               key = data[ ii ];
               jj = ii - 1;
               if( data[ jj ].value > key.value ) {
                  while(( jj >= li ) && ( data[ jj ].value > key.value )) {
                     data[ jj + 1 ] = data[ jj ];
                     jj--;
                  }
                  data[ jj + 1 ] = key;
               }
            }
            if( !sp ) {
               break;
            }
            li = stack[ --sp ];
            ri = stack[ --sp ];
         } else {
            // key selection and sentinel placement

            mi = ( li + ri ) >> 1;

            Sort3ByValue( data[ li ], data[ mi ], data[ ri ] );
            key = data[ mi ];
            data[ mi ] = data[ li ];
            data[ li ] = key;

            // Do quicksort
            ii = li + 1;
            jj = ri;
            for( ;; ) {
               while( data[ ii ].value < key.value ) {
                  ii++;
               }
               while( data[ jj ].value > key.value ) {
                  jj--;
               }
               if( ii >= jj ) {
                  data[ li ] = data[ jj ];
                  data[ jj ] = key;
                  break;
               }
               std::swap( data[ ii ], data[ jj ] );
               ii++;
               jj--;
            }
            // Well, this a bit of paranoia on my part... MvG
            DIP_THROW_IF( sp == maxSp, E::ARRAY_OVERFLOW );
            // Push the larger subfile on the stack ...
            if(( ri - ii ) > (( ii - 1 ) - li )) {
               stack[ sp++ ] = ri;
               stack[ sp++ ] = ii;
               ri = ii - 1;
            } else {
               stack[ sp++ ] = ii - 1;
               stack[ sp++ ] = li;
               li = ii;
            }
         }
      }
   } else {
      for( ;; ) {
         if(( ri - li ) < QSORT_MAGIC_SIZE ) {
            for( ii = li + 1; ii <= ri; ii++ ) {
               key = data[ ii ];
               jj = ii - 1;
               if( data[ jj ].offset > key.offset ) {
                  while(( jj >= li ) && ( data[ jj ].offset > key.offset )) {
                     data[ jj + 1 ] = data[ jj ];
                     jj--;
                  }
                  data[ jj + 1 ] = key;
               }
            }
            if( !sp ) {
               break;
            }
            li = stack[ --sp ];
            ri = stack[ --sp ];
         } else {
            // key selection and sentinel placement

            mi = ( li + ri ) >> 1;

            Sort3ByOffset( data[ li ], data[ mi ], data[ ri ] );
            key = data[ mi ];
            data[ mi ] = data[ li ];
            data[ li ] = key;

            // Do quicksort
            ii = li + 1;
            jj = ri;
            for( ;; ) {
               while( data[ ii ].offset < key.offset ) {
                  ii++;
               }
               while( data[ jj ].offset > key.offset ) {
                  jj--;
               }
               if( ii >= jj ) {
                  data[ li ] = data[ jj ];
                  data[ jj ] = key;
                  break;
               }
               std::swap( data[ ii ], data[ jj ] );
               ii++;
               jj--;
            }
            // Well, this a bit of paranoia on my part... MvG
            DIP_THROW_IF( sp == maxSp, E::ARRAY_OVERFLOW );
            // Push the larger subfile on the stack ...
            if(( ri - ii ) > (( ii - 1 ) - li )) {
               stack[ sp++ ] = ri;
               stack[ sp++ ] = ii;
               ri = ii - 1;
            } else {
               stack[ sp++ ] = ii - 1;
               stack[ sp++ ] = li;
               li = ii;
            }
         }
      }
   }
}

template< typename TPI >
void GDTProcessHeap(
      void const* input,
      sfloat* out,
      sfloat* distance,
      UnsignedArray const& sizes,
      IntegerArray const& strides,
      NeighborList const& neighborhood,
      IntegerArray const& offsets
) {
   TPI const* in = static_cast< TPI const* >( input );
   dip::sint size = static_cast< dip::sint >( sizes.product() );
   dip::sint width = static_cast< dip::sint >( sizes[ 0 ] );
   dip::sint height = static_cast< dip::sint >( sizes[ 1 ] );
   dip::sint depth = static_cast< dip::sint >( sizes.size() > 2 ? sizes[ 2 ] : 1 );
   dip::sint sx = strides[ 0 ];
   dip::sint sy = strides[ 1 ];
   dip::sint sz = depth > 1 ? strides[ 2 ] : 1;

   dip::uint nb_size = neighborhood.Size();
   UnsignedArray border = neighborhood.Border();
   dip::sint nb_x = static_cast< dip::sint >( border[ 0 ] );
   dip::sint nb_y = static_cast< dip::sint >( border[ 1 ] );
   dip::sint nb_z = static_cast< dip::sint >( border.size() > 2 ? border[ 2 ] : 0 );
   dip::sint const* address = offsets.data();
   std::vector< sfloat > neighborhoodDistances = neighborhood.CopyDistances< sfloat >();
   sfloat* metric = neighborhoodDistances.data(); // make copy so we can index with signed integers and not get compiler warnings.

   dip::sint x_end = width - nb_x;
   dip::sint y_end = height - nb_y;
   dip::sint z_end = depth - nb_z;

   /*
    * create heap
    * maximum heap size equals the image size (we neglect the border pixels),
    * but this maximum size is much larger than necessary for many applications
    * so we start with a smaller heap: 1/4-th of the max size.
    */
   dip::uint bottom = 1;
   dip::uint heapmax = static_cast< dip::uint >( width * height * depth );
   dip::uint heapsize = heapmax / 4;
   GDTNode init{ 0, std::numeric_limits< sfloat >::max() };
   std::vector< GDTNode > heap( heapsize + 1, init );

   dip::sint xx, yy, zz, offset;

   // top plane
   for( zz = 0; zz < nb_z; zz++ ) {
      for( yy = 0; yy < height; yy++ ) {
         offset = yy * sy + zz * sz;
         for( xx = 0; xx < width; xx++ ) {
            out[ offset ] = 0.0;
            offset += sx;
         }
      }
   }

   // middle section
   for( zz = nb_z; zz < z_end; zz++ ) {
      // left edge
      for( yy = 0; yy < nb_y; yy++ ) {
         offset = yy * sy + zz * sz;
         for( xx = 0; xx < width; xx++ ) {
            out[ offset ] = 0.0;
            offset += sx;
         }
      }
      for( yy = nb_y; yy < y_end; yy++ ) {
         // top X edge
         offset = yy * sy + zz * sz;
         for( xx = 0; xx < nb_x; xx++ ) {
            out[ offset ] = 0.0;
            offset += sx;
         }
         // seeds
         offset = nb_x * sx + yy * sy + zz * sz;
         for( xx = nb_x; xx < x_end; xx++ ) {
            if( out[ offset ] == 0.0 ) {
               for( dip::uint ll = 0; ll < nb_size; ll++ ) {
                  if( out[ offset + address[ ll ]] != 0.0 ) {
                     heap[ bottom ].offset = offset;
                     heap[ bottom ].value = 0.0;
                     bottom++;

                     // heap large enough?
                     if(( bottom > heapsize ) && ( heapsize != heapmax )) {
                        // reallocate heap
                        heap.resize( heapmax + 1, init );
                        // update heapsize
                        heapsize = heapmax;
                     }

                     // heap still large enough?
                     DIP_THROW_IF( bottom > heapsize, "bottom > heapsize" );
                     break;
                  }
               }
            } else {
               out[ offset ] = std::numeric_limits< sfloat >::max();
            }
            offset += sx;
         }

         // bottom X edge
         offset = x_end * sx + yy * sy + zz * sz;
         for( xx = x_end; xx < width; xx++ ) {
            out[ offset ] = 0.0;
            offset += sx;
         }
      }
      // right edge
      for( yy = y_end; yy < height; yy++ ) {
         offset = yy * sy + zz * sz;
         for( xx = 0; xx < width; xx++ ) {
            out[ offset ] = 0.0;
            offset += sx;
         }
      }
   }

   // bottom plane
   for( zz = z_end; zz < depth; zz++ ) {
      for( yy = 0; yy < height; yy++ ) {
         offset = yy * sy + zz * sz;
         for( xx = 0; xx < width; xx++ ) {
            out[ offset ] = 0.0;
            offset += sx;
         }
      }
   }

   // generate distances
   dip::sint neig, expanding;
   while( --bottom > 0 ) {
      // get pixel with lowest value
      expanding = heap[ 1 ].offset;

      // reshuffle heap
      dip::uint parent = 1;
      dip::uint child;
      while(( child = parent << 1 ) < bottom ) {
         // child==left, child|1==Right
         if( heap[ child | 1 ].value < heap[ child ].value ) {
            heap[ parent ] = heap[ child | 1 ];
            parent = child | 1;
         } else {
            heap[ parent ] = heap[ child ];
            parent = child;
         }
      }

      if( child == bottom ) {
         heap[ parent ] = heap[ child ];
      } else {
         if( parent != bottom ) {
            child = parent;
            while(( parent = child >> 1 ) &&
                  heap[ parent ].value > heap[ bottom ].value ) {
               heap[ child ] = heap[ parent ];
               child = parent;
            }
            heap[ child ] = heap[ bottom ];
         }
      }

      // check the validity of expanding
      if(( expanding < 0 ) || ( expanding > size )) {
         // TODO: This assumes that strides are positive???
         continue;
      }

      // wandering waves
      sfloat uptonow = out[ expanding ];

      for( dip::uint jj = 0; jj < nb_size; jj++ ) {
         neig = expanding + address[ jj ];
         sfloat value = uptonow + metric[ jj ] * static_cast< sfloat >( in[ neig ] );
         if( value < out[ neig ] ) {
            out[ neig ] = value;
            if( distance ) {
               distance[ neig ] = distance[ expanding ] + metric[ jj ];
            }

            // if heap is getting full, clean up ...
            if( bottom == heapsize ) {
               // to speed up things, we first sort the heap by offset
               GDTQuickSort( &heap[ 1 ], bottom - 1, SortType::OFFSET );

               // initialise clean-up
               GDTNode* oldHeap = heap.data();
               GDTNode* newHeap = heap.data();
               dip::uint newBottom = 1;
               newHeap[ 1 ] = oldHeap[ 1 ];

               // copy the other items, if it has the smallest value
               for( dip::uint ii = 2; ii < bottom; ii++ ) {
                  if( oldHeap[ ii ].offset == newHeap[ newBottom ].offset ) {
                     if( newHeap[ newBottom ].value > oldHeap[ ii ].value ) {
                        newHeap[ newBottom ] = oldHeap[ ii ];
                     }
                  } else {
                     newBottom++;
                     newHeap[ newBottom ] = oldHeap[ ii ];
                  }
               }
               newBottom++;

               // reset the remainder of the heap
               for( dip::uint ii = newBottom; ii <= heapsize; ii++ ) {
                  heap[ ii ] = init;
               }

               // restore things again, by sorting cleaned-up heap by value
               GDTQuickSort( &heap[ 1 ], newBottom - 1, SortType::VALUE );
               bottom = newBottom;
            }

            // if the "small" heap runs out of mem, allocate "large" heap
            if(( bottom > heapsize ) && ( heapsize != heapmax )) {
               // reallocate heap
               heap.resize( heapmax + 1, init );
               // update heapsize
               heapsize = heapmax;
            }

            // heap large enough?
            DIP_THROW_IF( bottom > heapmax, "bottom > heapsize" );

            // store on heap
            child = bottom++;
            while(( parent = child >> 1 ) && heap[ parent ].value > value ) {
               heap[ child ] = heap[ parent ];
               child = parent;
            }
            heap[ child ].offset = neig;
            heap[ child ].value = value;
         }
      }
   }
}

} // namespace

void GreyWeightedDistanceTransform(
      Image const& c_grey,
      Image const& in,
      Image& out,
      Metric metric,
      String const& outputMode
) {
   // check whether the grey and in images are OK
   DIP_THROW_IF( !in.IsForged() || !c_grey.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar() || !c_grey.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_grey.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   dip::uint dims = in.Dimensionality();
   DIP_THROW_IF(( dims < 2 ) || ( dims > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( in.Sizes() != c_grey.Sizes(), E::SIZES_DONT_MATCH );

   // we can only support non-negative images
   DIP_THROW_IF( Minimum( in ).As< dfloat >() < 0.0, "Minimum input value < 0.0" );

   // what will we output?
   bool outputGDT = false;
   bool outputDistance = false;
   if( outputMode == "GDT" ) {
      outputGDT = true;
   } else if( outputMode == "Euclidean" ) {
      outputDistance = true;
   } else if( outputMode == "both" ) {
      outputGDT = true;
      outputDistance = true;
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }

   // find pixel size to keep
   PixelSize pixelSize = c_grey.PixelSize();
   if( !pixelSize.IsDefined() ) {
      pixelSize = in.PixelSize(); // Let's try this one instead...
   }
   if( !metric.HasPixelSize() ) {
      metric.SetPixelSize( pixelSize );
   }

   // we must have contiguous data if we want to create another image with the same strides as `grey`
   Image grey = c_grey.QuickCopy();
   grey.ForceContiguousData();

   // create temporary images
   Image gdt;
   gdt.SetStrides( grey.Strides() );
   gdt.SetSizes( grey.Sizes() );
   gdt.SetDataType( DT_SFLOAT );
   gdt.Forge();
   DIP_ASSERT( gdt.Strides() == grey.Strides() );
   gdt.Copy( in );

   Image distance;
   if( outputDistance ) {
      distance.SetStrides( grey.Strides() );
      distance.SetSizes( grey.Sizes() );
      distance.SetDataType( DT_SFLOAT );
      distance.Forge();
      DIP_ASSERT( distance.Strides() == grey.Strides());
      distance.Fill( 0 );
   }

   // get neighborhoods and metrics
   NeighborList neighborhood{ metric, dims };
   IntegerArray offsets = neighborhood.ComputeOffsets( grey.Strides() );

   // call templated function
   DIP_OVL_CALL_REAL( GDTProcessHeap, (
         grey.Origin(),
         static_cast< sfloat* >( gdt.Origin() ),
         distance.IsForged() ? static_cast< sfloat* >( distance.Origin() ) : nullptr,
         grey.Sizes(), grey.Strides(), neighborhood, offsets
   ), grey.DataType());

   // copy to output image
   if( outputGDT && outputDistance ) {
      out.ReForge( gdt.Sizes(), 2, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
      out[ 0 ].Copy( gdt );
      out[ 1 ].Copy( distance );
   } else if( outputDistance ) {
      out.Copy( distance );
   } else {
      out.Copy( gdt );
   }
   out.SetPixelSize( pixelSize );
}

} // namespace dip
