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

#include "pydip.h"
#include "diplib/color.h"
#include "diplib/display.h"
#include "diplib/file_io.h"
#include "diplib/simple_file_io.h"
#include "diplib/geometry.h"

namespace pybind11 {
namespace detail {

// Cast dip::FileInformation to Python dict, one way only.
template<>
class type_caster< dip::FileInformation > {
   public:
      using type = dip::FileInformation;
      bool load( handle, bool ) { // no conversion from Python to DIPlib
         return false;
      }
      static handle cast( dip::FileInformation const& src, return_value_policy, handle ) {
         py::dict out;
         out["name"] = py::cast( src.name );
         out["fileType"] = py::cast( src.fileType );
         out["dataType"] = py::cast( src.dataType );
         out["significantBits"] = py::cast( src.significantBits );
         out["sizes"] = py::cast( src.sizes );
         out["tensorElements"] = py::cast( src.tensorElements );
         out["colorSpace"] = py::cast( src.colorSpace );
         out["pixelSize"] = py::cast( src.pixelSize );
         out["origin"] = py::cast( src.origin );
         out["numberOfImages"] = py::cast( src.numberOfImages );
         out["history"] = py::cast( src.history );
         return out.release();
      }
   PYBIND11_TYPE_CASTER( type, _( "FileInformation" ));
};

} // namespace detail
} // namespace pybind11

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

} // namespace

