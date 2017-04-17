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
#include "diplib/linear.h"

void init_linear( py::module& m ) {
   m.def( "Uniform", [](
                dip::Image const& in,
                dip::dfloat const& size,
                dip::String const& shape,
                dip::StringArray const& boundaryCondition
          ) {
             return dip::Uniform( in, { size, shape }, boundaryCondition );
          },
          "in"_a,
          "sizes"_a = 7.0,
          "shape"_a = "elliptic",
          "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Uniform", [](
                dip::Image const& in,
                dip::FloatArray const& sizes,
                dip::String const& shape,
                dip::StringArray const& boundaryCondition
          ) {
             return dip::Uniform( in, { sizes, shape }, boundaryCondition );
          },
          "in"_a,
          "sizes"_a,
          "shape"_a = "elliptic",
          "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Uniform", [](
                dip::Image const& in,
                dip::Image const& kernel,
                dip::StringArray const& boundaryCondition
          ) {
             return dip::Uniform( in, kernel, boundaryCondition );
          },
          "in"_a,
          "kernel"_a,
          "boundaryCondition"_a = dip::StringArray{} );
}
