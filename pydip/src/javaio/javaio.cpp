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

#include "diplib.h"
#include "diplib/javaio.h"
#include "diplib/simple_file_io.h"

#include <pybind11/pybind11.h>

using namespace pybind11::literals;
namespace py = pybind11;

// Copied from pydip.h, rather than including that header here.
py::function ReverseDimensionsFcn;
inline bool ReverseDimensions() {
   return ReverseDimensionsFcn().cast< bool >();
}

PYBIND11_MODULE( PyDIPjavaio, m ) {

   m.def( "_SetReverseDimensionsFcn", [ & ]( py::function fcn ){ ReverseDimensionsFcn = std::move( fcn ); },
          "Set the ReverseDimensionsFcn function in PyDIPjavaio." );

   // diplib/javaio.h
   m.def( "ImageReadJavaIO", []( dip::String const& filename, dip::String const& interface ) {
             auto out = dip::javaio::ImageReadJavaIO( filename, interface );
             if( !ReverseDimensions() ) {
                out.ReverseDimensions();
             }
             return out;
          }, "filename"_a, "interface"_a = dip::javaio::bioformatsInterface );

   // diplib/simple_file_io.h
   // We redefine ImageRead here, the version in PyDIP_bin is without DIPjavaio.
   m.def( "ImageRead", []( dip::String const& filename, dip::String const& format ) {
             auto out = dip::ImageRead( filename, format );
             if( !ReverseDimensions() ) {
                out.ReverseDimensions();
             }
             return out;
          }, "filename"_a, "format"_a = "" );
}
