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

#include <sstream>

#include "pydip.h"
#include "diplib/morphology.h"
#include "diplib/binary.h"

namespace {

dip::String StructuringElementRepr( dip::StructuringElement const& s ) {
   std::ostringstream os;
   os << "<";
   if( s.IsCustom() ) {
      os << "Custom StructuringElement";
      if( s.IsFlat() ) {
         os << ", flat";
      } else {
         os << ", grey-valued";
      }
   } else {
      switch( s.Shape() ) {
         case dip::StructuringElement::ShapeCode::RECTANGULAR:
            os << "Rectangular";
            break;
         case dip::StructuringElement::ShapeCode::ELLIPTIC:
            os << "Elliptic";
            break;
         case dip::StructuringElement::ShapeCode::DIAMOND:
            os << "Diamond";
            break;
         case dip::StructuringElement::ShapeCode::OCTAGONAL:
            os << "Octagonal";
            break;
         case dip::StructuringElement::ShapeCode::LINE:
            os << "Line";
            break;
         case dip::StructuringElement::ShapeCode::FAST_LINE:
            os << "Fast line";
            break;
         case dip::StructuringElement::ShapeCode::PERIODIC_LINE:
            os << "Periodic line";
            break;
         case dip::StructuringElement::ShapeCode::DISCRETE_LINE:
            os << "Discrete line";
            break;
         case dip::StructuringElement::ShapeCode::INTERPOLATED_LINE:
            os << "Interpolated line";
            break;
         case dip::StructuringElement::ShapeCode::PARABOLIC:
            os << "Parabolic";
            break;
         default:
            os << "Unknown";
            break;
      }
      os << " StructuringElement with parameters " << s.Params();
   }
   if( s.IsMirrored() ) {
      os << ", mirrored";
   }
   os << ">";
   return os.str();
}

} // namespace

