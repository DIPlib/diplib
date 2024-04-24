/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022, Cris Luengo.
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
#include "diplib/mapping.h"
#include "diplib/histogram.h"

void init_math( py::module& m ) {

   // diplib/math.h
   m.def( "Add", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::Add( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "Add", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::Add( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "Add", []( py::object const& lhs, py::object const& rhs ) { return dip::Add( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Add", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Add( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Subtract", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::Subtract( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "Subtract", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::Subtract( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "Subtract", []( py::object const& lhs, py::object const& rhs ) { return dip::Subtract( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Subtract", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Subtract( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Multiply", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::Multiply( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "Multiply", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::Multiply( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "Multiply", []( py::object const& lhs, py::object const& rhs ) { return dip::Multiply( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Multiply", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Multiply( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "MultiplySampleWise", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::MultiplySampleWise( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "MultiplySampleWise", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::MultiplySampleWise( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "MultiplySampleWise", []( py::object const& lhs, py::object const& rhs ) { return dip::MultiplySampleWise( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "MultiplySampleWise", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::MultiplySampleWise( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "MultiplyConjugate", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::MultiplyConjugate( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "MultiplyConjugate", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::MultiplyConjugate( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "MultiplyConjugate", []( py::object const& lhs, py::object const& rhs ) { return dip::MultiplyConjugate( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "MultiplyConjugate", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::MultiplyConjugate( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Divide", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::Divide( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "Divide", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::Divide( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "Divide", []( py::object const& lhs, py::object const& rhs ) { return dip::Divide( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Divide", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Divide( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "SafeDivide", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::SafeDivide( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "SafeDivide", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::SafeDivide( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "SafeDivide", []( py::object const& lhs, py::object const& rhs ) { return dip::SafeDivide( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "SafeDivide", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::SafeDivide( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Modulo", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::Modulo( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "Modulo", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::Modulo( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "Modulo", []( py::object const& lhs, py::object const& rhs ) { return dip::Modulo( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Modulo", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Modulo( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Power", []( py::object const& lhs, py::object const& rhs, dip::DataType dt ) { return dip::Power( ImageOrPixel( lhs ), ImageOrPixel( rhs ), dt ); }, "lhs"_a, "rhs"_a, "datatype"_a );
   m.def( "Power", []( py::object const& lhs, py::object const& rhs, dip::Image& out, dip::DataType dt ) { dip::Power( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out, dt ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a, "datatype"_a );
   m.def( "Power", []( py::object const& lhs, py::object const& rhs ) { return dip::Power( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Power", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Power( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Invert", py::overload_cast< dip::Image const& >( &dip::Invert ), "in"_a );
   m.def( "Invert", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Invert ), "in"_a, py::kw_only(), "out"_a );
   m.def( "And", []( py::object const& lhs, py::object const& rhs ) { return dip::And( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "And", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::And( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Or", []( py::object const& lhs, py::object const& rhs ) { return dip::Or( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Or", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Or( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Xor", []( py::object const& lhs, py::object const& rhs ) { return dip::Xor( ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "lhs"_a, "rhs"_a );
   m.def( "Xor", []( py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::Xor( ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Not", py::overload_cast< dip::Image const& >( &dip::Not ), "in"_a );
   m.def( "Not", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Not ), "in"_a, py::kw_only(), "out"_a );
   m.def( "InRange", []( dip::Image const& in, py::object const& lhs, py::object const& rhs ) { return dip::InRange( in, ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "in"_a, "lhs"_a, "rhs"_a );
   m.def( "InRange", []( dip::Image const& in, py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::InRange( in, ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "in"_a, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "OutOfRange", []( dip::Image const& in, py::object const& lhs, py::object const& rhs ) { return dip::OutOfRange( in, ImageOrPixel( lhs ), ImageOrPixel( rhs )); }, "in"_a, "lhs"_a, "rhs"_a );
   m.def( "OutOfRange", []( dip::Image const& in, py::object const& lhs, py::object const& rhs, dip::Image& out ) { dip::OutOfRange( in, ImageOrPixel( lhs ), ImageOrPixel( rhs ), out ); }, "in"_a, "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );

   m.def( "SquareModulus", py::overload_cast< dip::Image const& >( &dip::SquareModulus ), "in"_a );
   m.def( "SquareModulus", py::overload_cast< dip::Image const&, dip::Image& >( &dip::SquareModulus ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Phase", py::overload_cast< dip::Image const& >( &dip::Phase ), "in"_a );
   m.def( "Phase", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Phase ), "in"_a, py::kw_only(), "out"_a );
   m.def( "FlushToZero", py::overload_cast< dip::Image const& >( &dip::FlushToZero ), "in"_a );
   m.def( "FlushToZero", py::overload_cast< dip::Image const&, dip::Image& >( &dip::FlushToZero ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Round", py::overload_cast< dip::Image const& >( &dip::Round ), "in"_a );
   m.def( "Round", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Round ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Ceil", py::overload_cast< dip::Image const& >( &dip::Ceil ), "in"_a );
   m.def( "Ceil", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Ceil ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Floor", py::overload_cast< dip::Image const& >( &dip::Floor ), "in"_a );
   m.def( "Floor", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Floor ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Truncate", py::overload_cast< dip::Image const& >( &dip::Truncate ), "in"_a );
   m.def( "Truncate", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Truncate ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Fraction", py::overload_cast< dip::Image const& >( &dip::Fraction ), "in"_a );
   m.def( "Fraction", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Fraction ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Reciprocal", py::overload_cast< dip::Image const& >( &dip::Reciprocal ), "in"_a );
   m.def( "Reciprocal", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Reciprocal ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Square", py::overload_cast< dip::Image const& >( &dip::Square ), "in"_a );
   m.def( "Square", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Square ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Sqrt", py::overload_cast< dip::Image const& >( &dip::Sqrt ), "in"_a );
   m.def( "Sqrt", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Sqrt ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Exp", py::overload_cast< dip::Image const& >( &dip::Exp ), "in"_a );
   m.def( "Exp", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Exp ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Exp2", py::overload_cast< dip::Image const& >( &dip::Exp2 ), "in"_a );
   m.def( "Exp2", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Exp2 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Exp10", py::overload_cast< dip::Image const& >( &dip::Exp10 ), "in"_a );
   m.def( "Exp10", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Exp10 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Ln", py::overload_cast< dip::Image const& >( &dip::Ln ), "in"_a );
   m.def( "Ln", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Ln ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Log2", py::overload_cast< dip::Image const& >( &dip::Log2 ), "in"_a );
   m.def( "Log2", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Log2 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Log10", py::overload_cast< dip::Image const& >( &dip::Log10 ), "in"_a );
   m.def( "Log10", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Log10 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Sin", py::overload_cast< dip::Image const& >( &dip::Sin ), "in"_a );
   m.def( "Sin", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Sin ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Cos", py::overload_cast< dip::Image const& >( &dip::Cos ), "in"_a );
   m.def( "Cos", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Cos ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Tan", py::overload_cast< dip::Image const& >( &dip::Tan ), "in"_a );
   m.def( "Tan", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Tan ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Asin", py::overload_cast< dip::Image const& >( &dip::Asin ), "in"_a );
   m.def( "Asin", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Asin ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Acos", py::overload_cast< dip::Image const& >( &dip::Acos ), "in"_a );
   m.def( "Acos", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Acos ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Atan", py::overload_cast< dip::Image const& >( &dip::Atan ), "in"_a );
   m.def( "Atan", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Atan ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Sinh", py::overload_cast< dip::Image const& >( &dip::Sinh ), "in"_a );
   m.def( "Sinh", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Sinh ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Cosh", py::overload_cast< dip::Image const& >( &dip::Cosh ), "in"_a );
   m.def( "Cosh", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Cosh ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Tanh", py::overload_cast< dip::Image const& >( &dip::Tanh ), "in"_a );
   m.def( "Tanh", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Tanh ), "in"_a, py::kw_only(), "out"_a );
   m.def( "BesselJ0", py::overload_cast< dip::Image const& >( &dip::BesselJ0 ), "in"_a );
   m.def( "BesselJ0", py::overload_cast< dip::Image const&, dip::Image& >( &dip::BesselJ0 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "BesselJ1", py::overload_cast< dip::Image const& >( &dip::BesselJ1 ), "in"_a );
   m.def( "BesselJ1", py::overload_cast< dip::Image const&, dip::Image& >( &dip::BesselJ1 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "BesselJN", py::overload_cast< dip::Image const&, dip::uint >( &dip::BesselJN ), "in"_a, "alpha"_a );
   m.def( "BesselJN", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::BesselJN ), "in"_a, py::kw_only(), "out"_a, "alpha"_a );
   m.def( "BesselY0", py::overload_cast< dip::Image const& >( &dip::BesselY0 ), "in"_a );
   m.def( "BesselY0", py::overload_cast< dip::Image const&, dip::Image& >( &dip::BesselY0 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "BesselY1", py::overload_cast< dip::Image const& >( &dip::BesselY1 ), "in"_a );
   m.def( "BesselY1", py::overload_cast< dip::Image const&, dip::Image& >( &dip::BesselY1 ), "in"_a, py::kw_only(), "out"_a );
   m.def( "BesselYN", py::overload_cast< dip::Image const&, dip::uint >( &dip::BesselYN ), "in"_a, "alpha"_a );
   m.def( "BesselYN", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::BesselYN ), "in"_a, py::kw_only(), "out"_a, "alpha"_a );
   m.def( "LnGamma", py::overload_cast< dip::Image const& >( &dip::LnGamma ), "in"_a );
   m.def( "LnGamma", py::overload_cast< dip::Image const&, dip::Image& >( &dip::LnGamma ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Erf", py::overload_cast< dip::Image const& >( &dip::Erf ), "in"_a );
   m.def( "Erf", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Erf ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Erfc", py::overload_cast< dip::Image const& >( &dip::Erfc ), "in"_a );
   m.def( "Erfc", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Erfc ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Sinc", py::overload_cast< dip::Image const& >( &dip::Sinc ), "in"_a );
   m.def( "Sinc", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Sinc ), "in"_a, py::kw_only(), "out"_a );
   m.def( "IsNotANumber", py::overload_cast< dip::Image const& >( &dip::IsNotANumber ), "in"_a );
   m.def( "IsNotANumber", py::overload_cast< dip::Image const&, dip::Image& >( &dip::IsNotANumber ), "in"_a, py::kw_only(), "out"_a );
   m.def( "IsInfinite", py::overload_cast< dip::Image const& >( &dip::IsInfinite ), "in"_a );
   m.def( "IsInfinite", py::overload_cast< dip::Image const&, dip::Image& >( &dip::IsInfinite ), "in"_a, py::kw_only(), "out"_a );
   m.def( "IsFinite", py::overload_cast< dip::Image const& >( &dip::IsFinite ), "in"_a );
   m.def( "IsFinite", py::overload_cast< dip::Image const&, dip::Image& >( &dip::IsFinite ), "in"_a, py::kw_only(), "out"_a );

   m.def( "Abs", py::overload_cast< dip::Image const& >( &dip::Abs ), "in"_a );
   m.def( "Abs", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Abs ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Modulus", py::overload_cast< dip::Image const& >( &dip::Modulus ), "in"_a );
   m.def( "Modulus", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Modulus ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Real", py::overload_cast< dip::Image const& >( &dip::Real ), "in"_a );
   m.def( "Imaginary", py::overload_cast< dip::Image const& >( &dip::Imaginary ), "in"_a );
   m.def( "Conjugate", py::overload_cast< dip::Image const& >( &dip::Conjugate ), "in"_a );
   m.def( "Conjugate", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Conjugate ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Sign", py::overload_cast< dip::Image const& >( &dip::Sign ), "in"_a );
   m.def( "Sign", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Sign ), "in"_a, py::kw_only(), "out"_a );
   m.def( "NearestInt", py::overload_cast< dip::Image const& >( &dip::NearestInt ), "in"_a );
   m.def( "NearestInt", py::overload_cast< dip::Image const&, dip::Image& >( &dip::NearestInt ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Supremum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Supremum ), "in1"_a, "in2"_a );
   m.def( "Supremum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::Supremum ), "in1"_a, "in2"_a, py::kw_only(), "out"_a );
   m.def( "Supremum", py::overload_cast< dip::ImageConstRefArray const& >( &dip::Supremum ), "image_array"_a );
   m.def( "Supremum", py::overload_cast< dip::ImageConstRefArray const&, dip::Image& >( &dip::Supremum ), "image_array"_a, py::kw_only(), "out"_a );
   m.def( "Infimum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Infimum ), "in1"_a, "in2"_a );
   m.def( "Infimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::Infimum ), "in1"_a, "in2"_a, py::kw_only(), "out"_a );
   m.def( "Infimum", py::overload_cast< dip::ImageConstRefArray const& >( &dip::Infimum ), "image_array"_a );
   m.def( "Infimum", py::overload_cast< dip::ImageConstRefArray const&, dip::Image& >( &dip::Infimum ), "image_array"_a, py::kw_only(), "out"_a );
   m.def( "SignedInfimum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::SignedInfimum ), "in1"_a, "in2"_a );
   m.def( "SignedInfimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::SignedInfimum ), "in1"_a, "in2"_a, py::kw_only(), "out"_a );
   m.def( "LinearCombination", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat >( &dip::LinearCombination ),
          "a"_a, "b"_a, "aWeight"_a = 0.5, "bWeight"_a = 0.5 );
   m.def( "LinearCombination", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat >( &dip::LinearCombination ),
          "a"_a, "b"_a, py::kw_only(), "out"_a, "aWeight"_a = 0.5, "bWeight"_a = 0.5 );
   m.def( "LinearCombination", py::overload_cast< dip::Image const&, dip::Image const&, dip::dcomplex, dip::dcomplex >( &dip::LinearCombination ),
          "a"_a, "b"_a, "aWeight"_a, "bWeight"_a );
   m.def( "LinearCombination", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dcomplex, dip::dcomplex >( &dip::LinearCombination ),
          "a"_a, "b"_a, py::kw_only(), "out"_a, "aWeight"_a, "bWeight"_a );

   m.def( "Atan2", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Atan2 ), "y"_a, "x"_a );
   m.def( "Atan2", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::Atan2 ), "y"_a, "x"_a, py::kw_only(), "out"_a );
   m.def( "Hypot", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Hypot ), "a"_a, "b"_a );
   m.def( "Hypot", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::Hypot ), "a"_a, "b"_a, py::kw_only(), "out"_a );
   m.def( "Transpose", py::overload_cast< dip::Image const& >( &dip::Transpose ), "in"_a );
   m.def( "ConjugateTranspose", py::overload_cast< dip::Image const& >( &dip::ConjugateTranspose ), "in"_a );
   m.def( "ConjugateTranspose", py::overload_cast< dip::Image const&, dip::Image& >( &dip::ConjugateTranspose ), "in"_a, py::kw_only(), "out"_a );
   m.def( "DotProduct", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::DotProduct ), "lhs"_a, "rhs"_a );
   m.def( "DotProduct", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::DotProduct ), "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "CrossProduct", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::CrossProduct ), "lhs"_a, "rhs"_a );
   m.def( "CrossProduct", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::CrossProduct ), "lhs"_a, "rhs"_a, py::kw_only(), "out"_a );
   m.def( "Norm", //py::overload_cast< dip::Image const& >( &dip::Norm ), // Fails to resolve!
          static_cast< dip::Image ( * )( dip::Image const& ) >( &dip::Norm ), "in"_a );
   m.def( "Norm", //py::overload_cast< dip::Image const&, dip::Image& >( &dip::Norm ), // Fails to resolve!
          static_cast< void ( * )( dip::Image const&, dip::Image& ) >( &dip::Norm ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Angle", py::overload_cast< dip::Image const& >( &dip::Angle ), "in"_a );
   m.def( "Angle", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Angle ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Orientation", py::overload_cast< dip::Image const& >( &dip::Orientation ), "in"_a );
   m.def( "Orientation", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Orientation ), "in"_a, py::kw_only(), "out"_a );
   m.def( "CartesianToPolar", py::overload_cast< dip::Image const& >( &dip::CartesianToPolar ), "in"_a );
   m.def( "CartesianToPolar", py::overload_cast< dip::Image const&, dip::Image& >( &dip::CartesianToPolar ), "in"_a, py::kw_only(), "out"_a );
   m.def( "PolarToCartesian", py::overload_cast< dip::Image const& >( &dip::PolarToCartesian ), "in"_a );
   m.def( "PolarToCartesian", py::overload_cast< dip::Image const&, dip::Image& >( &dip::PolarToCartesian ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Determinant", py::overload_cast< dip::Image const& >( &dip::Determinant ), "in"_a );
   m.def( "Determinant", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Determinant ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Trace", //py::overload_cast< dip::Image const& >( &dip::Trace ), // Fails to resolve!
          static_cast< dip::Image ( * )( dip::Image const& ) >( &dip::Trace ), "in"_a );
   m.def( "Trace", //py::overload_cast< dip::Image const&, dip::Image& >( &dip::Trace ), // Fails to resolve!
          static_cast< void ( * )( dip::Image const&, dip::Image& ) >( &dip::Trace ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Rank", py::overload_cast< dip::Image const& >( &dip::Rank ), "in"_a );
   m.def( "Rank", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Rank ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Eigenvalues", py::overload_cast< dip::Image const& >( &dip::Eigenvalues ), "in"_a );
   m.def( "Eigenvalues", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Eigenvalues ), "in"_a, py::kw_only(), "out"_a );
   m.def( "LargestEigenvalue", py::overload_cast< dip::Image const& >( &dip::LargestEigenvalue ), "in"_a );
   m.def( "LargestEigenvalue", py::overload_cast< dip::Image const&, dip::Image& >( &dip::LargestEigenvalue ), "in"_a, py::kw_only(), "out"_a );
   m.def( "SmallestEigenvalue", py::overload_cast< dip::Image const& >( &dip::SmallestEigenvalue ), "in"_a );
   m.def( "SmallestEigenvalue", py::overload_cast< dip::Image const&, dip::Image& >( &dip::SmallestEigenvalue ), "in"_a, py::kw_only(), "out"_a );
   m.def( "EigenDecomposition", []( dip::Image const& in ) {
             dip::Image out, eigenvectors;
             dip::EigenDecomposition( in, out, eigenvectors );
             return py::make_tuple( out, eigenvectors );
          }, "in"_a,
          "Returns a tuple containing the `out` image and the `eigenvectors` image." );
   m.def( "EigenDecomposition", py::overload_cast< dip::Image const&, dip::Image&, dip::Image& >( &dip::EigenDecomposition ), "in"_a, py::kw_only(), "out"_a, "eigenvectors"_a );
   m.def( "LargestEigenvector", py::overload_cast< dip::Image const& >( &dip::LargestEigenvector ), "in"_a );
   m.def( "LargestEigenvector", py::overload_cast< dip::Image const&, dip::Image& >( &dip::LargestEigenvector ), "in"_a, py::kw_only(), "out"_a );
   m.def( "SmallestEigenvector", py::overload_cast< dip::Image const& >( &dip::SmallestEigenvector ), "in"_a );
   m.def( "SmallestEigenvector", py::overload_cast< dip::Image const&, dip::Image& >( &dip::SmallestEigenvector ), "in"_a, py::kw_only(), "out"_a );
   m.def( "Inverse", py::overload_cast< dip::Image const& >( &dip::Inverse ), "in"_a );
   m.def( "Inverse", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Inverse ), "in"_a, py::kw_only(), "out"_a );
   m.def( "PseudoInverse", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::PseudoInverse ), "in"_a, "tolerance"_a = 1e-7 );
   m.def( "PseudoInverse", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::PseudoInverse ), "in"_a, py::kw_only(), "out"_a, "tolerance"_a = 1e-7 );
   m.def( "SingularValues", py::overload_cast< dip::Image const& >( &dip::SingularValues ), "in"_a );
   m.def( "SingularValues", py::overload_cast< dip::Image const&, dip::Image& >( &dip::SingularValues ), "in"_a, py::kw_only(), "out"_a );
   m.def( "SingularValueDecomposition", []( dip::Image const& in ) {
             dip::Image U, S, V;
             dip::SingularValueDecomposition( in, U, S, V );
             return py::make_tuple( U, S, V );
          }, "in"_a,
          "Returns a tuple containing the `U` image, the `S` image, and the `V` image." );
   m.def( "SingularValueDecomposition", py::overload_cast< dip::Image const&, dip::Image&, dip::Image&, dip::Image& >( &dip::SingularValueDecomposition ), "in"_a, py::kw_only(), "U"_a, "out"_a, "V"_a );
   m.def( "Identity", py::overload_cast< dip::Image const& >( &dip::Identity ), "in"_a );
   m.def( "Identity", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Identity ), "in"_a, py::kw_only(), "out"_a );

   m.def( "SumTensorElements", py::overload_cast< dip::Image const& >( &dip::SumTensorElements ), "in"_a );
   m.def( "SumTensorElements", py::overload_cast< dip::Image const&, dip::Image& >( &dip::SumTensorElements ), "in"_a, py::kw_only(), "out"_a );
   m.def( "ProductTensorElements", py::overload_cast< dip::Image const& >( &dip::ProductTensorElements ), "in"_a );
   m.def( "ProductTensorElements", py::overload_cast< dip::Image const&, dip::Image& >( &dip::ProductTensorElements ), "in"_a, py::kw_only(), "out"_a );
   m.def( "AllTensorElements", py::overload_cast< dip::Image const& >( &dip::AllTensorElements ), "in"_a );
   m.def( "AllTensorElements", py::overload_cast< dip::Image const&, dip::Image& >( &dip::AllTensorElements ), "in"_a, py::kw_only(), "out"_a );
   m.def( "AnyTensorElement", py::overload_cast< dip::Image const& >( &dip::AnyTensorElement ), "in"_a );
   m.def( "AnyTensorElement", py::overload_cast< dip::Image const&, dip::Image& >( &dip::AnyTensorElement ), "in"_a, py::kw_only(), "out"_a );
   m.def( "MaximumTensorElement", py::overload_cast< dip::Image const& >( &dip::MaximumTensorElement ), "in"_a );
   m.def( "MaximumTensorElement", py::overload_cast< dip::Image const&, dip::Image& >( &dip::MaximumTensorElement ), "in"_a, py::kw_only(), "out"_a );
   m.def( "MinimumTensorElement", py::overload_cast< dip::Image const& >( &dip::MinimumTensorElement ), "in"_a );
   m.def( "MinimumTensorElement", py::overload_cast< dip::Image const&, dip::Image& >( &dip::MinimumTensorElement ), "in"_a, py::kw_only(), "out"_a );
   m.def( "MeanTensorElement", py::overload_cast< dip::Image const& >( &dip::MeanTensorElement ), "in"_a );
   m.def( "MeanTensorElement", py::overload_cast< dip::Image const&, dip::Image& >( &dip::MeanTensorElement ), "in"_a, py::kw_only(), "out"_a );
   m.def( "GeometricMeanTensorElement", py::overload_cast< dip::Image const& >( &dip::GeometricMeanTensorElement ), "in"_a );
   m.def( "GeometricMeanTensorElement", py::overload_cast< dip::Image const&, dip::Image& >( &dip::GeometricMeanTensorElement ), "in"_a, py::kw_only(), "out"_a );

   m.def( "Select", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image const&, dip::String const& >( &dip::Select ),
          "in1"_a, "in2"_a, "in3"_a, "in4"_a, "selector"_a );
   m.def( "Select", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::String const& >( &dip::Select ),
          "in1"_a, "in2"_a, "in3"_a, "in4"_a, py::kw_only(), "out"_a, "selector"_a );
   m.def( "Select", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::Select ),
          "in1"_a, "in2"_a, "mask"_a );
   m.def( "Select", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image& >( &dip::Select ),
          "in1"_a, "in2"_a, "mask"_a, py::kw_only(), "out"_a );
   m.def( "Toggle", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::Toggle ),
          "in1"_a, "in2"_a, "in3"_a );
   m.def( "Toggle", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image& >( &dip::Toggle ),
          "in1"_a, "in2"_a, "in3"_a, py::kw_only(), "out"_a );

   // diplib/mapping.h
   m.def( "Clip", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Clip ),
          "in"_a, "low"_a = 0.0, "high"_a = 255.0, "mode"_a = dip::S::BOTH );
   m.def( "Clip", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Clip ),
          "in"_a, py::kw_only(), "out"_a, "low"_a = 0.0, "high"_a = 255.0, "mode"_a = dip::S::BOTH );
   m.def( "ClipLow", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::ClipLow ), "in"_a, "low"_a = 0.0 );
   m.def( "ClipLow", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::ClipLow ), "in"_a, py::kw_only(), "out"_a, "low"_a = 0.0 );
   m.def( "ClipHigh", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::ClipHigh ), "in"_a, "high"_a = 255.0 );
   m.def( "ClipHigh", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::ClipHigh ), "in"_a, py::kw_only(), "out"_a, "high"_a = 255.0 );
   m.def( "ErfClip", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::ErfClip ),
          "in"_a, "low"_a = 128.0, "high"_a = 64.0, "mode"_a = dip::S::RANGE );
   m.def( "ErfClip", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::ErfClip ),
          "in"_a, py::kw_only(), "out"_a, "low"_a = 128.0, "high"_a = 64.0, "mode"_a = dip::S::RANGE );
   m.def( "Zero", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::Zero ), "in"_a, "threshold"_a = 128.0 );
   m.def( "Zero", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::Zero ), "in"_a, py::kw_only(), "out"_a, "threshold"_a = 128.0 );
   m.def( "Shrinkage", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::Shrinkage ),
          "in"_a, "threshold"_a = 128.0 );
   m.def( "Shrinkage", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::Shrinkage ),
          "in"_a, py::kw_only(), "out"_a, "threshold"_a = 128.0 );
   m.def( "ContrastStretch", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::ContrastStretch ),
          "in"_a, "lowerBound"_a = 0.0, "upperBound"_a = 100.0, "outMin"_a = 0.0, "outMax"_a = 255.0, "method"_a = dip::S::LINEAR, "parameter1"_a = 1.0, "parameter2"_a = 0.0 );
   m.def( "ContrastStretch", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::ContrastStretch ),
          "in"_a, py::kw_only(), "out"_a, "lowerBound"_a = 0.0, "upperBound"_a = 100.0, "outMin"_a = 0.0, "outMax"_a = 255.0, "method"_a = dip::S::LINEAR, "parameter1"_a = 1.0, "parameter2"_a = 0.0 );

   m.def( "HistogramEqualization", py::overload_cast< dip::Image const&, dip::uint >( &dip::HistogramEqualization ),
          "in"_a, "nBins"_a = 256 );
   m.def( "HistogramEqualization", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::HistogramEqualization ),
          "in"_a, py::kw_only(), "out"_a, "nBins"_a = 256 );
   m.def( "HistogramMatching", py::overload_cast< dip::Image const&, dip::Histogram const& >( &dip::HistogramMatching ),
          "in"_a, "example"_a );
   m.def( "HistogramMatching", py::overload_cast< dip::Image const&, dip::Image&, dip::Histogram const& >( &dip::HistogramMatching ),
          "in"_a, py::kw_only(), "out"_a, "example"_a );
   m.def( "HistogramMatching", []( dip::Image const& in, dip::Image const& example ) {
             DIP_THROW_IF( example.Dimensionality() != 1, "Example histogram must be 1D" );
             dip::uint nBins = example.Size( 0 );
             // Create a histogram of the right dimensions
             dip::Histogram::Configuration config( 0.0, static_cast< int >( nBins ), 1.0 );
             dip::Histogram exampleHistogram( config );
             // Fill it with the input
             dip::Image guts = exampleHistogram.GetImage().QuickCopy();
             guts.Copy( example ); // Copies data from example to data segment in guts, which is shared with the image in exampleHistogram. This means we're changing the histogram.
             return dip::HistogramMatching( in, exampleHistogram );
          }, "in"_a, "example"_a,
          "Like the function above, but takes the example histogram as an image.\n"
          "For backwards compatibility." );

}
