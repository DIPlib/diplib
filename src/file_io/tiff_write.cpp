/*
 * DIPlib 3.0
 * This file contains definitions for TIFF writing
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

#include <tiffio.h>

namespace dip {

namespace {

static const char* TIFF_WRITE_TAG = "Error writing tag to TIFF file";

#define WRITE_TIFF_TAG( tiff, tag, value ) do { if( !TIFFSetField( tiff, tag, value )) { DIP_THROW_RUNTIME( TIFF_WRITE_TAG ); }} while(false)

class TiffFile {
   public:
      explicit TiffFile( String const& filename ) {
         // Set error and warning handlers, these are library-wide!
         TIFFSetErrorHandler( nullptr );
         TIFFSetWarningHandler( nullptr );
         // Open the file for writing
         if( FileHasExtension( filename )) {
            tiff_ = TIFFOpen( filename.c_str(), "w" );
         } else {
            tiff_ = TIFFOpen( FileAddExtension( filename, "tif" ).c_str(), "w" );
         }
         if( tiff_ == nullptr ) {
            DIP_THROW_RUNTIME( "Could not open the specified file" );
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
   private:
      TIFF* tiff_ = nullptr;
};

static uint16 CompressionTranslate( String const& compression ) {
   if( compression.empty() || ( compression == "deflate" )) {
      return COMPRESSION_DEFLATE;
   } else if( compression == "LZW" ) {
      return COMPRESSION_LZW;
   } else if( compression == "PackBits" ) {
      return COMPRESSION_PACKBITS;
   } else if( compression == "JPEG" ) {
      return COMPRESSION_JPEG;
   } else if( compression == "none" ) {
      return COMPRESSION_NONE;
   } else {
      DIP_THROW_INVALID_FLAG( compression );
   }
}

void FillBuffer1(
      uint8* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      IntegerArray const& strides
) {
   dip::sint kk = 7;
   uint8 byte = 0;
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8 const* src_pixel = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         if( kk < 0 ) {
            kk = 7;
            *dest = byte;
            ++dest;
            byte = 0;
         }
         if( *src_pixel ) {
            byte = static_cast< uint8 >( byte | ( 1u << kk )); // GCC likes to warn about `a |= b`, when both `a` and `b` are uint8.
         }
         src_pixel += strides[ 0 ];
         --kk;
      }
      if( kk != 7 ) {
         kk = 7;
         *dest = byte;
         ++dest;
         byte = 0;
      }
      src += strides[ 1 ];
   }
}

void FillBuffer8(
      uint8* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      IntegerArray const& strides
) {
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8 const* src_pixel = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         *dest = *src_pixel;
         ++dest;
         src_pixel += strides[ 0 ];
      }
      src += strides[ 1 ];
   }
}

void FillBufferN(
      uint8* dest,
      uint8 const* src,
      dip::uint width,
      dip::uint height,
      IntegerArray const& strides,
      dip::uint sizeOf
) {
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8 const* src_pixel = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         memcpy( dest, src_pixel, sizeOf );
         dest += sizeOf;
         src_pixel += static_cast< dip::sint >( sizeOf ) * strides[ 0 ];
      }
      src += static_cast< dip::sint >( sizeOf ) * strides[ 1 ];
   }
}

void FillBufferMultiChannel8(
      uint8* dest,
      uint8 const* src,
      dip::uint tensorElements,
      dip::uint width,
      dip::uint height,
      dip::sint tensorStride,
      IntegerArray const& strides
) {
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8 const* src_pixel = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         uint8 const* src_sample = src_pixel;
         for( dip::uint kk = 0; kk < tensorElements; ++kk ) {
            *dest = *src_sample;
            ++dest;
            src_sample += tensorStride;
         }
         src_pixel += strides[ 0 ];
      }
      src += strides[ 1 ];
   }
}

void FillBufferMultiChannelN(
      uint8* dest,
      uint8 const* src,
      dip::uint tensorElements,
      dip::uint width,
      dip::uint height,
      dip::sint tensorStride,
      IntegerArray const& strides,
      dip::uint sizeOf
) {
   dip::sint stride_row = strides[ 1 ] * static_cast< dip::sint >( sizeOf );
   dip::sint stride_pixel = strides[ 0 ] * static_cast< dip::sint >( sizeOf );
   dip::sint stride_sample = tensorStride * static_cast< dip::sint >( sizeOf );
   for( dip::uint ii = 0; ii < height; ++ii ) {
      uint8 const* src_pixel = src;
      for( dip::uint jj = 0; jj < width; ++jj ) {
         uint8 const* src_sample = src_pixel;
         for( dip::uint kk = 0; kk < tensorElements; ++kk ) {
            memcpy( dest, src_sample, sizeOf );
            dest += sizeOf;
            src_sample += stride_sample;
         }
         src_pixel += stride_pixel;
      }
      src += stride_row;
   }
}

void WriteTIFFStrips(
      Image const& image,
      TiffFile& tiff
) {
   dip::uint tensorElements = image.TensorElements();
   dip::uint imageWidth = image.Size( 0 );
   uint32 imageLength = static_cast< uint32 >( image.Size( 1 ));
   dip::sint tensorStride = image.TensorStride();
   IntegerArray const& strides = image.Strides();
   dip::uint sizeOf = image.DataType().SizeOf();
   bool binary = image.DataType().IsBinary();

   uint32 rowsPerStrip = TIFFDefaultStripSize( tiff, 0 );
   WRITE_TIFF_TAG( tiff, TIFFTAG_ROWSPERSTRIP, rowsPerStrip );

   // Write it to the file
   tmsize_t scanline = TIFFScanlineSize( tiff );
   if( binary ) {
      DIP_ASSERT( static_cast< dip::uint >( scanline ) == div_ceil< dip::uint >( image.Size( 0 ), 8 ));
      DIP_ASSERT( tensorElements == 1 );
   } else {
      DIP_ASSERT( static_cast< dip::uint >( scanline ) == image.Size( 0 ) * tensorElements * sizeOf );
   }
   if( image.HasNormalStrides() && !binary ) {
      // Simple writing
      tstrip_t strip = 0;
      uint8* data = static_cast< uint8* >( image.Origin() );
      for( uint32 row = 0; row < imageLength; row += rowsPerStrip ) {
         uint32 nrow = row + rowsPerStrip > imageLength ? imageLength - row : rowsPerStrip;
         if( TIFFWriteEncodedStrip( tiff, strip, data, nrow * scanline ) < 0 ) {
            DIP_THROW_RUNTIME( "Error writing data" );
         }
         data += static_cast< dip::sint >( nrow * sizeOf ) * image.Stride( 1 );
         ++strip;
      }
   } else {
      // Writing requires an intermediate buffer, filled using strides
      std::vector< uint8 > buf( static_cast< dip::uint >( TIFFStripSize( tiff )));
      tstrip_t strip = 0;
      uint8* data = static_cast< uint8* >( image.Origin() );
      for( uint32 row = 0; row < imageLength; row += rowsPerStrip ) {
         uint32 nrow = row + rowsPerStrip > imageLength ? imageLength - row : rowsPerStrip;
         if( tensorElements == 1 ) {
            if( binary) {
               FillBuffer1( buf.data(), data, imageWidth, nrow, strides );
            } else if( sizeOf == 1 ) {
               FillBuffer8( buf.data(), data, imageWidth, nrow, strides );
            } else {
               FillBufferN( buf.data(), data, imageWidth, nrow, strides, sizeOf );
            }
         } else {
            if( sizeOf == 1 ) {
               FillBufferMultiChannel8( buf.data(), data, tensorElements, imageWidth, nrow, tensorStride, strides );
            } else {
               FillBufferMultiChannelN( buf.data(), data, tensorElements, imageWidth, nrow, tensorStride, strides, sizeOf );
            }
         }
         if( TIFFWriteEncodedStrip( tiff, strip, buf.data(), nrow * scanline ) < 0 ) {
            DIP_THROW_RUNTIME( "Error writing data" );
         }
         data += static_cast< dip::sint >( nrow * sizeOf ) * image.Stride( 1 );
         ++strip;
      }
   }
}

} // namespace

void ImageWriteTIFF(
      Image const& image,
      String const& filename,
      String const& compression,
      dip::uint jpegLevel
) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( image.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   // TODO: Implement writing of 3D images as a stack of 2D images

   // Get image info and quit if we can't write
   DIP_THROW_IF(( image.Size( 0 ) > std::numeric_limits< uint32 >::max() ) ||
                ( image.Size( 1 ) > std::numeric_limits< uint32 >::max() ), "Image size too large for TIFF file" );
   uint32 imageWidth = static_cast< uint32 >( image.Size( 0 ));
   uint32 imageLength = static_cast< uint32 >( image.Size( 1 ));
   dip::uint sizeOf = image.DataType().SizeOf();
   uint16 bitsPerSample = 0;
   uint16 sampleFormat = 0;
   if( image.DataType().IsBinary() ) {
      DIP_THROW_IF( !image.IsScalar(), E::IMAGE_NOT_SCALAR ); // Binary images should not have multiple samples per pixel
   } else {
      bitsPerSample = static_cast< uint16 >( sizeOf * 8 );
      switch( image.DataType() ) {
         case DT_UINT8:
         case DT_UINT16:
         case DT_UINT32:
         case DT_UINT64:
            sampleFormat = SAMPLEFORMAT_UINT;
            break;
         case DT_SINT8:
         case DT_SINT16:
         case DT_SINT32:
         case DT_SINT64:
            sampleFormat = SAMPLEFORMAT_INT;
            break;
         case DT_SFLOAT:
         case DT_DFLOAT:
            sampleFormat = SAMPLEFORMAT_IEEEFP;
            break;
         default:
            DIP_THROW( "Data type of image is not compatible with TIFF" );
            break;
      }
   }
   uint16 compmode = CompressionTranslate( compression );

   // Create the TIFF file and set the tags
   TiffFile tiff( filename );

   if( image.DataType().IsBinary() ) {
      WRITE_TIFF_TAG( tiff, TIFFTAG_PHOTOMETRIC, uint16( PHOTOMETRIC_MINISBLACK ));
   } else if( image.ColorSpace() == "RGB" ) {
      WRITE_TIFF_TAG( tiff, TIFFTAG_PHOTOMETRIC, uint16( PHOTOMETRIC_RGB ));
   } else if( image.ColorSpace() == "Lab" ) {
      WRITE_TIFF_TAG( tiff, TIFFTAG_PHOTOMETRIC, uint16( PHOTOMETRIC_CIELAB ));
   } else if(( image.ColorSpace() == "CMY" ) || ( image.ColorSpace() == "CMYK" )) {
      WRITE_TIFF_TAG( tiff, TIFFTAG_PHOTOMETRIC, uint16( PHOTOMETRIC_SEPARATED ));
   } else {
      WRITE_TIFF_TAG( tiff, TIFFTAG_PHOTOMETRIC, uint16( PHOTOMETRIC_MINISBLACK ));
   }

   WRITE_TIFF_TAG( tiff, TIFFTAG_IMAGEWIDTH, imageWidth );
   WRITE_TIFF_TAG( tiff, TIFFTAG_IMAGELENGTH, imageLength );

   if( !image.DataType().IsBinary() ) {
      WRITE_TIFF_TAG( tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample );
      WRITE_TIFF_TAG( tiff, TIFFTAG_SAMPLEFORMAT, sampleFormat );
      WRITE_TIFF_TAG( tiff, TIFFTAG_SAMPLESPERPIXEL, static_cast< uint16 >( image.TensorElements() ));
      if( image.TensorElements() > 1 ) {
         WRITE_TIFF_TAG( tiff, TIFFTAG_PLANARCONFIG, uint16( PLANARCONFIG_CONTIG ));
         // This is the standard way of writing channels (planes), PLANARCONFIG_SEPARATE is not required to be
         // supported by all readers.
      }
   }

   WRITE_TIFF_TAG( tiff, TIFFTAG_COMPRESSION, compmode );
   if( compmode == COMPRESSION_JPEG ) {
      jpegLevel = clamp< dip::uint >( jpegLevel, 1, 100 );
      WRITE_TIFF_TAG( tiff, TIFFTAG_JPEGQUALITY, static_cast< int >( jpegLevel ));
      WRITE_TIFF_TAG( tiff, TIFFTAG_JPEGCOLORMODE, int( JPEGCOLORMODE_RGB ));
   }

   DIP_STACK_TRACE_THIS( WriteTIFFStrips( image, tiff ));

   TIFFSetField( tiff, TIFFTAG_SOFTWARE, "DIPlib " DIP_VERSION_STRING );

   auto ps = image.PixelSize( 0 );
   if( ps.units.HasSameDimensions( Units::Meter() )) {
      ps.RemovePrefix();
      TIFFSetField( tiff, TIFFTAG_XRESOLUTION, static_cast< float >( 0.01 / ps.magnitude ));
   }
   ps = image.PixelSize( 1 );
   if( ps.units.HasSameDimensions( Units::Meter() )) {
      ps.RemovePrefix();
      TIFFSetField( tiff, TIFFTAG_YRESOLUTION, static_cast< float >( 0.01 / ps.magnitude ));
   }
   TIFFSetField( tiff, TIFFTAG_RESOLUTIONUNIT, uint16( RESUNIT_CENTIMETER ));
}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE( "[DIPlib] testing TIFF file reading and writing" ) {
   dip::Image image = dip::ImageReadTIFF( DIP__EXAMPLES_DIR "/fractal1.tiff" );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 6 * dip::Units::Micrometer(), 300 * dip::Units::Nanometer() } );

   dip::ImageWriteTIFF( image, "test1.tif" );
   dip::Image result = dip::ImageReadTIFF( "test1" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));

   // Try reading it into an image with non-standard strides
   result.Strip();
   auto strides = result.Strides();
   strides[ 0 ] = static_cast< dip::sint >( result.Size( 1 ));
   strides[ 1 ] = 1;
   result.SetStrides( strides );
   result.Forge();
   dip::ImageReadTIFF( result, "test1" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));

   // Turn it on its side so the image to write has non-standard strides
   image.SwapDimensions( 0, 1 );
   dip::ImageWriteTIFF( image, "test2.tif" );
   result = dip::ImageReadTIFF( "test2" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));
}

#endif // DIP__ENABLE_DOCTEST

#else // DIP__HAS_TIFF

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

static const char* NOT_AVAILABLE = "DIPlib was compiled without TIFF support.";

void ImageWriteTIFF( Image const&, String const&, String const&, dip::uint ) {
   DIP_THROW( NOT_AVAILABLE );
}

}

#endif // DIP__HAS_TIFF
