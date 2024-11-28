/*
 * (c)2019-2023, Cris Luengo.
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

#include "diplib/file_io.h"

#include <cstdio>
#include <cstring>
#include <utility>
#include <vector>

#include "diplib.h"

#include "jpeglib.h"
#include "jerror.h"
#include <csetjmp>

// JPEG error handling stuff - modified from example.c in libjpeg source
namespace {

extern "C" {
   void my_error_exit( j_common_ptr cinfo ); // NOLINT(*-use-anonymous-namespace)
}


struct my_error_mgr {
   struct jpeg_error_mgr pub{};  // "public" fields
   std::jmp_buf setjmp_buffer{}; // for return to caller
   dip::String& message;         // error message

   my_error_mgr( dip::String& str ) : message( str ) {}
};

void my_error_exit( j_common_ptr cinfo ) {
   // cinfo->err really points to a my_error_mgr struct, so coerce pointer
   auto* myerr = reinterpret_cast< my_error_mgr* >( cinfo->err );
   // Format error message and store, so that we can throw the proper message later
   myerr->message.resize( JMSG_LENGTH_MAX );
   cinfo->err->format_message( cinfo, &myerr->message[0] );
   // Return control to the setjmp point
   std::longjmp( myerr->setjmp_buffer, 1 );
}

#define DIP_DECLARE_JPEG_EXIT( message ) \
std::jmp_buf setjmp_buffer; dip::String error_msg; if( setjmp( setjmp_buffer )) { DIP_THROW_RUNTIME(( message ) + error_msg ); }

} // namespace

namespace dip {

namespace {

constexpr char const* ERROR_READING_JPEG = "Error reading JPEG file: ";

class JpegInput {
   public:
      JpegInput( String filename, std::jmp_buf const& setjmp_buffer, String& error_msg )
            : filename_( std::move( filename )), infile_( std::fopen( filename_.c_str(), "rb" )), jerr_( error_msg ) {
         if( infile_ == nullptr ) {
            filename_ = FileAppendExtension( filename_, "jpg" ); // Try with "jpg" extension
            infile_ = std::fopen( filename_.c_str(), "rb" );
            if( infile_ == nullptr ) {
               filename_.back() = 'e';
               filename_.push_back( 'g' ); // Try with "jpeg" extension
               infile_ = std::fopen( filename_.c_str(), "rb" );
            }
         }
         if( infile_ == nullptr ) {
            DIP_THROW_RUNTIME( "Could not open the specified JPEG file" );
         }
         cinfo_.err = jpeg_std_error( &jerr_.pub );
         jerr_.pub.error_exit = my_error_exit;
         std::memcpy( jerr_.setjmp_buffer, setjmp_buffer, sizeof( setjmp_buffer ));
         jpeg_create_decompress( &cinfo_ );
         initialized_ = true;
         jpeg_stdio_src( &cinfo_, infile_ );
         jpeg_read_header( &cinfo_, TRUE );
      }
      JpegInput( void const* buffer, dip::uint length, std::jmp_buf const& setjmp_buffer, String& error_msg )
            : jerr_( error_msg ) {
         DIP_THROW_IF( !buffer, "Input buffer pointer must be valid" );
         DIP_THROW_IF( length == 0, "Empty input buffer" );
         cinfo_.err = jpeg_std_error( &jerr_.pub );
         jerr_.pub.error_exit = my_error_exit;
         std::memcpy( jerr_.setjmp_buffer, setjmp_buffer, sizeof( setjmp_buffer ));
         jpeg_create_decompress( &cinfo_ );
         initialized_ = true;
         jpeg_mem_src( &cinfo_, static_cast< unsigned char const* >( buffer ), length );
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
      jpeg_decompress_struct cinfo_{};
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

void ImageReadJPEG( Image& out, JpegInput& jpeg, FileInformation const& info ) {
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
}

// Code below up to and including InitMemoryDestination() (originally jpeg_mem_dest()) were adapted from libjpeg/jdatadst.c

constexpr dip::uint OUTPUT_BUF_SIZE = 4096;

extern "C" {
   void MemDest_Initialize( j_compress_ptr cinfo );
   boolean MemDest_EmptyBuffer( j_compress_ptr cinfo );
   void MemDest_Finalize( j_compress_ptr cinfo );
}

struct MemoryDestinationManager {
   jpeg_destination_mgr pub;   // public fields
   OutputBuffer& buffer;
};

void MemDest_Initialize( j_compress_ptr /*cinfo*/ ) {
  // no work necessary here
}

boolean MemDest_EmptyBuffer( j_compress_ptr cinfo ) {
   // This is called if the buffer has been filled up. We double the buffer size, and set the
   // output pointer and size to the second half of the new buffer.
   MemoryDestinationManager* dest = reinterpret_cast< MemoryDestinationManager* >( cinfo->dest );
   dip::uint curr_size = dest->buffer.capacity();
   dest->buffer.assure_capacity( curr_size * 2 );
   dest->pub.next_output_byte = dest->buffer.data() + curr_size;
   dest->pub.free_in_buffer = dest->buffer.capacity() - curr_size;
   return TRUE;
}

