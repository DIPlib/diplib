/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017-2018, Flagship Biosciences, Inc., written by Cris Luengo.
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
#include "diplib/boundary.h"
#include "diplib/color.h"
#include "diplib/display.h"
#include "diplib/file_io.h"
#include "diplib/generation.h"
#include "diplib/geometry.h"
#include "diplib/histogram.h"
#include "diplib/lookup_table.h"
#include "diplib/mapping.h"

namespace {

dip::ColorSpaceManager colorSpaceManager;
dip::Random randomNumberGenerator;

dip::Image Display(
      dip::Image const& input,
      dip::String const& mappingMode = "lin",
      dip::dfloat lower = 0.0,
      dip::dfloat upper = 1.0,
      dip::String const& complexMode = "abs",
      dip::String const& projectionMode = "mean",
      dip::UnsignedArray const& coordinates = {},
      dip::uint dim1 = 0,
      dip::uint dim2 = 1
) {
   dip::ImageDisplay imageDisplay( input, &colorSpaceManager );
   if( mappingMode.empty() ) {
      imageDisplay.SetRange( dip::ImageDisplay::Limits{ lower, upper } );
   } else {
      imageDisplay.SetRange( mappingMode );
   }
   imageDisplay.SetComplexMode( complexMode );
   if( input.Dimensionality() > 2 ) {
      imageDisplay.SetGlobalStretch( true );
      imageDisplay.SetProjectionMode( projectionMode );
      if( !coordinates.empty()) {
         imageDisplay.SetCoordinates( coordinates );
      }
   }
   if( input.Dimensionality() >= 2 ) { // also for 2D images, you can rotate the output this way
      imageDisplay.SetDirection( dim1, dim2 );
   }
   return imageDisplay.Output();
}

dip::Image DisplayRange(
      dip::Image const& input,
      dip::FloatArray const& range,
      dip::String const& complexMode = "abs",
      dip::String const& projectionMode = "mean",
      dip::UnsignedArray const& coordinates = {},
      dip::uint dim1 = 0,
      dip::uint dim2 = 1
) {
   if( range.empty() ) {
      return Display( input, "lin", 0.0, 1.0, complexMode, projectionMode, coordinates, dim1, dim2 );
   }
   DIP_THROW_IF( range.size() != 2, "Range must be a 2-tuple" );
   return Display( input, "", range[ 0 ], range[ 1 ], complexMode, projectionMode, coordinates, dim1, dim2 );
}

dip::Image DisplayMode(
      dip::Image const& input,
      dip::String const& mappingMode = "lin",
      dip::String const& complexMode = "abs",
      dip::String const& projectionMode = "mean",
      dip::UnsignedArray const& coordinates = {},
      dip::uint dim1 = 0,
      dip::uint dim2 = 1
) {
   return Display( input, mappingMode, 0.0, 1.0, complexMode, projectionMode, coordinates, dim1, dim2 );
}

} // namespace

