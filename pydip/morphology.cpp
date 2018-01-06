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
      os << " SE with parameters " << s.Params();
   }
   if( s.IsMirrored() ) {
      os << ", mirrored";
   }
   os << ">";
   return os.str();
}

} // namespace

void init_morphology( py::module& m ) {
   auto se = py::class_< dip::StructuringElement >( m, "SE", "Represents the structuring element to use in morphological operations\n(dip::StructuringElement in DIPlib)." );
   se.def( py::init<>() );
   se.def( py::init< dip::Image const& >(), "image"_a );
   se.def( py::init< dip::String const& >(), "shape"_a );
   se.def( py::init< dip::dfloat, dip::String const& >(), "param"_a, "shape"_a = "elliptic" );
   se.def( py::init< dip::FloatArray, dip::String const& >(), "param"_a, "shape"_a = "elliptic" );
   se.def( "Mirror", &dip::StructuringElement::Mirror );
   se.def( "__repr__", &StructuringElementRepr );
   py::implicitly_convertible< py::buffer, dip::StructuringElement >();
   py::implicitly_convertible< py::str, dip::StructuringElement >();
   py::implicitly_convertible< py::float_, dip::StructuringElement >();
   py::implicitly_convertible< py::int_, dip::StructuringElement >();
   py::implicitly_convertible< py::list, dip::StructuringElement >();

   // diplib/morphology.h
   m.def( "Dilation", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Dilation ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Erosion", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Erosion ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Closing", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Closing ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Opening", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::Opening ),
          "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "Tophat", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::Tophat ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = "texture", "polarity"_a = "white", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalThreshold", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalThreshold ),
         "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = "texture", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalGist", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalGist ),
         "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = "texture", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalRange", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalRange ),
         "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = "texture", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalGradientMagnitude", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalGradientMagnitude ),
         "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Lee", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::Lee ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = "texture", "sign"_a = "unsigned", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalSmoothing", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalSmoothing ),
         "in"_a, "se"_a = dip::StructuringElement{}, "mode"_a = "average", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MultiScaleMorphologicalGradient", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const&, dip::StringArray const& >( &dip::MultiScaleMorphologicalGradient ),
         "in"_a, "upperSize"_a = 9, "lowerSize"_a = 3, "filterShape"_a = "elliptic", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalLaplace", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalLaplace ),
         "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "RankFilter", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::String const&, dip::StringArray const& >( &dip::RankFilter ),
         "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "order"_a = "increasing", "boundaryCondition"_a = dip::StringArray{} );
   m.def( "RankMinClosing", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::RankMinClosing ),
         "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "RankMaxOpening", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::RankMaxOpening ),
         "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "Watershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::Watershed ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{} );
   m.def( "SeededWatershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::SeededWatershed ),
          "in"_a, "seeds"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{} );
   m.def( "Maxima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::Maxima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "output"_a = "binary" );
   m.def( "Minima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::Minima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "output"_a = "binary" );
   m.def( "WatershedMinima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMinima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = "binary" );
   m.def( "WatershedMaxima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMaxima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = "binary" );
   m.def( "MorphologicalReconstruction", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::MorphologicalReconstruction ),
          "marker"_a, "in"_a, "connectivity"_a = 1, "direction"_a = "dilation" );
   m.def( "HMinima", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint >( &dip::HMinima ),
          "in"_a, "h"_a, "connectivity"_a = 1 );
   m.def( "HMaxima", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint >( &dip::HMaxima ),
          "in"_a, "h"_a, "connectivity"_a = 1 );
   m.def( "AreaOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::AreaOpening ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a, "connectivity"_a = 1, "polarity"_a = "opening" );
   m.def( "AreaClosing", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint >( &dip::AreaClosing ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a, "connectivity"_a = 1 );
   m.def( "PathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::PathOpening ),
          "in"_a, "mask"_a = dip::Image{}, "length"_a = 7, "polarity"_a = "opening", "mode"_a = "normal" );
   m.def( "DirectedPathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::IntegerArray const&, dip::String const&, dip::String const& >( &dip::DirectedPathOpening ),
          "in"_a, "mask"_a = dip::Image{}, "filterParam"_a = dip::IntegerArray{}, "polarity"_a = "opening", "mode"_a = "normal" );
   m.def( "OpeningByReconstruction", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::OpeningByReconstruction ),
          "in"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 1, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "ClosingByReconstruction", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::ClosingByReconstruction ),
          "in"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 1, "boundaryCondition"_a = dip::StringArray{} );

   // diplib/binary.h
   m.def( "BinaryDilation", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryDilation ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = "background" );
   m.def( "BinaryErosion", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryErosion ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = "object" );
   m.def( "BinaryClosing", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryClosing ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = "special" );
   m.def( "BinaryOpening", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryOpening ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = "special" );
   m.def( "BinaryPropagation", py::overload_cast< dip::Image const&, dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryPropagation ),
         "inSeed"_a, "inMask"_a, "connectivity"_a = 1, "iterations"_a = 0, "edgeCondition"_a = "background" );
   m.def( "EdgeObjectsRemove", py::overload_cast< dip::Image const&, dip::uint >( &dip::EdgeObjectsRemove ),
         "in"_a, "connectivity"_a = 1 );

   m.def( "ConditionalThickening2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThickening2D ),
         "in"_a, "mask"_a, "iterations"_a = 0, "endPixelCondition"_a = "keep", "edgeCondition"_a = "background" );
   m.def( "ConditionalThinning2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThinning2D ),
         "in"_a, "mask"_a, "iterations"_a = 0, "endPixelCondition"_a = "keep", "edgeCondition"_a = "background" );

   m.def( "BinaryAreaOpening", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaOpening ),
          "in"_a, "filterSize"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "BinaryAreaClosing", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaClosing ),
          "in"_a, "filterSize"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );

   m.def( "EuclideanSkeleton", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::EuclideanSkeleton ),
          "in"_a, "endPixelCondition"_a = "natural", "edgeCondition"_a = "background" );

   m.def( "CountNeighbors", py::overload_cast< dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::CountNeighbors ),
         "in"_a, "connectivity"_a = 0, "mode"_a = "foreground", "edgeCondition"_a = "background" );
   m.def( "MajorityVote", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::MajorityVote ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = "background" );
   m.def( "GetSinglePixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetSinglePixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = "background" );
   m.def( "GetEndPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetEndPixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = "background" );
   m.def( "GetLinkPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetLinkPixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = "background" );
   m.def( "GetBranchPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetBranchPixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = "background" );
}