void MemDest_Finalize( j_compress_ptr cinfo ) {
   // To finish off, we set the buffer size to the used portion
   MemoryDestinationManager* dest = reinterpret_cast< MemoryDestinationManager* >( cinfo->dest );
   dip::sint curr_size = dest->pub.next_output_byte - dest->buffer.data();
   DIP_ASSERT( curr_size > 0 );
   dest->buffer.set_size( static_cast< dip::uint >( curr_size ));
}

void InitMemoryDestination( j_compress_ptr cinfo, OutputBuffer& buffer ) {
   buffer.assure_capacity( OUTPUT_BUF_SIZE );
   MemoryDestinationManager* dest = new MemoryDestinationManager{
      {
         buffer.data(),
         buffer.capacity(),
         MemDest_Initialize,
         MemDest_EmptyBuffer,
         MemDest_Finalize
      },
      buffer
   };
   cinfo->dest = reinterpret_cast< jpeg_destination_mgr* >( dest );
}

void CleanupMemoryDestination( j_compress_ptr cinfo ) {
   if( cinfo->dest ) {
      delete reinterpret_cast< MemoryDestinationManager* >( cinfo->dest );
      cinfo->dest = nullptr;
   }
}

class JpegOutput {
   public:
      JpegOutput( String const& filename, std::jmp_buf const& setjmp_buffer, String& error_msg ) : jerr_( error_msg ) {
         // Open the file for writing
         if( FileHasExtension( filename )) {
            outfile_ = std::fopen(filename.c_str(), "wb");
         } else {
            outfile_ = std::fopen( FileAppendExtension( filename, "jpg" ).c_str(), "wb" );
         }
         if( outfile_ == nullptr ) {
            DIP_THROW_RUNTIME( "Could not open file for writing" );
         }
         cinfo_.err = jpeg_std_error( &jerr_.pub );
         jerr_.pub.error_exit = my_error_exit;
         std::memcpy( jerr_.setjmp_buffer, setjmp_buffer, sizeof( setjmp_buffer ));
         jpeg_create_compress( &cinfo_ );
         cinfo_.dest = nullptr;
         initialized_ = true;
         jpeg_stdio_dest( &cinfo_, outfile_ );
      }
      JpegOutput( OutputBuffer& buffer, std::jmp_buf const& setjmp_buffer, String& error_msg ) : jerr_( error_msg ) {
         // Open the file for writing
         cinfo_.err = jpeg_std_error( &jerr_.pub );
         jerr_.pub.error_exit = my_error_exit;
         std::memcpy( jerr_.setjmp_buffer, setjmp_buffer, sizeof( setjmp_buffer ));
         jpeg_create_compress( &cinfo_ );
         cinfo_.dest = nullptr;
         initialized_ = true; // NOLINT(*-prefer-member-initializer)
         InitMemoryDestination( &cinfo_, buffer );
         mem_buffer_ = true;
      }
      JpegOutput( JpegOutput const& ) = delete;
      JpegOutput( JpegOutput&& ) = delete;
      JpegOutput& operator=( JpegOutput const& ) = delete;
      JpegOutput& operator=( JpegOutput&& ) = delete;
      ~JpegOutput() {
         if( mem_buffer_ ) {
            CleanupMemoryDestination( &cinfo_ );
         }
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
      std::vector< dip::uint8 > buffer_;
      jpeg_compress_struct cinfo_{};
      my_error_mgr jerr_;
      bool initialized_ = false;
      bool mem_buffer_ = false;
};

void ImageWriteJPEG( Image const& image, JpegOutput& jpeg, dip::uint jpegLevel ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( image.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   int nchan = static_cast< int >( image.TensorElements() );
   DIP_THROW_IF(( nchan != 1 ) && ( nchan != 3 ), "Can only write JPEG image with 1 or 3 tensor elements" );

   // Set image properties
   jpeg.cinfo().image_width = static_cast< JDIMENSION >( image.Size( 0 ));
   jpeg.cinfo().image_height = static_cast< JDIMENSION >( image.Size( 1 ));
   jpeg.cinfo().input_components = nchan;
   jpeg.cinfo().in_color_space = nchan > 1 ? JCS_RGB : JCS_GRAYSCALE;
   jpeg_set_defaults( jpeg.cinfoptr() );
   jpeg_set_quality( jpeg.cinfoptr(), static_cast< int >( clamp< dip::uint >( jpegLevel, 1, 100 )), FALSE );
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

} // namespace

FileInformation ImageReadJPEG( Image& out, String const& filename ) {
   DIP_DECLARE_JPEG_EXIT( ERROR_READING_JPEG );
   JpegInput jpeg( filename, setjmp_buffer, error_msg );
   FileInformation info = GetJPEGInfo( jpeg );
   ImageReadJPEG( out, jpeg, info );
   return info;
}

FileInformation ImageReadJPEGInfo( String const& filename ) {
   DIP_DECLARE_JPEG_EXIT( ERROR_READING_JPEG );
   JpegInput jpeg( filename, setjmp_buffer, error_msg );
   FileInformation info = GetJPEGInfo( jpeg );
   return info;
}

bool ImageIsJPEG( String const& filename ) {
   try {
      DIP_DECLARE_JPEG_EXIT( ERROR_READING_JPEG );
      JpegInput jpeg( filename, setjmp_buffer, error_msg );
   } catch( ... ) {
      return false;
   }
   return true;
}

FileInformation ImageReadJPEG( Image& out, void const* buffer, dip::uint length ) {
   DIP_DECLARE_JPEG_EXIT( ERROR_READING_JPEG );
   JpegInput jpeg( buffer, length, setjmp_buffer, error_msg );
   FileInformation info = GetJPEGInfo( jpeg );
   ImageReadJPEG( out, jpeg, info );
   return info;
}

FileInformation ImageReadJPEGInfo( void const* buffer, dip::uint length ) {
   DIP_DECLARE_JPEG_EXIT( ERROR_READING_JPEG );
   JpegInput jpeg( buffer, length, setjmp_buffer, error_msg );
   FileInformation info = GetJPEGInfo( jpeg );
   return info;
}

void ImageWriteJPEG( Image const& image, String const& filename, dip::uint jpegLevel ) {
   DIP_DECLARE_JPEG_EXIT( "Error writing JPEG file: " );
   JpegOutput jpeg( filename, setjmp_buffer, error_msg );
   ImageWriteJPEG( image, jpeg, jpegLevel );
}

void ImageWriteJPEG( Image const& image, OutputBuffer& buffer, dip::uint jpegLevel ) {
   DIP_DECLARE_JPEG_EXIT( "Error writing JPEG file: " );
   JpegOutput jpeg( buffer, setjmp_buffer, error_msg );
   ImageWriteJPEG( image, jpeg, jpegLevel );
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE( "[DIPlib] testing JPEG file reading and writing" ) {
   dip::Image image( { 119, 83 }, 3, dip::DT_UINT8 );
   image.Fill( 0 );
   dip::DrawBandlimitedBall( image, 70, { 60, 40 }, { 120, 200, 50 } );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 8 * dip::Units::Micrometer(), 400 * dip::Units::Nanometer() } );

   dip::ImageWriteJPEG( image, "test1.jpg", 100 );
   dip::Image result = dip::ImageReadJPEG( "test1" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::APPROX, 12 ));
   DOCTEST_CHECK( image.PixelSize() == result.PixelSize() );