void init_assorted( py::module& m ) {
   // diplib/boundary.h
   m.def( "ExtendImage", py::overload_cast< dip::Image const&, dip::UnsignedArray const&, dip::StringArray const&, dip::StringSet const& >( &dip::ExtendImage ),
          "in"_a, "borderSizes"_a, "boundaryCondition"_a = dip::StringArray{}, "mode"_a = dip::StringSet{} );
   m.def( "ExtendRegion", py::overload_cast< dip::Image&, dip::RangeArray const&, dip::StringArray const& >( &dip::ExtendRegion ),
          "image"_a, "ranges"_a, "boundaryCondition"_a = dip::StringArray{} );

   // diplib/color.h
   auto mcol = m.def_submodule("ColorSpaceManager", "A Tool to convert images from one color space to another.");
   mcol.def( "Convert", []( dip::Image const& in, dip::String const& colorSpaceName ){ return colorSpaceManager.Convert( in, colorSpaceName ); }, "in"_a, "colorSpaceName"_a = "RGB" );
   mcol.def( "IsDefined", []( dip::String const& colorSpaceName ){ return colorSpaceManager.IsDefined( colorSpaceName ); }, "colorSpaceName"_a = "RGB" );
   mcol.def( "NumberOfChannels", []( dip::String const& colorSpaceName ){ return colorSpaceManager.NumberOfChannels( colorSpaceName ); }, "colorSpaceName"_a = "RGB" );
   mcol.def( "CanonicalName", []( dip::String const& colorSpaceName ){ return colorSpaceManager.CanonicalName( colorSpaceName ); }, "colorSpaceName"_a = "RGB" );
   // TODO: WhitePoint stuff

   // diplib/display.h
   m.def( "ImageDisplay", &DisplayRange, "in"_a, "range"_a, "complexMode"_a = "abs", "projectionMode"_a = "mean", "coordinates"_a = dip::UnsignedArray{}, "dim1"_a = 0, "dim2"_a = 1 );
   m.def( "ImageDisplay", &DisplayMode, "in"_a, "mappingMode"_a = "", "complexMode"_a = "abs", "projectionMode"_a = "mean", "coordinates"_a = dip::UnsignedArray{}, "dim1"_a = 0, "dim2"_a = 1 );
   m.def( "ApplyColorMap", py::overload_cast< dip::Image const&, dip::String const& >( &dip::ApplyColorMap ), "in"_a, "colorMap"_a = "grey" );
   m.def( "Overlay", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image::Pixel const& >( &dip::Overlay ), "in"_a, "overlay"_a, "color"_a = dip::Image::Pixel{ 255, 0, 0 } );

   // diplib/file_io.h
   m.def( "ImageReadICS", py::overload_cast< dip::String const&, dip::RangeArray const&, dip::Range const&, dip::String const& >( &dip::ImageReadICS ),
          "filename"_a, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageReadICS", py::overload_cast< dip::String const&, dip::UnsignedArray const&, dip::UnsignedArray const&, dip::UnsignedArray const&, dip::Range const&, dip::String const& >( &dip::ImageReadICS ),
          "filename"_a, "origin"_a = dip::UnsignedArray{}, "sizes"_a = dip::UnsignedArray{}, "spacing"_a = dip::UnsignedArray{}, "channels"_a = dip::Range{}, "mode"_a = "" );
   m.def( "ImageIsICS", &dip::ImageIsICS, "filename"_a );
   m.def( "ImageWriteICS", py::overload_cast< dip::Image const&, dip::String const&, dip::StringArray const&, dip::uint, dip::StringSet const& >( &dip::ImageWriteICS ),
          "image"_a, "filename"_a, "history"_a = dip::StringArray{}, "significantBits"_a = 0, "options"_a = dip::StringSet {} );

   m.def( "ImageReadTIFF", py::overload_cast< dip::String const&, dip::Range const&, dip::RangeArray const&, dip::Range const& >( &dip::ImageReadTIFF ),
          "filename"_a, "imageNumbers"_a = dip::Range{ 0 }, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{} );
   m.def( "ImageReadTIFFSeries", py::overload_cast< dip::StringArray const& >( &dip::ImageReadTIFFSeries ), "filenames"_a );
   m.def( "ImageIsTIFF", &dip::ImageIsTIFF, "filename"_a );
   m.def( "ImageWriteTIFF", py::overload_cast< dip::Image const&, dip::String const&, dip::String const&, dip::uint >( &dip::ImageWriteTIFF ),
          "image"_a, "filename"_a, "compression"_a = "", "jpegLevel"_a = 80 );

   // diplib/generation.h
   m.def( "FillDelta", &dip::FillDelta, "out"_a, "origin"_a = "" );
   m.def( "CreateDelta", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::CreateDelta ), "sizes"_a, "origin"_a = "" );

   m.def( "SetBorder", &dip::SetBorder, "out"_a, "value"_a = dip::Image::Pixel{ 0 }, "sizes"_a = dip::UnsignedArray{ 1 } );
   m.def( "DrawLine", &dip::DrawLine, "out"_a, "start"_a, "end"_a, "value"_a = dip::Image::Pixel{ 1 }, "blend"_a = dip::S::ASSIGN );
   m.def( "DrawLines", &dip::DrawLines, "out"_a, "points"_a, "value"_a = dip::Image::Pixel{ 1 }, "blend"_a = dip::S::ASSIGN );
   m.def( "DrawEllipsoid", &dip::DrawEllipsoid, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 } );
   m.def( "DrawDiamond", &dip::DrawDiamond, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 } );
   m.def( "DrawBox", &dip::DrawBox, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 } );
   m.def( "DrawBandlimitedPoint", &dip::DrawBandlimitedPoint, "out"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 }, "sigmas"_a = dip::FloatArray{ 1.0 }, "truncation"_a = 3.0 );
   m.def( "DrawBandlimitedLine", &dip::DrawBandlimitedLine, "out"_a, "start"_a, "end"_a, "value"_a = dip::Image::Pixel{ 1 }, "sigma"_a = 1.0, "truncation"_a = 3.0 );
   m.def( "DrawBandlimitedBall", &dip::DrawBandlimitedBall, "out"_a, "diameter"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 }, "mode"_a = dip::S::FILLED, "sigma"_a = 1.0, "truncation"_a = 3.0 );
   m.def( "DrawBandlimitedBox", &dip::DrawBandlimitedBox, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 }, "mode"_a = dip::S::FILLED, "sigma"_a = 1.0, "truncation"_a = 3.0 );
   m.def( "GaussianEdgeClip", py::overload_cast< dip::Image const&, dip::Image::Pixel const&, dip::dfloat, dip::dfloat >( &dip::GaussianEdgeClip ),
          "in"_a, "value"_a = dip::Image::Pixel{ 1 }, "sigma"_a = 1.0, "truncation"_a = 3.0 );
   m.def( "GaussianLineClip", py::overload_cast< dip::Image const&, dip::Image::Pixel const&, dip::dfloat, dip::dfloat >( &dip::GaussianLineClip ),
          "in"_a, "value"_a = dip::Image::Pixel{ 1 }, "sigma"_a = 1.0, "truncation"_a = 3.0 );

   m.def( "CreateGauss", py::overload_cast< dip::FloatArray const&, dip::UnsignedArray const&, dip::dfloat, dip::UnsignedArray const& >( &dip::CreateGauss ),
          "sigmas"_a, "order"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 3.0, "exponents"_a = dip::UnsignedArray{ 0 } );
   m.def( "CreateGabor", py::overload_cast< dip::FloatArray const&, dip::FloatArray const&, dip::dfloat >( &dip::CreateGabor ),
          "sigmas"_a, "frequencies"_a, "truncation"_a = 3.0 );

   m.def( "FTEllipsoid", py::overload_cast< dip::UnsignedArray const&, dip::FloatArray const&, dip::dfloat >( &dip::FTEllipsoid ),
         "sizes"_a, "radius"_a = dip::FloatArray{ 1 }, "amplitude"_a = 1 );
   m.def( "FTBox", py::overload_cast< dip::UnsignedArray const&, dip::FloatArray const&, dip::dfloat >( &dip::FTBox ),
          "sizes"_a, "length"_a = dip::FloatArray{ 1 }, "amplitude"_a = 1 );
   m.def( "FTCross", py::overload_cast< dip::UnsignedArray const&, dip::FloatArray const&, dip::dfloat >( &dip::FTCross ),
         "sizes"_a, "length"_a = dip::FloatArray{ 1 }, "amplitude"_a = 1 );
   m.def( "FTGaussian", py::overload_cast< dip::UnsignedArray const&, dip::FloatArray const&, dip::dfloat, dip::dfloat >( &dip::FTGaussian ),
         "sizes"_a, "sigma"_a, "amplitude"_a = 1, "truncation"_a = 3 );
   m.def( "TestObject", [](
               dip::UnsignedArray const& sizes,
               dip::String objectShape,
               dip::FloatArray const& objectSizes,
               dip::dfloat objectAmplitude,
               bool randomShift,
               dip::String generationMethod,
               dip::dfloat modulationDepth,
               dip::FloatArray const& modulationFrequency,
               dip::String pointSpreadFunction,
               dip::dfloat oversampling,
               dip::dfloat backgroundValue,
               dip::dfloat signalNoiseRatio,
               dip::dfloat gaussianNoise,
               dip::dfloat poissonNoise
          ) {
               dip::TestObjectParams params;
               params.objectShape = objectShape;
               params.objectSizes = objectSizes;
               params.objectAmplitude = objectAmplitude;
               params.randomShift = randomShift;
               params.generationMethod = generationMethod;
               params.modulationDepth = modulationDepth;
               params.modulationFrequency = modulationFrequency;
               params.pointSpreadFunction = pointSpreadFunction;
               params.oversampling = oversampling;
               params.backgroundValue = backgroundValue;
               params.signalNoiseRatio = signalNoiseRatio;
               params.gaussianNoise = gaussianNoise;
               params.poissonNoise = poissonNoise;
               return dip::TestObject( sizes, params, randomNumberGenerator );
          }, "sizes"_a,
             "objectShape"_a = dip::S::ELLIPSOID,
             "objectSizes"_a = dip::FloatArray{ 10 },
             "objectAmplitude"_a = 1.0,
             "randomShift"_a = false,
             "generationMethod"_a = dip::S::GAUSSIAN,
             "modulationDepth"_a = 0.0,
             "modulationFrequency"_a = dip::FloatArray{},
             "pointSpreadFunction"_a = dip::S::NONE,
             "oversampling"_a = 1.0,
             "backgroundValue"_a = 0.01,
             "signalNoiseRatio"_a = 0.0,
             "gaussianNoise"_a = 1.0,
             "poissonNoise"_a = 1.0 );

   m.def( "FillRamp", &dip::FillRamp, "out"_a, "dimension"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreateRamp", py::overload_cast< dip::UnsignedArray const&, dip::uint, dip::StringSet const& >( &dip::CreateRamp ), "sizes"_a, "dimension"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillXCoordinate", &dip::FillXCoordinate, "out"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreateXCoordinate", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const& >( &dip::CreateXCoordinate ), "sizes"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillYCoordinate", &dip::FillYCoordinate, "out"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreateYCoordinate", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const& >( &dip::CreateYCoordinate ), "sizes"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillZCoordinate", &dip::FillZCoordinate, "out"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreateZCoordinate", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const& >( &dip::CreateZCoordinate ), "sizes"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillRadiusCoordinate", &dip::FillRadiusCoordinate, "out"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreateRadiusCoordinate", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const& >( &dip::CreateRadiusCoordinate ), "sizes"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillRadiusSquareCoordinate", &dip::FillRadiusSquareCoordinate, "out"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreateRadiusSquareCoordinate", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const& >( &dip::CreateRadiusSquareCoordinate ), "sizes"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillPhiCoordinate", &dip::FillPhiCoordinate, "out"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreatePhiCoordinate", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const& >( &dip::CreatePhiCoordinate ), "sizes"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillThetaCoordinate", &dip::FillThetaCoordinate, "out"_a, "mode"_a = dip::StringSet{} );
   m.def( "CreateThetaCoordinate", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const& >( &dip::CreateThetaCoordinate ), "sizes"_a, "mode"_a = dip::StringSet{} );
   m.def( "FillCoordinates", &dip::FillCoordinates, "out"_a, "mode"_a = dip::StringSet{}, "system"_a = dip::S::CARTESIAN );
   m.def( "CreateCoordinates", py::overload_cast< dip::UnsignedArray const&, dip::StringSet const&, dip::String const& >( &dip::CreateCoordinates ),
          "sizes"_a, "mode"_a = dip::StringSet{}, "system"_a = dip::S::CARTESIAN );
   m.def( "FillDistanceToPoint", &dip::FillDistanceToPoint, "out"_a, "point"_a, "distance"_a = dip::S::EUCLIDEAN, "scaling"_a = dip::FloatArray{} );
   m.def( "DistanceToPoint", py::overload_cast< dip::UnsignedArray const&, dip::FloatArray const&, dip::String const&, dip::FloatArray const& >( &dip::DistanceToPoint ),
         "sizes"_a, "point"_a, "distance"_a = dip::S::EUCLIDEAN, "scaling"_a = dip::FloatArray{} );
   m.def( "EuclideanDistanceToPoint", py::overload_cast< dip::UnsignedArray const&, dip::FloatArray const&, dip::FloatArray const& >( &dip::EuclideanDistanceToPoint ),
         "sizes"_a, "point"_a, "scaling"_a = dip::FloatArray{} );
   m.def( "CityBlockDistanceToPoint", py::overload_cast< dip::UnsignedArray const&, dip::FloatArray const&, dip::FloatArray const& >( &dip::CityBlockDistanceToPoint ),
         "sizes"_a, "point"_a, "scaling"_a = dip::FloatArray{} );

   m.def( "UniformNoise", []( dip::Image const& in, dip::dfloat lowerBound, dip::dfloat upperBound ){ return dip::UniformNoise( in, randomNumberGenerator, lowerBound, upperBound ); },
          "in"_a, "lowerBound"_a = 0.0, "upperBound"_a = 1.0 );
   m.def( "GaussianNoise", []( dip::Image const& in, dip::dfloat variance ){ return dip::GaussianNoise( in, randomNumberGenerator, variance ); },
          "in"_a, "variance"_a = 1.0 );
   m.def( "PoissonNoise", []( dip::Image const& in, dip::dfloat conversion ){ return dip::PoissonNoise( in, randomNumberGenerator, conversion ); },
          "in"_a, "conversion"_a = 1.0 );
   m.def( "BinaryNoise", []( dip::Image const& in, dip::dfloat p10, dip::dfloat p01 ){ return dip::BinaryNoise( in, randomNumberGenerator, p10, p01 ); },
          "in"_a, "p10"_a = 0.05, "p01"_a = 0.05 );
   m.def( "SaltPepperNoise", []( dip::Image const& in, dip::dfloat p0, dip::dfloat p1, dip::dfloat white ){ return dip::SaltPepperNoise( in, randomNumberGenerator, p0, p1, white ); },
          "in"_a, "p0"_a = 0.05, "p1"_a = 0.05, "white"_a = 1.0 );
   m.def( "FillColoredNoise", []( dip::Image& out, dip::dfloat variance, dip::dfloat color ){ dip::FillColoredNoise( out, randomNumberGenerator, variance, color ); },
          "out"_a, "variance"_a = 1.0, "color"_a = -2.0 );
   m.def( "ColoredNoise", []( dip::Image const& in, dip::dfloat variance, dip::dfloat color  ){ return dip::ColoredNoise( in, randomNumberGenerator, variance, color ); },
          "in"_a, "variance"_a = 1.0, "color"_a = -2.0 );

   // diplib/geometry.h
   m.def( "Wrap", py::overload_cast< dip::Image const&, dip::IntegerArray const& >( &dip::Wrap ), "in"_a, "wrap"_a );
   m.def( "Subsampling", py::overload_cast< dip::Image const&, dip::UnsignedArray const& >( &dip::Subsampling ), "in"_a, "sample"_a );
   m.def( "Resampling", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::Resampling ),
          "in"_a, "zoom"_a = dip::FloatArray{ 1.0 }, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = "", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Shift", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::Shift ),
          "in"_a, "shift"_a = dip::FloatArray{ 0.0 }, "interpolationMethod"_a = dip::S::FOURIER, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::FloatCoordinateArray const&, dip::String const& >( &dip::ResampleAt ),
          "in"_a, "coordinates"_a, "method"_a = dip::S::LINEAR );
   m.def( "ResampleAt", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const& >( &dip::ResampleAt ),
          "in"_a, "coordinates"_a, "method"_a = dip::S::LINEAR );
   m.def( "Skew", py::overload_cast< dip::Image const&, dip::FloatArray, dip::uint, dip::String const&, dip::StringArray const& >( &dip::Skew ),
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

   m.def( "Tile", py::overload_cast< dip::ImageConstRefArray const&, dip::UnsignedArray const& >( &dip::Tile ),
          "in"_a, "tiling"_a = dip::UnsignedArray{} );
   m.def( "TileTensorElements", py::overload_cast< dip::Image const& >( &dip::TileTensorElements ),
          "in"_a );
   m.def( "Concatenate", py::overload_cast< dip::ImageConstRefArray const&, dip::uint >( &dip::Concatenate ),
          "in"_a, "dimension"_a = 0 );
   m.def( "Concatenate", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::Concatenate ),
          "in1"_a, "in2"_a, "dimension"_a = 0 );

   // diplib/histogram.h
   m.def( "Histogram", []( dip::Image const& input, dip::Image const& mask, dip::uint nBins ){
      dip::Histogram::Configuration config( input.DataType() );
      config.nBins = nBins;
      config.mode = dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE;
      dip::Histogram histogram( input, mask, config );
      dip::Image im = histogram.GetImage();
      std::vector< dip::FloatArray > bins( histogram.Dimensionality() );
      for( dip::uint ii = 0; ii < bins.size(); ++ii ) {
         bins[ ii ] = histogram.BinCenters( ii );
      }
      return py::make_tuple( im, bins ).release();
   }, "input"_a, "mask"_a = dip::Image{}, "nBins"_a = 256 );
   m.def( "Histogram", []( dip::Image const& input1, dip::Image const& input2, dip::Image const& mask ){
      dip::Histogram histogram( input1, input2, mask );
      dip::Image im = histogram.GetImage();
      std::vector< dip::FloatArray > bins( 2 );
      bins[ 0 ] = histogram.BinCenters( 0 );
      bins[ 1 ] = histogram.BinCenters( 1 );
      return py::make_tuple( im, bins ).release();
   }, "input1"_a, "input2"_a, "mask"_a = dip::Image{} );
   // TODO: Histogram should be an object, then we can access the `ReverseLookup` method.

   // diplib/lookup_table.h
   m.def( "LookupTable", []( dip::Image const& in, dip::Image const& lut, dip::FloatArray const& index, dip::String const& interpolation,
                             dip::String const& mode, dip::dfloat lowerValue, dip::dfloat upperValue ) {
      dip::LookupTable lookupTable( lut, index );
      if( mode == "clamp" ) {
         lookupTable.ClampOutOfBoundsValues(); // is the default...
      } else if( mode == "values" ) {
         lookupTable.SetOutOfBoundsValue( lowerValue, upperValue );
      } else if( mode == "keep" ) {
         lookupTable.KeepInputValueOnOutOfBounds();
      } else {
         DIP_THROW_INVALID_FLAG( mode );
      }
      return lookupTable.Apply( in, interpolation );
   }, "in"_a, "lut"_a, "index"_a = dip::FloatArray{}, "interpolation"_a = dip::S::LINEAR, "mode"_a = "clamp", "lowerValue"_a = 0.0, "upperValue"_a = 0.0
   );

   // diplib/mapping.h
   m.def( "Clip", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Clip ),
         "in"_a, "low"_a = 0.0, "high"_a = 255.0, "mode"_a = dip::S::BOTH );
   m.def( "ClipLow", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::ClipLow ), "in"_a, "low"_a = 0.0 );
   m.def( "ClipHigh", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::ClipHigh ), "in"_a, "high"_a = 255.0 );
   m.def( "ErfClip", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::ErfClip ),
         "in"_a, "low"_a = 128.0, "high"_a = 64.0, "mode"_a = dip::S::RANGE );
   m.def( "ContrastStretch", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::ContrastStretch ),
         "in"_a, "lowerBound"_a = 0.0, "upperBound"_a = 100.0, "outMin"_a = 0.0, "outMax"_a = 255.0, "method"_a = dip::S::LINEAR, "parameter1"_a = 1.0, "parameter2"_a = 0.0 );

   m.def( "HistogramEqualization", py::overload_cast< dip::Image const&, dip::uint >( &dip::HistogramEqualization ),
          "in"_a, "nBins"_a = 256 );
   m.def( "HistogramMatching", []( dip::Image const& in, dip::Image const& example ){
      DIP_THROW_IF( example.Dimensionality() != 1, "Example histogram must be 1D" );
      dip::uint nBins = example.Size( 0 );
      // Create a histogram of the right dimensions
      dip::Histogram::Configuration config( 0.0, static_cast< int >( nBins ), 1.0 );
      dip::Histogram exampleHistogram( config );
      // Fill it with the input
      dip::Image guts = exampleHistogram.GetImage().QuickCopy();
      guts.Copy( example ); // Copies data from example to data segment in guts, which is shared with the image in exampleHistogram. This means we're changing the histogram.
      return dip::HistogramMatching( in, exampleHistogram );
   }, "in"_a, "example"_a );

}
