/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017, Flagship Biosciences, Inc., written by Cris Luengo.
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
#include "diplib/analysis.h"
#include "diplib/distance.h"
#include "diplib/microscopy.h"
#include "diplib/regions.h"
#include "diplib/segmentation.h"

void init_analysis( py::module& m ) {

   // diplib/analysis.h

   auto loc = py::class_< dip::SubpixelLocationResult >( m, "SubpixelLocationResult", "The result of a call to PyDIP.SubpixelLocation." );
   loc.def_readonly( "coordinates", &dip::SubpixelLocationResult::coordinates );
   loc.def_readonly( "value", &dip::SubpixelLocationResult::value );
   loc.def( "__repr__", []( dip::SubpixelLocationResult const& self ) {
               std::ostringstream os;
               os << "<SubpixelLocationResult at " << self.coordinates << " with value " << self.value << ">";
               return os.str();
            } );

   m.def( "SubpixelLocation", &dip::SubpixelLocation,
          "in"_a, "position"_a, "polarity"_a = dip::S::MAXIMUM, "method"_a = dip::S::PARABOLIC_SEPARABLE );
   m.def( "SubpixelMaxima", &dip::SubpixelMaxima,
          "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::PARABOLIC_SEPARABLE );
   m.def( "SubpixelMinima", &dip::SubpixelMinima,
          "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::PARABOLIC_SEPARABLE );
   m.def( "CrossCorrelationFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const&, dip::String const&, dip::String const& >( &dip::CrossCorrelationFT ),
          "in1"_a, "in2"_a, "in1Representation"_a = dip::S::SPATIAL, "in2Representation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "normalize"_a = dip::S::NORMALIZE );
   m.def( "FindShift", &dip::FindShift,
          "in1"_a, "in2"_a, "method"_a = "MTS", "parameter"_a = 0, "maxShift"_a = std::numeric_limits< dip::uint >::max() );
   m.def( "StructureTensor", py::overload_cast< dip::Image const&, dip::Image const&, dip::FloatArray const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::StructureTensor ),
          "in"_a, "mask"_a = dip::Image{}, "gradientSigmas"_a = dip::FloatArray{ 1.0 }, "tensorSigmas"_a = dip::FloatArray{ 5.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "StructureTensorAnalysis", py::overload_cast< dip::Image const&, dip::StringArray const& >( &dip::StructureTensorAnalysis ),
          "in"_a, "outputs"_a );

   // diplib/distance.h

   m.def( "EuclideanDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::EuclideanDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::FAST );
   m.def( "VectorDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::VectorDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::FAST );
   m.def( "GreyWeightedDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const&, dip::Metric const&, dip::String const& >( &dip::GreyWeightedDistanceTransform ),
          "grey"_a, "in"_a, "metric"_a, "outputMode"_a = "GDT" );

   // diplib/microscopy.h

   // diplib/regions.h

   m.def( "Label", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::uint, dip::StringArray const& >( &dip::Label ),
          "binary"_a, "connectivity"_a = 0, "minSize"_a = 0, "maxSize"_a = 0, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "GetObjectLabels", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const& >( &dip::GetObjectLabels ),
          "label"_a, "mask"_a = dip::Image{}, "background"_a = dip::S::EXCLUDE );
   m.def( "Relabel", py::overload_cast< dip::Image const& >( &dip::Relabel ), "label"_a );
   m.def( "SmallObjectsRemove", py::overload_cast< dip::Image const&, dip::uint, dip::uint >( &dip::SmallObjectsRemove ),
          "in"_a, "threshold"_a, "connectivity"_a = 0 );

   // diplib/segmentation.h

   m.def( "IsodataThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "nThresholds"_a = 1 );
   m.def( "OtsuThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::OtsuThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::MinimumErrorThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "TriangleThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::TriangleThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "distance"_a = 2.0 );
   m.def( "VolumeThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::VolumeThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "volumeFraction"_a = 0.5 );
   m.def( "FixedThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::FixedThreshold ),
          "in"_a, "threshold"_a, "foreground"_a = 1.0, "background"_a = 0.0, "output"_a = dip::S::BINARY );
   m.def( "RangeThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::RangeThreshold ),
          "in"_a, "lowerBound"_a, "upperBound"_a, "foreground"_a = 1.0, "background"_a = 0.0, "output"_a = dip::S::BINARY );
   m.def( "HysteresisThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat >( &dip::HysteresisThreshold ),
          "in"_a, "lowThreshold"_a, "highThreshold"_a );
   m.def( "MultipleThresholds", py::overload_cast< dip::Image const&, dip::FloatArray const& >( &dip::MultipleThresholds ),
          "in"_a, "thresholds"_a );
   m.def( "Threshold", []( dip::Image const& in, dip::String const& method, dip::dfloat parameter ) {
             dip::Image out;
             dip::dfloat threshold = Threshold( in, out, method, parameter );
             return py::make_tuple( out, threshold ).release();
          }, "in"_a, "method"_a = dip::S::OTSU, "parameter"_a = dip::infinity );
   m.def( "Canny", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::dfloat >( &dip::Canny ),
          "in"_a, "sigmas"_a, "lower"_a, "upper"_a );
}
