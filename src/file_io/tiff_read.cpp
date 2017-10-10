/*
 * DIPlib 3.0
 * This file contains definitions for TIFF reading
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

#ifdef DIP__HAS_TIFF

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/generic_iterators.h"

#include <tiffio.h>

namespace dip {

namespace {

constexpr char const* TIFF_NO_TAG = "Invalid TIFF: Required tag not found";
constexpr char const* TIFF_TILES_NOT_SUPPORTED = "Tiled TIFF format not yet supported";
constexpr char const* TIFF_DIRECTORY_NOT_FOUND = "Could not find the requested image in the file";

#define READ_REQUIRED_TIFF_TAG( tiff, tag, ... ) do { if( !TIFFGetField( tiff, tag, __VA_ARGS__ )) { DIP_THROW_RUNTIME( TIFF_NO_TAG ); }} while(false)

class TiffFile {
   public:
      explicit TiffFile( String filename ) : filename_( std::move( filename )) {
         // Set error and warning handlers, these are library-wide!
         TIFFSetErrorHandler( nullptr );
         TIFFSetWarningHandler( nullptr );
         // Open the file for reading
         tiff_ = TIFFOpen( filename_.c_str(), "rc" ); // c == Disable the use of strip chopping when reading images.
         if( tiff_ == nullptr ) {
            if( !FileHasExtension( filename_ )) {
               filename_ = FileAddExtension( filename_, "tif" ); // Try with "tif" extension
               tiff_ = TIFFOpen( filename_.c_str(), "rc" );
               if( tiff_ == nullptr ) {
                  filename_ = filename_ + 'f'; // Try with "tiff" extension
                  tiff_ = TIFFOpen( filename_.c_str(), "rc" );
               }
            }
         }
         if( tiff_ == nullptr ) {
            DIP_THROW_RUNTIME( "Could not open the specified TIFF file" );
         }
      }
      TiffFile( TiffFile const& ) = delete;
      TiffFile( TiffFile&& ) = delete;
      TiffFile& operator=( TiffFile const& ) = delete;
      TiffFile& operator=( TiffFile&& ) = delete;
      ~TiffFile() {
         if( tiff_ ) {
            TIFFClose( tiff_ );
            tiff_ = nullptr;
         }
      }
      // Implicit cast to TIFF*
      operator TIFF*() { return tiff_; }
      // Retrieve file name
      String const& FileName() const { return filename_; }
   private:
      TIFF* tiff_ = nullptr;
      String filename_;
};

DataType FindTIFFDataType( TiffFile& tiff ) {
   uint16 bitsPerSample;
   if( !TIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample )) {
      bitsPerSample = 1; // Binary images don't carry this tag
   }
   uint16 sampleFormat;
   if( !TIFFGetField( tiff, TIFFTAG_SAMPLEFORMAT, &sampleFormat )) {
      sampleFormat = SAMPLEFORMAT_UINT;
   }
   switch( sampleFormat ) {
      case SAMPLEFORMAT_UINT:
         switch( bitsPerSample ) {
            case 1:
               return DT_BIN;
            case 8:
               return DT_UINT8;
            case 16:
               return DT_UINT16;
            case 32:
               return DT_UINT32;
            default:
               DIP_THROW_RUNTIME( "Unsupported TIFF: Unknown bit depth" );
               break;
         }
      case SAMPLEFORMAT_INT:
         switch( bitsPerSample ) {
            case 8:
               return DT_SINT8;
            case 16:
               return DT_SINT16;
            case 32:
               return DT_SINT32;
            default:
               DIP_THROW_RUNTIME( "Unsupported TIFF: Unknown bit depth" );
         }
      case SAMPLEFORMAT_IEEEFP:
         switch( bitsPerSample ) {
            case 8: // I have a TIFF file that says to be 8-bit IEEEFP, but is not.
               return DT_UINT8;
            case 32:
               return DT_SFLOAT;
            case 64:
               return DT_DFLOAT;
            default:
               DIP_THROW_RUNTIME( "Unsupported TIFF: Unknown bit depth" );
         }
      default:
         DIP_THROW_RUNTIME( "Unsupported TIFF: Unknown pixel format" );
   }
}

struct GetTIFFInfoData {
   FileInformation fileInformation;
   uint16 photometricInterpretation;
};

GetTIFFInfoData GetTIFFInfo( TiffFile& tiff ) {
   GetTIFFInfoData data;

   data.fileInformation.name = tiff.FileName();
   data.fileInformation.fileType = "TIFF";

   // Image sizes
   uint32 imageWidth, imageLength;
   READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_IMAGEWIDTH, &imageWidth );
   READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_IMAGELENGTH, &imageLength );
   data.fileInformation.sizes = { imageWidth, imageLength };
   uint16 samplesPerPixel;
   if( !TIFFGetField( tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel )) {
      samplesPerPixel = 1;
   }
   data.fileInformation.tensorElements = samplesPerPixel;

   // Photometric interpretation
   if( !TIFFGetField( tiff, TIFFTAG_PHOTOMETRIC, &data.photometricInterpretation )) {
      data.photometricInterpretation = PHOTOMETRIC_MINISBLACK;
   }
   switch( data.photometricInterpretation ) {
      case PHOTOMETRIC_YCBCR:
         DIP_THROW_RUNTIME( "Unsupported TIFF: Class Y image (YCbCr)" );
         // Mostly because these often have sub-sampled Cb and Cr, and sometimes they don't even bother setting the tags for it.
      case PHOTOMETRIC_LOGLUV:
      case PHOTOMETRIC_LOGL:
         DIP_THROW_RUNTIME( "Unsupported TIFF: Log-compressed image (LogLuv or LogL)" );
      case PHOTOMETRIC_PALETTE:
         data.fileInformation.colorSpace = "RGB";
         data.fileInformation.tensorElements = 3;
         data.fileInformation.dataType = DT_UINT16;
         break;
      case PHOTOMETRIC_RGB:
         if( samplesPerPixel == 3 ) {
            data.fileInformation.colorSpace = "RGB";
         }
         break;
      case PHOTOMETRIC_SEPARATED:
         // Assume CMYK, but we should really be reading the ink values...
         switch( samplesPerPixel ) {
            case 3:
               data.fileInformation.colorSpace = "CMY";
               break;
            case 4:
               data.fileInformation.colorSpace = "CMYK";
               break;
            default:
               break;
         }
         break;
      case PHOTOMETRIC_CIELAB:
      case PHOTOMETRIC_ICCLAB:
      case PHOTOMETRIC_ITULAB:
         data.fileInformation.colorSpace = "Lab";
         break;
      case PHOTOMETRIC_MINISWHITE:
      case PHOTOMETRIC_MINISBLACK:
      case PHOTOMETRIC_MASK:
      default:
         break;
   }

   // Data type
   if( data.photometricInterpretation != PHOTOMETRIC_PALETTE ) {
      data.fileInformation.dataType = FindTIFFDataType( tiff );
   }
   if( data.fileInformation.dataType == DT_BIN ) {
      data.fileInformation.significantBits = 1;
   } else {
      data.fileInformation.significantBits = data.fileInformation.dataType.SizeOf() * 8;
   }

   // Physical dimensions
   uint16 resolutionUnit;
   if( !TIFFGetField( tiff, TIFFTAG_RESOLUTIONUNIT, &resolutionUnit )) {
      resolutionUnit = 0;
   }
   PhysicalQuantity pixelSizeMultiplier = 1;
   switch( resolutionUnit ) {
      case RESUNIT_NONE:
         break;
      case RESUNIT_INCH:
      default:
         pixelSizeMultiplier = 0.0254 * Units::Meter();
         break;
      case RESUNIT_CENTIMETER:
         pixelSizeMultiplier = 0.01 * Units::Meter();
         break;
   }
   float resolution;
   PhysicalQuantity ps = 1;
   if( TIFFGetField( tiff, TIFFTAG_XRESOLUTION, &resolution )) {
      ps = ( 1.0 / static_cast< double >( resolution )) * pixelSizeMultiplier;
      ps.Normalize();
   }
   data.fileInformation.pixelSize.Set( 0, ps );
   ps = 1;
   if( TIFFGetField( tiff, TIFFTAG_YRESOLUTION, &resolution )) {
      ps = ( 1.0 / static_cast< double >( resolution )) * pixelSizeMultiplier;
      ps.Normalize();
   }
   data.fileInformation.pixelSize.Set( 1, ps );

   // Number of images in file
   data.fileInformation.numberOfImages = TIFFNumberOfDirectories( tiff );

   return data;
}

//
// Color Map
//

inline void ExpandColourMap4(
      uint16* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      dip::sint tensorStride,
      IntegerArray const& strides,
      uint16 const* ColourMapRed,
      uint16 const* ColourMapGreen,
      uint16 const* ColourMapBlue
) {
   dip::sint green = tensorStride;
   dip::sint blue = 2 * tensorStride;
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint16* dest_pixel = dest;
      for( dip::uint jj = 0; jj < width; ) {
         dip::uint index = ( static_cast< dip::uint >( *src ) >> 4 ) & 0x0Fu;
         *dest_pixel = ColourMapRed[ index ];
         *( dest_pixel + green ) = ColourMapGreen[ index ];
         *( dest_pixel + blue ) = ColourMapBlue[ index ];
         dest_pixel += strides[ 0 ];
         ++jj;
         if( jj >= width ) {
            ++src;
            break;
         }
         index = ( *src ) & 0x0Fu;
         *dest_pixel = ColourMapRed[ index ];
         *( dest_pixel + green ) = ColourMapGreen[ index ];
         *( dest_pixel + blue ) = ColourMapBlue[ index ];
         dest_pixel += strides[ 0 ];
         ++jj;
         ++src;
      }
      dest += strides[ 1 ];
   }
}

inline void ExpandColourMap8(
      uint16* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      dip::sint tensorStride,
      IntegerArray const& strides,
      uint16 const* ColourMapRed,
      uint16 const* ColourMapGreen,
      uint16 const* ColourMapBlue
) {
   dip::sint green = tensorStride;
   dip::sint blue = 2 * tensorStride;
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint16* dest_pixel = dest;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         *dest_pixel = ColourMapRed[ *src ];
         *( dest_pixel + green ) = ColourMapGreen[ *src ];
         *( dest_pixel + blue ) = ColourMapBlue[ *src ];
         dest_pixel += strides[ 0 ];
         ++src;
      }
      dest += strides[ 1 ];
   }
}

void ReadTIFFColorMap(
      Image& image,
      TiffFile& tiff,
      GetTIFFInfoData& data
) {
   // Test for tiled TIFF files. These we can't handle (yet).
   uint32 tileWidth;
   if( TIFFGetField( tiff, TIFFTAG_TILEWIDTH, &tileWidth )) {
      DIP_THROW_RUNTIME( TIFF_TILES_NOT_SUPPORTED );
   }

   // Read the tags
   uint16 bitsPerSample;
   READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample );
   if(( bitsPerSample != 4 ) && ( bitsPerSample != 8 )) {
      DIP_THROW_RUNTIME( "Unsupported TIFF: Unknown bit depth" );
   }
   //std::vector< uint16 > CMRed( 1u << bitsPerSample );
   //std::vector< uint16 > CMGreen( 1u << bitsPerSample );
   //std::vector< uint16 > CMBlue( 1u << bitsPerSample );
   uint16* CMRed;
   uint16* CMGreen;
   uint16* CMBlue;
   READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_COLORMAP, &CMRed, &CMGreen, &CMBlue );

   // Forge the image
   image.ReForge( data.fileInformation.sizes, 3, DT_UINT16 );
   uint16* imagedata = static_cast< uint16* >( image.Origin() );

   // Read the image data stripwise
   uint32 imageWidth = static_cast< uint32 >( image.Size( 0 ));
   uint32 imageLength = static_cast< uint32 >( image.Size( 1 ));
   dip::uint scanline = static_cast< dip::uint >( TIFFScanlineSize( tiff ));
   if( bitsPerSample == 4 ) {
      DIP_ASSERT( scanline == div_ceil< dip::uint >( image.Size( 0 ), 2 ));
   } else {
      DIP_ASSERT( scanline == image.Size( 0 ));
   }
   std::vector< uint8 > buf( static_cast< dip::uint >( TIFFStripSize( tiff )));
   uint32 rowsPerStrip;
   TIFFGetFieldDefaulted( tiff, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip );
   uint32 nStrips = TIFFNumberOfStrips( tiff );
   uint32 row = 0;
   for( uint32 strip = 0; strip < nStrips; ++strip ) {
      dip::uint nrow = row + rowsPerStrip > imageLength ? imageLength - row : rowsPerStrip;
      if( TIFFReadEncodedStrip( tiff, strip, buf.data(), static_cast< tmsize_t >( nrow * scanline )) < 0 ) {
         DIP_THROW_RUNTIME( "Error reading data" );
      }
      if( bitsPerSample == 4 ) {
         ExpandColourMap4( imagedata, buf.data(), imageWidth, nrow, image.TensorStride(), image.Strides(), CMRed, CMGreen, CMBlue );
      } else {
         ExpandColourMap8( imagedata, buf.data(), imageWidth, nrow, image.TensorStride(), image.Strides(), CMRed, CMGreen, CMBlue );
      }
      imagedata += static_cast< dip::sint >( nrow ) * image.Stride( 1 );
      row += rowsPerStrip;
   }
}

//
// Binary
//

inline void CopyBuffer1(
      uint8* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      IntegerArray const& strides
) {
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8* dest_pixel = dest;
      dip::sint kk = 7;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         *dest_pixel = (( *src ) & ( 1 << kk )) ? 1 : 0;
         dest_pixel += strides[ 0 ];
         --kk;
         if( kk < 0 ) {
            kk = 7;
            ++src;
         }
      }
      if( kk != 7 ) {
         //kk = 7;
         ++src;
      }
      dest += strides[ 1 ];
   }
}

inline void CopyBufferInv1(
      uint8* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      IntegerArray const& strides
) {
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8* dest_pixel = dest;
      dip::sint kk = 7;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         *dest_pixel = (( *src ) & ( 1 << kk )) ? 0 : 1;
         dest_pixel += strides[ 0 ];
         --kk;
         if( kk < 0 ) {
            kk = 7;
            ++src;
         }
      }
      if( kk != 7 ) {
         //kk = 7;
         ++src;
      }
      dest += strides[ 1 ];
   }
}

void ReadTIFFBinary(
      Image& image,
      TiffFile& tiff,
      GetTIFFInfoData& data
) {
   // Test for tiled TIFF files. These we can't handle (yet).
   uint32 tileWidth;
   if( TIFFGetField( tiff, TIFFTAG_TILEWIDTH, &tileWidth )) {
      DIP_THROW_RUNTIME( TIFF_TILES_NOT_SUPPORTED );
   }

   // Forge the image
   image.ReForge( data.fileInformation.sizes, data.fileInformation.tensorElements, DT_BIN );
   uint8* imagedata = static_cast< uint8* >( image.Origin() );

   // Read the image data stripwise
   uint32 imageWidth = static_cast< uint32 >( image.Size( 0 ));
   uint32 imageLength = static_cast< uint32 >( image.Size( 1 ));
   dip::uint scanline = static_cast< dip::uint >( TIFFScanlineSize( tiff ));
   DIP_ASSERT( scanline == div_ceil< dip::uint >( image.Size( 0 ), 8 ));
   std::vector< uint8 > buf( static_cast< dip::uint >( TIFFStripSize( tiff )));
   uint32 rowsPerStrip;
   TIFFGetFieldDefaulted( tiff, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip );
   uint32 nStrips = TIFFNumberOfStrips( tiff );
   uint32 row = 0;
   for( uint32 strip = 0; strip < nStrips; ++strip ) {
      uint32 nrow = ( row + rowsPerStrip > imageLength ? imageLength - row : rowsPerStrip );
      if( TIFFReadEncodedStrip( tiff, strip, buf.data(), static_cast< tmsize_t >( nrow * scanline )) < 0 ) {
         DIP_THROW_RUNTIME( "Error reading data" );
      }
      if( data.photometricInterpretation == PHOTOMETRIC_MINISWHITE ) {
         CopyBufferInv1( imagedata, buf.data(), imageWidth, nrow, image.Strides() );
      } else {
         CopyBuffer1( imagedata, buf.data(), imageWidth, nrow, image.Strides() );
      }
      imagedata += static_cast< dip::sint >( nrow ) * image.Stride( 1 );
      row += rowsPerStrip;
   }
}

//
// Grey-value (including multi-channel, color, etc)
//

inline void CopyBuffer8(
      uint8* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      IntegerArray const& destStrides,
      dip::sint srcStride // src has x-stride = 1, y-stride = srcStride.
) {
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8* dest_pixel = dest;
      uint8 const* src_sample = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         *dest_pixel = *src_sample;
         dest_pixel += destStrides[ 0 ];
         ++src_sample;
      }
      dest += destStrides[ 1 ];
      src += srcStride;
   }
}

inline void CopyBufferN(
      uint8* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      IntegerArray const& destStrides,
      dip::sint srcStride, // src has x-stride = 1, y-stride = srcStride.
      dip::uint sizeOf
) {
   dip::sint stride_row = destStrides[ 1 ] * static_cast< dip::sint >( sizeOf );
   dip::sint stride_pixel = destStrides[ 0 ] * static_cast< dip::sint >( sizeOf );
   dip::sint stride_src = srcStride * static_cast< dip::sint >( sizeOf );
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8* dest_pixel = dest;
      uint8 const* src_sample = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         memcpy( dest_pixel, src_sample, sizeOf );
         dest_pixel += stride_pixel;
         src_sample += sizeOf;
      }
      dest += stride_row;
      src += stride_src;
   }
}

inline void CopyBufferMultiChannel8(
      uint8* dest,
      uint8 const* src,
      dip::uint tensorElements,
      dip::uint width,
      dip::uint height,
      dip::sint tensorStride, // for dest
      IntegerArray const& destStrides,
      dip::sint srcStride // src has tensor-stride = 1, x-stride = tensorElements, y-stride = srcStride.
) {
   //std::cout << "[CopyBufferMultiChannel8] tensorElements = " << tensorElements << ", width = " << width << ", height = " << height << std::endl;
   //std::cout << "[CopyBufferMultiChannel8] tensorStride = " << tensorStride << ", destStrides[0] = " << destStrides[0] << ", destStrides[1] = " << destStrides[1] << std::endl;
   //std::cout << "[CopyBufferMultiChannel8] srcStride = " << srcStride << std::endl;
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8* dest_pixel = dest;
      uint8 const* src_sample = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         uint8* dest_sample = dest_pixel;
         for( dip::uint kk = 0; kk < tensorElements; ++kk ) {
            *dest_sample = *src_sample;
            dest_sample += tensorStride;
            ++src_sample;
         }
         dest_pixel += destStrides[ 0 ];
      }
      dest += destStrides[ 1 ];
      src += srcStride;
   }
}

inline void CopyBufferMultiChannelN(
      uint8* dest,
      uint8 const* src,
      dip::uint tensorElements,
      dip::uint width,
      dip::uint height,
      dip::sint tensorStride, // for dest
      IntegerArray const& destStrides,
      dip::sint srcStride, // src has tensor-stride = 1, x-stride = tensorElements, y-stride = srcStride.
      dip::uint sizeOf
) {
   dip::sint stride_row = destStrides[ 1 ] * static_cast< dip::sint >( sizeOf );
   dip::sint stride_pixel = destStrides[ 0 ] * static_cast< dip::sint >( sizeOf );
   dip::sint stride_sample = tensorStride * static_cast< dip::sint >( sizeOf );
   dip::sint stride_src = srcStride * static_cast< dip::sint >( sizeOf );
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8* dest_pixel = dest;
      uint8 const* src_sample = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         uint8* dest_sample = dest_pixel;
         for( dip::uint kk = 0; kk < tensorElements; ++kk ) {
            memcpy( dest_sample, src_sample, sizeOf );
            dest_sample += stride_sample;
            src_sample += sizeOf;
         }
         dest_pixel += stride_pixel;
      }
      dest += stride_row;
      src += stride_src;
   }
}

inline bool StridesAreNormal(
      dip::uint tensorElements,
      dip::sint tensorStride,
      UnsignedArray const& sizes,
      IntegerArray const& strides
) {
   if( tensorStride != 1 ) {
      return false;
   }
   dip::sint total = static_cast< dip::sint >( tensorElements );
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      if( strides[ ii ] != total ) {
         return false;
      }
      total *= static_cast< dip::sint >( sizes[ ii ] );
   }
   return true;
}

void ReadTIFFData(
      uint8* imagedata,
      UnsignedArray const& sizes,
      IntegerArray const& strides,
      dip::uint tensorElements,
      dip::sint tensorStride,
      DataType dataType,
      TiffFile& tiff
) {
   dip::uint sizeOf = dataType.SizeOf();

   // Planar configuration?
   uint16 planarConfiguration = PLANARCONFIG_SEPARATE;
   if( tensorElements > 1 ) {
      if( !TIFFGetField( tiff, TIFFTAG_PLANARCONFIG, &planarConfiguration )) {
         planarConfiguration = PLANARCONFIG_CONTIG; // Default
      }
   }

   // Strips or tiles?
   uint32 tileWidth;
   if( TIFFGetField( tiff, TIFFTAG_TILEWIDTH, &tileWidth )) {
      // --- Tiled TIFF file ---
      uint32 tileLength;
      READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_TILELENGTH, &tileLength );
      auto tileSize = TIFFTileSize( tiff );
      //std::cout << "[ReadTIFFData] tileSize = " << tileSize << std::endl;
      //std::cout << "[ReadTIFFData] tileWidth = " << tileWidth << ", tileLength = " << tileLength << std::endl;
      //std::cout << "[ReadTIFFData] tensorElements = " << tensorElements << ", sizeOf = " << sizeOf << std::endl;
      std::vector< uint8 > buf( static_cast< dip::uint >( tileSize ));
      //uint32 nTiles = TIFFNumberOfTiles( tiff );
      if( planarConfiguration == PLANARCONFIG_CONTIG ) {
         // 1234123412341234....
         // We know that tensorElements > 1, otherwise we force to PLANARCONFIG_SEPARATE
         DIP_ASSERT( static_cast< dip::uint >( tileSize ) == tileWidth * tileLength * tensorElements * sizeOf );
         dip::sint tileStride = static_cast< dip::sint >( tensorElements * tileWidth );
         uint32 tile = 0;
         for( dip::uint y = 0; y < sizes[ 1 ]; y += tileLength ) {
            uint8* imagedataRow = imagedata;
            dip::uint copyHeight = std::min< dip::uint >( sizes[ 1 ] - y, tileLength );
            for( dip::uint x = 0; x < sizes[ 0 ]; x += tileWidth ) {
               dip::uint copyWidth = std::min< dip::uint >( sizes[ 0 ] - x, tileWidth );
               TIFFReadEncodedTile( tiff, tile, buf.data(), tileSize );
               if( sizeOf == 1 ) {
                  CopyBufferMultiChannel8( imagedataRow, buf.data(), tensorElements, copyWidth, copyHeight,
                                           tensorStride, strides, tileStride );
                  imagedataRow += static_cast< dip::sint >( tileWidth ) * strides[ 0 ];
               } else {
                  CopyBufferMultiChannelN( imagedataRow, buf.data(), tensorElements, copyWidth, copyHeight,
                                           tensorStride, strides, tileStride, sizeOf );
                  imagedataRow += static_cast< dip::sint >( tileWidth * sizeOf ) * strides[ 0 ];
               }
               ++tile;
            }
            imagedata += static_cast< dip::sint >( tileLength * sizeOf ) * strides[ 1 ];
         }
      } else if( planarConfiguration == PLANARCONFIG_SEPARATE ) {
         // 1111...2222...3333...4444...
         DIP_ASSERT( static_cast< dip::uint >( tileSize ) == tileWidth * tileLength * sizeOf );
         dip::sint tileStride = tileWidth;
         uint8* imagebase = imagedata;
         uint32 tile = 0;
         for( uint16 plane = 0; plane < tensorElements; ++plane ) {
            imagedata = imagebase + static_cast< dip::sint >( plane * sizeOf ) * tensorStride;
            for( dip::uint y = 0; y < sizes[ 1 ]; y += tileLength ) {
               uint8* imagedataRow = imagedata;
               dip::uint copyHeight = std::min< dip::uint >( sizes[ 1 ] - y, tileLength );
               for( dip::uint x = 0; x < sizes[ 0 ]; x += tileWidth ) {
                  dip::uint copyWidth = std::min< dip::uint >( sizes[ 0 ] - x, tileWidth );
                  TIFFReadEncodedTile( tiff, tile, buf.data(), tileSize );
                  if( sizeOf == 1 ) {
                     CopyBuffer8( imagedataRow, buf.data(), copyWidth, copyHeight, strides, tileStride );
                     imagedataRow += static_cast< dip::sint >( tileWidth ) * strides[ 0 ];
                  } else {
                     CopyBufferN( imagedataRow, buf.data(), copyWidth, copyHeight, strides, tileStride, sizeOf );
                     imagedataRow += static_cast< dip::sint >( tileWidth * sizeOf ) * strides[ 0 ];
                  }
                  ++tile;
               }
               imagedata += static_cast< dip::sint >( tileLength * sizeOf ) * strides[ 1 ];
            }
         }
      } else {
         DIP_THROW_RUNTIME( "Unsupported TIFF: unknown PlanarConfiguration value" );
      }
   } else {
      // --- Striped TIFF file ---
      uint32 rowsPerStrip;
      TIFFGetFieldDefaulted( tiff, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip );
      tsize_t stripSize = TIFFStripSize( tiff );
      uint32 nStrips = TIFFNumberOfStrips( tiff );
      if( planarConfiguration == PLANARCONFIG_CONTIG ) {
         // 1234123412341234....
         // We know that tensorElements > 1, otherwise we force to PLANARCONFIG_SEPARATE
         DIP_ASSERT( static_cast< dip::uint >( TIFFScanlineSize( tiff )) == sizes[ 0 ] * tensorElements * sizeOf );
         if( StridesAreNormal( tensorElements, tensorStride, sizes, strides )) {
            uint32 row = 0;
            for( uint32 strip = 0; strip < nStrips; ++strip ) {
               dip::uint nrow = ( row + rowsPerStrip > sizes[ 1 ] ? sizes[ 1 ] - row : rowsPerStrip );
               if( TIFFReadEncodedStrip( tiff, strip, imagedata, stripSize ) < 0 ) {
                  DIP_THROW_RUNTIME( "Error reading data (planar config cont)" );
               }
               imagedata += static_cast< dip::sint >( nrow * sizeOf ) * strides[ 1 ];
               row += rowsPerStrip;
            }
         } else {
            std::vector< uint8 > buf( static_cast< dip::uint >( stripSize ));
            dip::sint bufferStride = static_cast< dip::sint >( tensorElements * sizes[ 0 ] );
            uint32 row = 0;
            for( uint32 strip = 0; strip < nStrips; ++strip ) {
               dip::uint nrow = ( row + rowsPerStrip > sizes[ 1 ] ? sizes[ 1 ] - row : rowsPerStrip );
               if( TIFFReadEncodedStrip( tiff, strip, buf.data(), stripSize ) < 0 ) {
                  DIP_THROW_RUNTIME( "Error reading data (planar config cont)" );
               }
               if( sizeOf == 1 ) {
                  CopyBufferMultiChannel8( imagedata, buf.data(), tensorElements, sizes[ 0 ], nrow,
                                          tensorStride, strides, bufferStride );
                  imagedata += static_cast< dip::sint >( nrow ) * strides[ 1 ];
               } else {
                  CopyBufferMultiChannelN( imagedata, buf.data(), tensorElements, sizes[ 0 ], nrow,
                                          tensorStride, strides, bufferStride, sizeOf );
                  imagedata += static_cast< dip::sint >( nrow * sizeOf ) * strides[ 1 ];
               }
               row += rowsPerStrip;
            }
         }
      } else if( planarConfiguration == PLANARCONFIG_SEPARATE ) {
         // 1111...2222...3333...4444...
         DIP_ASSERT( static_cast< dip::uint >( TIFFScanlineSize( tiff )) == sizes[ 0 ] * sizeOf );
         DIP_ASSERT( nStrips % tensorElements == 0 );
         nStrips /= static_cast< uint32 >( tensorElements );
         uint8* imagebase = imagedata;
         uint32 stripOffset = 0;
         if( StridesAreNormal( 1, 1, sizes, strides )) {
            for( uint16 plane = 0; plane < tensorElements; ++plane ) {
               imagedata = imagebase + static_cast< dip::sint >( plane * sizeOf ) * tensorStride;
               uint32 row = 0;
               for( uint32 strip = 0; strip < nStrips; ++strip ) {
                  dip::uint nrow = ( row + rowsPerStrip > sizes[ 1 ] ? sizes[ 1 ] - row : rowsPerStrip );
                  if( TIFFReadEncodedStrip( tiff, stripOffset + strip, imagedata, stripSize ) < 0 ) {
                     DIP_THROW_RUNTIME( "Error reading data (planar config separate)" );
                  }
                  imagedata += static_cast< dip::sint >( nrow * sizeOf ) * strides[ 1 ];
                  row += rowsPerStrip;
               }
               stripOffset += nStrips;
            }
         } else {
            std::vector< uint8 > buf( static_cast< dip::uint >( stripSize ));
            dip::sint bufferStride = static_cast< dip::sint >( sizes[ 0 ] );
            for( uint16 plane = 0; plane < tensorElements; ++plane ) {
               imagedata = imagebase + static_cast< dip::sint >( plane * sizeOf ) * tensorStride;
               uint32 row = 0;
               for( uint32 strip = 0; strip < nStrips; ++strip ) {
                  dip::uint nrow = ( row + rowsPerStrip > sizes[ 1 ] ? sizes[ 1 ] - row : rowsPerStrip );
                  if( TIFFReadEncodedStrip( tiff, stripOffset + strip, buf.data(), stripSize ) < 0 ) {
                     DIP_THROW_RUNTIME( "Error reading data (planar config separate)" );
                  }
                  if( sizeOf == 1 ) {
                     CopyBuffer8( imagedata, buf.data(), sizes[ 0 ], nrow, strides, bufferStride );
                     imagedata += static_cast< dip::sint >( nrow ) * strides[ 1 ];
                  } else {
                     CopyBufferN( imagedata, buf.data(), sizes[ 0 ], nrow, strides, bufferStride, sizeOf );
                     imagedata += static_cast< dip::sint >( nrow * sizeOf ) * strides[ 1 ];
                  }
                  row += rowsPerStrip;
               }
               stripOffset += nStrips;
            }
         }
      } else {
         DIP_THROW_RUNTIME( "Unsupported TIFF: unknown PlanarConfiguration value" );
      }
   }
}

void ReadTIFFGreyValue(
      Image& image,
      TiffFile& tiff,
      GetTIFFInfoData& data
) {
   // Forge the image
   image.ReForge( data.fileInformation.sizes, data.fileInformation.tensorElements, data.fileInformation.dataType );
   uint8* imagedata = static_cast< uint8* >( image.Origin() );

   // Read the image data
   DIP_STACK_TRACE_THIS( ReadTIFFData(
         imagedata, image.Sizes(), image.Strides(), image.TensorElements(), image.TensorStride(),
         image.DataType(), tiff ));

   if( data.photometricInterpretation == PHOTOMETRIC_MINISWHITE ) {
      Invert( image, image );
   }
}

void ImageReadTIFFStack(
      Image& image,
      TiffFile& tiff,
      GetTIFFInfoData& data,
      Range const& imageNumbers
) {
   // Forge the image
   data.fileInformation.sizes.push_back( imageNumbers.Size() );
   image.ReForge( data.fileInformation.sizes, data.fileInformation.tensorElements, data.fileInformation.dataType );
   uint8* imagedata = static_cast< uint8* >( image.Origin() );
   dip::sint z_stride = image.Stride( 2 ) * static_cast< dip::sint >( data.fileInformation.dataType.SizeOf() );

   // Read the image data for first plane
   DIP_STACK_TRACE_THIS( ReadTIFFData(
         imagedata, image.Sizes(), image.Strides(), image.TensorElements(), image.TensorStride(),
         image.DataType(), tiff ));

   // Read the image data for other planes
   dip::uint directory = imageNumbers.Offset();
   for( dip::uint ii = 1; ii < image.Size( 2 ); ++ii ) {
      imagedata += z_stride;
      if( imageNumbers.start > imageNumbers.stop ) {
         directory -= imageNumbers.step;
      } else {
         directory += imageNumbers.step;
      }
      if( TIFFSetDirectory( tiff, static_cast< uint16 >( directory )) == 0 ) {
         DIP_THROW_RUNTIME( TIFF_DIRECTORY_NOT_FOUND );
      }

      // Test image plane to make sure it matches expectations
      uint32 temp32;
      READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_IMAGEWIDTH, &temp32 );
      if( temp32 != image.Size( 0 )) {
         DIP_THROW_RUNTIME( "Reading multi-slice TIFF: width of images not consistent" );
      }
      READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_IMAGELENGTH, &temp32 );
      if( temp32 != image.Size( 1 )) {
         DIP_THROW_RUNTIME( "Reading multi-slice TIFF: length of images not consistent" );
      }
      uint16 photometricInterpretation;
      if( !TIFFGetField( tiff, TIFFTAG_PHOTOMETRIC, &photometricInterpretation )) {
         photometricInterpretation = PHOTOMETRIC_MINISBLACK;
      }
      DataType dataType;
      uint16 samplesPerPixel;
      if( photometricInterpretation == PHOTOMETRIC_PALETTE ) {
         dataType = DT_UINT16;
         samplesPerPixel = 3;
      } else {
         DIP_STACK_TRACE_THIS( dataType = FindTIFFDataType( tiff ));
         if( !TIFFGetField( tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel )) {
            samplesPerPixel = 1;
         }
      }
      if( dataType != image.DataType() ) {
         DIP_THROW_RUNTIME( "Reading multi-slice TIFF: data type not consistent" );
      }
      if( samplesPerPixel != image.TensorElements() ) {
         DIP_THROW_RUNTIME( "Reading multi-slice TIFF: samples per pixel not consistent" );
      }

      // Read the image data for this plane
      DIP_STACK_TRACE_THIS( ReadTIFFData(
            imagedata, image.Sizes(), image.Strides(), image.TensorElements(), image.TensorStride(),
            image.DataType(), tiff ));
   }
}

} // namespace

FileInformation ImageReadTIFF(
      Image& out,
      String const& filename,
      Range imageNumbers
) {
   // Open TIFF file
   TiffFile tiff( filename );

   // Go to the right directory
   dip::uint numberOfImages = TIFFNumberOfDirectories( tiff );
   DIP_STACK_TRACE_THIS( imageNumbers.Fix( numberOfImages ));
   uint16 imageNumber = static_cast< uint16 >( imageNumbers.Offset() );
   if( TIFFSetDirectory( tiff, imageNumber ) == 0 ) {
      DIP_THROW_RUNTIME( TIFF_DIRECTORY_NOT_FOUND );
   }

   // Get info
   GetTIFFInfoData data;
   DIP_STACK_TRACE_THIS( data = GetTIFFInfo( tiff ));

   if( imageNumbers.start != imageNumbers.stop ) {
      // Read in multiple pages as a 3D image
      DIP_STACK_TRACE_THIS( ImageReadTIFFStack( out, tiff, data, imageNumbers ));
   } else {
      // Hack by Bernd Rieger to recognize Leica 12 bit TIFFs
      // These are written as color-mapped images, but they are not
      if( data.photometricInterpretation == PHOTOMETRIC_PALETTE ) {
         uint16 bitsPerSample;
         String artist( 128, ' ' );
         if(( TIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample )) &&
            ( TIFFGetField( tiff, TIFFTAG_ARTIST, &( artist[ 0 ] )))) {
            if(( artist == "Yves Nicodem" ) || ( artist == "TCS User" )) {
               data.fileInformation.colorSpace = "";
               data.photometricInterpretation = PHOTOMETRIC_MINISBLACK;
            }
         }
      }
      if( data.photometricInterpretation == PHOTOMETRIC_PALETTE ) {
         DIP_STACK_TRACE_THIS( ReadTIFFColorMap( out, tiff, data ));
      } else {
         if( data.fileInformation.dataType.IsBinary() ) {
            DIP_STACK_TRACE_THIS( ReadTIFFBinary( out, tiff, data ));
         } else {
            DIP_STACK_TRACE_THIS( ReadTIFFGreyValue( out, tiff, data ));
         }
      }
   }

   out.SetColorSpace( data.fileInformation.colorSpace );
   out.SetPixelSize( data.fileInformation.pixelSize );

   return data.fileInformation;
}

void ImageReadTIFFSeries(
      Image& out,
      StringArray const& filenames
) {
   DIP_THROW_IF( filenames.size() < 1, E::ARRAY_ILLEGAL_SIZE );

   // Read in first image
   Image tmp;
   dip::uint ii = 0;
   DIP_STACK_TRACE_THIS( ImageReadTIFF( tmp, filenames[ ii ] )); // TODO: Read in first image plane or all image planes?

   // Prepare the output image
   UnsignedArray sizes = tmp.Sizes();
   sizes.push_back( filenames.size() );
   out.ReForge( sizes, tmp.TensorElements(), tmp.DataType() );

   // Iterate over the sub-images through the last dimension
   ImageSliceIterator it( out, out.Dimensionality() - 1 );

   // Write the first image into the output
   it->Copy( tmp );
   // Make sure we copy over the color space information also
   if( tmp.IsColor() ) {
      out.SetColorSpace( tmp.ColorSpace() );
   }

   // Read in the rest of the images, and write them into the output
   while( ++ii, ++it ) {
      DIP_STACK_TRACE_THIS( ImageReadTIFF( tmp, filenames[ ii ] ));
      try {
         it->Copy( tmp );
      } catch( Error const& ) {
         DIP_THROW_RUNTIME( "Images in series do not have consistent sizes" );
      }
   }
}

FileInformation ImageReadTIFFInfo(
      String const& filename,
      dip::uint imageNumber
) {
   // Open TIFF file
   TiffFile tiff( filename );

   // Go to the right directory
   if( imageNumber > 0 ) {
      if( TIFFSetDirectory( tiff, static_cast< uint16 >( imageNumber )) == 0 ) {
         DIP_THROW_RUNTIME( TIFF_DIRECTORY_NOT_FOUND );
      }
   }

   // Get info
   GetTIFFInfoData data;
   DIP_STACK_TRACE_THIS( data = GetTIFFInfo( tiff ));

   return data.fileInformation;
}

bool ImageIsTIFF(
      String const& filename
) {
   TIFF* tiff = TIFFOpen( filename.c_str(), "r" );
   if( tiff != nullptr ) {
      TIFFClose( tiff );
      return true;
   }
   return false;
}

} // namespace dip

#else // DIP__HAS_TIFF

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

FileInformation ImageReadTIFF( Image&, String const&, Range ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

void ImageReadTIFFSeries( Image&, StringArray const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

FileInformation ImageReadTIFFInfo( String const&, dip::uint ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

bool ImageIsTIFF( String const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

}

#endif // DIP__HAS_TIFF
