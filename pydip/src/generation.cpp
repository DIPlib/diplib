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
#include "diplib/boundary.h"
#include "diplib/generation.h"

#include "diplib/chain_code.h"

void init_generation( py::module& m ) {

   // diplib/boundary.h
   m.def( "ExtendImage", py::overload_cast< dip::Image const&, dip::UnsignedArray, dip::StringArray const&, dip::StringSet const& >( &dip::ExtendImage ),
          "in"_a, "borderSizes"_a, "boundaryCondition"_a = dip::StringArray{}, "mode"_a = dip::StringSet{} );
   m.def( "ExtendRegion", py::overload_cast< dip::Image&, dip::RangeArray const&, dip::StringArray const& >( &dip::ExtendRegion ),
          "image"_a, "ranges"_a, "boundaryCondition"_a = dip::StringArray{} );

   // diplib/generation.h
   m.def( "FillDelta", &dip::FillDelta, "out"_a, "origin"_a = "" );
   m.def( "CreateDelta", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::CreateDelta ), "sizes"_a, "origin"_a = "" );

   m.def( "SetBorder", &dip::SetBorder, "out"_a, "value"_a = dip::Image::Pixel{ 0 }, "sizes"_a = dip::UnsignedArray{ 1 } );
   m.def( "ApplyWindow", py::overload_cast< dip::Image const&, dip::String const&, dip::dfloat >( &dip::ApplyWindow ),
          "in"_a, "type"_a = "Hamming", "parameter"_a = 0.5 );
   m.def( "DrawLine", &dip::DrawLine, "out"_a, "start"_a, "end"_a, "value"_a = dip::Image::Pixel{ 1 }, "blend"_a = dip::S::ASSIGN );
   m.def( "DrawLines", &dip::DrawLines, "out"_a, "points"_a, "value"_a = dip::Image::Pixel{ 1 }, "blend"_a = dip::S::ASSIGN );
   m.def( "DrawPolygon2D", &dip::DrawPolygon2D, "out"_a, "polygon"_a, "value"_a = dip::Image::Pixel{ 1 }, "mode"_a = dip::S::FILLED);
   m.def( "DrawEllipsoid", &dip::DrawEllipsoid, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 } );
   m.def( "DrawDiamond", &dip::DrawDiamond, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 } );
   m.def( "DrawBox", &dip::DrawBox, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 } );
   m.def( "DrawBandlimitedPoint", &dip::DrawBandlimitedPoint, "out"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 }, "sigmas"_a = dip::FloatArray{ 1.0 }, "truncation"_a = 3.0 );
   m.def( "DrawBandlimitedLine", &dip::DrawBandlimitedLine, "out"_a, "start"_a, "end"_a, "value"_a = dip::Image::Pixel{ 1 }, "sigma"_a = 1.0, "truncation"_a = 3.0 );
   m.def( "DrawBandlimitedBall", &dip::DrawBandlimitedBall, "out"_a, "diameter"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 }, "mode"_a = dip::S::FILLED, "sigma"_a = 1.0, "truncation"_a = 3.0 );
   m.def( "DrawBandlimitedBox", &dip::DrawBandlimitedBox, "out"_a, "sizes"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 }, "mode"_a = dip::S::FILLED, "sigma"_a = 1.0, "truncation"_a = 3.0 );
   m.def( "BlendBandlimitedMask", &dip::BlendBandlimitedMask, "out"_a, "mask"_a, "value"_a = dip::Image( { 255 } ), "pos"_a = dip::IntegerArray{} );
   m.def( "DrawText", []( dip::Image& out, dip::String const& text, dip::FloatArray const& origin, dip::String const& font,
                          dip::dfloat size, dip::Image::Pixel const& value, dip::dfloat orientation, dip::String const& align ) {
             dip::FreeTypeTool freeTypeTool( font );
             freeTypeTool.SetSize( size );
             freeTypeTool.DrawText( out, text, origin, value, orientation, align );
          }, "out"_a, "text"_a, "origin"_a, "font"_a, "size"_a = 12.0, "value"_a = dip::Image::Pixel{ 1 }, "orientation"_a = 0, "align"_a = dip::S::LEFT,
          "Function that calls `dip::FreeTypeTool::DrawText`." );
   m.def( "DrawText", []( dip::String const& text, dip::String const& font, dip::dfloat size, dip::dfloat orientation ) {
             dip::FreeTypeTool freeTypeTool( font );
             freeTypeTool.SetSize( size );
             return freeTypeTool.DrawText( text, orientation ).image;
          }, "text"_a, "font"_a, "size"_a = 12.0, "orientation"_a = 0,
          "Function that calls the alternate version of `dip::FreeTypeTool::DrawText`,\n"
          "returning a tightly cropped image around the rendered text." );
   m.def( "DrawText", py::overload_cast< dip::Image&, dip::String const&, dip::FloatArray, dip::Image::Pixel const&, dip::dfloat, dip::String const& >( &dip::DrawText ),
          "out"_a, "text"_a, "origin"_a, "value"_a = dip::Image::Pixel{ 1 }, "orientation"_a = 0, "align"_a = dip::S::LEFT,
          "Corresponds to `dip::DrawText`." );
   m.def( "DrawText", py::overload_cast< dip::String const&, dip::dfloat >( &dip::DrawText ), "text"_a, "orientation"_a = 0,
          "Corresponds to the alternate version of `dip::DrawText`, returning a tightly\n"
          "cropped image around the rendered text." );
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
             return dip::TestObject( sizes, params, RandomNumberGenerator() );
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
          "poissonNoise"_a = 1.0,
          "Like the C++ function, but with individual input values rather than a single\n"
          "`dip::TestObjectParams` object collecting all algorithm parameters.\n"
          "Also uses an internal `dip::Random` object." );
   m.def( "FillPoissonPointProcess", []( dip::Image& out, dip::dfloat density ){
             dip::FillPoissonPointProcess( out, RandomNumberGenerator(), density );
          },
          "in"_a, "density"_a = 0.01,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "CreatePoissonPointProcess", []( dip::UnsignedArray const& sizes, dip::dfloat density ){
             return dip::CreatePoissonPointProcess( sizes, RandomNumberGenerator(), density );
          },
          "sizes"_a, "density"_a = 0.01,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "FillRandomGrid", []( dip::Image& out, dip::dfloat density, dip::String const& type, dip::String const& mode ){
             dip::FillRandomGrid( out, RandomNumberGenerator(), density, type, mode );
          },
          "in"_a, "density"_a = 0.01, "type"_a = dip::S::RECTANGULAR, "mode"_a = dip::S::TRANSLATION,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "CreateRandomGrid", []( dip::UnsignedArray const& sizes, dip::dfloat density, dip::String const& type, dip::String const& mode ){
             return dip::CreateRandomGrid( sizes, RandomNumberGenerator(), density, type, mode );
          },
          "sizes"_a, "density"_a = 0.01, "type"_a = dip::S::RECTANGULAR, "mode"_a = dip::S::TRANSLATION,
          "Like the C++ function, but using an internal `dip::Random` object." );

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

   m.def( "UniformNoise", []( dip::Image const& in, dip::dfloat lowerBound, dip::dfloat upperBound ){
             return dip::UniformNoise( in, RandomNumberGenerator(), lowerBound, upperBound );
          },
          "in"_a, "lowerBound"_a = 0.0, "upperBound"_a = 1.0,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "GaussianNoise", []( dip::Image const& in, dip::dfloat variance ){
             return dip::GaussianNoise( in, RandomNumberGenerator(), variance );
          },
          "in"_a, "variance"_a = 1.0,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "PoissonNoise", []( dip::Image const& in, dip::dfloat conversion ){
             return dip::PoissonNoise( in, RandomNumberGenerator(), conversion );
          },
          "in"_a, "conversion"_a = 1.0,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "BinaryNoise", []( dip::Image const& in, dip::dfloat p10, dip::dfloat p01 ){
             return dip::BinaryNoise( in, RandomNumberGenerator(), p10, p01 );
          },
          "in"_a, "p10"_a = 0.05, "p01"_a = 0.05,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "SaltPepperNoise", []( dip::Image const& in, dip::dfloat p0, dip::dfloat p1, dip::dfloat white ){
             return dip::SaltPepperNoise( in, RandomNumberGenerator(), p0, p1, white );
          },
          "in"_a, "p0"_a = 0.05, "p1"_a = 0.05, "white"_a = 1.0,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "FillColoredNoise", []( dip::Image& out, dip::dfloat variance, dip::dfloat color ){
             dip::FillColoredNoise( out, RandomNumberGenerator(), variance, color );
          },
          "out"_a, "variance"_a = 1.0, "color"_a = -2.0,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "ColoredNoise", []( dip::Image const& in, dip::dfloat variance, dip::dfloat color  ){
             return dip::ColoredNoise( in, RandomNumberGenerator(), variance, color );
          },
          "in"_a, "variance"_a = 1.0, "color"_a = -2.0,
          "Like the C++ function, but using an internal `dip::Random` object." );

   m.def( "ReseedRng", [](){ RandomNumberGenerator().Seed(); },
          "Randomly reseed the random number generator used by the noise, grid and\n"
          "object generation functions." );
   m.def( "ReseedRng", []( dip::uint seed ){ RandomNumberGenerator().Seed( seed ); },
          "Reseed the random number generator used by the noise, grid and object\n"
          "generation functions." );

}