void init_assorted( py::module& m ) {

   // diplib/color.h
   auto mcol = m.def_submodule( "ColorSpaceManager",
                                "A Tool to convert images from one color space to another.\n\n"
                                "This is a submodule that uses a static `dip::ColorSpaceManager` object.\n"
                                "Functions defined in this module correspond to the object member functions\n"
                                "in C++." );
   mcol.def( "Convert", []( dip::Image const& in, dip::String const& colorSpaceName ){ return colorSpaceManager().Convert( in, colorSpaceName ); }, "in"_a, "colorSpaceName"_a = "RGB" );
   mcol.def( "IsDefined", []( dip::String const& colorSpaceName ){ return colorSpaceManager().IsDefined( colorSpaceName ); }, "colorSpaceName"_a = "RGB" );
   mcol.def( "NumberOfChannels", []( dip::String const& colorSpaceName ){ return colorSpaceManager().NumberOfChannels( colorSpaceName ); }, "colorSpaceName"_a = "RGB" );
   mcol.def( "CanonicalName", []( dip::String const& colorSpaceName ){ return colorSpaceManager().CanonicalName( colorSpaceName ); }, "colorSpaceName"_a = "RGB" );
   // TODO: WhitePoint stuff

   // diplib/display.h
   m.def( "ImageDisplay", [](
               dip::Image const& input,
               dip::FloatArray const& range,
               dip::String const& complexMode,
               dip::String const& projectionMode,
               dip::UnsignedArray const& coordinates,
               dip::uint dim1,
               dip::uint dim2
          ) {
            return ImageDisplay( input, range, {}, complexMode, projectionMode, coordinates, dim1, dim2 );
          }, "in"_a, "range"_a = dip::FloatArray{}, "complexMode"_a = "abs", "projectionMode"_a = "mean", "coordinates"_a = dip::UnsignedArray{}, "dim1"_a = 0, "dim2"_a = 1,
          "A function that performs the functionality of the `dip::ImageDisplay` class." );
   m.def( "ImageDisplay", [](
               dip::Image const& input,
               dip::String const& mappingMode,
               dip::String const& complexMode,
               dip::String const& projectionMode,
               dip::UnsignedArray const& coordinates,
               dip::uint dim1,
               dip::uint dim2
          ) {
            return ImageDisplay( input, {}, mappingMode, complexMode, projectionMode, coordinates, dim1, dim2 );
          }, "in"_a, "mappingMode"_a = "", "complexMode"_a = "abs", "projectionMode"_a = "mean", "coordinates"_a = dip::UnsignedArray{}, "dim1"_a = 0, "dim2"_a = 1,
          "Overload of the above that allows a `mappingMode` to be given instead of\n"
          "a `range`." );
   m.def( "ApplyColorMap", py::overload_cast< dip::Image const&, dip::String const& >( &dip::ApplyColorMap ), "in"_a, "colorMap"_a = "grey" );
   m.def( "Overlay", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image::Pixel const& >( &dip::Overlay ), "in"_a, "overlay"_a, "color"_a = dip::Image::Pixel{ 255, 0, 0 } );
   m.def( "MarkLabelEdges", py::overload_cast< dip::Image const&, dip::uint >( &dip::MarkLabelEdges ), "in"_a, "factor"_a = 2 );

   // diplib/file_io.h
   m.def( "ImageReadICS", py::overload_cast< dip::String const&, dip::RangeArray const&, dip::Range const&, dip::String const& >( &dip::ImageReadICS ),
          "filename"_a, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageReadICS", py::overload_cast< dip::String const&, dip::UnsignedArray const&, dip::UnsignedArray const&, dip::UnsignedArray const&, dip::Range const&, dip::String const& >( &dip::ImageReadICS ),
          "filename"_a, "origin"_a = dip::UnsignedArray{}, "sizes"_a = dip::UnsignedArray{}, "spacing"_a = dip::UnsignedArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageReadICSInfo", &dip::ImageReadICSInfo, "filename"_a );
   m.def( "ImageIsICS", &dip::ImageIsICS, "filename"_a );
   m.def( "ImageWriteICS", &dip::ImageWriteICS, "image"_a, "filename"_a, "history"_a = dip::StringArray{}, "significantBits"_a = 0, "options"_a = dip::StringSet {} );

   m.def( "ImageReadTIFF", py::overload_cast< dip::String const&, dip::Range const&, dip::RangeArray const&, dip::Range const&, dip::String const& >( &dip::ImageReadTIFF ),
          "filename"_a, "imageNumbers"_a = dip::Range{ 0 }, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{}, "useColorMap"_a = dip::S::APPLY );
   m.def( "ImageReadTIFFSeries", py::overload_cast< dip::StringArray const&, dip::String const& >( &dip::ImageReadTIFFSeries ), "filenames"_a, "useColorMap"_a = dip::S::APPLY );
   m.def( "ImageReadTIFFInfo", &dip::ImageReadTIFFInfo, "filename"_a, "imageNumber"_a = 0 );
   m.def( "ImageIsTIFF", &dip::ImageIsTIFF, "filename"_a );
   m.def( "ImageWriteTIFF", &dip::ImageWriteTIFF, "image"_a, "filename"_a, "compression"_a = "", "jpegLevel"_a = 80 );

   m.def( "ImageReadJPEG", py::overload_cast< dip::String const& >( &dip::ImageReadJPEG ), "filename"_a );
   m.def( "ImageReadJPEGInfo", &dip::ImageReadJPEGInfo, "filename"_a );
   m.def( "ImageIsJPEG", &dip::ImageIsJPEG, "filename"_a );
   m.def( "ImageWriteJPEG", &dip::ImageWriteJPEG, "image"_a, "filename"_a, "jpegLevel"_a = 80 );

   m.def( "ImageReadNPY", py::overload_cast< dip::String const& >( &dip::ImageReadNPY ), "filename"_a );
   m.def( "ImageReadNPYInfo", &dip::ImageReadNPYInfo, "filename"_a );
   m.def( "ImageIsNPY", &dip::ImageIsNPY, "filename"_a );
   m.def( "ImageWriteNPY", &dip::ImageWriteNPY, "image"_a, "filename"_a );

   // diplib/simple_file_io.h
   m.def( "ImageRead", py::overload_cast< dip::String const&, dip::String const& >( &dip::ImageRead ), "filename"_a, "format"_a = "" );
   m.def( "ImageWrite", &dip::ImageWrite, "image"_a, "filename"_a, "format"_a = "", "compression"_a = "" );

   // diplib/geometry.h
   m.def( "Wrap", py::overload_cast< dip::Image const&, dip::IntegerArray >( &dip::Wrap ), "in"_a, "wrap"_a );
   m.def( "Subsampling", py::overload_cast< dip::Image const&, dip::UnsignedArray const& >( &dip::Subsampling ), "in"_a, "sample"_a );
   m.def( "Resampling", py::overload_cast< dip::Image const&, dip::FloatArray, dip::FloatArray, dip::String const&, dip::StringArray const& >( &dip::Resampling ),
          "in"_a, "zoom"_a = dip::FloatArray{ 1.0 }, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Shift", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::Shift ),
          "in"_a, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = dip::S::FOURIER, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, "map"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::FloatCoordinateArray const&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, "coordinates"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::Image::Pixel const& >( &dip::ResampleAt ),
          "in"_a, "coordinates"_a, "method"_a = dip::S::LINEAR, "fill"_a = dip::Image::Pixel{ 0 } );
   m.def( "Skew", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::uint, dip::String const&, dip::StringArray const& >( &dip::Skew ),
          "in"_a, "shearArray"_a, "axis"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Skew", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Skew ),
          "in"_a, "shear"_a, "skew"_a, "axis"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Rotation ),
          "in"_a, "angle"_a, "dimension1"_a, "dimension2"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::S::ADD_ZEROS );
   m.def( "Rotation2D", py::overload_cast< dip::Image const&, dip::dfloat, dip::String const&, dip::String const& >( &dip::Rotation2D ),
          "in"_a, "angle"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation3D", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint, dip::String const&, dip::String const& >( &dip::Rotation3D ),
          "in"_a, "angle"_a, "axis"_a = 2, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "Rotation3D", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const&, dip::String const& >( &dip::Rotation3D ),
          "in"_a, "alpha"_a, "beta"_a, "gamma"_a, "interpolationMethod"_a = "", "boundaryCondition"_a = "" );
   m.def( "RotationMatrix2D", py::overload_cast< dip::dfloat >( &dip::RotationMatrix2D ),
          "angle"_a );
   m.def( "RotationMatrix3D", py::overload_cast< dip::dfloat, dip::dfloat, dip::dfloat >( &dip::RotationMatrix3D ),
          "alpha"_a, "beta"_a, "gamma"_a );
   m.def( "RotationMatrix3D", py::overload_cast< dip::FloatArray const&, dip::dfloat >( &dip::RotationMatrix3D ),
          "vector"_a, "angle"_a );
   m.def( "AffineTransform", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const& >( &dip::AffineTransform ),
          "in"_a, "matrix"_a, "interpolationMethod"_a = dip::S::LINEAR );
   m.def( "WarpControlPoints", py::overload_cast< dip::Image const&, dip::FloatCoordinateArray const&, dip::FloatCoordinateArray const&, dip::dfloat, dip::String const& >( &dip::WarpControlPoints ),
          "in"_a, "inCoordinates"_a, "outCoordinates"_a, "regularizationLambda"_a = 0.0, "interpolationMethod"_a = dip::S::LINEAR );
   m.def( "LogPolarTransform2D", py::overload_cast< dip::Image const&, dip::String const& >( &dip::LogPolarTransform2D ),
          "in"_a, "interpolationMethod"_a = dip::S::LINEAR );

   m.def( "Tile", py::overload_cast< dip::ImageConstRefArray const&, dip::UnsignedArray >( &dip::Tile ),
          "in_array"_a, "tiling"_a = dip::UnsignedArray{} );
   m.def( "TileTensorElements", py::overload_cast< dip::Image const& >( &dip::TileTensorElements ),
          "in"_a );
   m.def( "Concatenate", py::overload_cast< dip::ImageConstRefArray const&, dip::uint >( &dip::Concatenate ),
          "in_array"_a, "dimension"_a = 0 );
   m.def( "Concatenate", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::Concatenate ),
          "in1"_a, "in2"_a, "dimension"_a = 0 );
   m.def( "JoinChannels", py::overload_cast< dip::ImageConstRefArray const& >( &dip::JoinChannels ),
          "in_array"_a );

}