   // Try reading it into an image with non-standard strides
   result.Strip();
   result.SetStrides( { static_cast< dip::sint >( result.Size( 1 )), 1 } );
   result.SetTensorStride( static_cast< dip::sint >( result.NumberOfPixels() ));
   result.Forge();
   result.Protect();
   dip::ImageReadJPEG( result, "test1" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::APPROX, 12 ));
   DOCTEST_CHECK( image.PixelSize() == result.PixelSize() );
   result.Protect( false );

   // Turn it on its side so the image to write has non-standard strides
   image.SwapDimensions( 0, 1 );
   dip::ImageWriteJPEG( image, "test2.jpg", 100 );
   result = dip::ImageReadJPEG( "test2" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::APPROX, 12 ));
   image.SwapDimensions( 0, 1 ); // swap back

   // We cannot write a 2-channel image to JPEG
   DOCTEST_CHECK_THROWS( dip::ImageWriteJPEG( image[ dip::Range( 0, 1 ) ], "fail.jpg" ));

   // Write scalar image (note non-standard strides!)
   image = image[ 0 ];
   dip::ImageWriteJPEG( image, "test3.jpg", 100 );
   result = dip::ImageReadJPEG( "test3" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::APPROX, 12 ));

   // Write and read from buffer
   auto buffer = dip::ImageWriteJPEG( image, 100 );
   auto info = dip::ImageReadJPEG( result, buffer.data(), buffer.size() );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::APPROX, 12 ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST

#else // DIP_CONFIG_HAS_JPEG

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

constexpr char const* NOT_AVAILABLE = "DIPlib was compiled without JPEG support.";

FileInformation ImageReadJPEG( Image& /*out*/, String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadJPEGInfo( String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

bool ImageIsJPEG( String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadJPEG( Image& /*out*/, void const* /*buffer*/, dip::uint /*length*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadJPEGInfo( void const* /*buffer*/, dip::uint /*length*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

void ImageWriteJPEG( Image const& /*image*/, String const& /*filename*/, dip::uint /*jpegLevel*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

void ImageWriteJPEG( Image const& /*image*/, OutputBuffer& /*buffer*/, dip::uint /*jpegLevel*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

}

#endif // DIP_CONFIG_HAS_JPEG
