/*
 * (c)2019-2020, Wouter Caarls
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

#include "../pydip.h"
#include "diplib/javaio.h"
#include "diplib/simple_file_io.h"

#include <pybind11/pybind11.h>

using namespace pybind11::literals;
namespace py = pybind11;

bool AreDimensionsReversed() {
   return static_cast< py::object >( py::module_::import( "diplib" ).attr( "AreDimensionsReversed" ))().cast< bool >();
}

PYBIND11_MODULE( PyDIPjavaio, m ) {
   m.doc() = "The portion of the PyDIP module that contains the C++ DIPjavaio bindings.";

   // diplib/javaio.h
   m.def( "ImageReadJavaIO", []( dip::String const& filename, dip::String const& interface, dip::uint imageNumber ) {
      auto out = dip::javaio::ImageReadJavaIO( filename, interface, imageNumber );
      if( !AreDimensionsReversed() ) {
         out.ReverseDimensions();
      }
      return out;
   }, "filename"_a, "interface"_a = dip::javaio::bioformatsInterface, "imageNumber"_a = 0, doc_strings::dip·javaio·ImageReadJavaIO·Image·L·String·CL·String·CL·dip·uint· );
   m.def( "ImageReadJavaIO", []( dip::Image& out, dip::String const& filename, dip::String const& interface, dip::uint imageNumber ) {
      auto fi = dip::javaio::ImageReadJavaIO( out, filename, interface, imageNumber );
      if( !AreDimensionsReversed() ) {
         out.ReverseDimensions();
         ReverseDimensions( fi );
      }
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a, "interface"_a = dip::javaio::bioformatsInterface, "imageNumber"_a = 0, doc_strings::dip·javaio·ImageReadJavaIO·Image·L·String·CL·String·CL·dip·uint· );

   // diplib/simple_file_io.h
   // We redefine ImageRead here, the version in PyDIP_bin is without DIPjavaio.
   m.def( "ImageRead", []( dip::String const& filename, dip::String const& format ) {
      auto out = dip::ImageRead( filename, format );
      if( !AreDimensionsReversed() ) {
         out.ReverseDimensions();
      }
      return out;
   }, "filename"_a, "format"_a = "", doc_strings::dip·ImageRead·Image·L·String·CL·String· );
   m.def( "ImageRead", []( dip::Image& out, dip::String const& filename, dip::String const& format ) {
      auto fi = dip::ImageRead( out, filename, format );
      if( !AreDimensionsReversed() ) {
         out.ReverseDimensions();
         ReverseDimensions( fi );
      }
      return fi;
   }, py::kw_only(), "out"_a, "filename"_a, "format"_a = "", doc_strings::dip·ImageRead·Image·L·String·CL·String· );
}
