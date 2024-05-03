/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022, Cris Luengo.
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

#include <sstream>

#include "pydip.h"
#include "diplib/color.h"
#include "diplib/display.h"
#include "diplib/file_io.h"
#undef DIP_CONFIG_HAS_DIPJAVAIO  // We want the version of dip::ImageRead() here to not use DIPjavaio.
#include "diplib/simple_file_io.h"
#include "diplib/geometry.h"
#include "diplib/testing.h"

namespace {

dip::ColorSpaceManager& colorSpaceManager() {
   static dip::ColorSpaceManager manager;
   return manager;
}

dip::Image ImageDisplay(
      dip::Image const& input,
      dip::FloatArray const& range,
      dip::String const& mappingMode,
      dip::String const& complexMode,
      dip::String const& projectionMode,
      dip::UnsignedArray const& coordinates,
      dip::uint dim1,
      dip::uint dim2
) {
   dip::ImageDisplay imageDisplay( input, &colorSpaceManager() );
   if( !mappingMode.empty() ) {
      imageDisplay.SetRange( mappingMode );
   } else if( range.empty() ) {
      imageDisplay.SetRange( "lin" );
   } else {
      DIP_THROW_IF( range.size() != 2, "Range must be a 2-tuple" );
      imageDisplay.SetRange( dip::ImageDisplay::Limits{ range[ 0 ], range[ 1 ] } );
   }
   imageDisplay.SetComplexMode( complexMode );
   if( input.Dimensionality() > 2 ) {
      imageDisplay.SetGlobalStretch( true );
      imageDisplay.SetProjectionMode( projectionMode );
   }
   if( input.Dimensionality() >= 2 ) {
      if( !coordinates.empty() ) {
         imageDisplay.SetCoordinates( coordinates );
      }
      imageDisplay.SetDirection( dim1, dim2 );
   }
   return imageDisplay.Output();
}

struct PointerAndLength {
   char* ptr;
   dip::uint length;
};

PointerAndLength GetBytesPointerAndLength( py::bytes* buffer ) {
   char* ptr = nullptr;
   Py_ssize_t length = 0;
   if( PyBytes_AsStringAndSize( buffer->ptr(), &ptr, &length ) != 0 ) {
      throw dip::RunTimeError( "Failed to get pointer to data in Python bytes object" );
   }
   return { ptr, static_cast< dip::uint >( length ) };
}

} // namespace

