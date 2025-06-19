/*
 * (c)2024-2025, Cris Luengo.
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

#ifdef DIP_CONFIG_HAS_PNG

#include "diplib/file_io.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <utility>
#include <vector>

#include "diplib.h"

#include "spng.h"
#include "zlib.h"

namespace dip {

namespace {

#define PNG_THROW_READ_ERROR String msg = spng_strerror( ret ); msg = "Error reading PNG file: " + msg; DIP_THROW_RUNTIME( msg )
#define PNG_THROW_WRITE_ERROR String msg = spng_strerror( ret ); msg = "Error writing PNG file: " + msg; DIP_THROW_RUNTIME( msg )

class PngInput {
   public:
      explicit PngInput( String filename ) : filename_( std::move( filename )), infile_(std::fopen( filename_.c_str(), "rb" )) {
         if( infile_ == nullptr ) {
            filename_ = FileAppendExtension( filename_, "png" ); // Try with "png" extension
            infile_ = std::fopen( filename_.c_str(), "rb" );
            if( infile_ == nullptr ) {
               DIP_THROW_RUNTIME( "Could not open the specified PNG file" );
            }
         }
         ctx_ = spng_ctx_new( 0 );
         if( !ctx_ ) {
            DIP_THROW_RUNTIME( "Could not create a PNG context" );
         }
         // Set memory usage limits for storing standard and unknown chunks, this is important when reading untrusted files!
         std::size_t limit = 1024ul * 1024ul * 64ul;
         spng_set_chunk_limits( ctx_, limit, limit );
         // Set source file
         spng_set_png_file( ctx_, infile_ );
         // Read header
         if( int ret = spng_get_ihdr( ctx_, &ihdr_ )) {
            PNG_THROW_READ_ERROR;
         }
      }
      PngInput( void const* buffer, dip::uint length ) {
         DIP_THROW_IF( !buffer, "Input buffer pointer must be valid" );
         DIP_THROW_IF( length == 0, "Empty input buffer" );
         ctx_ = spng_ctx_new( 0 );
         if( !ctx_ ) {
            DIP_THROW_RUNTIME( "Could not create a PNG context" );
         }
         // Set memory usage limits for storing standard and unknown chunks, this is important when reading untrusted files!
         std::size_t limit = 1024ul * 1024ul * 64ul;
         spng_set_chunk_limits( ctx_, limit, limit );
         // Set source buffer
         spng_set_png_buffer( ctx_, buffer, length );
         // Read header
         if( int ret = spng_get_ihdr( ctx_, &ihdr_ )) {
            PNG_THROW_READ_ERROR;
         }
      }
      PngInput( PngInput const& ) = delete;
      PngInput( PngInput&& ) = delete;
      PngInput& operator=( PngInput const& ) = delete;
      PngInput& operator=( PngInput&& ) = delete;
      ~PngInput() {
         if( ctx_ ) {
            spng_ctx_free( ctx_ );
         }
         if( infile_ ) {
            std::fclose( infile_ );
         }
      }
      // Retrieve PNG context
      spng_ctx* Context() { return ctx_; }
      // Retrieve PNG header
      spng_ihdr const& Header() const { return ihdr_; }
      // Retrieve file name
      String const& FileName() const { return filename_; }
   private:
      String filename_;
      FILE* infile_ = nullptr;
      spng_ctx* ctx_ = nullptr;
      spng_ihdr ihdr_ = { 0, 0, 0, 0, 0, 0, 0 };
};

inline dip::uint max3( dip::uint a, dip::uint b, dip::uint c ) {
   return std::max( a, std::max( b, c ));
}

FileInformation GetPNGInfo( PngInput& png ) {
   FileInformation fileInformation;
   fileInformation.name = png.FileName();
   fileInformation.fileType = "PNG";
   fileInformation.numberOfImages = 1;
   dip::uint nChannels = 0;
   switch( png.Header().color_type ) {
      case SPNG_COLOR_TYPE_GRAYSCALE: nChannels = 1; break;
      case SPNG_COLOR_TYPE_TRUECOLOR:
      case SPNG_COLOR_TYPE_INDEXED: nChannels = 3; break;
      case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA: nChannels = 2; break;
      case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA: nChannels = 4; break;
      default:
         DIP_THROW_RUNTIME( "Error reading PNG: Illegal color type tag." );
   }
   spng_sbit sbit{ 0, 0, 0, 0, 0 };
   if( spng_get_sbit( png.Context(), &sbit ) == 0 ) {
      // We're ignoring the alpha channel bits.
      fileInformation.significantBits = nChannels < 3 ? sbit.grayscale_bits
                                                      : max3( sbit.red_bits, sbit.green_bits, sbit.blue_bits);
   } else {
      fileInformation.significantBits = png.Header().bit_depth;
   }
   if(( png.Header().bit_depth == 1 ) && ( nChannels == 1 )) {
      fileInformation.dataType = DT_BIN;
   } else {
      fileInformation.dataType = png.Header().bit_depth == 16 ? DT_UINT16 : DT_UINT8;
   }
   fileInformation.tensorElements = nChannels;
   fileInformation.colorSpace = nChannels == 3 ? "sRGB" : ( nChannels == 4 ? "sRGBA" : "" );
   fileInformation.sizes = { png.Header().width, png.Header().height };
   spng_phys phys{ 0, 0, 0 };
   int ret = spng_get_phys( png.Context(), &phys );
   if(( ret == 0 ) && ( phys.ppu_x > 0 ) && ( phys.ppu_y > 0 )) {
      PhysicalQuantity units = phys.unit_specifier ? PhysicalQuantity::Meter() : Units::Pixel();
      fileInformation.pixelSize = {{ ( units / static_cast< dfloat >( phys.ppu_x )).Normalize(),
                                     ( units / static_cast< dfloat >( phys.ppu_y )).Normalize() }};
   }
   return fileInformation;
}

void ImageReadPNG(
      Image& out,
      PngInput& png,
      FileInformation const& info
) {
   // Allocate image
   out.ReForge( info.sizes, info.tensorElements, info.dataType, Option::AcceptDataTypeChange::DONT_ALLOW );
   out.SetPixelSize( info.pixelSize );
   out.SetColorSpace( info.colorSpace );

   // Read data
   // We read in the format that's in the file, unless the file uses a color map, in which case we output RGB.
   int fmt = SPNG_FMT_PNG;
   if( png.Header().color_type == SPNG_COLOR_TYPE_INDEXED ) {
      fmt = SPNG_FMT_RGB8;
   } else if( png.Header().bit_depth < 8 ) {
      DIP_THROW_IF( png.Header().color_type != SPNG_COLOR_TYPE_GRAYSCALE, "Error reading PNG file: unsupported bit depth and color type combination" );
      fmt = SPNG_FMT_G8;
   }
   std::size_t image_size = 0;
   if( int ret = spng_decoded_image_size( png.Context(), fmt, &image_size )) {
      PNG_THROW_READ_ERROR;
   }
   DIP_THROW_IF( image_size != out.NumberOfSamples() * out.DataType().SizeOf(), "Incongruent buffer size" );
   if( out.HasNormalStrides() ) {
      // Decode the image in one go, this is the most common case.
      if( int ret = spng_decode_image( png.Context(), out.Origin(), image_size, fmt, 0)) {
         PNG_THROW_READ_ERROR;
      }
   } else {
      // Decode the image line by line, into a line buffer.
      // Note this is an odd case, it happens only when the caller has specific memory layout requirements
      // (i.e. through a pre-allocated, protected image, or through an external interface like MATLAB's).
      if( int ret = spng_decode_image( png.Context(), nullptr, 0, fmt, SPNG_DECODE_PROGRESSIVE )) {
         PNG_THROW_READ_ERROR;
      }
      std::size_t row_buffer_size = image_size / png.Header().height;
      Image row_buffer( { info.sizes[ 0 ] }, info.tensorElements, info.dataType );
      DIP_THROW_IF( row_buffer_size != row_buffer.NumberOfSamples() * row_buffer.DataType().SizeOf(), "Incongruent buffer size" );
      spng_row_info row_info = { 0, 0, 0, 0 };
      int ret = 0;
      while( !ret ) {
         ret = spng_get_row_info( png.Context(), &row_info );
         if( ret == 0 || ret == SPNG_EOI ) {
            ret = spng_decode_row( png.Context(), row_buffer.Origin(), row_buffer_size );
            if( ret == 0 || ret == SPNG_EOI ) {
               // I don't know how efficient this is, but it handles all possible memory organizations for us.
               out.At( Range(), Range( static_cast< dip::sint >( row_info.row_num ))) = row_buffer;
            }
         }
      }
      if( ret != SPNG_EOI ) {
         PNG_THROW_READ_ERROR;
      }
   }
}

class PngOutput {
   public:
      explicit PngOutput( String const& filename ) {
         if( FileHasExtension( filename )) {
            outfile_ = std::fopen(filename.c_str(), "wb");
         } else {
            outfile_ = std::fopen( FileAppendExtension( filename, "png" ).c_str(), "wb" );
         }
         if( outfile_ == nullptr ) {
            DIP_THROW_RUNTIME( "Could not open file for writing" );
         }
         ctx_ = spng_ctx_new( SPNG_CTX_ENCODER );
         if( !ctx_ ) {
            DIP_THROW_RUNTIME( "Could not create a PNG context" );
         }
         if( int ret = spng_set_png_file( ctx_, outfile_ )) {
            PNG_THROW_WRITE_ERROR;
         }
      }
      explicit PngOutput() : ctx_(spng_ctx_new( SPNG_CTX_ENCODER )) {
         if( int ret = spng_set_option( ctx_, SPNG_ENCODE_TO_BUFFER, 1 )) {
            PNG_THROW_WRITE_ERROR;
         }
      }
      PngOutput( PngOutput const& ) = delete;
      PngOutput( PngOutput&& ) = delete;
      PngOutput& operator=( PngOutput const& ) = delete;
      PngOutput& operator=( PngOutput&& ) = delete;
      ~PngOutput() {
         if( ctx_ ) {
            spng_ctx_free( ctx_ );
         }
         if( outfile_ ) {
            std::fclose( outfile_ );
         }
      }
      // Retrieve PNG context
      spng_ctx* Context() { return ctx_; }
   private:
      FILE* outfile_ = nullptr;
      spng_ctx* ctx_ = nullptr;
};

void ImageWritePNG(
      Image const& image,
      PngOutput& png,
      dip::sint compressionLevel,
      StringSet const& filterChoice,
      dip::uint significantBits
) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( image.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( image.TensorElements() > 4, "PNG files only support images with 1 to 4 tensor elements." );
   DIP_THROW_IF(( image.Size( 0 ) > std::numeric_limits< std::uint32_t >::max() ) ||
                ( image.Size( 1 ) > std::numeric_limits< std::uint32_t >::max() ),
                "PNG cannot write an image this large. Use TIFF or ICS instead." );
   bool isBinary = image.DataType().IsBinary() && image.IsScalar();

   // Convert the image to uint8 if necessary
   Image image_out = image.QuickCopy();
   if( image_out.DataType() != DT_UINT16 ) {
      image_out.Convert( DT_UINT8 ); // No-op if already UINT8.
   }

   // Set image properties
   spng_ihdr ihdr = { 0, 0, 0, 0, 0, 0, 0 };
   ihdr.width = static_cast< std::uint32_t >( image_out.Size( 0 )); // We already tested that this is OK.
   ihdr.height = static_cast< std::uint32_t >( image_out.Size( 1 ));
   switch( image_out.TensorElements() ) {
      case 1: ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE; break;
      case 2: ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE_ALPHA; break;
      case 3: ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR; break;
      case 4: ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA; break;
      default: DIP_THROW( E::NOT_REACHABLE );
   }
   ihdr.bit_depth = isBinary ? 1 : static_cast< std::uint8_t >( image_out.DataType().SizeOf() * 8 );
   if( int ret = spng_set_ihdr( png.Context(), &ihdr )) {
      PNG_THROW_WRITE_ERROR;
   }

   // Encoding option: format
   int fmt = SPNG_FMT_PNG;
   // Encoding option: compression level and strategy
   int compressionStrategy = Z_DEFAULT_STRATEGY;
   // Note that using Z_FILTERED often leads to slightly larger files and slightly slower compression, even for filtered images.
   if( compressionLevel == -1 ) {
      compressionStrategy = Z_RLE;
      // The compression level is ignored in this mode
   } else {
      if( int ret = spng_set_option( png.Context(), SPNG_IMG_COMPRESSION_LEVEL, static_cast< int >( compressionLevel ))) {
         PNG_THROW_WRITE_ERROR;
      }
   }
   if( int ret = spng_set_option( png.Context(), SPNG_IMG_COMPRESSION_STRATEGY, compressionStrategy )) {
      PNG_THROW_WRITE_ERROR;
   }
   // Encoding option: filter choice
   int filterChoiceInt = 0;
   if( compressionLevel == 0 ) {
      // Don't use filters if we're not going to be compressing, they just waste time!
      filterChoiceInt = SPNG_DISABLE_FILTERING;
   } else {
      if( filterChoice.find( S::DISABLE ) != filterChoice.end() ) {
         DIP_THROW_IF( filterChoice.size() != 1, "The option 'disable' cannot be combined with other options." );
         filterChoiceInt = SPNG_DISABLE_FILTERING;
      }
      else if( filterChoice.find( S::ALL ) != filterChoice.end() ) {
         DIP_THROW_IF( filterChoice.size() != 1, "The option 'all' cannot be combined with other options." );
         filterChoiceInt = SPNG_FILTER_CHOICE_ALL;
      } else {
         for( auto const& opt : filterChoice ) {
            if( opt == S::NONE ) { filterChoiceInt |= SPNG_FILTER_CHOICE_NONE; }
            else if( opt == S::SUB ) { filterChoiceInt |= SPNG_FILTER_CHOICE_SUB; }
            else if( opt == S::UP ) { filterChoiceInt |= SPNG_FILTER_CHOICE_UP; }
            else if( opt == S::AVG ) { filterChoiceInt |= SPNG_FILTER_CHOICE_AVG; }
            else if( opt == S::PAETH ) { filterChoiceInt |= SPNG_FILTER_CHOICE_PAETH; }
         }
      }
   }
   if( int ret = spng_set_option( png.Context(), SPNG_FILTER_CHOICE, filterChoiceInt )) {
      PNG_THROW_WRITE_ERROR;
   }

   // Set number of significant bits if necessary
   if( significantBits > 0 ) {
      spng_sbit sbit{
         static_cast< std::uint8_t >( significantBits ),
         static_cast< std::uint8_t >( significantBits ),
         static_cast< std::uint8_t >( significantBits ),
         static_cast< std::uint8_t >( significantBits ),
         static_cast< std::uint8_t >( significantBits )
      };
      if( int ret = spng_set_sbit( png.Context(), &sbit )) {
         PNG_THROW_WRITE_ERROR;
      }
   }

   // Set pixel size if necessary
   if( image.HasPixelSize() ) {
      auto const& px = image.PixelSize();
      spng_phys phys{ 0, 0, 0 };
      if( px[ 0 ].units.HasSameDimensions( Units::Meter() ) &&
          px[ 1 ].units.HasSameDimensions( Units::Meter() )) {
         phys.unit_specifier = 1;
         phys.ppu_x = static_cast< std::uint32_t >( std::round(( Units::Meter() / px[ 0 ] ).RemovePrefix().magnitude ));
         phys.ppu_y = static_cast< std::uint32_t >( std::round(( Units::Meter() / px[ 1 ] ).RemovePrefix().magnitude ));
      } else {
         phys.ppu_x = static_cast< std::uint32_t >( std::round( 1 / px[ 0 ].magnitude ));
         phys.ppu_y = static_cast< std::uint32_t >( std::round( 1 / px[ 1 ].magnitude ));
      }
      if( int ret = spng_set_phys( png.Context(), &phys )) {
         PNG_THROW_WRITE_ERROR;
      }
   }

   // Write data
   if( isBinary ) {
      // For binary data we need to put 8 pixels into each byte
      // Here we know for sure that we have a single channel.
      if( int ret = spng_encode_image( png.Context(), nullptr, 0, fmt, SPNG_ENCODE_PROGRESSIVE | SPNG_ENCODE_FINALIZE )) {
         PNG_THROW_WRITE_ERROR;
      }
      dip::uint row_length = image_out.Size( 0 );
      dip::uint row_buffer_size = div_ceil( row_length, dip::uint( 8 ));
      dip::uint pixels_in_last_byte = row_length - ( row_buffer_size - 1 ) * 8;
      dip::uint n_rows = image_out.Size( 1 );
      std::size_t image_size = 0;
      if( int ret = spng_decoded_image_size( png.Context(), fmt, &image_size )) {
         PNG_THROW_WRITE_ERROR;
      }
      DIP_THROW_IF( row_buffer_size != image_size / n_rows, "Incongruent buffer size" );
      std::vector< dip::uint8 > row_buffer( row_buffer_size, 0 );
      int ret = 0;
      dip::bin const* img_ptr = static_cast< dip::bin const* >( image_out.Origin() );
      dip::IntegerArray const& strides = image_out.Strides();
      for( dip::uint ii = 0; ii < n_rows; ++ii ) {
         dip::bin const* line_ptr = img_ptr;
         dip::uint jj = 0;
         for( ; jj < row_buffer_size - 1; ++jj ) {
            dip::uint8 byte = 0;
            dip::uint8 bitmask = 128;
            for( dip::uint kk = 0; kk < 8; ++kk, ++line_ptr ) {
               if( *line_ptr ) {
                  byte |= bitmask;
               }
               bitmask = bitmask >> 1;
            }
            row_buffer[ jj ] = byte;
         }
         {
            // Last, possibly incomplete byte
            dip::uint8 byte = 0;
            dip::uint8 bitmask = 128;
            for( dip::uint kk = 0; kk < pixels_in_last_byte; ++kk, ++line_ptr ) {
               if( *line_ptr ) {
                  byte |= bitmask;
               }
               bitmask = bitmask >> 1;
            }
            row_buffer[ jj ] = byte;
         }
         ret = spng_encode_row( png.Context(), row_buffer.data(), row_buffer_size );
         if( ret ) {
            break;
         }
         img_ptr += strides[ 1 ];
      }
      if( ret != SPNG_EOI ) {
         PNG_THROW_WRITE_ERROR;
      }
   } else if( image_out.HasNormalStrides() ) {
      // We can write directly with a single function call
      std::size_t length = image_out.NumberOfSamples() * image_out.DataType().SizeOf();
      if( int ret = spng_encode_image( png.Context(), image_out.Origin(), length, fmt, SPNG_ENCODE_FINALIZE )) {
         PNG_THROW_WRITE_ERROR;
      }
   } else {
      // For non-normal strides, we copy each image line to a buffer and write line by line
      if( int ret = spng_encode_image( png.Context(), nullptr, 0, fmt, SPNG_ENCODE_PROGRESSIVE | SPNG_ENCODE_FINALIZE )) {
         PNG_THROW_WRITE_ERROR;
      }
      std::size_t image_size = 0;
      if( int ret = spng_decoded_image_size( png.Context(), fmt, &image_size )) {
         PNG_THROW_WRITE_ERROR;
      }
      dip::uint n_rows = image_out.Size( 1 );
      std::size_t row_buffer_size = image_size / n_rows;
      Image row_buffer( { image_out.Size( 0 ) }, image_out.TensorElements(), image_out.DataType() );
      DIP_THROW_IF( row_buffer_size != row_buffer.NumberOfSamples() * row_buffer.DataType().SizeOf(), "Incongruent buffer size" );
      int ret = 0;
      for( dip::uint ii = 0; ii < n_rows; ++ii ) {
         row_buffer.Copy( image_out.At( Range(), Range( static_cast< dip::sint >( ii ))) );
         ret = spng_encode_row( png.Context(), row_buffer.Origin(), row_buffer_size );
         if( ret ) {
            break;
         }
      }
      if( ret != SPNG_EOI ) {
         PNG_THROW_WRITE_ERROR;
      }
   }
}

} // namespace

FileInformation ImageReadPNG( Image& out, String const& filename ) {
   PngInput png( filename );
   FileInformation info = GetPNGInfo( png );
   ImageReadPNG( out, png, info );
   return info;
}

FileInformation ImageReadPNGInfo( String const& filename ) {
   PngInput png( filename );
   FileInformation info = GetPNGInfo( png );
   return info;
}

bool ImageIsPNG( String const& filename ) {
   try {
      PngInput png( filename );
   } catch( ... ) {
      return false;
   }
   return true;
}

FileInformation ImageReadPNG( Image& out, void const* buffer, dip::uint length ) {
   PngInput png( buffer, length );
   FileInformation info = GetPNGInfo( png );
   ImageReadPNG( out, png, info );
   return info;
}

FileInformation ImageReadPNGInfo( void const* buffer, dip::uint length ) {
   PngInput png( buffer, length );
   FileInformation info = GetPNGInfo( png );
   return info;
}

void ImageWritePNG(
      Image const& image,
      String const& filename,
      dip::sint compressionLevel,
      StringSet const& filterChoice,
      dip::uint significantBits
) {
   PngOutput png( filename );
   ImageWritePNG( image, png, compressionLevel, filterChoice, significantBits );
}

void ImageWritePNG(
      Image const& image,
      OutputBuffer& buffer,
      dip::sint compressionLevel,
      StringSet const& filterChoice,
      dip::uint significantBits
) {
   // libspng uses an internal buffer to write to -- can we subvert that to avoid the copy?
   PngOutput png;
   ImageWritePNG( image, png, compressionLevel, filterChoice, significantBits );
   dip::uint buf_len = 0;
   int ret = 0;
   void* buf_ptr = spng_get_png_buffer( png.Context(), &buf_len, &ret );
   if( ret ) {
      PNG_THROW_WRITE_ERROR;
   }
   // NOTE! we now own `buf_ptr`!
   try {
      buffer.assure_capacity( buf_len );
      DIP_ASSERT( buffer.capacity() >= buf_len );
      buffer.set_size( buf_len );
      std::copy_n( static_cast< dip::uint8* >( buf_ptr ), buf_len, buffer.data() );
      std::free( buf_ptr );
   } catch ( ... ) {
      std::free( buf_ptr );
      throw;
   }
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"
#include "diplib/random.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE( "[DIPlib] testing PNG file reading and writing" ) {
   dip::Image image( { 17, 7 }, 4, dip::DT_UINT8 );
   image.Fill( 0 );
   dip::Random rng;
   dip::UniformNoise( image, image, rng, 0, 255 );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 8 * dip::Units::Micrometer(), 400 * dip::Units::Nanometer() } );

   dip::ImageWritePNG( image, "test1.png" );
   dip::Image result = dip::ImageReadPNG( "test1" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));
   DOCTEST_CHECK( image.PixelSize() == result.PixelSize() );

   // Try reading it into an image with non-standard strides
   result.Strip();
   result.SetStrides( { static_cast< dip::sint >( result.Size( 1 )), 1 } );
   result.SetTensorStride( static_cast< dip::sint >( result.NumberOfPixels() ));
   result.Forge();
   result.Protect();
   dip::ImageReadPNG( result, "test1" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));
   DOCTEST_CHECK( image.PixelSize() == result.PixelSize() );
   result.Protect( false );

   // Turn it on its side so the image to write has non-standard strides
   image.SwapDimensions( 0, 1 );
   dip::ImageWritePNG( image, "test2.png" );
   result = dip::ImageReadPNG( "test2" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));
   image.SwapDimensions( 0, 1 ); // swap back

   // Write 3-channel image (note non-standard strides!)
   image = image[ dip::Range( 0, 2 ) ];
   dip::ImageWritePNG( image, "test3.png" );
   result = dip::ImageReadPNG( "test3" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));

   // Write 2-channel image (note non-standard strides!)
   image = image[ dip::Range( 0, 1 ) ];
   dip::ImageWritePNG( image, "test4.png" );
   result = dip::ImageReadPNG( "test4" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));

   // Write scalar image (note non-standard strides!)
   image = image[ 0 ];
   dip::ImageWritePNG( image, "test5.png" );
   result = dip::ImageReadPNG( "test5" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));

   // Write 16-bit scalar image
   image = dip::Image( { 19, 13 }, 1, dip::DT_UINT16 );
   image.Fill( 0 );
   dip::UniformNoise( image, image, rng, 0, 1024 );
   dip::ImageWritePNG( image, "test6.png", 6, { "all" }, 10 );
   auto info = dip::ImageReadPNG( result, "test6" );
   DOCTEST_CHECK( result.DataType() == dip::DT_UINT16 );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));
   DOCTEST_CHECK( info.significantBits == 10 );

   // Write binary scalar image
   image = dip::Image( { 19, 13 }, 1, dip::DT_BIN );
   image.Fill( 0 );
   dip::BinaryNoise( image, image, rng, 0.33, 0.33 );
   dip::ImageWritePNG( image, "test7.png" );
   info = dip::ImageReadPNG( result, "test7" );
   DOCTEST_CHECK( result.DataType() == dip::DT_BIN );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));
   DOCTEST_CHECK( info.significantBits == 1 );

   // Write and read from buffer
   auto buffer = dip::ImageWritePNG( image );
   info = dip::ImageReadPNG( result, buffer.data(), buffer.size() );
   DOCTEST_CHECK( result.DataType() == dip::DT_BIN );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result ));
   DOCTEST_CHECK( info.significantBits == 1 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST

#else // DIP_CONFIG_HAS_PNG

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

constexpr char const* NOT_AVAILABLE = "DIPlib was compiled without PNG support.";

FileInformation ImageReadPNG( Image& /*out*/, String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadPNGInfo( String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

bool ImageIsPNG( String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadPNG( Image& /*out*/, void const* /*buffer*/, dip::uint /*length*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadPNGInfo( void const* /*buffer*/, dip::uint /*length*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

void ImageWritePNG(
      Image const& /*image*/,
      String const& /*filename*/,
      dip::sint /*compressionLevel*/,
      StringSet const& /*filterChoice*/,
      dip::uint /*significantBits*/
) {
   DIP_THROW( NOT_AVAILABLE );
}

void ImageWritePNG(
      Image const& /*image*/,
      OutputBuffer& /*buffer*/,
      dip::sint /*compressionLevel*/,
      StringSet const& /*filterChoice*/,
      dip::uint /*significantBits*/
) {
   DIP_THROW( NOT_AVAILABLE );
}

}

#endif // DIP_CONFIG_HAS_PNG
