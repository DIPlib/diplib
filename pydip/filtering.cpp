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
#include "diplib/nonlinear.h" // TODO: include functions from diplib/nonlinear.h

// TODO: Expose `dip::Kernel` to Python like we expose `dip::StructuringElement`. It'll reduce duplication below and make stuff more user-friendly

void init_filtering( py::module& m ) {
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
          "sizes"_a = dip::FloatArray{ 7.0 },
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

   m.def( "Gauss", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::UnsignedArray const& derivativeOrder,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::dfloat truncation
          ) {
             return dip::Gauss( in, { sigma }, derivativeOrder, method, boundaryCondition, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "derivativeOrder"_a = dip::UnsignedArray{ 0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "truncation"_a = 3.0 );
   m.def( "Gauss", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::UnsignedArray const& derivativeOrder,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::dfloat truncation
          ) {
             return dip::Gauss( in, sigmas, derivativeOrder, method, boundaryCondition, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "derivativeOrder"_a = dip::UnsignedArray{ 0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "truncation"_a = 3.0 );

   m.def( "Derivative", [](
                dip::Image const& in,
                dip::UnsignedArray const& derivativeOrder,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::dfloat truncation
          ) {
             return dip::Derivative( in, derivativeOrder, { sigma }, method, boundaryCondition, truncation );
          },
          "in"_a,
          "derivativeOrder"_a = dip::UnsignedArray{ 0 },
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "truncation"_a = 3.0 );
   m.def( "Derivative", [](
                dip::Image const& in,
                dip::UnsignedArray const& derivativeOrder,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::dfloat truncation
          ) {
             return dip::Derivative( in, derivativeOrder, sigmas, method, boundaryCondition, truncation );
          },
          "in"_a,
          "derivativeOrder"_a = dip::UnsignedArray{ 0 },
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "truncation"_a = 3.0 );
   m.def( "Dx", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dx( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dy", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dy( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dxx", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dxx( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dyy", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dyy( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dzz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dzz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dxy", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dxy( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dxz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dxz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dyz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dyz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );

   m.def( "Gradient", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Gradient( in, { sigma }, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
   m.def( "Gradient", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Gradient( in, sigmas, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );

   m.def( "GradientMagnitude", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::GradientMagnitude( in, { sigma }, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
   m.def( "GradientMagnitude", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::GradientMagnitude( in, sigmas, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );

   m.def( "GradientDirection", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::GradientDirection( in, { sigma }, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
   m.def( "GradientDirection", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::GradientDirection( in, sigmas, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );

   m.def( "Curl", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Curl( in, { sigma }, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
   m.def( "Curl", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Curl( in, sigmas, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );

   m.def( "Divergence", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Divergence( in, { sigma }, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
   m.def( "Divergence", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Divergence( in, sigmas, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );

   m.def( "Hessian", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Hessian( in, { sigma }, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
   m.def( "Hessian", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Hessian( in, sigmas, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );

   m.def( "Laplace", [](
                dip::Image const& in,
                dip::dfloat sigma,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Laplace( in, { sigma }, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigma"_a = 1.0,
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
   m.def( "Laplace", [](
                dip::Image const& in,
                dip::FloatArray const& sigmas,
                dip::String const& method,
                dip::StringArray const& boundaryCondition,
                dip::BooleanArray const& process,
                dip::dfloat truncation
          ) {
             return dip::Laplace( in, sigmas, method, boundaryCondition, process, truncation );
          },
          "in"_a,
          "sigmas"_a = dip::FloatArray{ 1.0 },
          "method"_a = "best",
          "boundaryCondition"_a = dip::StringArray{},
          "process"_a = dip::BooleanArray{},
          "truncation"_a = 3.0 );
}