void init_morphology( py::module& m ) {

   auto se = py::class_< dip::StructuringElement >( m, "StructuringElement", doc_strings::dip·StructuringElement );
   se.def( py::init<>(), doc_strings::dip·StructuringElement·StructuringElement );
   se.def( py::init< dip::String const& >(), "shape"_a, doc_strings::dip·StructuringElement·StructuringElement·String·CL );
   se.def( py::init< dip::dfloat, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC, doc_strings::dip·StructuringElement·StructuringElement·dfloat··String·CL );
   se.def( py::init< dip::FloatArray, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC, doc_strings::dip·StructuringElement·StructuringElement·FloatArray··String·CL );
   se.def( py::init< dip::Image const& >(), "image"_a, doc_strings::dip·StructuringElement·StructuringElement·Image·CL );
   se.def( "Mirror", &dip::StructuringElement::Mirror, doc_strings::dip·StructuringElement·Mirror );
   se.def( "__repr__", &StructuringElementRepr );
   py::implicitly_convertible< py::str, dip::StructuringElement >();
   py::implicitly_convertible< py::float_, dip::StructuringElement >();
   py::implicitly_convertible< py::int_, dip::StructuringElement >();
   py::implicitly_convertible< py::list, dip::StructuringElement >();
   py::implicitly_convertible< py::tuple, dip::StructuringElement >();
   py::implicitly_convertible< dip::Image, dip::StructuringElement >();
   py::implicitly_convertible< py::buffer, dip::StructuringElement >();

   // diplib/morphology.h
   m.def( "Dilation", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Dilation ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Dilation·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Dilation", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Dilation ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Dilation·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Erosion", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Erosion ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Erosion·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Erosion", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Erosion ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Erosion·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Closing", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Closing ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Closing·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Closing", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Closing ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Closing·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Opening", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Opening ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Opening·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Opening", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Opening ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Opening·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );

   m.def( "Tophat", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::Tophat ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Tophat·Image·CL·Image·L·StructuringElement·CL·String·CL·String·CL·StringArray·CL );
   m.def( "Tophat", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::Tophat ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Tophat·Image·CL·Image·L·StructuringElement·CL·String·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalThreshold", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalThreshold ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalThreshold·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalThreshold", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalThreshold ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalThreshold·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalGist", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalGist ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalGist·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalGist", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalGist ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalGist·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalRange", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalRange ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalRange·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalRange", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalRange ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalRange·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalGradientMagnitude", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalGradientMagnitude ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalGradientMagnitude·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "MorphologicalGradientMagnitude", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalGradientMagnitude ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalGradientMagnitude·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "Lee", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::Lee ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "sign"_a = dip::S::UNSIGNED, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Lee·Image·CL·Image·L·StructuringElement·CL·String·CL·String·CL·StringArray·CL );
   m.def( "Lee", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::Lee ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "sign"_a = dip::S::UNSIGNED, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·Lee·Image·CL·Image·L·StructuringElement·CL·String·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalSmoothing", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalSmoothing ),
          "in"_a, "se"_a = dip::StructuringElement{}, "mode"_a = dip::S::AVERAGE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalSmoothing·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalSmoothing", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalSmoothing ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "mode"_a = dip::S::AVERAGE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalSmoothing·Image·CL·Image·L·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "MorphologicalSharpening", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalSharpening ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalSharpening·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "MorphologicalSharpening", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalSharpening ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalSharpening·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "MultiScaleMorphologicalGradient", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const&, dip::StringArray const& >( &dip::MultiScaleMorphologicalGradient ),
          "in"_a, "upperSize"_a = 9, "lowerSize"_a = 3, "filterShape"_a = dip::S::ELLIPTIC, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MultiScaleMorphologicalGradient·Image·CL·Image·L·dip·uint··dip·uint··String·CL·StringArray·CL );
   m.def( "MultiScaleMorphologicalGradient", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint, dip::String const&, dip::StringArray const& >( &dip::MultiScaleMorphologicalGradient ),
          "in"_a, py::kw_only(), "out"_a, "upperSize"_a = 9, "lowerSize"_a = 3, "filterShape"_a = dip::S::ELLIPTIC, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MultiScaleMorphologicalGradient·Image·CL·Image·L·dip·uint··dip·uint··String·CL·StringArray·CL );
   m.def( "MorphologicalLaplace", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalLaplace ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalLaplace·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );
   m.def( "MorphologicalLaplace", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalLaplace ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MorphologicalLaplace·Image·CL·Image·L·StructuringElement·CL·StringArray·CL );

   m.def( "RankFilter", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::String const&, dip::StringArray const& >( &dip::RankFilter ),
          "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "order"_a = dip::S::INCREASING, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·RankFilter·Image·CL·Image·L·StructuringElement·CL·dip·uint··String·CL·StringArray·CL );
   m.def( "RankFilter", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::uint, dip::String const&, dip::StringArray const& >( &dip::RankFilter ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "order"_a = dip::S::INCREASING, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·RankFilter·Image·CL·Image·L·StructuringElement·CL·dip·uint··String·CL·StringArray·CL );
   m.def( "RankMinClosing", py::overload_cast< dip::Image const&, dip::StructuringElement, dip::uint, dip::StringArray const& >( &dip::RankMinClosing ),
          "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·RankMinClosing·Image·CL·Image·L·StructuringElement··dip·uint··StringArray·CL );
   m.def( "RankMinClosing", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement, dip::uint, dip::StringArray const& >( &dip::RankMinClosing ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·RankMinClosing·Image·CL·Image·L·StructuringElement··dip·uint··StringArray·CL );
   m.def( "RankMaxOpening", py::overload_cast< dip::Image const&, dip::StructuringElement, dip::uint, dip::StringArray const& >( &dip::RankMaxOpening ),
          "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·RankMaxOpening·Image·CL·Image·L·StructuringElement··dip·uint··StringArray·CL );
   m.def( "RankMaxOpening", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement, dip::uint, dip::StringArray const& >( &dip::RankMaxOpening ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·RankMaxOpening·Image·CL·Image·L·StructuringElement··dip·uint··StringArray·CL );

   m.def( "Watershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::StringSet >( &dip::Watershed ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{}, doc_strings::dip·Watershed·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··StringSet· );
   m.def( "Watershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::uint, dip::StringSet >( &dip::Watershed ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{}, doc_strings::dip·Watershed·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··StringSet· );
   m.def( "SeededWatershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::SeededWatershed ),
          "in"_a, "seeds"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{}, doc_strings::dip·SeededWatershed·Image·CL·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··StringSet·CL );
   m.def( "SeededWatershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::SeededWatershed ),
          "in"_a, "seeds"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{}, doc_strings::dip·SeededWatershed·Image·CL·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··StringSet·CL );
   m.def( "CompactWatershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::StringSet const& >( &dip::CompactWatershed ),
          "in"_a, "seeds"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "compactness"_a = 1.0, "flags"_a = dip::StringSet{}, doc_strings::dip·CompactWatershed·Image·CL·Image·CL·Image·CL·Image·L·dip·uint··dfloat··StringSet·CL );
   m.def( "CompactWatershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::StringSet const& >( &dip::CompactWatershed ),
          "in"_a, "seeds"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "connectivity"_a = 1, "compactness"_a = 1.0, "flags"_a = dip::StringSet{}, doc_strings::dip·CompactWatershed·Image·CL·Image·CL·Image·CL·Image·L·dip·uint··dfloat··StringSet·CL );
   m.def( "StochasticWatershed", []( dip::Image const& in, dip::uint nSeeds, dip::uint nIterations, dip::dfloat noise, dip::String const& seeds ) {
             return StochasticWatershed( in, RandomNumberGenerator(), nSeeds, nIterations, noise, seeds );
          },
          "in"_a, "nSeeds"_a = 100, "nIterations"_a = 50, "noise"_a = 0, "seeds"_a = dip::S::HEXAGONAL,
          "Computes the stochastic watershed of `in`.\n"
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "StochasticWatershed", []( dip::Image const& in, dip::Image& out, dip::uint nSeeds, dip::uint nIterations, dip::dfloat noise, dip::String const& seeds ) {
             StochasticWatershed( in, out, RandomNumberGenerator(), nSeeds, nIterations, noise, seeds );
          },
          "in"_a, py::kw_only(), "out"_a, "nSeeds"_a = 100, "nIterations"_a = 50, "noise"_a = 0, "seeds"_a = dip::S::HEXAGONAL,
          "Computes the stochastic watershed of `in`.\n"
          "Like the C++ function, but using an internal `dip::Random` object." );

   m.def( "Maxima", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::Maxima ),
          "in"_a, "connectivity"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·Minima·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "Maxima", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::Maxima ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·Minima·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "Minima", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::Minima ),
          "in"_a, "connectivity"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·Maxima·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "Minima", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::Minima ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·Maxima·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "WatershedMinima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMinima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·WatershedMinima·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··String·CL );
   m.def( "WatershedMinima", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMinima ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·WatershedMinima·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··String·CL );
   m.def( "WatershedMaxima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMaxima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·WatershedMaxima·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··String·CL );
   m.def( "WatershedMaxima", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMaxima ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = dip::S::BINARY, doc_strings::dip·WatershedMaxima·Image·CL·Image·CL·Image·L·dip·uint··dfloat··dip·uint··String·CL );
   m.def( "UpperSkeleton2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const& >( &dip::UpperSkeleton2D ),
          "in"_a, "mask"_a = dip::Image{}, "endPixelCondition"_a = dip::S::NATURAL, doc_strings::dip·UpperSkeleton2D·Image·CL·Image·CL·Image·L·String·CL );
   m.def( "UpperSkeleton2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const& >( &dip::UpperSkeleton2D ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "endPixelCondition"_a = dip::S::NATURAL, doc_strings::dip·UpperSkeleton2D·Image·CL·Image·CL·Image·L·String·CL );
   m.def( "MorphologicalReconstruction", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::MorphologicalReconstruction ),
          "marker"_a, "in"_a, "connectivity"_a = 0, "direction"_a = dip::S::DILATION, doc_strings::dip·MorphologicalReconstruction·Image·CL·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "MorphologicalReconstruction", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::MorphologicalReconstruction ),
          "marker"_a, "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "direction"_a = dip::S::DILATION, doc_strings::dip·MorphologicalReconstruction·Image·CL·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "LimitedMorphologicalReconstruction", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::uint, dip::String const& >( &dip::LimitedMorphologicalReconstruction ),
          "marker"_a, "in"_a, "maxDistance"_a = 20, "connectivity"_a = 0, "direction"_a = dip::S::DILATION, doc_strings::dip·LimitedMorphologicalReconstruction·Image·CL·Image·CL·Image·L·dfloat··dip·uint··String·CL );
   m.def( "LimitedMorphologicalReconstruction", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::uint, dip::String const& >( &dip::LimitedMorphologicalReconstruction ),
          "marker"_a, "in"_a, py::kw_only(), "out"_a, "maxDistance"_a = 20, "connectivity"_a = 0, "direction"_a = dip::S::DILATION, doc_strings::dip·LimitedMorphologicalReconstruction·Image·CL·Image·CL·Image·L·dfloat··dip·uint··String·CL );
   m.def( "HMinima", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint >( &dip::HMinima ),
          "in"_a, "h"_a, "connectivity"_a = 0, doc_strings::dip·HMinima·Image·CL·Image·L·dfloat··dip·uint· );
   m.def( "HMinima", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::uint >( &dip::HMinima ),
          "in"_a, py::kw_only(), "out"_a, "h"_a, "connectivity"_a = 0, doc_strings::dip·HMinima·Image·CL·Image·L·dfloat··dip·uint· );
   m.def( "HMaxima", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint >( &dip::HMaxima ),
          "in"_a, "h"_a, "connectivity"_a = 0, doc_strings::dip·HMaxima·Image·CL·Image·L·dfloat··dip·uint· );
   m.def( "HMaxima", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::uint >( &dip::HMaxima ),
          "in"_a, py::kw_only(), "out"_a, "h"_a, "connectivity"_a = 0, doc_strings::dip·HMaxima·Image·CL·Image·L·dfloat··dip·uint· );
   m.def( "ImposeMinima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::ImposeMinima ),
          "in"_a, "marker"_a, "connectivity"_a = 0, doc_strings::dip·ImposeMinima·Image·CL·Image·CL·Image·L·dip·uint· );
   m.def( "ImposeMinima", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint >( &dip::ImposeMinima ),
          "in"_a, "marker"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, doc_strings::dip·ImposeMinima·Image·CL·Image·CL·Image·L·dip·uint· );
   m.def( "Leveling", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::Leveling ),
          "in"_a, "marker"_a, "connectivity"_a = 0, doc_strings::dip·Leveling·Image·CL·Image·CL·Image·L·dip·uint· );
   m.def( "Leveling", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint >( &dip::Leveling ),
          "in"_a, "marker"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, doc_strings::dip·Leveling·Image·CL·Image·CL·Image·L·dip·uint· );

   m.def( "AreaOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::AreaOpening ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a = 50, "connectivity"_a = 0, "polarity"_a = dip::S::OPENING, doc_strings::dip·AreaOpening·Image·CL·Image·CL·Image·L·dip·uint··dip·uint··String·CL );
   m.def( "AreaOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::uint, dip::String const& >( &dip::AreaOpening ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "filterSize"_a = 50, "connectivity"_a = 0, "polarity"_a = dip::S::OPENING, doc_strings::dip·AreaOpening·Image·CL·Image·CL·Image·L·dip·uint··dip·uint··String·CL );
   m.def( "AreaClosing", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint >( &dip::AreaClosing ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a = 50, "connectivity"_a = 0, doc_strings::dip·AreaClosing·Image·CL·Image·CL·Image·L·dip·uint··dip·uint· );
   m.def( "AreaClosing", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::uint >( &dip::AreaClosing ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "filterSize"_a = 50, "connectivity"_a = 0, doc_strings::dip·AreaClosing·Image·CL·Image·CL·Image·L·dip·uint··dip·uint· );
   m.def( "VolumeOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::uint, dip::String const& >( &dip::VolumeOpening ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a = 50, "connectivity"_a = 0, "polarity"_a = dip::S::OPENING, doc_strings::dip·VolumeOpening·Image·CL·Image·CL·Image·L·dfloat··dip·uint··String·CL );
   m.def( "VolumeOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::uint, dip::String const& >( &dip::VolumeOpening ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "filterSize"_a = 50, "connectivity"_a = 0, "polarity"_a = dip::S::OPENING, doc_strings::dip·VolumeOpening·Image·CL·Image·CL·Image·L·dfloat··dip·uint··String·CL );
   m.def( "VolumeClosing", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::uint >( &dip::VolumeClosing ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a = 50, "connectivity"_a = 0, doc_strings::dip·VolumeClosing·Image·CL·Image·CL·Image·L·dfloat··dip·uint· );
   m.def( "VolumeClosing", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::uint >( &dip::VolumeClosing ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "filterSize"_a = 50, "connectivity"_a = 0, doc_strings::dip·VolumeClosing·Image·CL·Image·CL·Image·L·dfloat··dip·uint· );
   m.def( "PathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::StringSet const& >( &dip::PathOpening ),
          "in"_a, "mask"_a = dip::Image{}, "length"_a = 7, "polarity"_a = dip::S::OPENING, "mode"_a = dip::StringSet{}, doc_strings::dip·PathOpening·Image·CL·Image·CL·Image·L·dip·uint··String·CL·StringSet·CL );
   m.def( "PathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const&, dip::StringSet const& >( &dip::PathOpening ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "length"_a = 7, "polarity"_a = dip::S::OPENING, "mode"_a = dip::StringSet{}, doc_strings::dip·PathOpening·Image·CL·Image·CL·Image·L·dip·uint··String·CL·StringSet·CL );
   m.def( "DirectedPathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::IntegerArray, dip::String const&, dip::StringSet const& >( &dip::DirectedPathOpening ),
          "in"_a, "mask"_a = dip::Image{}, "filterParam"_a = dip::IntegerArray{}, "polarity"_a = dip::S::OPENING, "mode"_a = dip::StringSet{}, doc_strings::dip·DirectedPathOpening·Image·CL·Image·CL·Image·L·IntegerArray··String·CL·StringSet·CL );
   m.def( "DirectedPathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::IntegerArray, dip::String const&, dip::StringSet const& >( &dip::DirectedPathOpening ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "filterParam"_a = dip::IntegerArray{}, "polarity"_a = dip::S::OPENING, "mode"_a = dip::StringSet{}, doc_strings::dip·DirectedPathOpening·Image·CL·Image·CL·Image·L·IntegerArray··String·CL·StringSet·CL );
   m.def( "OpeningByReconstruction", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::OpeningByReconstruction ),
          "in"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·OpeningByReconstruction·Image·CL·Image·L·StructuringElement·CL·dip·uint··StringArray·CL );
   m.def( "OpeningByReconstruction", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::OpeningByReconstruction ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·OpeningByReconstruction·Image·CL·Image·L·StructuringElement·CL·dip·uint··StringArray·CL );
   m.def( "ClosingByReconstruction", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::ClosingByReconstruction ),
          "in"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·ClosingByReconstruction·Image·CL·Image·L·StructuringElement·CL·dip·uint··StringArray·CL );
   m.def( "ClosingByReconstruction", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::ClosingByReconstruction ),
          "in"_a, py::kw_only(), "out"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 0, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·ClosingByReconstruction·Image·CL·Image·L·StructuringElement·CL·dip·uint··StringArray·CL );

   m.def( "AlternatingSequentialFilter", py::overload_cast< dip::Image const&, dip::Range const&, dip::String const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::AlternatingSequentialFilter ),
          "in"_a, "sizes"_a = dip::Range{ 3, 7, 2 }, "shape"_a = dip::S::ELLIPTIC, "mode"_a = dip::S::STRUCTURAL, "polarity"_a = dip::S::OPENCLOSE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·AlternatingSequentialFilter·Image·CL·Image·L·Range·CL·String·CL·String·CL·String·CL·StringArray·CL );
   m.def( "AlternatingSequentialFilter", py::overload_cast< dip::Image const&, dip::Image&, dip::Range const&, dip::String const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::AlternatingSequentialFilter ),
          "in"_a, py::kw_only(), "out"_a, "sizes"_a = dip::Range{ 3, 7, 2 }, "shape"_a = dip::S::ELLIPTIC, "mode"_a = dip::S::STRUCTURAL, "polarity"_a = dip::S::OPENCLOSE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·AlternatingSequentialFilter·Image·CL·Image·L·Range·CL·String·CL·String·CL·String·CL·StringArray·CL );
   m.def( "HitAndMiss", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::HitAndMiss ),
          "in"_a, "hit"_a, "miss"_a, "mode"_a = dip::S::UNCONSTRAINED, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·HitAndMiss·Image·CL·Image·L·StructuringElement·CL·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "HitAndMiss", py::overload_cast< dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::HitAndMiss ),
          "in"_a, py::kw_only(), "out"_a, "hit"_a, "miss"_a, "mode"_a = dip::S::UNCONSTRAINED, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·HitAndMiss·Image·CL·Image·L·StructuringElement·CL·StructuringElement·CL·String·CL·StringArray·CL );
   m.def( "HitAndMiss", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::StringArray const& >( &dip::HitAndMiss ),
          "in"_a, "se"_a, "mode"_a = dip::S::UNCONSTRAINED, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·HitAndMiss·Image·CL·Image·L·Image·CL·String·CL·StringArray·CL );
   m.def( "HitAndMiss", py::overload_cast< dip::Image const&, dip::Image&, dip::Image const&, dip::String const&, dip::StringArray const& >( &dip::HitAndMiss ),
          "in"_a, py::kw_only(), "out"_a, "se"_a, "mode"_a = dip::S::UNCONSTRAINED, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·HitAndMiss·Image·CL·Image·L·Image·CL·String·CL·StringArray·CL );

   // diplib/binary.h
   m.def( "BinaryDilation", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryDilation ),
          "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryDilation·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryDilation", py::overload_cast< dip::Image const&, dip::Image&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryDilation ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryDilation·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryErosion", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryErosion ),
          "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::OBJECT, doc_strings::dip·BinaryErosion·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryErosion", py::overload_cast< dip::Image const&, dip::Image&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryErosion ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::OBJECT, doc_strings::dip·BinaryErosion·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryClosing", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryClosing ),
          "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::SPECIAL, doc_strings::dip·BinaryClosing·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryClosing", py::overload_cast< dip::Image const&, dip::Image&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryClosing ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::SPECIAL, doc_strings::dip·BinaryClosing·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryOpening", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryOpening ),
          "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::SPECIAL, doc_strings::dip·BinaryOpening·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryOpening", py::overload_cast< dip::Image const&, dip::Image&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryOpening ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::SPECIAL, doc_strings::dip·BinaryOpening·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryPropagation", py::overload_cast< dip::Image const&, dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryPropagation ),
          "inSeed"_a, "inMask"_a, "connectivity"_a = 1, "iterations"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryPropagation·Image·CL·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "BinaryPropagation", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryPropagation ),
          "inSeed"_a, "inMask"_a, py::kw_only(), "out"_a, "connectivity"_a = 1, "iterations"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryPropagation·Image·CL·Image·CL·Image·L·dip·sint··dip·uint··String·CL );
   m.def( "EdgeObjectsRemove", py::overload_cast< dip::Image const&, dip::uint >( &dip::EdgeObjectsRemove ),
          "in"_a, "connectivity"_a = 1, doc_strings::dip·EdgeObjectsRemove·Image·CL·Image·L·dip·uint· );
   m.def( "EdgeObjectsRemove", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::EdgeObjectsRemove ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 1, doc_strings::dip·EdgeObjectsRemove·Image·CL·Image·L·dip·uint· );
   m.def( "FillHoles", py::overload_cast< dip::Image const&, dip::uint >( &dip::FillHoles ),
          "in"_a, "connectivity"_a = 1, doc_strings::dip·FillHoles·Image·CL·Image·L·dip·uint· );
   m.def( "FillHoles", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::FillHoles ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 1, doc_strings::dip·FillHoles·Image·CL·Image·L·dip·uint· );
   m.def( "IsotropicDilation", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::IsotropicDilation ),
          "in"_a, "distance"_a, doc_strings::dip·IsotropicDilation·Image·CL·Image·L·dfloat· );
   m.def( "IsotropicDilation", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::IsotropicDilation ),
          "in"_a, py::kw_only(), "out"_a, "distance"_a, doc_strings::dip·IsotropicDilation·Image·CL·Image·L·dfloat· );
   m.def( "IsotropicErosion", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::IsotropicErosion ),
          "in"_a, "distance"_a, doc_strings::dip·IsotropicErosion·Image·CL·Image·L·dfloat· );
   m.def( "IsotropicErosion", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::IsotropicErosion ),
          "in"_a, py::kw_only(), "out"_a, "distance"_a, doc_strings::dip·IsotropicErosion·Image·CL·Image·L·dfloat· );
   m.def( "IsotropicClosing", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::IsotropicClosing ),
          "in"_a, "distance"_a, doc_strings::dip·IsotropicClosing·Image·CL·Image·L·dfloat· );
   m.def( "IsotropicClosing", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::IsotropicClosing ),
          "in"_a, py::kw_only(), "out"_a, "distance"_a, doc_strings::dip·IsotropicClosing·Image·CL·Image·L·dfloat· );
   m.def( "IsotropicOpening", py::overload_cast< dip::Image const&, dip::dfloat >( &dip::IsotropicOpening ),
          "in"_a, "distance"_a, doc_strings::dip·IsotropicOpening·Image·CL·Image·L·dfloat· );
   m.def( "IsotropicOpening", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat >( &dip::IsotropicOpening ),
          "in"_a, py::kw_only(), "out"_a, "distance"_a, doc_strings::dip·IsotropicOpening·Image·CL·Image·L·dfloat· );

   m.def( "ConditionalThickening2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThickening2D ),
          "in"_a, "mask"_a = dip::Image{}, "iterations"_a = 0, "endPixelCondition"_a = dip::S::LOSE, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·ConditionalThickening2D·Image·CL·Image·CL·Image·L·dip·uint··String·CL·String·CL );
   m.def( "ConditionalThickening2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThickening2D ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "iterations"_a = 0, "endPixelCondition"_a = dip::S::LOSE, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·ConditionalThickening2D·Image·CL·Image·CL·Image·L·dip·uint··String·CL·String·CL );
   m.def( "ConditionalThinning2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThinning2D ),
          "in"_a, "mask"_a = dip::Image{}, "iterations"_a = 0, "endPixelCondition"_a = dip::S::LOSE, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·ConditionalThinning2D·Image·CL·Image·CL·Image·L·dip·uint··String·CL·String·CL );
   m.def( "ConditionalThinning2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThinning2D ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "iterations"_a = 0, "endPixelCondition"_a = dip::S::LOSE, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·ConditionalThinning2D·Image·CL·Image·CL·Image·L·dip·uint··String·CL·String·CL );

   m.def( "BinaryAreaOpening", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaOpening ),
          "in"_a, "filterSize"_a = 50, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryAreaOpening·Image·CL·Image·L·dip·uint··dip·uint··String·CL );
   m.def( "BinaryAreaOpening", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaOpening ),
          "in"_a, py::kw_only(), "out"_a, "filterSize"_a = 50, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryAreaOpening·Image·CL·Image·L·dip·uint··dip·uint··String·CL );
   m.def( "BinaryAreaClosing", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaClosing ),
          "in"_a, "filterSize"_a = 50, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryAreaClosing·Image·CL·Image·L·dip·uint··dip·uint··String·CL );
   m.def( "BinaryAreaClosing", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaClosing ),
          "in"_a, py::kw_only(), "out"_a, "filterSize"_a = 50, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·BinaryAreaClosing·Image·CL·Image·L·dip·uint··dip·uint··String·CL );

   m.def( "EuclideanSkeleton", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::EuclideanSkeleton ),
          "in"_a, "endPixelCondition"_a = dip::S::NATURAL, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·EuclideanSkeleton·Image·CL·Image·L·String·CL·String·CL );
   m.def( "EuclideanSkeleton", py::overload_cast< dip::Image const&, dip::Image&, dip::String const&, dip::String const& >( &dip::EuclideanSkeleton ),
          "in"_a, py::kw_only(), "out"_a, "endPixelCondition"_a = dip::S::NATURAL, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·EuclideanSkeleton·Image·CL·Image·L·String·CL·String·CL );

   m.def( "CountNeighbors", py::overload_cast< dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::CountNeighbors ),
          "in"_a, "connectivity"_a = 0, "mode"_a = dip::S::FOREGROUND, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·CountNeighbors·Image·CL·Image·L·dip·uint··dip·String·CL·dip·String·CL );
   m.def( "CountNeighbors", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const&, dip::String const& >( &dip::CountNeighbors ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "mode"_a = dip::S::FOREGROUND, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·CountNeighbors·Image·CL·Image·L·dip·uint··dip·String·CL·dip·String·CL );
   m.def( "MajorityVote", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::MajorityVote ),
          "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·MajorityVote·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "MajorityVote", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::MajorityVote ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·MajorityVote·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetSinglePixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetSinglePixels ),
          "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetSinglePixels·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetSinglePixels", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::GetSinglePixels ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetSinglePixels·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetEndPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetEndPixels ),
          "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetEndPixels·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetEndPixels", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::GetEndPixels ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetEndPixels·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetLinkPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetLinkPixels ),
          "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetLinkPixels·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetLinkPixels", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::GetLinkPixels ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetLinkPixels·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetBranchPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetBranchPixels ),
          "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetBranchPixels·Image·CL·Image·L·dip·uint··dip·String·CL );
   m.def( "GetBranchPixels", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::GetBranchPixels ),
          "in"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND, doc_strings::dip·GetBranchPixels·Image·CL·Image·L·dip·uint··dip·String·CL );

   auto intv = py::class_< dip::Interval >( m, "Interval", doc_strings::dip·Interval );
   intv.def( py::init< dip::Image >(), "image"_a, doc_strings::dip·Interval·Interval·dip·Image· );
   intv.def( py::init< dip::Image const&, dip::Image const& >(), "hit"_a, "miss"_a, doc_strings::dip·Interval·Interval·dip·Image··dip·Image· );
   py::implicitly_convertible< dip::Image, dip::Interval >();
   intv.def( "__repr__", []( dip::Interval const& self ) {
      std::ostringstream os;
      os << "<Interval, sizes " << self.Sizes() << '>';
      return os.str();
   } );
   intv.def( "Image", &dip::Interval::Image, py::return_value_policy::reference_internal, doc_strings::dip·Interval·Image·C );

   m.def( "SupGenerating", py::overload_cast< dip::Image const&, dip::Interval const&, dip::String const& >( &dip::SupGenerating ),
          "in"_a, "interval"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·SupGenerating·Image·CL·Image·L·Interval·CL·String·CL );
   m.def( "SupGenerating", py::overload_cast< dip::Image const&, dip::Image&, dip::Interval const&, dip::String const& >( &dip::SupGenerating ),
          "in"_a, py::kw_only(), "out"_a, "interval"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·SupGenerating·Image·CL·Image·L·Interval·CL·String·CL );
   m.def( "InfGenerating", py::overload_cast< dip::Image const&, dip::Interval const&, dip::String const& >( &dip::InfGenerating ),
          "in"_a, "interval"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·InfGenerating·Image·CL·Image·L·Interval·CL·String·CL );
   m.def( "InfGenerating", py::overload_cast< dip::Image const&, dip::Image&, dip::Interval const&, dip::String const& >( &dip::InfGenerating ),
          "in"_a, py::kw_only(), "out"_a, "interval"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·InfGenerating·Image·CL·Image·L·Interval·CL·String·CL );
   m.def( "UnionSupGenerating", py::overload_cast< dip::Image const&, dip::IntervalArray const&, dip::String const& >( &dip::UnionSupGenerating ),
          "in"_a, "intervals"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·UnionSupGenerating·Image·CL·Image·L·IntervalArray·CL·String·CL );
   m.def( "UnionSupGenerating", py::overload_cast< dip::Image const&, dip::Image&, dip::IntervalArray const&, dip::String const& >( &dip::UnionSupGenerating ),
          "in"_a, py::kw_only(), "out"_a, "intervals"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·UnionSupGenerating·Image·CL·Image·L·IntervalArray·CL·String·CL );
   m.def( "UnionSupGenerating2D", py::overload_cast< dip::Image const&, dip::Interval const&, dip::uint, dip::String const&, dip::String const& >( &dip::UnionSupGenerating2D ),
          "in"_a, "interval"_a, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·UnionSupGenerating2D·Image·CL·Image·L·Interval·CL·dip·uint··String·CL·String·CL );
   m.def( "UnionSupGenerating2D", py::overload_cast< dip::Image const&, dip::Image&, dip::Interval const&, dip::uint, dip::String const&, dip::String const& >( &dip::UnionSupGenerating2D ),
          "in"_a, py::kw_only(), "out"_a, "interval"_a, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·UnionSupGenerating2D·Image·CL·Image·L·Interval·CL·dip·uint··String·CL·String·CL );
   m.def( "IntersectionInfGenerating", py::overload_cast< dip::Image const&, dip::IntervalArray const&, dip::String const& >( &dip::IntersectionInfGenerating ),
          "in"_a, "intervals"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·IntersectionInfGenerating·Image·CL·Image·L·IntervalArray·CL·String·CL );
   m.def( "IntersectionInfGenerating", py::overload_cast< dip::Image const&, dip::Image&, dip::IntervalArray const&, dip::String const& >( &dip::IntersectionInfGenerating ),
          "in"_a, py::kw_only(), "out"_a, "intervals"_a, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·IntersectionInfGenerating·Image·CL·Image·L·IntervalArray·CL·String·CL );
   m.def( "IntersectionInfGenerating2D", py::overload_cast< dip::Image const&, dip::Interval const&, dip::uint, dip::String const&, dip::String const& >( &dip::IntersectionInfGenerating2D ),
          "in"_a, "interval"_a, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·IntersectionInfGenerating2D·Image·CL·Image·L·Interval·CL·dip·uint··String·CL·String·CL );
   m.def( "IntersectionInfGenerating2D", py::overload_cast< dip::Image const&, dip::Image&, dip::Interval const&, dip::uint, dip::String const&, dip::String const& >( &dip::IntersectionInfGenerating2D ),
          "in"_a, py::kw_only(), "out"_a, "interval"_a, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·IntersectionInfGenerating2D·Image·CL·Image·L·Interval·CL·dip·uint··String·CL·String·CL );
   m.def( "Thickening", py::overload_cast< dip::Image const&, dip::Image const&, dip::IntervalArray const&, dip::uint, dip::String const& >( &dip::Thickening ),
          "in"_a, "mask"_a = dip::Image{}, "intervals"_a, "iterations"_a = 0, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thickening·Image·CL·Image·CL·Image·L·IntervalArray·CL·dip·uint··String·CL );
   m.def( "Thickening", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::IntervalArray const&, dip::uint, dip::String const& >( &dip::Thickening ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "intervals"_a, "iterations"_a = 0, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thickening·Image·CL·Image·CL·Image·L·IntervalArray·CL·dip·uint··String·CL );
   m.def( "Thickening2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Interval const&, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Thickening2D ),
          "in"_a, "mask"_a = dip::Image{}, "interval"_a, "iterations"_a = 0, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thickening2D·Image·CL·Image·CL·Image·L·Interval·CL·dip·uint··dip·uint··String·CL·String·CL );
   m.def( "Thickening2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::Interval const&, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Thickening2D ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "interval"_a, "iterations"_a = 0, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thickening2D·Image·CL·Image·CL·Image·L·Interval·CL·dip·uint··dip·uint··String·CL·String·CL );
   m.def( "Thinning", py::overload_cast< dip::Image const&, dip::Image const&, dip::IntervalArray const&, dip::uint, dip::String const& >( &dip::Thinning ),
          "in"_a, "mask"_a = dip::Image{}, "intervals"_a, "iterations"_a = 0, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thinning·Image·CL·Image·CL·Image·L·IntervalArray·CL·dip·uint··String·CL );
   m.def( "Thinning", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::IntervalArray const&, dip::uint, dip::String const& >( &dip::Thinning ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "intervals"_a, "iterations"_a = 0, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thinning·Image·CL·Image·CL·Image·L·IntervalArray·CL·dip·uint··String·CL );
   m.def( "Thinning2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Interval const&, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Thinning2D ),
          "in"_a, "mask"_a = dip::Image{}, "interval"_a, "iterations"_a = 0, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thinning2D·Image·CL·Image·CL·Image·L·Interval·CL·dip·uint··dip·uint··String·CL·String·CL );
   m.def( "Thinning2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::Interval const&, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Thinning2D ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "interval"_a, "iterations"_a = 0, "rotationAngle"_a = 45, "rotationDirection"_a = dip::S::INTERLEAVED_CLOCKWISE, "boundaryCondition"_a = dip::S::ADD_ZEROS, doc_strings::dip·Thinning2D·Image·CL·Image·CL·Image·L·Interval·CL·dip·uint··dip·uint··String·CL·String·CL );

   m.def( "HomotopicThinningInterval2D", &dip::HomotopicThinningInterval2D, "connectivity"_a = 2, doc_strings::dip·HomotopicThinningInterval2D·dip·uint· );
   m.def( "HomotopicThickeningInterval2D", &dip::HomotopicThickeningInterval2D, "connectivity"_a = 2, doc_strings::dip·HomotopicThickeningInterval2D·dip·uint· );
   m.def( "EndPixelInterval2D", &dip::EndPixelInterval2D, "connectivity"_a = 2, doc_strings::dip·EndPixelInterval2D·dip·uint· );
   m.def( "HomotopicEndPixelInterval2D", &dip::HomotopicEndPixelInterval2D, "connectivity"_a = 2, doc_strings::dip·HomotopicEndPixelInterval2D·dip·uint· );
   m.def( "HomotopicInverseEndPixelInterval2D", &dip::HomotopicInverseEndPixelInterval2D, "connectivity"_a = 2, doc_strings::dip·HomotopicInverseEndPixelInterval2D·dip·uint· );
   m.def( "SinglePixelInterval", &dip::SinglePixelInterval, "nDims"_a = 2, doc_strings::dip·SinglePixelInterval·dip·uint· );
   m.def( "BranchPixelInterval2D", &dip::BranchPixelInterval2D, doc_strings::dip·BranchPixelInterval2D );
   m.def( "BoundaryPixelInterval2D", &dip::BoundaryPixelInterval2D, doc_strings::dip·BoundaryPixelInterval2D );
   m.def( "ConvexHullInterval2D", &dip::ConvexHullInterval2D, doc_strings::dip·ConvexHullInterval2D );

}
