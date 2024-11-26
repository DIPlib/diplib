/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022-2024, Cris Luengo.
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

#include <algorithm>
#include <sstream>
#include <utility>
#include <vector>

#include "pydip.h"
#include "diplib/linear.h"
#include "diplib/nonlinear.h"
#include "diplib/deconvolution.h"
#include "diplib/kernel.h"

namespace {

dip::String KernelRepr( dip::Kernel const& kernel ) {
   std::ostringstream os;
   os << '<' << kernel.ShapeString() << " Kernel";
   if( !kernel.IsCustom() ) {
      os << " with parameters " << kernel.Params();
   }
   if( kernel.HasWeights() ) {
      os << ", with weights";
   }
   if( kernel.IsMirrored() ) {
      os << ", mirrored";
   }
   os << '>';
   return os.str();
}

dip::String OneDFilterRepr( dip::OneDimensionalFilter const& filter ) {
   std::ostringstream os;
   os << "<OneDimensionalFilter with ";
   if( filter.isComplex ) {
      os << filter.filter.size() / 2 << " complex weights";
   } else {
      os << filter.filter.size() << " weights";
   }
   os << ", origin = " << filter.origin << ", symmetry = \"" << filter.symmetry << "\">";
   return os.str();
}

} // namespace

void init_filtering( py::module& m ) {

   auto kernel = py::class_< dip::Kernel >( m, "Kernel", doc_strings::dip·Kernel );
   kernel.def( py::init<>(), doc_strings::dip·Kernel·Kernel );
   kernel.def( py::init< dip::String const& >(), "shape"_a, doc_strings::dip·Kernel·Kernel·String·CL );
   kernel.def( py::init< dip::dfloat, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC, doc_strings::dip·Kernel·Kernel·dfloat··String·CL );
   kernel.def( py::init< dip::FloatArray, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC, doc_strings::dip·Kernel·Kernel·FloatArray··String·CL );
   kernel.def( py::init< dip::Image const& >(), "image"_a, doc_strings::dip·Kernel·Kernel·Image· );
   kernel.def( "Mirror", &dip::Kernel::Mirror, doc_strings::dip·Kernel·Mirror );
   kernel.def( "__repr__", &KernelRepr );
   py::implicitly_convertible< py::str, dip::Kernel >();
   py::implicitly_convertible< py::float_, dip::Kernel >();
   py::implicitly_convertible< py::int_, dip::Kernel >();
   py::implicitly_convertible< py::list, dip::Kernel >();
   py::implicitly_convertible< py::tuple, dip::Kernel >();
   py::implicitly_convertible< dip::Image, dip::Kernel >();
   py::implicitly_convertible< py::buffer, dip::Kernel >();

   auto odf = py::class_< dip::OneDimensionalFilter >( m, "OneDimensionalFilter", doc_strings::dip·OneDimensionalFilter );
   odf.def( py::init( []( std::vector< dip::dfloat > filter ) {
          dip::OneDimensionalFilter out;
          out.filter = std::move( filter );
          return out;
   } ), "filter"_a, "Create a OneDimensionalFilter object representing a general filter." );
   odf.def( py::init( []( std::vector< dip::dcomplex > const& filter ) {
          dip::OneDimensionalFilter out;
          dip::uint size = filter.size() * 2;
          out.filter.resize( size );
          std::copy_n( reinterpret_cast< dip::dfloat const* >( filter.data() ), size, out.filter.data() );
          out.isComplex = true;
          return out;
   } ), "filter"_a, "Create a OneDimensionalFilter object representing a general complex filter." );
   odf.def_readonly( "filter", &dip::OneDimensionalFilter::filter, doc_strings::dip·OneDimensionalFilter·filter );
   odf.def_readwrite( "origin", &dip::OneDimensionalFilter::origin, doc_strings::dip·OneDimensionalFilter·origin );
   odf.def_readwrite( "symmetry", &dip::OneDimensionalFilter::symmetry, doc_strings::dip·OneDimensionalFilter·symmetry );
   odf.def_readonly( "isComplex", &dip::OneDimensionalFilter::isComplex, doc_strings::dip·OneDimensionalFilter·isComplex );
   odf.def( "__repr__", &OneDFilterRepr );

   // diplib/linear.h
   m.def( "SeparateFilter", &dip::SeparateFilter, "filter"_a, doc_strings::dip·SeparateFilter·Image·CL );
   m.def( "SeparableConvolution", py::overload_cast< dip::Image const&, dip::OneDimensionalFilterArray const&, dip::StringArray const&, dip::BooleanArray >( &dip::SeparableConvolution ),
          "in"_a, "filter"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·SeparableConvolution·Image·CL·Image·L·OneDimensionalFilterArray·CL·StringArray·CL·BooleanArray· );
   m.def( "SeparableConvolution", py::overload_cast< dip::Image const&, dip::Image&, dip::OneDimensionalFilterArray const&, dip::StringArray const&, dip::BooleanArray >( &dip::SeparableConvolution ),
          "in"_a, py::kw_only(), "out"_a, "filter"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·SeparableConvolution·Image·CL·Image·L·OneDimensionalFilterArray·CL·StringArray·CL·BooleanArray· );
   m.def( "ConvolveFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::ConvolveFT ),
          "in"_a, "filter"_a, "inRepresentation"_a = dip::S::SPATIAL, "filterRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·ConvolveFT·Image·CL·Image·CL·Image·L·String·CL·String·CL·String·CL·StringArray·CL );
   m.def( "ConvolveFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::ConvolveFT ),
          "in"_a, "filter"_a, py::kw_only(), "out"_a, "inRepresentation"_a = dip::S::SPATIAL, "filterRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·ConvolveFT·Image·CL·Image·CL·Image·L·String·CL·String·CL·String·CL·StringArray·CL );
   m.def( "GeneralConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::StringArray const& >( &dip::GeneralConvolution ),
          "in"_a, "filter"_a, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·GeneralConvolution·Image·CL·Image·CL·Image·L·StringArray·CL );
   m.def( "GeneralConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::StringArray const& >( &dip::GeneralConvolution ),
          "in"_a, "filter"_a, py::kw_only(), "out"_a, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·GeneralConvolution·Image·CL·Image·CL·Image·L·StringArray·CL );
   m.def( "Convolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::StringArray const& >( &dip::Convolution ),
          "in"_a, "filter"_a, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Convolution·Image·CL·Image·CL·Image·L·String·CL·StringArray·CL );
   m.def( "Convolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::StringArray const& >( &dip::Convolution ),
          "in"_a, "filter"_a, py::kw_only(), "out"_a, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Convolution·Image·CL·Image·CL·Image·L·String·CL·StringArray·CL );
   m.def( "Uniform", py::overload_cast< dip::Image const&, dip::Kernel const&, dip::StringArray const& >( &dip::Uniform ),
          "in"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Uniform·Image·CL·Image·L·Kernel·CL·StringArray·CL );
   m.def( "Uniform", py::overload_cast< dip::Image const&, dip::Image&, dip::Kernel const&, dip::StringArray const& >( &dip::Uniform ),
          "in"_a, py::kw_only(), "out"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Uniform·Image·CL·Image·L·Kernel·CL·StringArray·CL );
   m.def( "GaussFIR", py::overload_cast< dip::Image const&, dip::FloatArray, dip::UnsignedArray, dip::StringArray const&, dip::dfloat >( &dip::GaussFIR ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·GaussFIR·Image·CL·Image·L·FloatArray··UnsignedArray··StringArray·CL·dfloat· );
   m.def( "GaussFIR", py::overload_cast< dip::Image const&, dip::FloatArray, dip::UnsignedArray, dip::StringArray const&, dip::dfloat >( &dip::GaussFIR ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·GaussFIR·Image·CL·Image·L·FloatArray··UnsignedArray··StringArray·CL·dfloat· );
   m.def( "GaussFT", py::overload_cast< dip::Image const&, dip::FloatArray, dip::UnsignedArray, dip::dfloat, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::GaussFT ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 3.0, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·GaussFT·Image·CL·Image·L·FloatArray··UnsignedArray··dfloat··String·CL·String·CL·StringArray·CL );
   m.def( "GaussFT", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::UnsignedArray, dip::dfloat, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::GaussFT ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 3.0, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·GaussFT·Image·CL·Image·L·FloatArray··UnsignedArray··dfloat··String·CL·String·CL·StringArray·CL );
   m.def( "GaussIIR", py::overload_cast< dip::Image const&, dip::FloatArray, dip::UnsignedArray, dip::StringArray const&, dip::UnsignedArray, dip::String const&, dip::dfloat >( &dip::GaussIIR ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "boundaryCondition"_a = dip::StringArray{}, "filterOrder"_a = dip::UnsignedArray{}, "designMethod"_a = dip::S::DISCRETE_TIME_FIT, "truncation"_a = 3.0, doc_strings::dip·GaussIIR·Image·CL·Image·L·FloatArray··UnsignedArray··StringArray·CL·UnsignedArray··String·CL·dfloat· );
   m.def( "GaussIIR", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::UnsignedArray, dip::StringArray const&, dip::UnsignedArray, dip::String const&, dip::dfloat >( &dip::GaussIIR ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "boundaryCondition"_a = dip::StringArray{}, "filterOrder"_a = dip::UnsignedArray{}, "designMethod"_a = dip::S::DISCRETE_TIME_FIT, "truncation"_a = 3.0, doc_strings::dip·GaussIIR·Image·CL·Image·L·FloatArray··UnsignedArray··StringArray·CL·UnsignedArray··String·CL·dfloat· );
   m.def( "Gauss", py::overload_cast< dip::Image const&, dip::FloatArray, dip::UnsignedArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Gauss ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Gauss·Image·CL·Image·L·FloatArray··UnsignedArray··String·CL·StringArray·CL·dfloat· );
   m.def( "Gauss", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::UnsignedArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Gauss ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Gauss·Image·CL·Image·L·FloatArray··UnsignedArray··String·CL·StringArray·CL·dfloat· );
   m.def( "FiniteDifference", py::overload_cast< dip::Image const&, dip::UnsignedArray, dip::String const&, dip::StringArray const&, dip::BooleanArray >( &dip::FiniteDifference ),
          "in"_a, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "smoothFlag"_a = dip::S::SMOOTH, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·FiniteDifference·Image·CL·Image·L·UnsignedArray··String·CL·StringArray·CL·BooleanArray· );
   m.def( "FiniteDifference", py::overload_cast< dip::Image const&, dip::Image&, dip::UnsignedArray, dip::String const&, dip::StringArray const&, dip::BooleanArray >( &dip::FiniteDifference ),
          "in"_a, py::kw_only(), "out"_a, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "smoothFlag"_a = dip::S::SMOOTH, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·FiniteDifference·Image·CL·Image·L·UnsignedArray··String·CL·StringArray·CL·BooleanArray· );
   m.def( "SobelGradient", py::overload_cast< dip::Image const&, dip::uint, dip::StringArray const& >( &dip::SobelGradient ),
          "in"_a, "dimension"_a = 0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·SobelGradient·Image·CL·Image·L·dip·uint··StringArray·CL );
   m.def( "SobelGradient", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::StringArray const& >( &dip::SobelGradient ),
          "in"_a, py::kw_only(), "out"_a, "dimension"_a = 0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·SobelGradient·Image·CL·Image·L·dip·uint··StringArray·CL );
   m.def( "Derivative", py::overload_cast< dip::Image const&, dip::UnsignedArray, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Derivative ),
          "in"_a, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Derivative·Image·CL·Image·L·UnsignedArray··FloatArray··String·CL·StringArray·CL·dfloat· );
   m.def( "Derivative", py::overload_cast< dip::Image const&, dip::Image&, dip::UnsignedArray, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Derivative ),
          "in"_a, py::kw_only(), "out"_a, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Derivative·Image·CL·Image·L·UnsignedArray··FloatArray··String·CL·StringArray·CL·dfloat· );
   m.def( "Dx", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dx ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dx·Image·CL·Image·L·FloatArray· );
   m.def( "Dx", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dx ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dx·Image·CL·Image·L·FloatArray· );
   m.def( "Dy", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dy ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dy·Image·CL·Image·L·FloatArray· );
   m.def( "Dy", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dy ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dy·Image·CL·Image·L·FloatArray· );
   m.def( "Dz", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dz ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dz·Image·CL·Image·L·FloatArray· );
   m.def( "Dz", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dz ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dz·Image·CL·Image·L·FloatArray· );
   m.def( "Dxx", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dxx ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dxx·Image·CL·Image·L·FloatArray· );
   m.def( "Dxx", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dxx ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dxx·Image·CL·Image·L·FloatArray· );
   m.def( "Dyy", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dyy ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dyy·Image·CL·Image·L·FloatArray· );
   m.def( "Dyy", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dyy ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dyy·Image·CL·Image·L·FloatArray· );
   m.def( "Dzz", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dzz ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dzz·Image·CL·Image·L·FloatArray· );
   m.def( "Dzz", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dzz ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dzz·Image·CL·Image·L·FloatArray· );
   m.def( "Dxy", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dxy ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dxy·Image·CL·Image·L·FloatArray· );
   m.def( "Dxy", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dxy ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dxy·Image·CL·Image·L·FloatArray· );
   m.def( "Dxz", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dxz ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dxz·Image·CL·Image·L·FloatArray· );
   m.def( "Dxz", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dxz ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dxz·Image·CL·Image·L·FloatArray· );
   m.def( "Dyz", py::overload_cast< dip::Image const&, dip::FloatArray >( &dip::Dyz ), "in"_a, "sigma"_a = 1.0, doc_strings::dip·Dyz·Image·CL·Image·L·FloatArray· );
   m.def( "Dyz", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray >( &dip::Dyz ), "in"_a, py::kw_only(), "out"_a, "sigma"_a = 1.0, doc_strings::dip·Dyz·Image·CL·Image·L·FloatArray· );
   m.def( "Gradient", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Gradient ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Gradient·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Gradient", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Gradient ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Gradient·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "GradientMagnitude", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::GradientMagnitude ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·GradientMagnitude·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "GradientMagnitude", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::GradientMagnitude ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·GradientMagnitude·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "GradientDirection", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::GradientDirection ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·GradientDirection·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "GradientDirection", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::GradientDirection ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·GradientDirection·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Curl", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Curl ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Curl·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Curl", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Curl ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Curl·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Divergence", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Divergence ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Divergence·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Divergence", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Divergence ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Divergence·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Hessian", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Hessian ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Hessian·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Hessian", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Hessian ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Hessian·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Laplace", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Laplace ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Laplace·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Laplace", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Laplace ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Laplace·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Dgg", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Dgg ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Dgg·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Dgg", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::Dgg ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·Dgg·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "LaplacePlusDgg", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::LaplacePlusDgg ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·LaplacePlusDgg·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "LaplacePlusDgg", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::LaplacePlusDgg ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·LaplacePlusDgg·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "LaplaceMinusDgg", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::LaplaceMinusDgg ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·LaplaceMinusDgg·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "LaplaceMinusDgg", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::LaplaceMinusDgg ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·LaplaceMinusDgg·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "Sharpen", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Sharpen ),
          "in"_a, "weight"_a = 1.0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Sharpen·Image·CL·Image·L·dfloat··FloatArray··String·CL·StringArray·CL·dfloat· );
   m.def( "Sharpen", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Sharpen ),
          "in"_a, py::kw_only(), "out"_a, "weight"_a = 1.0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Sharpen·Image·CL·Image·L·dfloat··FloatArray··String·CL·StringArray·CL·dfloat· );
   m.def( "UnsharpMask", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::UnsharpMask ),
          "in"_a, "weight"_a = 1.0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·UnsharpMask·Image·CL·Image·L·dfloat··FloatArray··String·CL·StringArray·CL·dfloat· );
   m.def( "UnsharpMask", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::UnsharpMask ),
          "in"_a, py::kw_only(), "out"_a, "weight"_a = 1.0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·UnsharpMask·Image·CL·Image·L·dfloat··FloatArray··String·CL·StringArray·CL·dfloat· );
   m.def( "GaborFIR", py::overload_cast< dip::Image const&, dip::FloatArray, dip::FloatArray const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::GaborFIR ),
          "in"_a, "sigmas"_a, "frequencies"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·GaborFIR·Image·CL·Image·L·FloatArray··FloatArray·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "GaborFIR", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::FloatArray const&, dip::StringArray const&, dip::BooleanArray, dip::dfloat >( &dip::GaborFIR ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a, "frequencies"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0, doc_strings::dip·GaborFIR·Image·CL·Image·L·FloatArray··FloatArray·CL·StringArray·CL·BooleanArray··dfloat· );
   m.def( "GaborIIR", py::overload_cast< dip::Image const&, dip::FloatArray, dip::FloatArray const&, dip::StringArray const&, dip::BooleanArray, dip::IntegerArray const&, dip::dfloat >( &dip::GaborIIR ),
          "in"_a, "sigmas"_a, "frequencies"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "order"_a = dip::IntegerArray{}, "truncation"_a = 3.0, doc_strings::dip·GaborIIR·Image·CL·Image·L·FloatArray··FloatArray·CL·StringArray·CL·BooleanArray··IntegerArray·CL·dfloat· );
   m.def( "GaborIIR", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::FloatArray const&, dip::StringArray const&, dip::BooleanArray, dip::IntegerArray const&, dip::dfloat >( &dip::GaborIIR ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a, "frequencies"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "order"_a = dip::IntegerArray{}, "truncation"_a = 3.0, doc_strings::dip·GaborIIR·Image·CL·Image·L·FloatArray··FloatArray·CL·StringArray·CL·BooleanArray··IntegerArray·CL·dfloat· );
   m.def( "Gabor2D", py::overload_cast< dip::Image const&, dip::FloatArray, dip::dfloat, dip::dfloat, dip::StringArray const&, dip::dfloat >( &dip::Gabor2D ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 5.0, 5.0 }, "frequency"_a = 0.1, "direction"_a = dip::pi, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Gabor2D·Image·CL·Image·L·FloatArray··dfloat··dfloat··StringArray·CL·dfloat· );
   m.def( "Gabor2D", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::dfloat, dip::dfloat, dip::StringArray const&, dip::dfloat >( &dip::Gabor2D ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 5.0, 5.0 }, "frequency"_a = 0.1, "direction"_a = dip::pi, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·Gabor2D·Image·CL·Image·L·FloatArray··dfloat··dfloat··StringArray·CL·dfloat· );
   m.def( "LogGaborFilterBank", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::uint, dip::String const&, dip::String const& >( &dip::LogGaborFilterBank ),
          "in"_a, "wavelengths"_a = dip::FloatArray{ 3.0, 6.0, 12.0, 24.0 }, "bandwidth"_a = 0.75, "nOrientations"_a = 6, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, doc_strings::dip·LogGaborFilterBank·Image·CL·Image·L·FloatArray·CL·dfloat··dip·uint··String·CL·String·CL );
   m.def( "LogGaborFilterBank", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::dfloat, dip::uint, dip::String const&, dip::String const& >( &dip::LogGaborFilterBank ),
          "in"_a, py::kw_only(), "out"_a, "wavelengths"_a = dip::FloatArray{ 3.0, 6.0, 12.0, 24.0 }, "bandwidth"_a = 0.75, "nOrientations"_a = 6, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, doc_strings::dip·LogGaborFilterBank·Image·CL·Image·L·FloatArray·CL·dfloat··dip·uint··String·CL·String·CL );
   m.def( "NormalizedConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::NormalizedConvolution ),
          "in"_a, "mask"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{ dip::S::ADD_ZEROS }, "truncation"_a = 3.0, doc_strings::dip·NormalizedConvolution·Image·CL·Image·CL·Image·L·FloatArray·CL·String·CL·StringArray·CL·dfloat· );
   m.def( "NormalizedConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::NormalizedConvolution ),
          "in"_a, "mask"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{ dip::S::ADD_ZEROS }, "truncation"_a = 3.0, doc_strings::dip·NormalizedConvolution·Image·CL·Image·CL·Image·L·FloatArray·CL·String·CL·StringArray·CL·dfloat· );
   m.def( "NormalizedDifferentialConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::NormalizedDifferentialConvolution ),
          "in"_a, "mask"_a, "dimension"_a = 0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{ dip::S::ADD_ZEROS }, "truncation"_a = 3.0, doc_strings::dip·NormalizedDifferentialConvolution·Image·CL·Image·CL·Image·L·dip·uint··FloatArray·CL·String·CL·StringArray·CL·dfloat· );
   m.def( "NormalizedDifferentialConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::NormalizedDifferentialConvolution ),
          "in"_a, "mask"_a, py::kw_only(), "out"_a, "dimension"_a = 0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{ dip::S::ADD_ZEROS }, "truncation"_a = 3.0, doc_strings::dip·NormalizedDifferentialConvolution·Image·CL·Image·CL·Image·L·dip·uint··FloatArray·CL·String·CL·StringArray·CL·dfloat· );
   m.def( "MeanShiftVector", py::overload_cast< dip::Image const&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::MeanShiftVector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·MeanShiftVector·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·dfloat· );
   m.def( "MeanShiftVector", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::MeanShiftVector ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·MeanShiftVector·Image·CL·Image·L·FloatArray··String·CL·StringArray·CL·dfloat· );

   // diplib/nonlinear.h
   m.def( "PercentileFilter", py::overload_cast< dip::Image const&, dip::dfloat, dip::Kernel const&, dip::StringArray const& >( &dip::PercentileFilter ),
          "in"_a, "percentile"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·PercentileFilter·Image·CL·Image·L·dfloat··Kernel·CL·StringArray·CL );
   m.def( "PercentileFilter", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::Kernel const&, dip::StringArray const& >( &dip::PercentileFilter ),
          "in"_a, py::kw_only(), "out"_a, "percentile"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·PercentileFilter·Image·CL·Image·L·dfloat··Kernel·CL·StringArray·CL );
   m.def( "MedianFilter", py::overload_cast< dip::Image const&, dip::Kernel const&, dip::StringArray const& >( &dip::MedianFilter ),
          "in"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MedianFilter·Image·CL·Image·L·Kernel·CL·StringArray·CL );
   m.def( "MedianFilter", py::overload_cast< dip::Image const&, dip::Image&, dip::Kernel const&, dip::StringArray const& >( &dip::MedianFilter ),
          "in"_a, py::kw_only(), "out"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MedianFilter·Image·CL·Image·L·Kernel·CL·StringArray·CL );
   m.def( "VarianceFilter", py::overload_cast< dip::Image const&, dip::Kernel const&, dip::StringArray const& >( &dip::VarianceFilter ),
          "in"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·VarianceFilter·Image·CL·Image·L·Kernel·CL·StringArray·CL );
   m.def( "VarianceFilter", py::overload_cast< dip::Image const&, dip::Image&, dip::Kernel const&, dip::StringArray const& >( &dip::VarianceFilter ),
          "in"_a, py::kw_only(), "out"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·VarianceFilter·Image·CL·Image·L·Kernel·CL·StringArray·CL );
   m.def( "SelectionFilter", py::overload_cast< dip::Image const&, dip::Image const&, dip::Kernel const&, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::SelectionFilter ),
          "in"_a, "control"_a, "kernel"_a = dip::Kernel{}, "threshold"_a = 0.0, "mode"_a = dip::S::MINIMUM, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·SelectionFilter·Image·CL·Image·CL·Image·L·Kernel·CL·dfloat··String·CL·StringArray·CL );
   m.def( "SelectionFilter", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::Kernel const&, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::SelectionFilter ),
          "in"_a, py::kw_only(), "out"_a, "control"_a, "kernel"_a = dip::Kernel{}, "threshold"_a = 0.0, "mode"_a = dip::S::MINIMUM, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·SelectionFilter·Image·CL·Image·CL·Image·L·Kernel·CL·dfloat··String·CL·StringArray·CL );
   m.def( "Kuwahara", py::overload_cast< dip::Image const&, dip::Kernel, dip::dfloat, dip::StringArray const& >( &dip::Kuwahara ),
          "in"_a, "kernel"_a = dip::Kernel{}, "threshold"_a = 0.0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Kuwahara·Image·CL·Image·L·Kernel··dfloat··StringArray·CL );
   m.def( "Kuwahara", py::overload_cast< dip::Image const&, dip::Image&, dip::Kernel, dip::dfloat, dip::StringArray const& >( &dip::Kuwahara ),
          "in"_a, py::kw_only(), "out"_a, "kernel"_a = dip::Kernel{}, "threshold"_a = 0.0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Kuwahara·Image·CL·Image·L·Kernel··dfloat··StringArray·CL );
   m.def( "NonMaximumSuppression", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::String const& >( &dip::NonMaximumSuppression ),
          "gradmag"_a, "gradient"_a, "mask"_a = dip::Image{}, "mode"_a = dip::S::INTERPOLATE, doc_strings::dip·NonMaximumSuppression·Image·CL·Image·CL·Image·CL·Image·L·String·CL );
   m.def( "NonMaximumSuppression", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::String const& >( &dip::NonMaximumSuppression ),
          "gradmag"_a, "gradient"_a, py::kw_only(), "mask"_a = dip::Image{}, "out"_a, "mode"_a = dip::S::INTERPOLATE, doc_strings::dip·NonMaximumSuppression·Image·CL·Image·CL·Image·CL·Image·L·String·CL );
   m.def( "MoveToLocalMinimum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::MoveToLocalMinimum ),
          "bin"_a, "weights"_a, doc_strings::dip·MoveToLocalMinimum·Image·CL·Image·CL·Image·L );
   m.def( "MoveToLocalMinimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::MoveToLocalMinimum ),
          "bin"_a, "weights"_a, py::kw_only(), "out"_a, doc_strings::dip·MoveToLocalMinimum·Image·CL·Image·CL·Image·L );
   m.def( "PeronaMalikDiffusion", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat, dip::String const& >( &dip::PeronaMalikDiffusion ),
          "in"_a, "iterations"_a = 5, "K"_a = 10, "stepSizeLambda"_a = 0.25, "g"_a = "Gauss", doc_strings::dip·PeronaMalikDiffusion·Image·CL·Image·L·dip·uint··dfloat··dfloat··String·CL );
   m.def( "PeronaMalikDiffusion", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::dfloat, dip::String const& >( &dip::PeronaMalikDiffusion ),
          "in"_a, py::kw_only(), "out"_a, "iterations"_a = 5, "K"_a = 10, "stepSizeLambda"_a = 0.25, "g"_a = "Gauss", doc_strings::dip·PeronaMalikDiffusion·Image·CL·Image·L·dip·uint··dfloat··dfloat··String·CL );
   m.def( "GaussianAnisotropicDiffusion", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat, dip::String const& >( &dip::GaussianAnisotropicDiffusion ),
          "in"_a, "iterations"_a = 5, "K"_a = 10, "stepSizeLambda"_a = 0.25, "g"_a = "Gauss", doc_strings::dip·GaussianAnisotropicDiffusion·Image·CL·Image·L·dip·uint··dfloat··dfloat··String·CL );
   m.def( "GaussianAnisotropicDiffusion", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::dfloat, dip::String const& >( &dip::GaussianAnisotropicDiffusion ),
          "in"_a, py::kw_only(), "out"_a, "iterations"_a = 5, "K"_a = 10, "stepSizeLambda"_a = 0.25, "g"_a = "Gauss", doc_strings::dip·GaussianAnisotropicDiffusion·Image·CL·Image·L·dip·uint··dfloat··dfloat··String·CL );
   m.def( "RobustAnisotropicDiffusion", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat >( &dip::RobustAnisotropicDiffusion ),
          "in"_a, "iterations"_a = 5, "sigma"_a = 10, "stepSizeLambda"_a = 0.25, doc_strings::dip·RobustAnisotropicDiffusion·Image·CL·Image·L·dip·uint··dfloat··dfloat· );
   m.def( "RobustAnisotropicDiffusion", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::dfloat >( &dip::RobustAnisotropicDiffusion ),
          "in"_a, py::kw_only(), "out"_a, "iterations"_a = 5, "sigma"_a = 10, "stepSizeLambda"_a = 0.25, doc_strings::dip·RobustAnisotropicDiffusion·Image·CL·Image·L·dip·uint··dfloat··dfloat· );
   m.def( "CoherenceEnhancingDiffusion", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::CoherenceEnhancingDiffusion ),
          "in"_a, "derivativeSigma"_a = 1, "regularizationSigma"_a = 3, "iterations"_a = 5, "flags"_a = dip::StringSet{}, doc_strings::dip·CoherenceEnhancingDiffusion·Image·CL·Image·L·dfloat··dfloat··dip·uint··StringSet·CL );
   m.def( "CoherenceEnhancingDiffusion", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::CoherenceEnhancingDiffusion ),
          "in"_a, py::kw_only(), "out"_a, "derivativeSigma"_a = 1, "regularizationSigma"_a = 3, "iterations"_a = 5, "flags"_a = dip::StringSet{}, doc_strings::dip·CoherenceEnhancingDiffusion·Image·CL·Image·L·dfloat··dfloat··dip·uint··StringSet·CL );
   m.def( "AdaptiveGauss", py::overload_cast< dip::Image const&, dip::ImageConstRefArray const&, dip::FloatArray const&, dip::UnsignedArray const&, dip::dfloat, dip::UnsignedArray const&, dip::String const&, dip::String const& >( &dip::AdaptiveGauss ),
          "in"_a, "params"_a, "sigmas"_a = dip::FloatArray{ 5.0, 1.0 }, "orders"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 2.0, "exponents"_a = dip::UnsignedArray{ 0 }, "interpolationMethod"_a = dip::S::LINEAR, "boundaryCondition"_a = dip::S::SYMMETRIC_MIRROR, doc_strings::dip·AdaptiveGauss·Image·CL·ImageConstRefArray·CL·Image·L·FloatArray·CL·UnsignedArray·CL·dfloat··UnsignedArray·CL·String·CL·String·CL );
   m.def( "AdaptiveGauss", py::overload_cast< dip::Image const&, dip::ImageConstRefArray const&, dip::Image&, dip::FloatArray const&, dip::UnsignedArray const&, dip::dfloat, dip::UnsignedArray const&, dip::String const&, dip::String const& >( &dip::AdaptiveGauss ),
          "in"_a, "params"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 5.0, 1.0 }, "orders"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 2.0, "exponents"_a = dip::UnsignedArray{ 0 }, "interpolationMethod"_a = dip::S::LINEAR, "boundaryCondition"_a = dip::S::SYMMETRIC_MIRROR, doc_strings::dip·AdaptiveGauss·Image·CL·ImageConstRefArray·CL·Image·L·FloatArray·CL·UnsignedArray·CL·dfloat··UnsignedArray·CL·String·CL·String·CL );
   m.def( "AdaptiveBanana", py::overload_cast< dip::Image const&, dip::ImageConstRefArray const&, dip::FloatArray const&, dip::UnsignedArray const&, dip::dfloat, dip::UnsignedArray const&, dip::String const&, dip::String const& >( &dip::AdaptiveBanana ),
          "in"_a, "params"_a, "sigmas"_a = dip::FloatArray{ 5.0, 1.0 }, "orders"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 2.0, "exponents"_a = dip::UnsignedArray{ 0 }, "interpolationMethod"_a = dip::S::LINEAR, "boundaryCondition"_a = dip::S::SYMMETRIC_MIRROR, doc_strings::dip·AdaptiveBanana·Image·CL·ImageConstRefArray·CL·Image·L·FloatArray·CL·UnsignedArray·CL·dfloat··UnsignedArray·CL·String·CL·String·CL );
   m.def( "AdaptiveBanana", py::overload_cast< dip::Image const&, dip::ImageConstRefArray const&, dip::Image&, dip::FloatArray const&, dip::UnsignedArray const&, dip::dfloat, dip::UnsignedArray const&, dip::String const&, dip::String const& >( &dip::AdaptiveBanana ),
          "in"_a, "params"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 5.0, 1.0 }, "orders"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 2.0, "exponents"_a = dip::UnsignedArray{ 0 }, "interpolationMethod"_a = dip::S::LINEAR, "boundaryCondition"_a = dip::S::SYMMETRIC_MIRROR, doc_strings::dip·AdaptiveBanana·Image·CL·ImageConstRefArray·CL·Image·L·FloatArray·CL·UnsignedArray·CL·dfloat··UnsignedArray·CL·String·CL·String·CL );
   m.def( "BilateralFilter", py::overload_cast< dip::Image const&, dip::Image const&, dip::FloatArray, dip::dfloat, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::BilateralFilter ),
          "in"_a, "estimate"_a = dip::Image{}, "spatialSigmas"_a = dip::FloatArray{ 2.0 }, "tonalSigma"_a = 30.0, "truncation"_a = 2.0, "method"_a = "xysep", "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·BilateralFilter·Image·CL·Image·CL·Image·L·FloatArray··dfloat··dfloat··String·CL·StringArray·CL );
   m.def( "BilateralFilter", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::FloatArray, dip::dfloat, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::BilateralFilter ),
          "in"_a, "estimate"_a, py::kw_only(), "out"_a, "spatialSigmas"_a = dip::FloatArray{ 2.0 }, "tonalSigma"_a = 30.0, "truncation"_a = 2.0, "method"_a = "xysep", "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·BilateralFilter·Image·CL·Image·CL·Image·L·FloatArray··dfloat··dfloat··String·CL·StringArray·CL );

   // diplib/deconvolution.h
   m.def( "WienerDeconvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image const&, dip::StringSet const& >( &dip::WienerDeconvolution ),
          "in"_a, "psf"_a, "signalPower"_a, "noisePower"_a, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·WienerDeconvolution·Image·CL·Image·CL·Image·CL·Image·CL·Image·L·StringSet·CL );
   m.def( "WienerDeconvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::StringSet const& >( &dip::WienerDeconvolution ),
          "in"_a, "psf"_a, "signalPower"_a, "noisePower"_a, py::kw_only(), "out"_a, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·WienerDeconvolution·Image·CL·Image·CL·Image·CL·Image·CL·Image·L·StringSet·CL );
   m.def( "WienerDeconvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::StringSet const& >( &dip::WienerDeconvolution ),
          "in"_a, "psf"_a, "regularization"_a = 1e-4, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·WienerDeconvolution·Image·CL·Image·CL·Image·L·dfloat··StringSet·CL );
   m.def( "WienerDeconvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::StringSet const& >( &dip::WienerDeconvolution ),
          "in"_a, "psf"_a, py::kw_only(), "out"_a, "regularization"_a = 1e-4, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·WienerDeconvolution·Image·CL·Image·CL·Image·L·dfloat··StringSet·CL );
   m.def( "TikhonovMiller", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::StringSet const& >( &dip::TikhonovMiller ),
          "in"_a, "psf"_a, "regularization"_a = 0.1, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·TikhonovMiller·Image·CL·Image·CL·Image·L·dfloat··StringSet·CL );
   m.def( "TikhonovMiller", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::StringSet const& >( &dip::TikhonovMiller ),
          "in"_a, "psf"_a, py::kw_only(), "out"_a, "regularization"_a = 0.1, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·TikhonovMiller·Image·CL·Image·CL·Image·L·dfloat··StringSet·CL );
   m.def( "IterativeConstrainedTikhonovMiller", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat, dip::uint, dip::dfloat, dip::StringSet const& >( &dip::IterativeConstrainedTikhonovMiller ),
          "in"_a, "psf"_a, "regularization"_a = 0.1, "tolerance"_a = 1e-6, "maxIterations"_a = 30, "stepSize"_a = 0.0, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·IterativeConstrainedTikhonovMiller·Image·CL·Image·CL·Image·L·dfloat··dfloat··dip·uint··dfloat··StringSet·CL );
   m.def( "IterativeConstrainedTikhonovMiller", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::uint, dip::dfloat, dip::StringSet const& >( &dip::IterativeConstrainedTikhonovMiller ),
          "in"_a, "psf"_a, py::kw_only(), "out"_a, "regularization"_a = 0.1, "tolerance"_a = 1e-6, "maxIterations"_a = 30, "stepSize"_a = 0.0, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·IterativeConstrainedTikhonovMiller·Image·CL·Image·CL·Image·L·dfloat··dfloat··dip·uint··dfloat··StringSet·CL );
   m.def( "RichardsonLucy", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::RichardsonLucy ),
          "in"_a, "psf"_a, "regularization"_a = 0.0, "nIterations"_a = 30, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·RichardsonLucy·Image·CL·Image·CL·Image·L·dfloat··dip·uint··StringSet·CL );
   m.def( "RichardsonLucy", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::RichardsonLucy ),
          "in"_a, "psf"_a, py::kw_only(), "out"_a, "regularization"_a = 0.0, "nIterations"_a = 30, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·RichardsonLucy·Image·CL·Image·CL·Image·L·dfloat··dip·uint··StringSet·CL );
   m.def( "FastIterativeShrinkageThresholding", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat, dip::uint, dip::uint, dip::StringSet const& >( &dip::FastIterativeShrinkageThresholding ),
          "in"_a, "psf"_a, "regularization"_a = 0.1, "tolerance"_a = 1e-6, "maxIterations"_a = 30, "nScales"_a = 3, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·FastIterativeShrinkageThresholding·Image·CL·Image·CL·Image·L·dfloat··dfloat··dip·uint··dip·uint··StringSet·CL );
   m.def( "FastIterativeShrinkageThresholding", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::uint, dip::uint, dip::StringSet const& >( &dip::FastIterativeShrinkageThresholding ),
          "in"_a, "psf"_a, py::kw_only(), "out"_a, "regularization"_a = 0.1, "tolerance"_a = 1e-6, "maxIterations"_a = 30, "nScales"_a = 3, "options"_a = dip::StringSet{ dip::S::PAD }, doc_strings::dip·FastIterativeShrinkageThresholding·Image·CL·Image·CL·Image·L·dfloat··dfloat··dip·uint··dip·uint··StringSet·CL );

}
