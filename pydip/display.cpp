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
#include "diplib/display.h"

namespace {

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
   dip::ColorSpaceManager colorSpaceManager;
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

void init_display( py::module& m ) {
   m.def( "ImageDisplay", &DisplayRange, "in"_a,
          "range"_a,
          "complexMode"_a = "abs",
          "projectionMode"_a = "mean",
          "coordinates"_a = dip::UnsignedArray{},
          "dim1"_a = 0,
          "dim2"_a = 1
   );
   m.def( "ImageDisplay", &DisplayMode, "in"_a,
          "mappingMode"_a = "",
          "complexMode"_a = "abs",
          "projectionMode"_a = "mean",
          "coordinates"_a = dip::UnsignedArray{},
          "dim1"_a = 0,
          "dim2"_a = 1
   );
}