void init_assorted( py::module& m ) {

   // diplib/color.h
   auto mcol = m.def_submodule( "ColorSpaceManager",
                                "A Tool to convert images from one color space to another.\n\n"
                                "This is a submodule that uses a static `dip::ColorSpaceManager` object.\n"
                                "Functions defined in this module correspond to the object member functions\n"
                                "in C++." );
   mcol.def( "Convert", []( dip::Image const& in, dip::String const& colorSpaceName ) { return colorSpaceManager().Convert( in, colorSpaceName ); },
             "in"_a, "colorSpaceName"_a = "RGB" );
   mcol.def( "Convert", []( dip::Image const& in, dip::Image& out, dip::String const& colorSpaceName ) { colorSpaceManager().Convert( in, out, colorSpaceName ); },
             "in"_a, py::kw_only(), "out"_a, "colorSpaceName"_a = "RGB" );
   mcol.def( "IsDefined", []( dip::String const& colorSpaceName ) { return colorSpaceManager().IsDefined( colorSpaceName ); },
             "colorSpaceName"_a = "RGB" );
   mcol.def( "NumberOfChannels", []( dip::String const& colorSpaceName ) { return colorSpaceManager().NumberOfChannels( colorSpaceName ); },
             "colorSpaceName"_a = "RGB" );
   mcol.def( "CanonicalName", []( dip::String const& colorSpaceName ) { return colorSpaceManager().CanonicalName( colorSpaceName ); },
             "colorSpaceName"_a = "RGB" );
   // TODO: WhitePoint stuff

   // diplib/display.h
   m.def( "ImageDisplay", []( dip::Image const& input,
                              dip::FloatArray const& range,
                              dip::String const& complexMode,
                              dip::String const& projectionMode,
                              dip::UnsignedArray const& coordinates,
                              dip::uint dim1,
                              dip::uint dim2 ) {
             return ImageDisplay( input, range, {}, complexMode, projectionMode, coordinates, dim1, dim2 );
          }, "in"_a, "range"_a = dip::FloatArray{}, "complexMode"_a = "abs", "projectionMode"_a = "mean", "coordinates"_a = dip::UnsignedArray{}, "dim1"_a = 0, "dim2"_a = 1,
          "A function that performs the functionality of the `dip::ImageDisplay` class." );
   m.def( "ImageDisplay", []( dip::Image const& input,
                              dip::String const& mappingMode,
                              dip::String const& complexMode,
                              dip::String const& projectionMode,
                              dip::UnsignedArray const& coordinates,
                              dip::uint dim1,
                              dip::uint dim2 ) {
             return ImageDisplay( input, {}, mappingMode, complexMode, projectionMode, coordinates, dim1, dim2 );
          }, "in"_a, "mappingMode"_a, "complexMode"_a = "abs", "projectionMode"_a = "mean", "coordinates"_a = dip::UnsignedArray{}, "dim1"_a = 0, "dim2"_a = 1,
          "Overload of the above that allows a `mappingMode` to be given instead of\n"
          "a `range`." );
   m.def( "ApplyColorMap", py::overload_cast< dip::Image const&, dip::String const& >( &dip::ApplyColorMap ),
          "in"_a, "colorMap"_a = "grey" );
   m.def( "ApplyColorMap", py::overload_cast< dip::Image const&, dip::Image&, dip::String const& >( &dip::ApplyColorMap ),
          "in"_a, py::kw_only(), "out"_a, "colorMap"_a = "grey" );
   m.def( "Overlay", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image::Pixel const& >( &dip::Overlay ),
          "in"_a, "overlay"_a, "color"_a = dip::Image::Pixel{ 255, 0, 0 } );
   m.def( "Overlay", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::Image::Pixel const& >( &dip::Overlay ),
          "in"_a, "overlay"_a, py::kw_only(), "out"_a, "color"_a = dip::Image::Pixel{ 255, 0, 0 } );
   m.def( "MarkLabelEdges", py::overload_cast< dip::Image const&, dip::uint >( &dip::MarkLabelEdges ),
          "in"_a, "factor"_a = 2 );
   m.def( "MarkLabelEdges", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::MarkLabelEdges ),
          "in"_a, py::kw_only(), "out"_a, "factor"_a = 2 );

   // diplib/file_io.h
   m.def( "ImageReadICS", []( dip::String const& filename, dip::RangeArray const& roi, dip::Range const& channels, dip::String const& mode ) {
      auto out = dip::ImageReadICS( filename, roi, channels, mode );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageReadICS", []( dip::Image& out, dip::String const& filename, dip::RangeArray const& roi, dip::Range const& channels, dip::String const& mode ) {
      auto fi = dip::ImageReadICS( out, filename, roi, channels, mode );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageReadICS", []( dip::String const& filename, dip::UnsignedArray const& origin, dip::UnsignedArray const& sizes, dip::UnsignedArray const& spacing, dip::Range const& channels, dip::String const& mode ) {
      auto out = dip::ImageReadICS( filename, origin, sizes, spacing, channels, mode );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a, "origin"_a = dip::UnsignedArray{}, "sizes"_a = dip::UnsignedArray{}, "spacing"_a = dip::UnsignedArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageReadICS", []( dip::Image& out, dip::String const& filename, dip::UnsignedArray const& origin, dip::UnsignedArray const& sizes, dip::UnsignedArray const& spacing, dip::Range const& channels, dip::String const& mode ) {
      auto fi = dip::ImageReadICS( out, filename, origin, sizes, spacing, channels, mode );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a, "origin"_a = dip::UnsignedArray{}, "sizes"_a = dip::UnsignedArray{}, "spacing"_a = dip::UnsignedArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageReadICSInfo", []( dip::String const& filename ) {
      auto fi = dip::ImageReadICSInfo( filename );
      OptionallyReverseDimensions( fi );
      return fi;
   }, "filename"_a );
   m.def( "ImageIsICS", &dip::ImageIsICS, "filename"_a );
   m.def( "ImageWriteICS", []( dip::Image const& image, dip::String const& filename, dip::StringArray const& history, dip::uint significantBits, dip::StringSet const& options ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      dip::ImageWriteICS( tmp, filename, history, significantBits, options );
   }, "image"_a, "filename"_a, "history"_a = dip::StringArray{}, "significantBits"_a = 0, "options"_a = dip::StringSet{} );

   m.def( "ImageReadTIFF", []( dip::String const& filename, dip::Range const& imageNumbers, dip::RangeArray const& roi, dip::Range const& channels, dip::String const& useColorMap ) {
      auto out = dip::ImageReadTIFF( filename, imageNumbers, roi, channels, useColorMap );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a, "imageNumbers"_a = dip::Range{ 0 }, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{}, "useColorMap"_a = dip::S::APPLY );
   m.def( "ImageReadTIFF", []( dip::Image& out, dip::String const& filename, dip::Range const& imageNumbers, dip::RangeArray const& roi, dip::Range const& channels, dip::String const& useColorMap ) {
      auto fi = dip::ImageReadTIFF( out, filename, imageNumbers, roi, channels, useColorMap );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a, "imageNumbers"_a = dip::Range{ 0 }, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{}, "useColorMap"_a = dip::S::APPLY );
   m.def( "ImageReadTIFFSeries", []( dip::StringArray const& filenames, dip::String const& useColorMap ) {
      auto out = dip::ImageReadTIFFSeries( filenames, useColorMap );
      OptionallyReverseDimensions( out );
      return out;
   }, "filenames"_a, "useColorMap"_a = dip::S::APPLY );
   m.def( "ImageReadTIFFSeries", []( dip::Image& out, dip::StringArray const& filenames, dip::String const& useColorMap ) {
      dip::ImageReadTIFFSeries( out, filenames, useColorMap );
      OptionallyReverseDimensions( out );
   }, py::kw_only(), "out"_a, "filenames"_a, "useColorMap"_a = dip::S::APPLY );
   m.def( "ImageReadTIFFInfo", []( dip::String const& filename, dip::uint imageNumber ) {
      auto fi = dip::ImageReadTIFFInfo( filename, imageNumber );
      OptionallyReverseDimensions( fi );
      return fi;
   }, "filename"_a, "imageNumber"_a = 0 );
   m.def( "ImageIsTIFF", &dip::ImageIsTIFF, "filename"_a );
   m.def( "ImageWriteTIFF", []( dip::Image const& image, dip::String const& filename, dip::String const& compression, dip::uint jpegLevel ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      dip::ImageWriteTIFF( tmp, filename, compression, jpegLevel );
   }, "image"_a, "filename"_a, "compression"_a = "", "jpegLevel"_a = 80 );

   m.def( "ImageReadJPEG", []( py::bytes* buffer ) {
      auto buf = GetBytesPointerAndLength( buffer );
      auto out = dip::ImageReadJPEG( buf.ptr, buf.length );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a );
   m.def( "ImageReadJPEG", []( dip::Image& out, py::bytes* buffer ) {
      auto buf = GetBytesPointerAndLength( buffer );
      auto fi = dip::ImageReadJPEG( out, buf.ptr, buf.length );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "buffer"_a );
   m.def( "ImageReadJPEG", []( dip::String const& filename ) {
      auto out = dip::ImageReadJPEG( filename );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a );
   m.def( "ImageReadJPEG", []( dip::Image& out, dip::String const& filename ) {
      auto fi = dip::ImageReadJPEG( out, filename );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a );
   m.def( "ImageReadJPEGInfo", []( py::bytes* buffer ) {
      auto buf = GetBytesPointerAndLength( buffer );
      auto fi = dip::ImageReadJPEGInfo( buf.ptr, buf.length );
      OptionallyReverseDimensions( fi );
      return fi;
   }, "buffer"_a );
   m.def( "ImageReadJPEGInfo", []( dip::String const& filename ) {
      auto fi = dip::ImageReadJPEGInfo( filename );
      OptionallyReverseDimensions( fi );
      return fi;
   }, "filename"_a );
   m.def( "ImageIsJPEG", &dip::ImageIsJPEG, "filename"_a );
   m.def( "ImageWriteJPEG", []( dip::Image const& image, dip::String const& filename, dip::uint jpegLevel ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      dip::ImageWriteJPEG( tmp, filename, jpegLevel );
   }, "image"_a, "filename"_a, "jpegLevel"_a = 80 );
   m.def( "ImageWriteJPEG", []( dip::Image const& image, dip::uint jpegLevel ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      auto buffer = dip::ImageWriteJPEG( tmp, jpegLevel );
      return py::bytes( reinterpret_cast< char const* >( buffer.data() ), buffer.size() );
   }, "image"_a, "jpegLevel"_a = 80 );

   m.def( "ImageReadPNG", []( py::bytes* buffer ) {
      auto buf = GetBytesPointerAndLength( buffer );
      auto out = dip::ImageReadPNG( buf.ptr, buf.length );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a );
   m.def( "ImageReadPNG", []( dip::Image& out, py::bytes* buffer ) {
      auto buf = GetBytesPointerAndLength( buffer );
      auto fi = dip::ImageReadPNG( out, buf.ptr, buf.length );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "buffer"_a );
   m.def( "ImageReadPNG", []( dip::String const& filename ) {
      auto out = dip::ImageReadPNG( filename );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a );
   m.def( "ImageReadPNG", []( dip::Image& out, dip::String const& filename ) {
      auto fi = dip::ImageReadPNG( out, filename );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a );
   m.def( "ImageReadPNGInfo", []( py::bytes* buffer ) {
      auto buf = GetBytesPointerAndLength( buffer );
      auto fi = dip::ImageReadPNGInfo( buf.ptr, buf.length );
      OptionallyReverseDimensions( fi );
      return fi;
   }, "buffer"_a );
   m.def( "ImageReadPNGInfo", []( dip::String const& filename ) {
      auto fi = dip::ImageReadPNGInfo( filename );
      OptionallyReverseDimensions( fi );
      return fi;
   }, "filename"_a );
   m.def( "ImageIsPNG", &dip::ImageIsPNG, "filename"_a );
   m.def( "ImageWritePNG", []( dip::Image const& image,
                               dip::String const& filename,
                               dip::sint compressionLevel,
                               dip::StringSet const& filterChoice,
                               dip::uint significantBits ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      dip::ImageWritePNG( tmp, filename, compressionLevel, filterChoice, significantBits );
   }, "image"_a, "filename"_a, "compressionLevel"_a = 6, "filterChoice"_a = dip::StringSet{ dip::S::ALL }, "significantBits"_a = 0 );
   m.def( "ImageWritePNG", []( dip::Image const& image,
                               dip::sint compressionLevel,
                               dip::StringSet const& filterChoice,
                               dip::uint significantBits ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      auto buffer = dip::ImageWritePNG( tmp, compressionLevel, filterChoice, significantBits );
      return py::bytes( reinterpret_cast< char const* >( buffer.data() ), buffer.size() );
   }, "image"_a, "compressionLevel"_a = 6, "filterChoice"_a = dip::StringSet{ dip::S::ALL }, "significantBits"_a = 0 );

   m.def( "ImageReadNPY", []( dip::String const& filename ) {
      auto out = dip::ImageReadNPY( filename );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a );
   m.def( "ImageReadNPY", []( dip::Image& out, dip::String const& filename ) {
      auto fi = dip::ImageReadNPY( out, filename );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a );
   m.def( "ImageReadNPYInfo", []( dip::String const& filename ) {
      auto fi = dip::ImageReadNPYInfo( filename );
      OptionallyReverseDimensions( fi );
      return fi;
   }, "filename"_a );
   m.def( "ImageIsNPY", &dip::ImageIsNPY, "filename"_a );
   m.def( "ImageWriteNPY", []( dip::Image const& image, dip::String const& filename ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      dip::ImageWriteNPY( tmp, filename );
   }, "image"_a, "filename"_a );

   // diplib/simple_file_io.h
   m.def( "ImageRead", []( dip::String const& filename, dip::String const& format ) {
      auto out = dip::ImageRead( filename, format );
      OptionallyReverseDimensions( out );
      return out;
   }, "filename"_a, "format"_a = "" );
   m.def( "ImageRead", []( dip::Image& out, dip::String const& filename, dip::String const& format ) {
      auto fi = dip::ImageRead( out, filename, format );
      OptionallyReverseDimensions( out );
      OptionallyReverseDimensions( fi );
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a, "format"_a = "" );
   m.def( "ImageWrite", []( dip::Image const& image, dip::String const& filename, dip::String const& format, dip::String const& compression ) {
      auto tmp = image;
      OptionallyReverseDimensions( tmp );
      dip::ImageWrite( tmp, filename, format, compression );
   }, "image"_a, "filename"_a, "format"_a = "", "compression"_a = "" );

   // diplib/geometry.h
   m.def( "Wrap", py::overload_cast< dip::Image const&, dip::IntegerArray >( &dip::Wrap ),
          "in"_a, "wrap"_a );
   m.def( "Wrap", py::overload_cast< dip::Image const&, dip::Image&, dip::IntegerArray >( &dip::Wrap ),
          "in"_a, py::kw_only(), "out"_a, "wrap"_a );
   m.def( "Subsampling", py::overload_cast< dip::Image const&, dip::UnsignedArray const& >( &dip::Subsampling ),
          "in"_a, "sample"_a );
   m.def( "Subsampling", py::overload_cast< dip::Image const&, dip::Image&, dip::UnsignedArray const& >( &dip::Subsampling ),
          "in"_a, py::kw_only(), "out"_a, "sample"_a );
   m.def( "Resampling", py::overload_cast< dip::Image const&, dip::FloatArray, dip::FloatArray, dip::String const&, dip::StringArray const& >( &dip::Resampling ),
          "in"_a, "zoom"_a = dip::FloatArray{ 1.0 }, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Resampling", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::FloatArray, dip::String const&, dip::StringArray const& >( &dip::Resampling ),
          "in"_a, py::kw_only(), "out"_a, "zoom"_a = dip::FloatArray{ 1.0 }, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Shift", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::Shift ),
          "in"_a, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = dip::S::FOURIER, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Shift", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::Shift ),
          "in"_a, py::kw_only(), "out"_a, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = dip::S::FOURIER, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "ShiftFT", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::ShiftFT ),
          "in"_a, "shift"_a = dip::FloatArray{ 0.0 } );
   m.def( "ShiftFT", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::ShiftFT ),
          "in"_a, py::kw_only(), "out"_a, "shift"_a = dip::FloatArray{ 0.0 } );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::FloatCoordinateArray const&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, "coordinates"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatCoordinateArray const&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, py::kw_only(), "out"_a, "coordinates"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, "coordinates"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, "map"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, "map"_a, py::kw_only(), "out"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "Skew", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::uint, dip::String const&, dip::StringArray const& >( &dip::Skew ),
          "in"_a, "shearArray"_a, "axis"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Skew", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::uint, dip::String const&, dip::StringArray const& >( &dip::Skew ),
          "in"_a, py::kw_only(), "out"_a, "shearArray"_a, "axis"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Skew", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Skew ),
          "in"_a, "shear"_a, "skew"_a, "axis"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Skew", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Skew ),
          "in"_a, py::kw_only(), "out"_a, "shear"_a, "skew"_a, "axis"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Rotation ),
          "in"_a, "angle"_a, "dimension1"_a, "dimension2"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::S::ADD_ZEROS );
   m.def( "Rotation", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Rotation ),
          "in"_a, py::kw_only(), "out"_a, "angle"_a, "dimension1"_a, "dimension2"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::S::ADD_ZEROS );
   m.def( "Rotation2D", py::overload_cast< dip::Image const&, dip::dfloat, dip::String const&, dip::String const& >( &dip::Rotation2D ),
          "in"_a, "angle"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation2D", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::String const&, dip::String const& >( &dip::Rotation2D ),
          "in"_a, py::kw_only(), "out"_a, "angle"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation3D", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint, dip::String const&, dip::String const& >( &dip::Rotation3D ),
          "in"_a, "angle"_a, "axis"_a = 2, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation3D", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::uint, dip::String const&, dip::String const& >( &dip::Rotation3D ),
          "in"_a, py::kw_only(), "out"_a, "angle"_a, "axis"_a = 2, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation3D", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const&, dip::String const& >( &dip::Rotation3D ),
          "in"_a, "alpha"_a, "beta"_a, "gamma"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation3D", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const&, dip::String const& >( &dip::Rotation3D ),
          "in"_a, py::kw_only(), "out"_a, "alpha"_a, "beta"_a, "gamma"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "RotationMatrix2D", py::overload_cast< dip::dfloat >( &dip::RotationMatrix2D ),
          "angle"_a );
   m.def( "RotationMatrix3D", py::overload_cast< dip::dfloat, dip::dfloat, dip::dfloat >( &dip::RotationMatrix3D ),
          "alpha"_a, "beta"_a, "gamma"_a );
   m.def( "RotationMatrix3D", py::overload_cast< dip::FloatArray const&, dip::dfloat >( &dip::RotationMatrix3D ),
          "vector"_a, "angle"_a );
   m.def( "AffineTransform", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const& >( &dip::AffineTransform ),
          "in"_a, "matrix"_a, "interpolationMethod"_a = dip::S::LINEAR );
   m.def( "AffineTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::String const& >( &dip::AffineTransform ),
          "in"_a, py::kw_only(), "out"_a, "matrix"_a, "interpolationMethod"_a = dip::S::LINEAR );
   m.def( "WarpControlPoints", py::overload_cast< dip::Image const&, dip::FloatCoordinateArray const&, dip::FloatCoordinateArray const&, dip::dfloat, dip::String const& >( &dip::WarpControlPoints ),
          "in"_a, "inCoordinates"_a, "outCoordinates"_a, "regularizationLambda"_a = 0.0, "interpolationMethod"_a = dip::S::LINEAR );
   m.def( "WarpControlPoints", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatCoordinateArray const&, dip::FloatCoordinateArray const&, dip::dfloat, dip::String const& >( &dip::WarpControlPoints ),
          "in"_a, py::kw_only(), "out"_a, "inCoordinates"_a, "outCoordinates"_a, "regularizationLambda"_a = 0.0, "interpolationMethod"_a = dip::S::LINEAR );
   m.def( "LogPolarTransform2D", py::overload_cast< dip::Image const&, dip::String const& >( &dip::LogPolarTransform2D ),
          "in"_a, "interpolationMethod"_a = dip::S::LINEAR );
   m.def( "LogPolarTransform2D", py::overload_cast< dip::Image const&, dip::Image&, dip::String const& >( &dip::LogPolarTransform2D ),
          "in"_a, py::kw_only(), "out"_a, "interpolationMethod"_a = dip::S::LINEAR );

   m.def( "Tile", py::overload_cast< dip::ImageConstRefArray const&, dip::UnsignedArray >( &dip::Tile ),
          "in_array"_a, "tiling"_a = dip::UnsignedArray{} );
   m.def( "Tile", py::overload_cast< dip::ImageConstRefArray const&, dip::Image&, dip::UnsignedArray >( &dip::Tile ),
          "in_array"_a, py::kw_only(), "out"_a, "tiling"_a = dip::UnsignedArray{} );
   m.def( "TileTensorElements", py::overload_cast< dip::Image const& >( &dip::TileTensorElements ),
          "in"_a );
   m.def( "TileTensorElements", py::overload_cast< dip::Image const&, dip::Image& >( &dip::TileTensorElements ),
          "in"_a, py::kw_only(), "out"_a );
   m.def( "Concatenate", py::overload_cast< dip::ImageConstRefArray const&, dip::uint >( &dip::Concatenate ),
          "in_array"_a, "dimension"_a = 0 );
   m.def( "Concatenate", py::overload_cast< dip::ImageConstRefArray const&, dip::Image&, dip::uint >( &dip::Concatenate ),
          "in_array"_a, py::kw_only(), "out"_a, "dimension"_a = 0 );
   m.def( "Concatenate", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::Concatenate ),
          "in1"_a, "in2"_a, "dimension"_a = 0 );
   m.def( "Concatenate", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint >( &dip::Concatenate ),
          "in1"_a, "in2"_a, py::kw_only(), "out"_a, "dimension"_a = 0 );
   m.def( "JoinChannels", py::overload_cast< dip::ImageConstRefArray const& >( &dip::JoinChannels ),
          "in_array"_a );
   m.def( "JoinChannels", py::overload_cast< dip::ImageConstRefArray const&, dip::Image& >( &dip::JoinChannels ),
          "in_array"_a, py::kw_only(), "out"_a );

   // diplib/testing.h
   auto mtesting = m.def_submodule( "testing", "Functions to help test and debug your code." );
   auto timer = py::class_< dip::testing::Timer >( mtesting, "Timer", "A timer object to help time algorithm execution." );
   timer.def( py::init<>() );
   timer.def( "__repr__", []( dip::testing::Timer const& timer ) {
      std::ostringstream os;
      os << timer;
      return os.str();
   } );
   timer.def( "Reset", &dip::testing::Timer::Reset );
   timer.def( "Stop", &dip::testing::Timer::Stop );
   timer.def( "GetCpu", &dip::testing::Timer::GetCpu );
   timer.def( "GetWall", &dip::testing::Timer::GetWall );
   timer.def( "CpuResolution", &dip::testing::Timer::CpuResolution );
   timer.def( "WallResolution", &dip::testing::Timer::WallResolution );

}
