/*
 * DIPlib 3.0
 * This file contains definitions for JPEG reading and writing
 *
 * (c)2019, Cris Luengo.
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

#ifdef DIP_CONFIG_HAS_JPEG

#include "diplib.h"
#include "diplib/file_io.h"

#include "jpeglib.h"
#include <setjmp.h>

namespace dip {

namespace {

// JPEG error handling stuff - modified from example.c in libjpeg source
struct my_error_mgr {
   struct jpeg_error_mgr pub;   // "public" fields
   jmp_buf setjmp_buffer;      // for return to caller
};
using my_error_ptr = struct my_error_mgr*;

void my_error_exit( j_common_ptr cinfo ) {
   // cinfo->err really points to a my_error_mgr struct, so coerce pointer
   my_error_ptr myerr = reinterpret_cast<my_error_ptr>(cinfo->err);
   // Return control to the setjmp point
   longjmp( myerr->setjmp_buffer, 1 );
}

void my_output_message( j_common_ptr ) {} // Don't do anything with messages!

class JpegInput {
   public:
      JpegInput( String filename ) : filename_( std::move( filename )) {
         infile_ = std::fopen( filename_.c_str(), "rb" );
         if( infile_ == nullptr ) {
            if( !FileHasExtension( filename_ )) {
               filename_ = FileAddExtension( filename_, "jpg" ); // Try with "jpg" extension
               infile_ = std::fopen( filename_.c_str(), "rb" );
               if( infile_ == nullptr ) {
                  filename_ = FileAddExtension( filename_, "jpeg" ); // Try with "jpeg" extension
                  infile_ = std::fopen( filename_.c_str(), "rb" );
               }
            }
         }
         if( infile_ == nullptr ) {
            DIP_THROW_RUNTIME( "Could not open the specified JPEG file" );
         }
         cinfo_.err = jpeg_std_error( &jerr_.pub );
         jerr_.pub.error_exit = my_error_exit;
         jerr_.pub.output_message = my_output_message;
         if( setjmp( jerr_.setjmp_buffer )) {
            // If we get here, the JPEG code has signaled an error.
            DIP_THROW_RUNTIME( "Error reading JPEG file." );
         }
         jpeg_create_decompress( &cinfo_ );
         initialized_ = true;
         jpeg_stdio_src( &cinfo_, infile_ );
         jpeg_read_header( &cinfo_, TRUE );
      }
      JpegInput( JpegInput const& ) = delete;
      JpegInput( JpegInput&& ) = delete;
      JpegInput& operator=( JpegInput const& ) = delete;
      JpegInput& operator=( JpegInput&& ) = delete;
      ~JpegInput() {
         if( initialized_ ) {
            jpeg_destroy_decompress( &cinfo_ );
         }
         if( infile_ ) {
            std::fclose( infile_ );
         }
      }
      // Retrieve jpeg_decompress_struct
      jpeg_decompress_struct& cinfo() { return cinfo_; }
      j_decompress_ptr cinfoptr() { return &cinfo_; }
      // Retrieve file name
      String const& FileName() const { return filename_; }
   private:
      String filename_;
      FILE* infile_ = nullptr;
      jpeg_decompress_struct cinfo_;
      my_error_mgr jerr_;
      bool initialized_ = false;
};

class JpegOutput {
   public:
      explicit JpegOutput( String const& filename ) {
         // Open the file for writing
         if( FileHasExtension( filename )) {
            outfile_ = std::fopen(filename.c_str(), "wb");
         } else {
            outfile_ = std::fopen( FileAddExtension( filename, "jpg" ).c_str(), "wb" );
         }
         if( outfile_ == nullptr ) {
            DIP_THROW_RUNTIME( "Could not open file for writing" );
         }
         cinfo_.err = jpeg_std_error( &jerr_.pub );
         jerr_.pub.error_exit = my_error_exit;
         jerr_.pub.output_message = my_output_message;
         if( setjmp( jerr_.setjmp_buffer )) {
            // If we get here, the JPEG code has signaled an error.
            DIP_THROW_RUNTIME( "Error writing JPEG file." );
         }
         jpeg_create_compress( &cinfo_ );
         initialized_ = true;
         jpeg_stdio_dest( &cinfo_, outfile_ );
      }
      JpegOutput( JpegOutput const& ) = delete;
      JpegOutput( JpegOutput&& ) = delete;
      JpegOutput& operator=( JpegOutput const& ) = delete;
      JpegOutput& operator=( JpegOutput&& ) = delete;
      ~JpegOutput() {
         if( initialized_ ) {
            jpeg_destroy_compress( &cinfo_ );
         }
         if( outfile_ ) {
            std::fclose( outfile_ );
         }
      }
      // Retrieve jpeg_decompress_struct
      jpeg_compress_struct& cinfo() { return cinfo_; }
      j_compress_ptr cinfoptr() { return &cinfo_; }
   private:
      FILE* outfile_ = nullptr;
      jpeg_compress_struct cinfo_;
      my_error_mgr jerr_;
      bool initialized_ = false;
};

FileInformation GetJPEGInfo( JpegInput& jpeg ) {
   FileInformation fileInformation;
   fileInformation.name = jpeg.FileName();
   fileInformation.fileType = "JPEG";
   fileInformation.numberOfImages = 1;
   fileInformation.significantBits = 8;
   fileInformation.dataType = DT_UINT8;
   fileInformation.tensorElements = static_cast< dip::uint >( jpeg.cinfo().num_components );
   fileInformation.colorSpace = fileInformation.tensorElements == 3 ? "sRGB" : "";
   fileInformation.sizes = { jpeg.cinfo().image_width, jpeg.cinfo().image_height };
   PhysicalQuantity units;
   switch (jpeg.cinfo().density_unit) {
      default: // no units
         units = Units::Pixel();
         break;
      case 1: // dots per inch
         units = PhysicalQuantity::Inch();
         break;
      case 2: // dots per cm
         units = PhysicalQuantity::Centimeter();
   }
   fileInformation.pixelSize = {{ units / static_cast< dfloat >( jpeg.cinfo().X_density ),
                                  units / static_cast< dfloat >( jpeg.cinfo().Y_density ) }};
   return fileInformation;
}

} // namespace

FileInformation ImageReadJPEG(
      Image& out,
      String const& filename
) {
   // Open the file
   JpegInput jpeg( filename );

   // Get info
   FileInformation info = GetJPEGInfo( jpeg );

   // Allocate image
   int nchan = jpeg.cinfo().num_components;
   jpeg.cinfo().out_color_space = nchan > 1 ? JCS_RGB : JCS_GRAYSCALE;
   out.ReForge( info.sizes, info.tensorElements, DT_UINT8, Option::AcceptDataTypeChange::DONT_ALLOW );
   out.SetPixelSize( info.pixelSize );
   out.SetColorSpace( info.colorSpace );

   // Read data
   jpeg_start_decompress( jpeg.cinfoptr() );
   std::vector< JSAMPLE > buffer( info.sizes[ 0 ] * static_cast< unsigned >( nchan )); // casting to unsigned rather than dip::uint to shut up GCC warning.
   dip::uint8* imagedata = static_cast< dip::uint8* >( out.Origin() );
   auto stride = out.Strides();
   auto tStride = out.TensorStride();
   for( dip::uint ii = 0; ii < info.sizes[ 1 ]; ++ii ) {
      JSAMPLE* indata = buffer.data();
      jpeg_read_scanlines( jpeg.cinfoptr(), &indata, 1 );
      dip::uint8* outdata = imagedata;
      if( nchan > 1 ) {
         for( dip::uint jj = 0; jj < info.sizes[ 0 ]; ++jj ) {
            for( int kk = 0; kk < nchan; ++kk ) {
               *( outdata + kk * tStride ) = *indata;
               ++indata;
            }
            outdata += stride[ 0 ];
         }
      } else {
         for( dip::uint jj = 0; jj < info.sizes[ 0 ]; ++jj ) {
            *outdata = *indata;
            ++indata;
            outdata += stride[ 0 ];
         }
      }
      imagedata += stride[ 1 ];
   }
   jpeg_finish_decompress( jpeg.cinfoptr() );

   return info;
}

FileInformation ImageReadJPEGInfo( String const& filename ) {
   JpegInput jpeg( filename );
   FileInformation info = GetJPEGInfo( jpeg );
   return info;
}

bool ImageIsJPEG( String const& filename ) {
   try {
      JpegInput jpeg( filename );
   } catch( ... ) {
      return false;
   }
   return true;
}

void ImageWriteJPEG(
      Image const& image,
      String const& filename,
      dip::uint jpegLevel
) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( image.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   jpegLevel = clamp< dip::uint >( jpegLevel, 1, 100 );

   // Open the file
   JpegOutput jpeg( filename );

   // Set image properties
   int nchan = static_cast< int >( image.TensorElements() );
   jpeg.cinfo().image_width = static_cast< JDIMENSION >( image.Size( 0 ));
   jpeg.cinfo().image_height = static_cast< JDIMENSION >( image.Size( 1 ));
   jpeg.cinfo().input_components = nchan;
   jpeg.cinfo().in_color_space = nchan > 1 ? JCS_RGB : JCS_GRAYSCALE;
   jpeg_set_defaults( jpeg.cinfoptr() );
   jpeg_set_quality( jpeg.cinfoptr(), static_cast< int >( jpegLevel ), FALSE );
   jpeg.cinfo().density_unit = 2; // dots per cm
   jpeg.cinfo().X_density = static_cast< UINT16 >( 0.01 / image.PixelSize( 0 ).RemovePrefix().magnitude ); // let's assume it's meter
   jpeg.cinfo().Y_density = static_cast< UINT16 >( 0.01 / image.PixelSize( 1 ).RemovePrefix().magnitude );

   // Convert the image to uint8 if necessary
   Image image_u8 = image.QuickCopy();
   image_u8.Convert( DT_UINT8 );

   // Write data
   jpeg_start_compress( jpeg.cinfoptr(), TRUE );
   std::vector< JSAMPLE > buffer( image.Size( 0 ) * static_cast< dip::uint >( nchan ));
   dip::uint8* imagedata = static_cast< dip::uint8* >( image_u8.Origin() );
   auto stride = image_u8.Strides();
   auto tStride = image_u8.TensorStride();
   for( dip::uint ii = 0; ii < image.Size( 1 ); ++ii ) {
      JSAMPLE* outdata = buffer.data();
      dip::uint8* indata = imagedata;
      for( dip::uint jj = 0; jj < image.Size( 0 ); ++jj ) {
         for( int kk = 0; kk < nchan; ++kk ) {
            *outdata = *( indata + kk * tStride );
            ++outdata;
         }
         indata += stride[ 0 ];
      }
      outdata = buffer.data();
      jpeg_write_scanlines( jpeg.cinfoptr(), &outdata, 1 );
      imagedata += stride[ 1 ];
   }
   jpeg_finish_compress( jpeg.cinfoptr());
}

} // namespace dip

#else // DIP_CONFIG_HAS_JPEG

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

static char const* NOT_AVAILABLE = "DIPlib was compiled without JPEG support.";

FileInformation ImageReadJPEG( Image&, String const& ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadJPEGInfo( String const& ) {
   DIP_THROW( NOT_AVAILABLE );
}

bool ImageIsJPEG( String const& ) {
   DIP_THROW( NOT_AVAILABLE );
}

void ImageWriteJPEG( Image const&, String const&, dip::uint ) {
   DIP_THROW( NOT_AVAILABLE );
}

}

#endif // DIP_CONFIG_HAS_JPEG
