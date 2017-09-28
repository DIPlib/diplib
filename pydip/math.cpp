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
#include "diplib/math.h"

void init_math( py::module& m ) {
   m.def( "SquareModulus", py::overload_cast< dip::Image const& >( &dip::SquareModulus ), "in"_a );
   m.def( "Phase", py::overload_cast< dip::Image const& >( &dip::Phase ), "in"_a );
   m.def( "Round", py::overload_cast< dip::Image const& >( &dip::Round ), "in"_a );
   m.def( "Ceil", py::overload_cast< dip::Image const& >( &dip::Ceil ), "in"_a );
   m.def( "Floor", py::overload_cast< dip::Image const& >( &dip::Floor ), "in"_a );
   m.def( "Truncate", py::overload_cast< dip::Image const& >( &dip::Truncate ), "in"_a );
   m.def( "Fraction", py::overload_cast< dip::Image const& >( &dip::Fraction ), "in"_a );
   m.def( "Reciprocal", py::overload_cast< dip::Image const& >( &dip::Reciprocal ), "in"_a );
   m.def( "Square", py::overload_cast< dip::Image const& >( &dip::Square ), "in"_a );
   m.def( "Sqrt", py::overload_cast< dip::Image const& >( &dip::Sqrt ), "in"_a );
   m.def( "Exp", py::overload_cast< dip::Image const& >( &dip::Exp ), "in"_a );
   m.def( "Exp2", py::overload_cast< dip::Image const& >( &dip::Exp2 ), "in"_a );
   m.def( "Exp10", py::overload_cast< dip::Image const& >( &dip::Exp10 ), "in"_a );
   m.def( "Ln", py::overload_cast< dip::Image const& >( &dip::Ln ), "in"_a );
   m.def( "Log2", py::overload_cast< dip::Image const& >( &dip::Log2 ), "in"_a );
   m.def( "Log10", py::overload_cast< dip::Image const& >( &dip::Log10 ), "in"_a );
   m.def( "Sin", py::overload_cast< dip::Image const& >( &dip::Sin ), "in"_a );
   m.def( "Cos", py::overload_cast< dip::Image const& >( &dip::Cos ), "in"_a );
   m.def( "Tan", py::overload_cast< dip::Image const& >( &dip::Tan ), "in"_a );
   m.def( "Asin", py::overload_cast< dip::Image const& >( &dip::Asin ), "in"_a );
   m.def( "Acos", py::overload_cast< dip::Image const& >( &dip::Acos ), "in"_a );
   m.def( "Atan", py::overload_cast< dip::Image const& >( &dip::Atan ), "in"_a );
   m.def( "Sinh", py::overload_cast< dip::Image const& >( &dip::Sinh ), "in"_a );
   m.def( "Cosh", py::overload_cast< dip::Image const& >( &dip::Cosh ), "in"_a );
   m.def( "Tanh", py::overload_cast< dip::Image const& >( &dip::Tanh ), "in"_a );
   m.def( "BesselJ0", py::overload_cast< dip::Image const& >( &dip::BesselJ0 ), "in"_a );
   m.def( "BesselJ1", py::overload_cast< dip::Image const& >( &dip::BesselJ1 ), "in"_a );
   m.def( "BesselJN", py::overload_cast< dip::Image const&, dip::uint >( &dip::BesselJN ), "in"_a, "alpha"_a  );
   m.def( "BesselY0", py::overload_cast< dip::Image const& >( &dip::BesselY0 ), "in"_a );
   m.def( "BesselY1", py::overload_cast< dip::Image const& >( &dip::BesselY1 ), "in"_a );
   m.def( "BesselYN", py::overload_cast< dip::Image const&, dip::uint >( &dip::BesselYN ), "in"_a, "alpha"_a  );
   m.def( "LnGamma", py::overload_cast< dip::Image const& >( &dip::LnGamma ), "in"_a );
   m.def( "Erf", py::overload_cast< dip::Image const& >( &dip::Erf ), "in"_a );
   m.def( "Erfc", py::overload_cast< dip::Image const& >( &dip::Erfc ), "in"_a );
   m.def( "Sinc", py::overload_cast< dip::Image const& >( &dip::Sinc ), "in"_a );
   m.def( "IsNotANumber", py::overload_cast< dip::Image const& >( &dip::IsNotANumber ), "in"_a );
   m.def( "IsInfinite", py::overload_cast< dip::Image const& >( &dip::IsInfinite ), "in"_a );
   m.def( "IsFinite", py::overload_cast< dip::Image const& >( &dip::IsFinite ), "in"_a );

   m.def( "Abs", py::overload_cast< dip::Image const& >( &dip::Abs ), "in"_a );
   m.def( "Modulus", py::overload_cast< dip::Image const& >( &dip::Modulus ), "in"_a );
   m.def( "Real", py::overload_cast< dip::Image const& >( &dip::Real ), "in"_a );
   m.def( "Imaginary", py::overload_cast< dip::Image const& >( &dip::Imaginary ), "in"_a );
   m.def( "Conjugate", py::overload_cast< dip::Image const& >( &dip::Conjugate ), "in"_a );
   m.def( "Sign", py::overload_cast< dip::Image const& >( &dip::Sign ), "in"_a );
   m.def( "NearestInt", py::overload_cast< dip::Image const& >( &dip::NearestInt ), "in"_a );
   m.def( "Supremum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Supremum ), "in1"_a, "in2"_a );
   m.def( "Infimum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Infimum ), "in1"_a, "in2"_a );
   m.def( "SignedInfimum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::SignedInfimum ), "in1"_a, "in2"_a );
   m.def( "LinearCombination", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat >( &dip::LinearCombination ),
          "a"_a, "b"_a, "aWeight"_a = 0.5, "bWeight"_a = 0.5 );
   m.def( "LinearCombination", py::overload_cast< dip::Image const&, dip::Image const&, dip::dcomplex, dip::dcomplex >( &dip::LinearCombination ),
          "a"_a, "b"_a, "aWeight"_a, "bWeight"_a );

   m.def( "Atan2", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Atan2 ), "y"_a, "x"_a );
   m.def( "Hypot", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Hypot ), "a"_a, "b"_a );
   m.def( "Transpose", py::overload_cast< dip::Image const& >( &dip::Transpose ), "in"_a );
   m.def( "ConjugateTranspose", py::overload_cast< dip::Image const& >( &dip::ConjugateTranspose ), "in"_a );
   m.def( "DotProduct", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::DotProduct ), "lhs"_a, "rhs"_a );
   m.def( "CrossProduct", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::CrossProduct ), "lhs"_a, "rhs"_a );
   m.def( "Norm", //py::overload_cast< dip::Image const& >( &dip::Norm ), // Fails to resolve!
          static_cast< dip::Image ( * )( dip::Image const& ) >( &dip::Norm ), "in"_a );
   m.def( "Angle", py::overload_cast< dip::Image const& >( &dip::Angle ), "in"_a );
   m.def( "Orientation", py::overload_cast< dip::Image const& >( &dip::Orientation ), "in"_a );
   m.def( "CartesianToPolar", py::overload_cast< dip::Image const& >( &dip::CartesianToPolar ), "in"_a );
   m.def( "PolarToCartesian", py::overload_cast< dip::Image const& >( &dip::PolarToCartesian ), "in"_a );
   m.def( "Determinant", py::overload_cast< dip::Image const& >( &dip::Determinant ), "in"_a );
   m.def( "Trace", //py::overload_cast< dip::Image const& >( &dip::Trace ), // Fails to resolve!
          static_cast< dip::Image ( * )( dip::Image const& ) >( &dip::Trace ), "in"_a );
   m.def( "Rank", py::overload_cast< dip::Image const& >( &dip::Rank ), "in"_a );
   m.def( "Eigenvalues", py::overload_cast< dip::Image const& >( &dip::Eigenvalues ), "in"_a );
   m.def( "EigenDecomposition", []( dip::Image const& in ){
             dip::Image out, eigenvectors;
             dip::EigenDecomposition( in, out, eigenvectors );
             py::make_tuple( out, eigenvectors ).release();
          }, "in"_a );
   m.def( "Inverse", py::overload_cast< dip::Image const& >( &dip::Inverse ), "in"_a );
   m.def( "PseudoInverse", py::overload_cast< dip::Image const& >( &dip::PseudoInverse ), "in"_a );
   m.def( "SingularValues", py::overload_cast< dip::Image const& >( &dip::SingularValues ), "in"_a );
   m.def( "SingularValueDecomposition", []( dip::Image const& in ){
             dip::Image U, S, V;
             dip::SingularValueDecomposition( in, U, S, V );
             py::make_tuple( U, S, V ).release();
          }, "in"_a );
   m.def( "Identity", py::overload_cast< dip::Image const& >( &dip::Identity ), "in"_a );

   m.def( "Select", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image const&, dip::String const& >( &dip::Select ),
          "in1"_a , "in2"_a , "in3"_a, "in4"_a, "selector"_a );
   m.def( "Select", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::Select ),
          "in1"_a , "in2"_a , "mask"_a );
}
