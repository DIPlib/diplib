/*
 * DIPlib 3.0
 * This file contains definitions for image tiling functions
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/iterators.h"
#include "diplib/generic_iterators.h"

namespace dip {

namespace {

inline bool CompareAllBut( UnsignedArray const& s1, UnsignedArray const& s2, dip::uint dim ) {
   for( dip::uint ii = 0; ii < s1.size(); ++ii ) {
      if( ii != dim ) {
         if( s1[ ii ] != s2[ ii ] ) {
            return false;
         }
      }
   }
   return true;
}

} // namespace

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
   dip::uint nTiles = 1; // number of tiles
   dip::uint nDim = 0;   // number of dimensions we're tiling along
   dip::uint oneDim = 0; // the one dimension we're tiling along (if nDim == 1)
   for( dip::uint ii = 0; ii < tiling.size(); ++ii ) {
      nTiles *= tiling[ ii ];
      if( tiling[ ii ] > 1 ) {
         ++nDim;
         oneDim = ii;
      }

   }
   DIP_THROW_IF( nTiles < nImages, "There are more images than fit in the tiling" ); // Note that this also ensures that all elements of `tiling` are at least 1.
   if( nTiles == 1 ) {
      c_out = in[ 0 ].get();
      return;
   }
   bool oneDimTiling = nDim == 1;
   if( !oneDimTiling ) {
      oneDim = std::numeric_limits< dip::uint >::max();
   }
   // All inputs must be forged and of the same sizes
   auto const& first = in[ 0 ].get();
   auto inSize = first.Sizes();
   auto tensor = first.Tensor();
   auto nTElems = tensor.Elements();
   auto dataType = first.DataType();
   auto colorSpace = first.ColorSpace();
   auto pixelSize = first.PixelSize();
   for( dip::uint ii = 1; ii < nImages; ++ii ) {
      auto const& img = in[ ii ].get();
      DIP_THROW_IF( !img.IsForged(), E::IMAGE_NOT_FORGED );
      DIP_THROW_IF( img.TensorElements() != nTElems, E::NTENSORELEM_DONT_MATCH );
      DIP_THROW_IF( !CompareAllBut( inSize, img.Sizes(), oneDim ), E::SIZES_DONT_MATCH );
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
   // What size will the output image be?
   UnsignedArray outSize( std::max( inSize.size(), tiling.size() ), 1 );
   for( dip::uint ii = 0; ii < inSize.size(); ++ii ) {
      outSize[ ii ] = inSize[ ii ];
   }
   tiling.resize( outSize.size(), 1 );
   if( oneDimTiling ) {
      // outSize is same as inSize, but outSize[oneDim] is the sum of the size of all inputs along oneDim
      if( oneDim < inSize.size() ) {
         for( dip::uint ii = 1; ii < nImages; ++ii ) {
            auto const& img = in[ ii ].get();
            outSize[ oneDim ] += img.Size( oneDim );
         }
      } else {
         outSize[ oneDim ] = nImages;
      }
   } else {
      // outSize element-wise multiplied by tiling
      for( dip::uint ii = 0; ii < outSize.size(); ++ii ) {
         outSize[ ii ] *= tiling[ ii ];
      }
   }
   /*
   std::cout << "Creating an output image of sizes " << outSize << ", with input image of size " << inSize << '\n';
   std::cout << "   tiling = " << tiling << " nTiles = " << nTiles << " nImages = " << nImages << '\n';
   if( oneDimTiling ) {
      std::cout << "   oneDimTiling, oneDim = " << oneDim << '\n';
   }
   */
   // Create temporary output image, initialized to the actual output image
   // We do this in case `c_out` is the same as one of the images in `in`, reforging it would destroy
   // one of the inputs. Not initializing `out` to `c_out` would mean not being able to re-use a pixel
   // buffer already allocated for `c_out`, and would mean not using any external interface set in it.
   // NOTE that there is no way for `ReForge` to keep the data segment if `c_out` is the same as one of
   // the images in `in`, because out will always be larger than each of the images in `in` (the case
   // where the is one one image to tile has already been taken care of).
   Image out( c_out ); //
   out.ReForge( outSize, nTElems, dataType );
   out.ReshapeTensor( tensor );
   out.SetColorSpace( colorSpace );
   out.SetPixelSize( pixelSize );
   if( nImages < nTiles ) {
      out.Fill( 0 ); // This is not necessary if we have enough input images to tile the whole output image.
   }
   // Do the tiling
   if( oneDimTiling ) {
      // In this case, we need to increment the offset differently for each input image
      Image tmp = out.QuickCopy();
      auto stride = out.Stride( oneDim );
      for( dip::uint ii = 0; ii < nImages; ++ii ) {
         auto const& src = in[ ii ].get();
         if( oneDim < inSize.size() ) {
            inSize[ oneDim ] = src.Size( oneDim );
         }
         tmp.SetSizesUnsafe( inSize );
         tmp.Copy( src );
         if( oneDim < inSize.size() ) {
            tmp.ShiftOriginUnsafe( stride * static_cast< dip::sint >( inSize[ oneDim ] ));
         } else {
            tmp.ShiftOriginUnsafe( stride );
         }
      }
   } else {
      // In this case, all input images are guaranteed the same size
      Image tmp = out.QuickCopy();
      tmp.SetSizesUnsafe( inSize );
      auto origin = tmp.Origin();
      IntegerArray strides = out.Strides();
      for( dip::uint ii = 0; ii < inSize.size(); ++ii ) {
         strides[ ii ] *= static_cast< dip::sint >( inSize[ ii ] );
      }
      // A simple trick to do n-D iteration: create an image of sizes `tiling`, and iterate over that:
      Image tile( tiling, 1, DT_UINT8 );
      ImageIterator< uint8 > it( tile );
      for( dip::uint ii = 0; ii < nImages; ++ii, ++it ) {
         auto const& coords = it.Coordinates();
         tmp.SetOriginUnsafe( origin );
         tmp.ShiftOriginUnsafe( Image::Offset( coords, strides, tiling ));
         tmp.Copy( in[ ii ].get() );
      }
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
            imrefar.push_back( blank );
         } else {
            imrefar.push_back( imar[ static_cast< dip::uint >( lut[ row + col * M ] ) ] ); // Note we're transposing the elements here
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
   // where the is one one image to tile has already been taken care of).
   Image out( c_out ); //
   out.ReForge( sizes, nImages, dataType );
   out.SetPixelSize( pixelSize );
   // Copy the input images over
   auto it_out = dip::ImageTensorIterator( out );
   auto it_in = in.begin();
   do {
      it_out->Copy( it_in->get() );
   } while( ++it_in, ++it_out );
   // We're done, now swap `out` and `c_out`.
   c_out.swap( out );
}

} // namespace dip
