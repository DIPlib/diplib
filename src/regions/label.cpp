/*
 * (c)2017-2019, Cris Luengo.
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
#include "diplib/regions.h"
#include "diplib/union_find.h"
#include "diplib/neighborlist.h"
#include "diplib/iterators.h"
#include "diplib/boundary.h"
#include "diplib/framework.h" // for OptimalProcessingDim

#include "labelingGrana2016.h"

namespace dip {

namespace {

// Returns true if a pixel with relative coordinates `coords` is connected according to `connectivity`.
bool IsConnected( IntegerArray const& coords, dip::uint connectivity ) {
   dip::uint dist = 0;
   for( auto c : coords ) {
      c = std::abs( c );
      if( c > 1 ) {
         return false;
      } else if( c == 1 ) {
         ++dist;
      }
   }
   return dist <= connectivity;
}

// Returns true if relative coordinates `coords` is the previous pixel along the same line, given by `procDim`.
bool IsPrevious( IntegerArray const& coords, dip::uint procDim ) {
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      if( coords[ ii ] != ( ii == procDim ? -1 : 0 )) {
         return false;
      }
   }
   return true;
}

// A trivial connected component analysis routine that works for any dimensionality and any connectivity,
// to be used only for images that are too small for `LabelFirstPass`. This is the case when the largest
// dimension has size 1 or 2.
void LabelFirstPassTinyImage(
      Image& c_img,
      LabelRegionList& regions,
      NeighborList const& c_neighborList
) {
   // Select only those neighbors that are processed earlier
   NeighborList neighborList = c_neighborList.SelectBackward();
   IntegerArray neighborOffsets = neighborList.ComputeOffsets( c_img.Strides() );
   // Prepare other needed data
   LabelType lastLabel = regions.Create( 0 ); // This is the region for label 1, which we cannot use because unprocessed pixels have this value
   DIP_ASSERT( lastLabel == 1 );
   // Loop over every image line
   ImageIterator< LabelType > it( c_img );
   do {
      if( *it ) {
         lastLabel = 0;
         auto nl = neighborList.begin();
         auto no = neighborOffsets.begin();
         for( ; nl != neighborList.end(); ++no, ++nl ) {
            if( nl.IsInImage( it.Coordinates(), c_img.Sizes() )) {
               LabelType lab = it.Pointer()[ *no ];
               if( lab ) {
                  if( lastLabel ) {
                     lastLabel = regions.Union( lastLabel, lab );
                  } else {
                     lastLabel = lab;
                  }
               }
            }
         }
         if( lastLabel ) {
            ++( regions.Value( lastLabel ));
         } else {
            lastLabel = regions.Create( 1 );
         }
         *it = lastLabel;
      }
   } while( ++it );
}

// A union-find connected component analysis routine that works for any dimensionality and any connectivity.
void LabelFirstPass(
      Image& c_img,
      LabelRegionList& regions,
      NeighborList const& c_neighborList,
      dip::uint connectivity
) {
   dip::uint procDim = Framework::OptimalProcessingDim( c_img ); // this will typically be 0, because we've "standardized the strides".
   dip::uint length = c_img.Size( procDim );
   if( length < 3 ) {
      // Note that if length < 3, the image is very small all around, because `OptimalProcessingDim` will return a larger dimension if it exists.
      LabelFirstPassTinyImage( c_img, regions, c_neighborList );
      return;
   }
   // Select only those neighbors that are processed earlier
   NeighborList neighborList = c_neighborList.SelectBackward( procDim );
   IntegerArray neighborOffsets = neighborList.ComputeOffsets( c_img.Strides() );
   // Which neighbors are "forward" neighbors (m in description below)? Which neighbor is the "previous" neighbor (p in description below)?
   BooleanArray neighborIsForward( neighborList.Size(), false );
   dip::uint previousNeighborIndex = 0;
   for( dip::uint ii = 0; ii < neighborList.Size(); ++ii ) {
      IntegerArray cc = neighborList.Coordinates( ii );
      if( IsPrevious( cc, procDim )) {
         previousNeighborIndex = ii;
      } else {
         ++( cc[ procDim ] );
         if( !IsConnected( cc, connectivity )) {
            neighborIsForward[ ii ] = true;
         }
      }
   }
   // Prepare other needed data
   dip::sint stride = c_img.Stride( procDim );
   dip::sint endOffset = stride * static_cast< dip::sint >( length  - 1 );
   LabelType lastLabel = regions.Create( 0 ); // This is the region for label 1, which we cannot use because unprocessed pixels have this value
   DIP_ASSERT( lastLabel == 1 );
   // Loop over every image line
   ImageIterator< LabelType > it( c_img, procDim );
   do {
      // Which neighbors can we use on this line?
      //    +-+-+-+  `x` = current pixel
      //    |n|n|m|  `p` = previous pixel
      //    +-+-+-+  `n` = neighbor that is also a neighbor to `p`
      //    |p|x| |  `m` = neighbor that is not a neighbor to `p`
      //    +-+-+-+
      //    | | | |  If `p` is set, `x` gets the same label. We need to test only `m` pixels
      //    +-+-+-+  Otherwise, we test all pixels `n` and `m`.
      std::vector< dip::sint > allNeighbors;     // `n` and `m` pixels
      allNeighbors.reserve( neighborList.Size() );
      std::vector< dip::sint > forwardNeighbors; // `m` pixels only
      forwardNeighbors.reserve( neighborList.Size() );
      BooleanArray neighorIsInImage( neighborList.Size(), false );
      UnsignedArray coords = it.Coordinates();
      coords[ procDim ] = 1; // pretend we're in the middle of the line here, so we can properly test for neighbors being inside or outside the image
                             // NOTE! this will not work correctly if the image has less that 3 pixels along `procDim`.
      for( dip::uint ii = 0; ii < neighborList.Size(); ++ii ) {
         if(( ii != previousNeighborIndex ) && neighborList.IsInImage( ii, coords, c_img.Sizes() )) {
            neighorIsInImage[ ii ] = true;
            allNeighbors.push_back( neighborOffsets[ ii ] );
            if( neighborIsForward[ ii ] ) {
               forwardNeighbors.push_back( neighborOffsets[ ii ] );
            }
         }
      }
      lastLabel = 0;
      LabelType* img = it.Pointer();
      LabelType* end = img + endOffset;

      // First pixel on line:
      if( *img ) {
         coords[ procDim ] = 0;
         for( dip::uint ii = 0; ii < neighborList.Size(); ++ii ) {
            if( neighorIsInImage[ ii ] && neighborList.IsInImage( ii, coords, c_img.Sizes() )) {
               LabelType lab = img[ neighborOffsets[ ii ]];
               if( lab ) {
                  if( lastLabel ) {
                     lastLabel = regions.Union( lastLabel, lab );
                  } else {
                     lastLabel = lab;
                  }
               }
            }
         }
         if( lastLabel ) {
            ++( regions.Value( lastLabel ));
         } else {
            lastLabel = regions.Create( 1 );
         }
         *img = lastLabel;
      }
      img += stride;

      // The rest of the pixels:
      do {
         if( *img ) {
            if( lastLabel ) {
               for( auto nn : forwardNeighbors ) {
                  LabelType lab = img[ nn ];
                  if( lab ) {
                     lastLabel = regions.Union( lastLabel, lab );
                  }
               }
               ++( regions.Value( lastLabel ));
               *img = lastLabel;
            } else {
               for( auto nn : allNeighbors ) {
                  LabelType lab = img[ nn ];
                  if( lab ) {
                     if( lastLabel ) {
                        lastLabel = regions.Union( lastLabel, lab );
                     } else {
                        lastLabel = lab;
                     }
                  }
               }
               if( lastLabel ) {
                  ++( regions.Value( lastLabel ));
               } else {
                  lastLabel = regions.Create( 1 );
               }
               *img = lastLabel;
            }
         } else {
            lastLabel = 0;
         }
      } while(( img += stride ) != end );

      // The last pixel:
      if( *img ) {
         coords[ procDim ] = length - 1;
         for( dip::uint ii = 0; ii < neighborList.Size(); ++ii ) {
            if( neighborIsForward[ ii ] && neighorIsInImage[ ii ] && neighborList.IsInImage( ii, coords, c_img.Sizes() )) {
               LabelType lab = img[ neighborOffsets[ ii ]];
               if( lab ) {
                  if( lastLabel ) {
                     lastLabel = regions.Union( lastLabel, lab );
                  } else {
                     lastLabel = lab;
                  }
               }
            }
         }
         if( lastLabel ) {
            ++( regions.Value( lastLabel ));
         } else {
            lastLabel = regions.Create( 1 );
         }
         *img = lastLabel;
      }

   } while( ++it );

}

} // namespace

dip::uint Label(
      Image const& c_in,
      Image& c_out,
      dip::uint connectivity,
      dip::uint minSize,
      dip::uint maxSize,
      StringArray boundaryCondition
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );

   Image in = c_in.QuickCopy();
   auto pixelSize = c_in.PixelSize();
   c_out.ReForge( in, DT_LABEL );
   c_out.SetPixelSize( std::move( pixelSize ));
   Image out = c_out.QuickCopy();
   out.StandardizeStrides(); // Reorder dimensions so the looping is more efficient. Also removes singleton dimensions!

   LabelRegionList regions{ std::plus< dip::uint >{} };

   if( connectivity == 0 ) {
      connectivity = nDims;
   }

   // First scan
   dip::uint trueNDims = out.Dimensionality(); // If `c_in` had singleton dimensions, `out` will have fewer dimensions
   dip::uint trueConnectivity = std::min( connectivity, trueNDims );
   if(( trueNDims == 2 ) && ( trueConnectivity == 2 )) {
      out.Fill( 0 );
      Image granaIn = in.QuickCopy();
      Image granaOut = c_out.QuickCopy(); // Note use of `c_out` here, not `out`, because dimensions must agree with `in`.
      if( nDims > 2 ) {
         // This is the case where we had singleton dimensions
         granaIn.Squeeze();
         granaOut.Squeeze();
      }
      LabelFirstPass_Grana2016( granaIn, granaOut, regions );
      // This saves ~20% on an image 2k x 2k pixels: 0.0559 vs 0.0658s
      // (including MATLAB overhead, probably slightly larger relative difference without that overhead).
   } else {
      c_out.Copy( in ); // Copy `in` into `c_out`, not into `out`, which could be reshaped.
      NeighborList neighborList( { Metric::TypeCode::CONNECTED, trueConnectivity }, trueNDims );
      DIP_STACK_TRACE_THIS( LabelFirstPass( out, regions, neighborList, trueConnectivity ));
      regions.Union( 0, 1 ); // This gets rid of label 1, which we used internally, but otherwise causes the first region to get label 2.
   }

   // Handle boundary condition
   if( !boundaryCondition.empty() ) {
      // Replicate what ArrayUseParameter does (StringArray is not a DimensionArray)
      if( boundaryCondition.size() == 1 ) {
         boundaryCondition.resize( nDims, boundaryCondition[ 0 ] );
      } else if( boundaryCondition.size() != nDims ) {
         DIP_THROW( E::ARRAY_PARAMETER_WRONG_LENGTH );
      }
      // We use `c_out` here, not `out`, because we need to be sure of which dimension is being processed.
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if(( boundaryCondition[ ii ] == "" ) || ( boundaryCondition[ ii ] == S::SYMMETRIC_MIRROR )) {
            // Do nothing.
         } else if( boundaryCondition[ ii ] == S::PERIODIC ) {
            if( c_out.Size( ii ) > 2 ) { // >2 because there's no effect for fewer pixels.
               // Merge labels for objects touching opposite sides of image along this dimension.
               // We do a lot of out-of-bounds testing, which is relatively expensive. But this is a not-so-commonly
               // used feature, it's OK if it's not super-fast.
               DIP_START_STACK_TRACE
                  NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
                  IntegerArray neighborOffsets = neighborList.ComputeOffsets( c_out.Strides());
                  std::vector< dip::sint > otherSideOffsets; // Don't use IntegerArray, we will push_back
                  otherSideOffsets.reserve( neighborList.Size() );
                  std::vector< IntegerArray > otherSideCoords;
                  otherSideCoords.reserve( neighborList.Size() );
                  dip::sint acrossImage = c_out.Stride( ii ) * static_cast< dip::sint >( c_out.Size( ii ));
                  auto nl = neighborList.begin();
                  auto no = neighborOffsets.begin();
                  for( ; nl != neighborList.end(); ++no, ++nl ) {
                     if( nl.Coordinates()[ ii ] == -1 ) {
                        // This neighbor wraps around the image
                        otherSideOffsets.push_back( *no + acrossImage );
                        IntegerArray coords = nl.Coordinates();
                        coords[ ii ] += static_cast< dip::sint >( c_out.Size( ii ));
                        otherSideCoords.push_back( coords );
                     }
                  }
                  ImageIterator< LabelType > it( c_out, ii );
                  do {
                     for( dip::uint kk = 0; kk < otherSideCoords.size(); ++kk ) {
                        // Is this neighbor in the image?
                        IntegerArray coords = otherSideCoords[ kk ];
                        coords += it.Coordinates();
                        bool use = true;
                        for( dip::uint dd = 0; dd < coords.size(); ++dd ) { // coords.size() == nDims, using size() because GCC thinks we might be indexing out of bounds below.
                           // Relying on 2's complement conversion, coords can be a small negative value, which will
                           // convert to a very large unsigned value, and test larger than the image size.
                           if( static_cast< dip::uint >( coords[ dd ] ) >= c_out.Size( dd )) {
                              use = false;
                              break;
                           }
                        }
                        if( use ) {
                           LabelType lab1 = *it;
                           LabelType lab2 = *( it.Pointer() + otherSideOffsets[ kk ] );
                           if(( lab1 > 0 ) && ( lab2 > 0 )) {
                              regions.Union( lab1, lab2 );
                           }
                        }
                     }
                  } while( ++it );
               DIP_END_STACK_TRACE
            }
         } else if( boundaryCondition[ ii ] == "remove" ) {
            if( c_out.Size( ii ) > 1 ) { // Skip singleton dimensions
               DIP_START_STACK_TRACE
                  dip::sint otherSideOffset = static_cast< dip::sint >( c_out.Size( ii ) - 1 ) * c_out.Stride( ii );
                  ImageIterator< LabelType > it( c_out, ii );
                  LabelType prev0 = 0;
                  LabelType prevN = 0;
                  do {
                     LabelType lab = *it;
                     if(( lab != 0 ) && ( lab != prev0 )) {
                        regions.Union( lab, 0 );
                        prev0 = lab;
                     }
                     lab = *( it.Pointer() + otherSideOffset );
                     if(( lab != 0 ) && ( lab != prevN )) {
                        regions.Union( lab, 0 );
                        prevN = lab;
                     }
                  } while( ++it );
               DIP_END_STACK_TRACE
            }
         } else {
            DIP_THROW_INVALID_FLAG( boundaryCondition[ ii ] );
         }
      }
   }

   // Relabel
   dip::uint nLabel;
   if(( minSize > 0 ) && ( maxSize > 0 )) {
      nLabel = regions.Relabel(
            [ & ]( dip::uint size ){ return ( size >= minSize ) && ( size <= maxSize ); }
      );
   } else if( minSize > 0 ) {
      nLabel = regions.Relabel(
            [ & ]( dip::uint size ){ return size >= minSize; }
      );
   } else if( maxSize > 0 ) {
      nLabel = regions.Relabel(
            [ & ]( dip::uint size ){ return size <= maxSize; }
      );
   } else {
      nLabel = regions.Relabel();
   }

   // Second scan
   ImageIterator< LabelType > it( out );
   do {
      if( *it > 0 ) {
         *it = regions.Label( *it );
      }
   } while( ++it );

   return nLabel;
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"
#include "diplib/statistics.h"
#include "diplib/binary.h"

DOCTEST_TEST_CASE("[DIPlib] testing dip::Label") {

   // Two 3D intertwined helices. Generated data points using the following Python:
   /*
   import numpy as np
   import diplib as dip

   h = 2
   r = 25
   t = np.arange(-r, r)
   p = np.stack((h * t, r * np.cos(t), r * np.sin(t)))

   R = np.array(dip.RotationMatrix3D(30/180*np.pi, 45/180*np.pi, 15/180*np.pi)).reshape(3, 3)
   p = R @ p
   p = p.round().astype(int).transpose()

   s = 100
   p = p + np.array([s//2, s//2, s//2])[None,:]
   assert(np.amax(p) < s)
   assert(np.amin(p) >= 0)

   q = p + (R @ np.array([6, 0, 0])[:, None]).round().transpose().astype(int)
   assert(np.amax(q) < s)
   assert(np.amin(q) >= 0)

   img = dip.Image((s, s, s), 1, 'UINT8')
   img.Fill(0)
   dip.DrawLines(img, p.tolist(), 1, "add")
   dip.DrawLines(img, q.tolist(), 1, "add")
   */
   dip::CoordinateArray p{
         { 41, 98, 23 }, { 21, 93, 35 }, { 7,  74, 31 }, { 13, 56, 16 }, { 35, 56,  4 }, { 52, 72,  8 },
         { 51, 89, 26 }, { 32, 89, 42 }, { 15, 73, 43 }, { 15, 53, 29 }, { 34, 47, 15 }, { 54, 60, 14 },
         { 59, 78, 29 }, { 44, 84, 47 }, { 25, 71, 53 }, { 19, 50, 42 }, { 33, 40, 26 }, { 55, 48, 21 },
         { 66, 66, 32 }, { 56, 77, 51 }, { 35, 69, 61 }, { 24, 49, 55 }, { 34, 34, 39 }, { 55, 36, 29 },
         { 71, 54, 36 }, { 66, 69, 55 }, { 47, 66, 69 }, { 31, 48, 67 }, { 35, 29, 52 }, { 55, 27, 39 },
         { 74, 41, 41 }, { 75, 59, 58 }, { 59, 62, 75 }, { 40, 47, 78 }, { 37, 26, 65 }, { 54, 18, 50 },
         { 76, 29, 47 }, { 83, 47, 61 }, { 70, 56, 79 }, { 50, 45, 87 }, { 42, 25, 79 }, { 54, 12, 62 },
         { 76, 17, 55 }, { 89, 35, 64 }, { 81, 48, 83 }, { 61, 43, 95 }, { 48, 23, 91 }, { 54, 6,  75 },
         { 76,  7, 64 }, { 93, 23, 69 }
   };
   dip::CoordinateArray q{
         { 44, 95, 27 }, { 24, 90, 39 }, { 10, 71, 35 }, { 16, 53, 20 }, { 38, 53,  8 }, { 55, 69, 12 },
         { 54, 86, 30 }, { 35, 86, 46 }, { 18, 70, 47 }, { 18, 50, 33 }, { 37, 44, 19 }, { 57, 57, 18 },
         { 62, 75, 33 }, { 47, 81, 51 }, { 28, 68, 57 }, { 22, 47, 46 }, { 36, 37, 30 }, { 58, 45, 25 },
         { 69, 63, 36 }, { 59, 74, 55 }, { 38, 66, 65 }, { 27, 46, 59 }, { 37, 31, 43 }, { 58, 33, 33 },
         { 74, 51, 40 }, { 69, 66, 59 }, { 50, 63, 73 }, { 34, 45, 71 }, { 38, 26, 56 }, { 58, 24, 43 },
         { 77, 38, 45 }, { 78, 56, 62 }, { 62, 59, 79 }, { 43, 44, 82 }, { 40, 23, 69 }, { 57, 15, 54 },
         { 79, 26, 51 }, { 86, 44, 65 }, { 73, 53, 83 }, { 53, 42, 91 }, { 45, 22, 83 }, { 57,  9, 66 },
         { 79, 14, 59 }, { 92, 32, 68 }, { 84, 45, 87 }, { 64, 40, 99 }, { 51, 20, 95 }, { 57,  3, 79 },
         { 79,  4, 68 }, { 96, 20, 73 }
   };
   dip::uint s = 100;
   dip::Image img( { s, s, s }, 1, dip::DT_BIN );
   img.Fill( 0 );
   dip::DrawLines( img, p, { 1 }, "add" );
   dip::DrawLines( img, q, { 1 }, "add" );
   dip::Image lab;
   dip::uint n = dip::Label( img, lab, 3 );
   DOCTEST_CHECK( n == 2 );
   DOCTEST_CHECK( dip::Maximum( lab ).As< dip::LabelType >() == 2 );
   DOCTEST_CHECK( dip::All(( lab > 0 ) == img ).As< bool >() );

   dip::uint n2 = dip::Label( img, lab, 2 );
   dip::uint n1 = dip::Label( img, lab, 1 );
   DOCTEST_CHECK( n1 > n2 ); // I don't know how many labels we'll get, but we do know we'll have more as we reduce the connectivity.
   DOCTEST_CHECK( n2 > n );

   // Two 2D intertwined spirals. Generated data points using the following Python:
   /*
   t = np.arange(0, 3 * 6) / 6 * np.pi
   h = 5
   o = 10
   s = 100
   p = np.stack(((h * t + o) * np.cos(t), (h * t + o) * np.sin(t))).transpose()
   p = p + np.array([s/2, s/2])[None,:]
   p = p.round().astype(int)
   q = s - p

   img = dip.Image((s, s), 1, 'UINT8')
   img.Fill(0)
   dip.DrawLines(img, p.tolist(), 1, "add")
   dip.DrawLines(img, q.tolist(), 1, "add")
   */
   p = dip::CoordinateArray{
         { 60, 50 }, { 61, 56 }, { 58, 63 }, { 50, 68 }, { 40, 68 }, { 30, 62 },
         { 24, 50 }, { 25, 36 }, { 35, 23 }, { 50, 16 }, { 68, 19 }, { 84, 31 },
         { 91, 50 }, { 88, 72 }, { 73, 90 }, { 50, 99 }, { 24, 95 }, {  3, 77 }
   };
   q = dip::CoordinateArray{
         { 40, 50 }, { 39, 44 }, { 42, 37 }, { 50, 32 }, { 60, 32 }, { 70, 38 },
         { 76, 50 }, { 75, 64 }, { 65, 77 }, { 50, 84 }, { 32, 81 }, { 16, 69 },
         {  9, 50 }, { 12, 28 }, { 27, 10 }, { 50,  1 }, { 76,  5 }, { 97, 23 }
   };
   img = dip::Image( { s, s }, 1, dip::DT_BIN );
   img.Fill( 0 );
   dip::DrawLines( img, p, { 1 }, "add" );
   dip::DrawLines( img, q, { 1 }, "add" );
   n = dip::Label( img, lab, 2 );
   DOCTEST_CHECK( n == 2 );
   DOCTEST_CHECK( dip::Maximum( lab ).As< dip::LabelType >() == 2 );
   DOCTEST_CHECK( dip::All(( lab > 0 ) == img ).As< bool >() );

   dip::BinaryDilation( img, img, 1, 1 );
   n = dip::Label( img, lab, 2 );
   DOCTEST_CHECK( n == 2 );
   DOCTEST_CHECK( dip::Maximum( lab ).As< dip::LabelType >() == 2 );
   DOCTEST_CHECK( dip::All(( lab > 0 ) == img ).As< bool >() );

   n = dip::Label( img, lab, 1 );
   DOCTEST_CHECK( n == 2 );
   DOCTEST_CHECK( dip::Maximum( lab ).As< dip::LabelType >() == 2 );
   DOCTEST_CHECK( dip::All(( lab > 0 ) == img ).As< bool >() );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
