/*
 * (c)2018-2026, Cris Luengo.
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

#include "diplib/geometry.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "diplib.h"
#include "diplib/generic_iterators.h"
#include "diplib/iterators.h"

namespace dip {
void Tile(
      ImageConstRefArray const& in,
      Image& c_out,
      UnsignedArray tiling
) {
   dip::uint nImages = in.size();
   DIP_THROW_IF( nImages == 0, E::ARRAY_PARAMETER_EMPTY ); // If you want to create an empty image, just do that directly...
   DIP_THROW_IF( !in[ 0 ].get().IsForged(), E::IMAGE_NOT_FORGED );
   if( tiling.empty() ) {
      tiling.resize( 2, 1 );
      tiling[ 0 ] = static_cast< dip::uint >( ceil_cast( std::sqrt( nImages )));
      tiling[ 1 ] = ( nImages - 1 ) / tiling[ 0 ] + 1;
   }
   dip::uint nTiles = tiling.product();
   DIP_THROW_IF( nTiles < nImages, "There are more images than fit in the tiling" ); // Note that this also ensures that all elements of `tiling` are at least 1.
   if( nTiles == 1 ) {
      c_out = in[ 0 ].get();
      return;
   }
   // All inputs must be forged. Figure out output properties at the same time.
   auto const& first = in[ 0 ].get();
   auto nDims = first.Dimensionality();
   auto tensor = first.Tensor();
   auto nTElems = tensor.Elements();
   auto dataType = first.DataType();
   auto colorSpace = first.ColorSpace();
   auto pixelSize = first.PixelSize();
   for( dip::uint ii = 1; ii < nImages; ++ii ) {
      auto const& img = in[ ii ].get();
      DIP_THROW_IF( !img.IsForged(), E::IMAGE_NOT_FORGED );
      nDims = std::max( nDims, img.Dimensionality() );
      DIP_THROW_IF( img.TensorElements() != nTElems, E::NTENSORELEM_DONT_MATCH );
      if( img.Tensor() != tensor ) {
         tensor.ChangeShape();
      }
      dataType = DataType::SuggestDyadicOperation( dataType, img.DataType() );
      if( colorSpace.empty() ) {
         colorSpace = img.ColorSpace(); // We use the first color space that we come across
      }
      if( !pixelSize.IsDefined() ) {
         pixelSize = img.PixelSize(); // We use the first pixel size that we come across
      }
   }
   nDims = std::max( nDims, tiling.size() );
   tiling.resize( nDims, 1 );
   // A simple trick to do n-D iteration: create an image of sizes `tiling`, and iterate over that:
   Image tile( tiling, 1, DT_UINT8 );
   // Check that input images have matching sizes along rows and columns of the tiling. And compute the output size.
   std::vector< UnsignedArray > imageSizes( nDims );
   UnsignedArray outSize( nDims, 1 );
   for( dip::uint jj = 0; jj < nDims; ++jj ) {
      imageSizes[ jj ] = UnsignedArray( tiling[ jj ] );
      if( tiling[ jj ] == 1 ) {
         // Just check all images have the same size along this dimension.
         dip::uint expected_size = 0;
         for( dip::uint ii = 0; ii < nImages; ++ii ) {
            auto const& img = in[ ii ].get();
            dip::uint sz = jj < img.Dimensionality() ? img.Size( jj ) : 1;
            if( ii == 0 ) {
               expected_size = sz;
            } else {
               DIP_THROW_IF( expected_size != sz, E::SIZES_DONT_MATCH );
            }
         }
         outSize[ jj ] = expected_size;
      } else {
         // Compare sizes along this dimension for images that are along the same row/column
         ImageIterator< uint8 > it( tile );
         dip::uint sum = 0;
         for( dip::uint ii = 0; ii < nImages; ++ii, ++it ) {
            auto const& img = in[ ii ].get();
            dip::uint sz = jj < img.Dimensionality() ? img.Size( jj ) : 1;
            dip::uint kk = it.Coordinates()[ jj ];
            if( imageSizes[ jj ][ kk ] == 0 ) {
               imageSizes[ jj ][ kk ] = sz;
               sum += sz;
            } else {
               DIP_THROW_IF( imageSizes[ jj ][ kk ] != sz, E::SIZES_DONT_MATCH );
            }
         }
         outSize[ jj ] = sum;
      }
   }
   // Compute offsets from image sizes
   std::vector< UnsignedArray > gridOffsets( nDims );
   for( dip::uint jj = 0; jj < nDims; ++jj ) {
      dip::uint numImgs = imageSizes[ jj ].size();
      gridOffsets[ jj ] = UnsignedArray( numImgs, 0 );
      for( dip::uint ii = 1; ii < numImgs; ++ii ) {
         gridOffsets[ jj ][ ii ] = gridOffsets[ jj ][ ii - 1 ] + imageSizes[ jj ][ ii - 1 ];
      }
   }
   // Create temporary output image, initialized to the actual output image
   // We do this in case `c_out` is the same as one of the images in `in`, reforging it would destroy
   // one of the inputs. Not initializing `out` to `c_out` would mean not being able to re-use a pixel
   // buffer already allocated for `c_out`, and would mean not using any external interface set in it.
   // NOTE that there is no way for `ReForge` to keep the data segment if `c_out` is the same as one of
   // the images in `in`, because out will always be larger than each of the images in `in` (the case
   // where the is one image to tile has already been taken care of).
   Image out( c_out ); //
   out.ReForge( outSize, nTElems, dataType );
   out.ReshapeTensor( tensor );
   out.SetColorSpace( std::move( colorSpace ));
   out.SetPixelSize( std::move( pixelSize ));
   if( nImages < nTiles ) {
      out.Fill( 0 ); // This is not necessary if we have enough input images to tile the whole output image.
   }
   // Do the tiling
   Image tmp = out.QuickCopy();
   ImageIterator< uint8 > it( tile );
   for( dip::uint ii = 0; ii < nImages; ++ii, ++it ) {
      auto const& src = in[ ii ].get();
      auto const& gridCoords = it.Coordinates();
      UnsignedArray imgCoords( nDims );
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         imgCoords[ jj ] = gridOffsets[ jj ][ gridCoords[ jj ]];
      }
      tmp.SetOriginUnsafe( out.Pointer( imgCoords ));
      tmp.SetSizesUnsafe( src.Sizes() );
      tmp.Copy( src );
   }
   // We're done, now swap `out` and `c_out`.
   c_out.swap( out );
}

void TileTensorElements(
      Image const& in,
      Image& out
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      out = in;
      return;
   }
   // Create an image for each tensor component -- data is referenced, not copied!
   dip::uint nTensor = in.TensorElements();
   ImageArray imar( nTensor );
   for( dip::uint ii = 0; ii < nTensor; ++ii ) {
      imar[ ii ] = in[ ii ];
   }
   // Create an image reference array with the images in the right order to send to `dip::Tile`.
   dip::uint M = in.TensorRows();
   dip::uint N = in.TensorColumns();
   auto lut = in.Tensor().LookUpTable(); // Tensor element `(m,n)` can be found by lut[n*M+m]
   bool complete = true;
   for( auto ii : lut ) {
      if( ii < 0 ) {
         complete = false;
         break;
      }
   }
   Image blank;
   if( !complete ) {
      blank.ReForge( in.Sizes(), 1, in.DataType() );
      blank.Fill( 0 );
   }
   ImageConstRefArray imrefar;
   for( dip::uint row = 0; row < M; ++row ) {
      for( dip::uint col = 0; col < N; ++col ) {
         if( lut[ row + col * M ] < 0 ) {
            imrefar.emplace_back( blank );
         } else {
            imrefar.emplace_back( imar[ static_cast< dip::uint >( lut[ row + col * M ] ) ] ); // Note we're transposing the elements here
         }
      }
   }
   // Call `dip::Tile` and be done!
   Tile( imrefar, out, { N, M } );
}

void JoinChannels(
      ImageConstRefArray const& in,
      Image& c_out
) {
   dip::uint nImages = in.size();
   DIP_THROW_IF( nImages == 0, E::ARRAY_PARAMETER_EMPTY ); // If you want to create an empty image, just do that directly...
   DIP_THROW_IF( !in[ 0 ].get().IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in[ 0 ].get().IsScalar(), E::IMAGE_NOT_SCALAR );
   if( nImages == 1 ) {
      c_out = in[ 0 ].get();
      return;
   }
   // All inputs must be forged and of the same sizes
   auto const& first = in[ 0 ].get();
   auto sizes = first.Sizes();
   auto dataType = first.DataType();
   auto pixelSize = first.PixelSize();
   for( dip::uint ii = 1; ii < nImages; ++ii ) {
      auto const& img = in[ ii ].get();
      DIP_THROW_IF( !img.IsForged(), E::IMAGE_NOT_FORGED );
      DIP_THROW_IF( !img.IsScalar(), E::IMAGE_NOT_SCALAR );
      DIP_THROW_IF( sizes != img.Sizes(), E::SIZES_DONT_MATCH );
      dataType = DataType::SuggestDyadicOperation( dataType, img.DataType() );
      if( !pixelSize.IsDefined() ) {
         pixelSize = img.PixelSize(); // We use the first pixel size that we come across
      }
   }
   // Create temporary output image, initialized to the actual output image
   // We do this in case `c_out` is the same as one of the images in `in`, reforging it would destroy
   // one of the inputs. Not initializing `out` to `c_out` would mean not being able to re-use a pixel
   // buffer already allocated for `c_out`, and would mean not using any external interface set in it.
   // NOTE that there is no way for `ReForge` to keep the data segment if `c_out` is the same as one of
   // the images in `in`, because out will always be larger than each of the images in `in` (the case
   // where the is only one image to tile has already been taken care of).
   Image out( c_out ); //
   out.ReForge( sizes, nImages, dataType );
   out.SetPixelSize( std::move( pixelSize ));
   // Copy the input images over
   auto it_out = dip::ImageTensorIterator( out );
   auto it_in = in.begin();
   do {
      it_out->Copy( it_in->get() );
   } while( ++it_in, ++it_out );
   // We're done, now swap `out` and `c_out`.
   c_out.swap( out );
}

ImageArray Dice(
      Image const& in,
      UnsignedArray grid
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nImages = grid.product();
   DIP_THROW_IF( nImages == 0, E::INVALID_PARAMETER );
   ImageArray out( nImages );
   if( nImages == 1 ) {
      out[ 0 ] = in;
      return out;
   }
   // Compute output size, error if we can't split
   dip::uint nDims = std::min( in.Dimensionality(), grid.size() );
   UnsignedArray outSizes( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_THROW_IF( in.Size( ii ) < grid[ ii ], E::INVALID_PARAMETER );
      outSizes[ ii ] = in.Size( ii ) / grid[ ii ];
   }
   for( dip::uint ii = nDims; ii < grid.size(); ++ii ) { // In case the grid array is larger than the image dimensionality
      DIP_THROW_IF( grid[ ii ] > 1, E::INVALID_PARAMETER );
   }
   nDims = in.Dimensionality();
   grid.resize( nDims );
   // A simple trick to do n-D iteration: create an image of sizes `grid`, and iterate over that:
   Image tile( grid, 1, DT_UINT8 );
   ImageIterator< uint8 > it( tile );
   for( dip::uint ii = 0; ii < nImages; ++ii, ++it ) {
      auto const& gridCoords = it.Coordinates();
      UnsignedArray imgCoords( nDims );
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         imgCoords[ jj ] = outSizes[ jj ] * gridCoords[ jj ];
      }
      out[ ii ] = in;
      out[ ii ].SetSizesUnsafe( outSizes );
      out[ ii ].SetOriginUnsafe( in.Pointer( imgCoords ));
   }
   return out;
}

ImageArray SplitChannels(
      Image const& in
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   ImageArray out( in.TensorElements(), in );
   for( dip::uint ii = 0; ii < out.size(); ++ii ) {
      out[ ii ].SetTensorSizesUnsafe( 1 );
      out[ ii ].ShiftOriginUnsafe( static_cast< dip::sint >( ii ) * in.TensorStride() );
   }
   return out;
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"
#include "diplib/random.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Dice and dip::Tile" ) {
   dip::Random random;
   dip::Image img( { 50, 30 }, 3, dip::DT_UINT8 );
   img.ReshapeTensor( dip::Tensor( dip::Tensor::Shape::SYMMETRIC_MATRIX, 2, 2 ));  // A 2x2 symmetric tensor has 3 elements
   img.SetPixelSize( dip::PhysicalQuantityArray{ 0.4 * dip::Units::Micrometer(), 1.3 * dip::Units::Second() } );
   img.SetColorSpace( "CMYK" );  // This cannot have 6 tensor components, but that doesn't matter for this test.
   dip::UniformNoise( img, img, random, 0, 255 );
   auto imar = dip::Dice( img, { 2, 3 } );
   DOCTEST_CHECK( imar.size() == 6 );
   for( auto const& a : imar ) {
      DOCTEST_CHECK( a.IsForged() );
      DOCTEST_CHECK( a.TensorElements() == img.TensorElements() );
      DOCTEST_CHECK( a.TensorShape() == img.TensorShape() );
      DOCTEST_CHECK( a.PixelSize() == img.PixelSize() );
      DOCTEST_CHECK( a.ColorSpace() == img.ColorSpace() );
      DOCTEST_CHECK( a.Sizes() == dip::UnsignedArray{ 25, 10 } );
   }
   dip::Image out = dip::Tile( dip::CreateImageConstRefArray( imar ), { 2, 3 } );
   DOCTEST_CHECK( dip::testing::CompareImages( img, out ));
   DOCTEST_CHECK( !img.Aliases( out ));
   DOCTEST_CHECK( out.CompareProperties( img, dip::Option::CmpProp::All ));
}

DOCTEST_TEST_CASE( "[DIPlib] testing dip::SplitChannels and dip::JoinChannels" ) {
   dip::Random random;
   dip::Image img( { 50, 30 }, 6, dip::DT_UINT8 );
   img.ReshapeTensor( dip::Tensor( dip::Tensor::Shape::SYMMETRIC_MATRIX, 3, 3 ));  // A 3x3 symmetric tensor has 6 elements
   img.SetPixelSize( dip::PhysicalQuantityArray{ 0.4 * dip::Units::Micrometer(), 1.3 * dip::Units::Second() } );
   img.SetColorSpace( "CMYK" );  // This cannot have 6 tensor components, but that doesn't matter for this test.
   dip::UniformNoise( img, img, random, 0, 255 );
   auto imar = dip::SplitChannels( img );
   DOCTEST_CHECK( imar.size() == 6 );
   for( auto const& a : imar ) {
      DOCTEST_CHECK( a.IsForged() );
      DOCTEST_CHECK( a.IsScalar() );
   }
   dip::Image out = dip::JoinChannels( dip::CreateImageConstRefArray( imar ));
   DOCTEST_CHECK( dip::testing::CompareImages( img, out ));
   DOCTEST_CHECK( !img.Aliases( out ));
   DOCTEST_CHECK( out.PixelSize() == img.PixelSize() );
   DOCTEST_CHECK( out.TensorShape() == dip::Tensor::Shape::COL_VECTOR );  // This is the default shape
   DOCTEST_CHECK( out.ColorSpace().empty() );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
