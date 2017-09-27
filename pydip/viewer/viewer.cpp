/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0 viewer
 *
 * (c)2017, Wouter Caarls
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
#include "dipviewer.h"

#include <pybind11/pybind11.h>

using namespace pybind11::literals;

PYBIND11_MODULE( viewer, m ) {
   m.def( "Show", &dip::viewer::Show, "in"_a, "title"_a = "");
   m.def( "Draw", &dip::viewer::Draw );
   m.def( "Spin", &dip::viewer::Spin );
}
