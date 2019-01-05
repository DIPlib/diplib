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
#include "diplib/nonlinear.h"
#include "diplib/transform.h"

namespace {

dip::String KernelRepr( dip::Kernel const& s ) {
   std::ostringstream os;
   os << "<" << s.ShapeString() << " Kernel with parameters " << s.Params();
   if( s.HasWeights() ) {
      os << ", with weights";
   }
   if( s.IsMirrored() ) {
      os << ", mirrored";
   }
   os << ">";
   return os.str();
}

} // namespace

void init_filtering( py::module& m ) {
   auto kernel = py::class_< dip::Kernel >( m, "Kernel", "Represents the kernel to use in filtering operations." );
   kernel.def( py::init<>() );
   kernel.def( py::init< dip::Image const& >(), "image"_a );
   kernel.def( py::init< dip::String const& >(), "shape"_a );
   kernel.def( py::init< dip::dfloat, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC );
   kernel.def( py::init< dip::FloatArray, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC );
   kernel.def( "Mirror", &dip::Kernel::Mirror );
   kernel.def( "__repr__", &KernelRepr );
   py::implicitly_convertible< py::buffer, dip::Kernel >();
   py::implicitly_convertible< py::str, dip::Kernel >();
   py::implicitly_convertible< py::float_, dip::Kernel >();
   py::implicitly_convertible< py::int_, dip::Kernel >();
   py::implicitly_convertible< py::list, dip::Kernel >();

   // diplib/linear.h
   // TODO: SeparableConvolution, SeparateFilter
   m.def( "ConvolveFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const&, dip::String const& >( &dip::ConvolveFT ),
          "in"_a, "filter"_a, "inRepresentation"_a = dip::S::SPATIAL, "filterRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL );
   m.def( "GeneralConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::StringArray const& >( &dip::GeneralConvolution ),
          "in"_a, "filter"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Uniform", py::overload_cast< dip::Image const&, dip::Kernel const&, dip::StringArray const& >( &dip::Uniform ),
          "in"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Gauss", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::UnsignedArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Gauss ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "FiniteDifference", py::overload_cast< dip::Image const&, dip::UnsignedArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const& >( &dip::FiniteDifference ),
          "in"_a, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "smoothFlag"_a = dip::S::SMOOTH, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{} );
   m.def( "SobelGradient", py::overload_cast< dip::Image const&, dip::uint, dip::StringArray const& >( &dip::SobelGradient ),
          "in"_a, "dimension"_a = 0, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Derivative", py::overload_cast< dip::Image const&, dip::UnsignedArray const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Derivative ),
          "in"_a, "derivativeOrder"_a = dip::UnsignedArray{ 0 }, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "Dx", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dx( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dy", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dy( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dxx", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dxx( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dyy", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dyy( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dzz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dzz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dxy", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dxy( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dxz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dxz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Dyz", []( dip::Image const& in, dip::dfloat sigma ) { return dip::Dyz( in, { sigma } ); }, "in"_a, "sigma"_a = 1.0 );
   m.def( "Gradient", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::Gradient ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "GradientMagnitude", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::GradientMagnitude ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "GradientDirection", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::GradientDirection ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "Curl", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::Curl ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "Divergence", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::Divergence ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "Hessian", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::Hessian ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "Laplace", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::Laplace ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "Dgg", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::Dgg ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "LaplacePlusDgg", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::LaplacePlusDgg ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "LaplaceMinusDgg", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::LaplaceMinusDgg ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "Sharpen", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::Sharpen ),
          "in"_a, "weight"_a = 1.0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "UnsharpMask", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::UnsharpMask ),
          "in"_a, "weight"_a = 1.0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "GaborFIR", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::FloatArray const&, dip::StringArray const&, dip::BooleanArray const&, dip::dfloat >( &dip::GaborFIR ),
          "in"_a, "sigmas"_a, "frequencies"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "truncation"_a = 3.0 );
   m.def( "GaborIIR", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::FloatArray const&, dip::StringArray const&, dip::BooleanArray const&, dip::IntegerArray const&, dip::dfloat >( &dip::GaborIIR ),
          "in"_a, "sigmas"_a, "frequencies"_a, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, "order"_a = dip::IntegerArray{}, "truncation"_a = 3.0 );
   m.def( "Gabor2D", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::dfloat, dip::StringArray const&, dip::dfloat >( &dip::Gabor2D ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 5.0, 5.0 }, "frequency"_a = 0.1, "direction"_a = dip::pi, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "LogGaborFilterBank", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::uint, dip::String const&, dip::String const& >( &dip::LogGaborFilterBank ),
          "in"_a, "wavelengths"_a = dip::FloatArray{ 3.0, 6.0, 12.0, 24.0 }, "bandwidth"_a = 0.75, "nOrientations"_a = 6, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL );
   m.def( "NormalizedConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::NormalizedConvolution ),
          "in"_a, "mask"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray { dip::S::ADD_ZEROS }, "truncation"_a = 3.0 );
   m.def( "NormalizedDifferentialConvolution", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::NormalizedDifferentialConvolution ),
          "in"_a, "mask"_a, "dimension"_a = 0, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray { dip::S::ADD_ZEROS }, "truncation"_a = 3.0 );
   m.def( "MeanShiftVector", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::MeanShiftVector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );

   // diplib/nonlinear.h
   m.def( "Kuwahara", py::overload_cast< dip::Image const&, dip::Kernel const&, dip::dfloat, dip::StringArray const& >( &dip::Kuwahara ),
          "in"_a, "kernel"_a = dip::Kernel{}, "threshold"_a = 0.0, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "SelectionFilter", py::overload_cast< dip::Image const&, dip::Image const&, dip::Kernel const&, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::SelectionFilter ),
          "in"_a, "control"_a, "kernel"_a = dip::Kernel{}, "threshold"_a = 0.0, "mode"_a = dip::S::MINIMUM, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "VarianceFilter", py::overload_cast< dip::Image const&, dip::Kernel const&, dip::StringArray const& >( &dip::VarianceFilter ),
          "in"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MedianFilter", py::overload_cast< dip::Image const&, dip::Kernel const&, dip::StringArray const& >( &dip::MedianFilter ),
          "in"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "PercentileFilter", py::overload_cast< dip::Image const&, dip::dfloat, dip::Kernel const&, dip::StringArray const& >( &dip::PercentileFilter ),
          "in"_a, "percentile"_a, "kernel"_a = dip::Kernel{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "NonMaximumSuppression", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::String const& >( &dip::NonMaximumSuppression ),
          "gradmag"_a, "gradient"_a, "mask"_a = dip::Image{}, "mode"_a = dip::S::INTERPOLATE );
   m.def( "PeronaMalikDiffusion", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat, dip::String const& >( &dip::PeronaMalikDiffusion ),
          "in"_a, "iterations"_a = 5, "K"_a = 10, "lambda"_a = 0.25, "g"_a = "Gauss" );
   m.def( "GaussianAnisotropicDiffusion", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat, dip::String const& >( &dip::GaussianAnisotropicDiffusion ),
          "in"_a, "iterations"_a = 5, "K"_a = 10, "lambda"_a = 0.25, "g"_a = "Gauss" );
   m.def( "RobustAnisotropicDiffusion", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat >( &dip::RobustAnisotropicDiffusion ),
          "in"_a, "iterations"_a = 5, "sigma"_a = 10, "lambda"_a = 0.25 );
   m.def( "CoherenceEnhancingDiffusion", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::CoherenceEnhancingDiffusion ),
          "in"_a, "derivativeSigma"_a = 1, "regularizationSigma"_a = 3, "iterations"_a = 5, "flags"_a = dip::StringSet{} );
   m.def( "AdaptiveGauss", py::overload_cast< dip::Image const&, dip::ImageConstRefArray const&, dip::FloatArray const&, dip::UnsignedArray const&, dip::dfloat, dip::UnsignedArray const&, dip::String const&, dip::String const& >( &dip::AdaptiveGauss ),
          "in"_a, "params"_a, "sigmas"_a = dip::FloatArray{ 5.0, 1.0 }, "orders"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 2.0, "exponents"_a = dip::UnsignedArray{ 0 }, "interpolationMethod"_a = dip::S::LINEAR, "boundaryCondition"_a = dip::S::SYMMETRIC_MIRROR );
   m.def( "AdaptiveBanana", py::overload_cast< dip::Image const&, dip::ImageConstRefArray const&, dip::FloatArray const&, dip::UnsignedArray const&, dip::dfloat, dip::UnsignedArray const&, dip::String const&, dip::String const& >( &dip::AdaptiveBanana ),
          "in"_a, "params"_a, "sigmas"_a = dip::FloatArray{ 5.0, 1.0 }, "orders"_a = dip::UnsignedArray{ 0 }, "truncation"_a = 2.0, "exponents"_a = dip::UnsignedArray{ 0 }, "interpolationMethod"_a = dip::S::LINEAR, "boundaryCondition"_a = dip::S::SYMMETRIC_MIRROR );
   m.def( "BilateralFilter", py::overload_cast< dip::Image const&, dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::BilateralFilter ),
          "in"_a, "estimate"_a = dip::Image{}, "spatialSigmas"_a = dip::FloatArray{ 2.0 }, "tonalSigma"_a = 30.0, "truncation"_a = 2.0, "method"_a = "xysep", "boundaryCondition"_a = dip::StringArray{} );

   // diplib/transform.h
   m.def( "FourierTransform", py::overload_cast< dip::Image const&, dip::StringSet const&, dip::BooleanArray const& >( &dip::FourierTransform ),
          "in"_a, "options"_a = dip::StringSet{}, "process"_a = dip::BooleanArray{} );
   m.def( "OptimalFourierTransformSize", &dip::OptimalFourierTransformSize, "size"_a );
   m.def( "RieszTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const&, dip::BooleanArray const& >( &dip::RieszTransform ),
          "in"_a, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "process"_a = dip::BooleanArray{} );
}
