/* PyDIP 3.0, Python bindings for DIPlib 3.0 JavaIO
 *
 * (c)2019, Wouter Caarls
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

#include <pybind11/pybind11.h>

using namespace pybind11::literals;

PYBIND11_MODULE( PyDIPjavaio, m ) {
   m.def( "ImageReadJavaIO", [](dip::String const& filename, dip::String const& interface) { return dip::javaio::ImageReadJavaIO(filename, interface); }, "filename"_a, "interface"_a = "org/diplib/BioFormatsInterface");
}
