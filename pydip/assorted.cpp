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
#include "diplib/color.h"        // TODO: include functions from diplib/color.h
#include "diplib/file_io.h"
#include "diplib/generation.h"   // TODO: include functions from diplib/generation.h
#include "diplib/geometry.h"     // TODO: include functions from diplib/geometry.h
#include "diplib/histogram.h"    // TODO: include functions from diplib/histogram.h
#include "diplib/lookup_table.h" // TODO: include functions from diplib/lookup_table.h
#include "diplib/transform.h"    // TODO: include functions from diplib/transform.h

void init_assorted( py::module& m ) {
   m.def( "ImageReadICS", py::overload_cast< dip::String const&, dip::RangeArray const&, dip::Range const& >( &dip::ImageReadICS ),
          "filename"_a, "roi"_a = dip::RangeArray{}, "channels"_a = dip::Range{} );
   m.def( "ImageReadICS", py::overload_cast< dip::String const&, dip::UnsignedArray const&, dip::UnsignedArray const&, dip::UnsignedArray const&, dip::Range const& >( &dip::ImageReadICS ),
          "filename"_a, "origin"_a = dip::UnsignedArray{}, "sizes"_a = dip::UnsignedArray{}, "spacing"_a = dip::UnsignedArray{}, "channels"_a = dip::Range{} );
   m.def( "ImageIsICS", &dip::ImageIsICS, "filename"_a );
   m.def( "ImageWriteICS", py::overload_cast< dip::Image const&, dip::String const&, dip::StringArray const&, dip::uint, dip::StringSet const& >( &dip::ImageWriteICS ),
          "image"_a, "filename"_a, "history"_a = dip::StringArray{}, "significantBits"_a = 0, "options"_a = dip::StringSet {} );

   m.def( "ImageReadTIFF", py::overload_cast< dip::String const&, dip::Range const& >( &dip::ImageReadTIFF ),
          "filename"_a, "imageNumbers"_a = dip::Range{ 0 } );
   m.def( "ImageReadTIFFSeries", py::overload_cast< dip::StringArray const& >( &dip::ImageReadTIFFSeries ),
          "filenames"_a );
   m.def( "ImageIsTIFF", &dip::ImageIsTIFF, "filename"_a );
   m.def( "ImageWriteTIFF", py::overload_cast< dip::Image const&, dip::String const&, dip::String const&, dip::uint >( &dip::ImageWriteTIFF ),
          "image"_a, "filename"_a, "compression"_a = dip::String{}, "jpegLevel"_a = 80 );
}
