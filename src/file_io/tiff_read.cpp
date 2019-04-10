/*
 * DIPlib 3.0
 * This file contains definitions for TIFF reading
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

#ifdef DIP__HAS_TIFF

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/generic_iterators.h"

#include "file_io_support.h"

#include <tiffio.h>

namespace dip {

namespace {

constexpr char const* TIFF_NO_TAG = "Invalid TIFF: Required tag not found";
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
            case 64:
               return DT_UINT64;
            default:
               DIP_THROW_RUNTIME( "Unsupported TIFF: Unknown bit depth" );
         }
      case SAMPLEFORMAT_INT:
         switch( bitsPerSample ) {
            case 8:
               return DT_SINT8;
            case 16:
               return DT_SINT16;
            case 32:
               return DT_SINT32;
            case 64:
               return DT_SINT64;
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
   
   // Origin offset
   float position;
   PhysicalQuantity xPos = 0 * pixelSizeMultiplier, yPos = 0 * pixelSizeMultiplier;
   if( TIFFGetField( tiff, TIFFTAG_XPOSITION, &position )) {
      xPos = static_cast< double >( position ) * pixelSizeMultiplier;
      xPos.Normalize();
   }
   if( TIFFGetField( tiff, TIFFTAG_YPOSITION, &position )) {
      yPos = static_cast< double >( position ) * pixelSizeMultiplier;
      yPos.Normalize();
   }
   data.fileInformation.origin = { xPos, yPos };

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
      DIP_THROW_RUNTIME( "Tiled TIFF format not supported for colormapped images" );
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
      DIP_THROW_RUNTIME( "Tiled TIFF format not supported for binary images" );
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

inline void CopyBuffer2D_8bit(
      uint8* dest,
      uint8 const* src,
      dip::uint destSizeX,
      dip::uint destSizeY,
      dip::sint destStrideX,
      dip::sint destStrideY,
      dip::uint srcStrideX,
      dip::uint srcStrideY
) {
   for( dip::uint yy = 0; yy < destSizeY; ++yy ) {
      uint8* dest_pixel = dest;
      uint8 const* src_pixel = src;
      for( dip::uint xx = 0; xx < destSizeX; ++xx ) {
         *dest_pixel = *src_pixel;
         dest_pixel += destStrideX;
         src_pixel += srcStrideX;
      }
      dest += destStrideY;
      src += srcStrideY;
   }
}

inline void CopyBuffer2D(
      uint8* dest,
      uint8 const* src,
      dip::uint destSizeX,
      dip::uint destSizeY,
      dip::sint destStrideX,
      dip::sint destStrideY,
      dip::uint srcStrideX,
      dip::uint srcStrideY,
      dip::uint sizeOf
) {
   destStrideX *= static_cast< dip::sint >( sizeOf );
   destStrideY *= static_cast< dip::sint >( sizeOf );
   srcStrideX *= sizeOf;
   srcStrideY *= sizeOf;
   for( dip::uint yy = 0; yy < destSizeY; ++yy ) {
      uint8* dest_pixel = dest;
      uint8 const* src_pixel = src;
      for( dip::uint xx = 0; xx < destSizeX; ++xx ) {
         memcpy( dest_pixel, src_pixel, sizeOf );
         dest_pixel += destStrideX;
         src_pixel += srcStrideX;
      }
      dest += destStrideY;
      src += srcStrideY;
   }
}

inline void CopyBuffer3D_8bit(
      uint8* dest,
      uint8 const* src,
      dip::uint destSizeT,
      dip::uint destSizeX,
      dip::uint destSizeY,
      dip::sint destStrideT,
      dip::sint destStrideX,
      dip::sint destStrideY,
      dip::uint srcStrideT,
      dip::uint srcStrideX,
      dip::uint srcStrideY
) {
   for( dip::uint yy = 0; yy < destSizeY; ++yy ) {
      uint8* dest_pixel = dest;
      uint8 const* src_pixel = src;
      for( dip::uint xx = 0; xx < destSizeX; ++xx ) {
         uint8* dest_sample = dest_pixel;
         uint8 const* src_sample = src_pixel;
         for( dip::uint tt = 0; tt < destSizeT; ++tt ) {
            *dest_sample = *src_sample;
            dest_sample += destStrideT;
            src_sample += srcStrideT;
         }
         dest_pixel += destStrideX;
         src_pixel += srcStrideX;
      }
      dest += destStrideY;
      src += srcStrideY;
   }
}

inline void CopyBuffer3D(
      uint8* dest,
      uint8 const* src,
      dip::uint destSizeT,
      dip::uint destSizeX,
      dip::uint destSizeY,
      dip::sint destStrideT,
      dip::sint destStrideX,
      dip::sint destStrideY,
      dip::uint srcStrideT,
      dip::uint srcStrideX,
      dip::uint srcStrideY,
      dip::uint sizeOf
) {
   destStrideT *= static_cast< dip::sint >( sizeOf );
   destStrideX *= static_cast< dip::sint >( sizeOf );
   destStrideY *= static_cast< dip::sint >( sizeOf );
   srcStrideT *= sizeOf;
   srcStrideX *= sizeOf;
   srcStrideY *= sizeOf;
   for( dip::uint yy = 0; yy < destSizeY; ++yy ) {
      uint8* dest_pixel = dest;
      uint8 const* src_pixel = src;
      for( dip::uint xx = 0; xx < destSizeX; ++xx ) {
         uint8* dest_sample = dest_pixel;
         uint8 const* src_sample = src_pixel;
         for( dip::uint tt = 0; tt < destSizeT; ++tt ) {
            memcpy( dest_sample, src_sample, sizeOf );
            dest_sample += destStrideT;
            src_sample += srcStrideT;
         }
         dest_pixel += destStrideX;
         src_pixel += srcStrideX;
      }
      dest += destStrideY;
      src += srcStrideY;
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
      IntegerArray const& strides,
      dip::sint tensorStride,
      DataType dataType,
      TiffFile& tiff,
      FileInformation& data, // Shows how the data is stored in the file
      RoiSpec const& roiSpec // Shows how the data is stored in memory -- sizes might be smaller if reading ROI!
) {
   dip::uint sizeOf = dataType.SizeOf();

   // Planar configuration?
   uint16 planarConfiguration = PLANARCONFIG_SEPARATE;
   if( data.tensorElements > 1 ) {
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
      std::vector< uint8 > buf( static_cast< dip::uint >( tileSize ));
      //uint32 nTiles = TIFFNumberOfTiles( tiff );
      dip::uint firstTileX = ( roiSpec.roi[ 0 ].Offset() / tileWidth ) * tileWidth;
      dip::uint firstTileY = ( roiSpec.roi[ 1 ].Offset() / tileLength ) * tileLength;
      if( planarConfiguration == PLANARCONFIG_CONTIG ) {
         // 1234123412341234....
         // We know that data.tensorElements > 1, otherwise we force to PLANARCONFIG_SEPARATE
         //std::cout << "[ReadTIFFData] Tiles, Contiguous\n";
         DIP_ASSERT( static_cast< dip::uint >( tileSize ) == tileWidth * tileLength * data.tensorElements * sizeOf );
         dip::uint tileStrideY = data.tensorElements * tileWidth;
         dip::uint yPos = roiSpec.roi[ 1 ].Offset();
         for( dip::uint y = firstTileY; y <= roiSpec.roi[ 1 ].Last(); y += tileLength ) {
            dip::uint tileEndY = std::min( y + tileLength, data.sizes[ 1 ] );
            tileEndY = std::min( tileEndY, roiSpec.roi[ 1 ].Last() + 1 );
            if( yPos >= tileEndY ) {
               continue;
            }
            dip::uint copyHeight = div_ceil( tileEndY - yPos, roiSpec.roi[ 1 ].step );
            uint8* imagedataPtr = imagedata;
            dip::uint offsetY = ( yPos - y ) * tileStrideY;
            dip::uint xPos = roiSpec.roi[ 0 ].Offset();
            for( dip::uint x = firstTileX; x <= roiSpec.roi[ 0 ].Last(); x += tileWidth ) {
               dip::uint tileEndX = std::min( x + tileWidth, data.sizes[ 0 ] );
               tileEndX = std::min( tileEndX, roiSpec.roi[ 0 ].Last() + 1 );
               if( xPos >= tileEndX ) {
                  continue;
               }
               dip::uint copyWidth = div_ceil( tileEndX - xPos, roiSpec.roi[ 0 ].step );
               dip::uint offset = ( offsetY + ( xPos - x ) * data.tensorElements + roiSpec.channels.Offset() );
               uint32 tile = TIFFComputeTile( tiff, static_cast< uint32 >( x ), static_cast< uint32 >( y ), 0, 0 );
               TIFFReadEncodedTile( tiff, tile, buf.data(), tileSize );
               if( sizeOf == 1 ) {
                  CopyBuffer3D_8bit( imagedataPtr, buf.data() + offset, roiSpec.tensorElements, copyWidth, copyHeight,
                                     tensorStride, strides[ 0 ], strides[ 1 ],
                                     roiSpec.channels.step, data.tensorElements * roiSpec.roi[ 0 ].step, tileStrideY * roiSpec.roi[ 1 ].step );
                  imagedataPtr += static_cast< dip::sint >( copyWidth ) * strides[ 0 ];
               } else {
                  CopyBuffer3D( imagedataPtr, buf.data() + offset * sizeOf, roiSpec.tensorElements, copyWidth, copyHeight,
                                tensorStride, strides[ 0 ], strides[ 1 ],
                                roiSpec.channels.step, data.tensorElements * roiSpec.roi[ 0 ].step, tileStrideY * roiSpec.roi[ 1 ].step, sizeOf );
                  imagedataPtr += static_cast< dip::sint >( copyWidth * sizeOf ) * strides[ 0 ];
               }
               xPos += roiSpec.roi[ 0 ].step * copyWidth;
            }
            imagedata += static_cast< dip::sint >( copyHeight * sizeOf ) * strides[ 1 ];
            yPos += roiSpec.roi[ 1 ].step * copyHeight;
         }
      } else if( planarConfiguration == PLANARCONFIG_SEPARATE ) {
         // 1111...2222...3333...4444...
         //std::cout << "[ReadTIFFData] Tiles, Separate\n";
         DIP_ASSERT( static_cast< dip::uint >( tileSize ) == tileWidth * tileLength * sizeOf );
         dip::uint tileStrideY = tileWidth;
         for( auto plane : roiSpec.channels ) {
            uint8* imagedataRow = imagedata;
            dip::uint yPos = roiSpec.roi[ 1 ].Offset();
            for( dip::uint y = firstTileY; y <= roiSpec.roi[ 1 ].Last(); y += tileLength ) {
               dip::uint tileEndY = std::min( y + tileLength, data.sizes[ 1 ] );
               tileEndY = std::min( tileEndY, roiSpec.roi[ 1 ].Last() + 1 );
               if( yPos >= tileEndY ) {
                  continue;
               }
               dip::uint copyHeight = div_ceil( tileEndY - yPos, roiSpec.roi[ 1 ].step );
               uint8* imagedataPtr = imagedataRow;
               dip::uint offsetY = ( yPos - y ) * tileStrideY;
               dip::uint xPos = roiSpec.roi[ 0 ].Offset();
               for( dip::uint x = firstTileX; x <= roiSpec.roi[ 0 ].Last(); x += tileWidth ) {
                  dip::uint tileEndX = std::min( x + tileWidth, data.sizes[ 0 ] );
                  tileEndX = std::min( tileEndX, roiSpec.roi[ 0 ].Last() + 1 );
                  if( xPos >= tileEndX ) {
                     continue;
                  }
                  dip::uint copyWidth = div_ceil( tileEndX - xPos, roiSpec.roi[ 0 ].step );
                  dip::uint offset = ( offsetY + ( xPos - x ));
                  uint32 tile = TIFFComputeTile( tiff, static_cast< uint32 >( x ), static_cast< uint32 >( y ), 0, static_cast< uint16 >( plane ));
                  TIFFReadEncodedTile( tiff, tile, buf.data(), tileSize );
                  if( sizeOf == 1 ) {
                     //std::cout << "Copying " << copyWidth << "x" << copyHeight << " pixels from tile, pos = " << xPos << ", " << yPos << std::endl;
                     CopyBuffer2D_8bit( imagedataPtr, buf.data() + offset, copyWidth, copyHeight,
                                        strides[ 0 ], strides[ 1 ],
                                        roiSpec.roi[ 0 ].step, tileStrideY * roiSpec.roi[ 1 ].step );
                     imagedataPtr += static_cast< dip::sint >( copyWidth ) * strides[ 0 ];
                  } else {
                     CopyBuffer2D( imagedataPtr, buf.data() + offset * sizeOf, copyWidth, copyHeight,
                                   strides[ 0 ], strides[ 1 ],
                                   roiSpec.roi[ 0 ].step, tileStrideY * roiSpec.roi[ 1 ].step, sizeOf );
                     imagedataPtr += static_cast< dip::sint >( copyWidth * sizeOf ) * strides[ 0 ];
                  }
                  xPos += roiSpec.roi[ 0 ].step * copyWidth;
               }
               imagedataRow += static_cast< dip::sint >( copyHeight * sizeOf ) * strides[ 1 ];
               yPos += roiSpec.roi[ 1 ].step * copyHeight;
            }
            imagedata += static_cast< dip::sint >( sizeOf ) * tensorStride;
         }
      } else {
         DIP_THROW_RUNTIME( "Unsupported TIFF: unknown PlanarConfiguration value" );
      }
   } else {
      // --- Striped TIFF file ---
      uint32 stripHeight;
      TIFFGetFieldDefaulted( tiff, TIFFTAG_ROWSPERSTRIP, &stripHeight );
      tsize_t stripSize = TIFFStripSize( tiff );
      uint32 nStrips = TIFFNumberOfStrips( tiff );
      dip::uint firstStrip = ( roiSpec.roi[ 1 ].Offset() / stripHeight ) * stripHeight;
      if( planarConfiguration == PLANARCONFIG_CONTIG ) {
         // 1234123412341234....
         // We know that tensorElements > 1, otherwise we force to PLANARCONFIG_SEPARATE
         DIP_ASSERT( static_cast< dip::uint >( TIFFScanlineSize( tiff )) == data.sizes[ 0 ] * data.tensorElements * sizeOf );
         if( roiSpec.isFullImage && roiSpec.isAllChannels && StridesAreNormal( data.tensorElements, tensorStride, data.sizes, strides )) {
            //std::cout << "[ReadTIFFData] Stripes, Contiguous, isFullImage\n";
            uint32 yPos = 0;
            for( uint32 strip = 0; strip < nStrips; ++strip ) {
               dip::uint copyHeight = ( yPos + stripHeight > data.sizes[ 1 ] ? data.sizes[ 1 ] - yPos : stripHeight );
               if( TIFFReadEncodedStrip( tiff, strip, imagedata, stripSize ) < 0 ) {
                  DIP_THROW_RUNTIME( "Error reading data (planar config cont)" );
               }
               imagedata += static_cast< dip::sint >( copyHeight * sizeOf ) * strides[ 1 ];
               yPos += stripHeight;
            }
         } else {
            //std::cout << "[ReadTIFFData] Stripes, Contiguous\n";
            std::vector< uint8 > buf( static_cast< dip::uint >( stripSize ));
            dip::uint yPos = roiSpec.roi[ 1 ].Offset();
            dip::uint yStride = data.tensorElements * data.sizes[ 0 ];
            for( dip::uint y = firstStrip; y <= roiSpec.roi[ 1 ].Last(); y += stripHeight ) {
               dip::uint stripEnd = std::min( y + stripHeight, data.sizes[ 1 ] );
               stripEnd = std::min( stripEnd, roiSpec.roi[ 1 ].Last() + 1 );
               if( yPos >= stripEnd ) {
                  continue;
               }
               dip::uint copyHeight = div_ceil( stripEnd - yPos, roiSpec.roi[ 1 ].step );
               dip::uint offsetY = ( yPos - y ) * yStride;
               dip::uint offset = ( offsetY + roiSpec.roi[ 0 ].Offset() * data.tensorElements + roiSpec.channels.Offset() );
               uint32 strip = TIFFComputeStrip( tiff, static_cast< uint32 >( y ), 0 );
               if( TIFFReadEncodedStrip( tiff, strip, buf.data(), stripSize ) < 0 ) {
                  DIP_THROW_RUNTIME( "Error reading data (planar config cont)" );
               }
               if( sizeOf == 1 ) {
                  //std::cout << "Copying " << roiSpec.sizes[ 0 ] << "x" << copyHeight << " pixels from stripe, pos = " << roiSpec.roi[ 0 ].Offset() << ", " << yPos << std::endl;
                  CopyBuffer3D_8bit(
                        imagedata, buf.data() + offset, roiSpec.tensorElements, roiSpec.sizes[ 0 ], copyHeight,
                        tensorStride, strides[ 0 ], strides[ 1 ],
                        roiSpec.channels.step, data.tensorElements * roiSpec.roi[ 0 ].step, yStride * roiSpec.roi[ 1 ].step );
                  imagedata += static_cast< dip::sint >( copyHeight ) * strides[ 1 ];
               } else {
                  CopyBuffer3D(
                        imagedata, buf.data() + offset * sizeOf, roiSpec.tensorElements, roiSpec.sizes[ 0 ], copyHeight,
                        tensorStride, strides[ 0 ], strides[ 1 ],
                        roiSpec.channels.step, data.tensorElements * roiSpec.roi[ 0 ].step, yStride * roiSpec.roi[ 1 ].step, sizeOf );
                  imagedata += static_cast< dip::sint >( copyHeight * sizeOf ) * strides[ 1 ];
               }
               yPos += roiSpec.roi[ 1 ].step * copyHeight;
            }
         }
      } else if( planarConfiguration == PLANARCONFIG_SEPARATE ) {
         // 1111...2222...3333...4444...
         DIP_ASSERT( static_cast< dip::uint >( TIFFScanlineSize( tiff )) == data.sizes[ 0 ] * sizeOf );
         DIP_ASSERT( nStrips % data.tensorElements == 0 );
         nStrips /= static_cast< uint32 >( data.tensorElements );
         if( roiSpec.isFullImage && StridesAreNormal( 1, 1, data.sizes, strides )) {
            //std::cout << "[ReadTIFFData] Stripes, Separate, isFullImage\n";
            for( auto plane : roiSpec.channels ) {
               uint8* imagedataRow = imagedata;
               uint32 stripOffset = TIFFComputeStrip( tiff, 0, static_cast< uint16 >( plane ));
               uint32 yPos = 0;
               for( uint32 strip = 0; strip < nStrips; ++strip ) {
                  dip::uint copyHeight = ( yPos + stripHeight > data.sizes[ 1 ] ? data.sizes[ 1 ] - yPos : stripHeight );
                  if( TIFFReadEncodedStrip( tiff, stripOffset + strip, imagedataRow, stripSize ) < 0 ) {
                     DIP_THROW_RUNTIME( "Error reading data (planar config separate)" );
                  }
                  imagedataRow += static_cast< dip::sint >( copyHeight * sizeOf ) * strides[ 1 ];
                  yPos += stripHeight;
               }
               imagedata += static_cast< dip::sint >( sizeOf ) * tensorStride;
            }
         } else {
            //std::cout << "[ReadTIFFData] Stripes, Separate\n";
            std::vector< uint8 > buf( static_cast< dip::uint >( stripSize ));
            dip::uint yStride = data.sizes[ 0 ];
            for( auto plane : roiSpec.channels ) {
               uint8* imagedataRow = imagedata;
               dip::uint yPos = roiSpec.roi[ 1 ].Offset();
               for( dip::uint y = firstStrip; y <= roiSpec.roi[ 1 ].Last(); y += stripHeight ) {
                  dip::uint stripEnd = std::min( y + stripHeight, data.sizes[ 1 ] );
                  stripEnd = std::min( stripEnd, roiSpec.roi[ 1 ].Last() + 1 );
                  if( yPos >= stripEnd ) {
                     continue;
                  }
                  dip::uint copyHeight = div_ceil( stripEnd - yPos, roiSpec.roi[ 1 ].step );
                  dip::uint offsetY = ( yPos - y ) * yStride;
                  dip::uint offset = ( offsetY + roiSpec.roi[ 0 ].Offset() );
                  uint32 strip = TIFFComputeStrip( tiff, static_cast< uint32 >( y ), static_cast< uint16 >( plane ));
                  if( TIFFReadEncodedStrip( tiff, strip, buf.data(), stripSize ) < 0 ) {
                     DIP_THROW_RUNTIME( "Error reading data (planar config separate)" );
                  }
                  if( sizeOf == 1 ) {
                     CopyBuffer2D_8bit( imagedataRow, buf.data() + offset, roiSpec.sizes[ 0 ], copyHeight,
                                        strides[ 0 ], strides[ 1 ],
                                        roiSpec.roi[ 0 ].step, yStride * roiSpec.roi[ 1 ].step );
                     imagedataRow += static_cast< dip::sint >( copyHeight ) * strides[ 1 ];
                  } else {
                     CopyBuffer2D( imagedataRow, buf.data() + offset * sizeOf, roiSpec.sizes[ 0 ], copyHeight,
                                   strides[ 0 ], strides[ 1 ],
                                   roiSpec.roi[ 0 ].step, yStride * roiSpec.roi[ 1 ].step, sizeOf );
                     imagedataRow += static_cast< dip::sint >( copyHeight * sizeOf ) * strides[ 1 ];
                  }
                  yPos += roiSpec.roi[ 1 ].step * copyHeight;
               }
               imagedata += static_cast< dip::sint >( sizeOf ) * tensorStride;
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
      GetTIFFInfoData& data,
      RoiSpec const& roiSpec
) {
   if( image.IsForged() && roiSpec.isFullImage && (( image.Sizes() != roiSpec.sizes ) || ( image.TensorElements() != roiSpec.tensorElements ))) {
      // We need to reforge anyway, let's strip now.
      image.Strip();
   }
   if( !image.IsForged() && ( roiSpec.tensorElements > 1 )) {
      uint16 planarConfiguration = PLANARCONFIG_CONTIG;
      TIFFGetField( tiff, TIFFTAG_PLANARCONFIG, &planarConfiguration );
      if( planarConfiguration == PLANARCONFIG_SEPARATE ) {
         // Set the tensor dimension as the last one, it will speed up reading
         image.SetSizes( roiSpec.sizes );
         image.SetTensorSizes( 1 );
         image.SetNormalStrides();
         image.SetTensorSizes( roiSpec.tensorElements );
         image.SetTensorStride( static_cast< dip::sint >( roiSpec.sizes.product() ));
      }
   }

   // Forge the image
   image.ReForge( roiSpec.sizes, roiSpec.tensorElements, data.fileInformation.dataType );
   uint8* imagedata = static_cast< uint8* >( image.Origin() );

   // Read the image data
   DIP_STACK_TRACE_THIS( ReadTIFFData( imagedata, image.Strides(), image.TensorStride(), image.DataType(), tiff, data.fileInformation, roiSpec ));

   if( data.photometricInterpretation == PHOTOMETRIC_MINISWHITE ) {
      Invert( image, image );
   }
}

void ImageReadTIFFStack(
      Image& image,
      TiffFile& tiff,
      GetTIFFInfoData& data,
      Range const& imageNumbers,
      RoiSpec& roiSpec
) {
   // Forge the image
   data.fileInformation.sizes.push_back( imageNumbers.Size() );
   roiSpec.sizes.push_back( imageNumbers.Size() );
   roiSpec.mirror.push_back( false );
   std::cout << "[ImageReadTIFFStack] roiSpec.sizes = " << roiSpec.sizes << ", roiSpec.tensorElements = " << roiSpec.tensorElements << '\n';
   image.ReForge( roiSpec.sizes, roiSpec.tensorElements, data.fileInformation.dataType );
   uint8* imagedata = static_cast< uint8* >( image.Origin() );
   dip::sint z_stride = image.Stride( 2 ) * static_cast< dip::sint >( data.fileInformation.dataType.SizeOf() );

   // Read the image data for first plane
   DIP_STACK_TRACE_THIS( ReadTIFFData( imagedata, image.Strides(), image.TensorStride(), image.DataType(), tiff, data.fileInformation, roiSpec ));

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
      if( temp32 != data.fileInformation.sizes[ 0 ] ) {
         DIP_THROW_RUNTIME( "Reading multi-slice TIFF: width of images not consistent" );
      }
      READ_REQUIRED_TIFF_TAG( tiff, TIFFTAG_IMAGELENGTH, &temp32 );
      if( temp32 != data.fileInformation.sizes[ 1 ] ) {
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
      if( samplesPerPixel != data.fileInformation.tensorElements ) {
         DIP_THROW_RUNTIME( "Reading multi-slice TIFF: samples per pixel not consistent" );
      }

      // Read the image data for this plane
      DIP_STACK_TRACE_THIS( ReadTIFFData( imagedata, image.Strides(), image.TensorStride(), image.DataType(), tiff, data.fileInformation, roiSpec ));
   }
}

} // namespace

FileInformation ImageReadTIFF(
      Image& out,
      String const& filename,
      Range imageNumbers,
      RangeArray const& roi,
      Range const& channels
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

   // Check & fix ROI information
   //dip::uint nDims = imageNumbers.start == imageNumbers.stop ? 2 : 3;
   RoiSpec roiSpec;
   DIP_STACK_TRACE_THIS( roiSpec = CheckAndConvertRoi( roi, channels, data.fileInformation, 2 ));

   if( imageNumbers.start != imageNumbers.stop ) {
      // Read in multiple pages as a 3D image
      DIP_STACK_TRACE_THIS( ImageReadTIFFStack( out, tiff, data, imageNumbers, roiSpec ));
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
         DIP_THROW_IF( !roiSpec.isFullImage, "Reading ROI not supported for colormapped images" );
         DIP_STACK_TRACE_THIS( ReadTIFFColorMap( out, tiff, data ));
      } else {
         if( data.fileInformation.dataType.IsBinary() ) {
            DIP_THROW_IF( !roiSpec.isFullImage, "Reading ROI not supported for binary images" );
            DIP_STACK_TRACE_THIS( ReadTIFFBinary( out, tiff, data ));
         } else {
            DIP_STACK_TRACE_THIS( ReadTIFFGreyValue( out, tiff, data, roiSpec ));
         }
      }
   }

   if( roiSpec.isAllChannels ) {
      out.SetColorSpace( data.fileInformation.colorSpace );
   }
   out.SetPixelSize( data.fileInformation.pixelSize );

   // Apply the mirroring to the output image
   out.Mirror( roiSpec.mirror );

   return data.fileInformation;
}

FileInformation ImageReadTIFF(
      Image& out,
      String const& filename,
      Range const& imageNumbers,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      UnsignedArray const& spacing,
      Range const& channels
) {
   RangeArray roi;
   DIP_STACK_TRACE_THIS( roi = ConvertRoiSpec( origin, sizes, spacing ));
   DIP_THROW_IF( roi.size() > 2, E::ARRAY_PARAMETER_WRONG_LENGTH );
   return ImageReadTIFF( out, filename, imageNumbers, roi, channels );
}

void ImageReadTIFFSeries(
      Image& out,
      StringArray const& filenames
) {
   DIP_THROW_IF( filenames.size() < 1, E::ARRAY_PARAMETER_EMPTY );

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

static const char* NOT_AVAILABLE = "DIPlib was compiled without TIFF support.";

FileInformation ImageReadTIFF(
   Image&,
   String const&,
   Range,
   RangeArray const&,
   Range const&
) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadTIFF(
      Image&,
      String const&,
      Range const&,
      UnsignedArray const&,
      UnsignedArray const&,
      UnsignedArray const&,
      Range const&
) {
   DIP_THROW( NOT_AVAILABLE );
}

void ImageReadTIFFSeries( Image&, StringArray const& ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadTIFFInfo( String const&, dip::uint ) {
   DIP_THROW( NOT_AVAILABLE );
}

bool ImageIsTIFF( String const& ) {
   DIP_THROW( NOT_AVAILABLE );
}

}

#endif // DIP__HAS_TIFF
