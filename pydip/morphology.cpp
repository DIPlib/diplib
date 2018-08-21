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
   se.def( py::init< dip::dfloat, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC );
   se.def( py::init< dip::FloatArray, dip::String const& >(), "param"_a, "shape"_a = dip::S::ELLIPTIC );
   se.def( "Mirror", &dip::StructuringElement::Mirror );
   se.def( "__repr__", &StructuringElementRepr );
   py::implicitly_convertible< dip::Image, dip::StructuringElement >();
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
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalThreshold", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalThreshold ),
         "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalGist", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalGist ),
         "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalRange", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalRange ),
         "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalGradientMagnitude", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalGradientMagnitude ),
         "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "Lee", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::Lee ),
          "in"_a, "se"_a = dip::StructuringElement{}, "edgeType"_a = dip::S::TEXTURE, "sign"_a = dip::S::UNSIGNED, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalSmoothing", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::MorphologicalSmoothing ),
         "in"_a, "se"_a = dip::StructuringElement{}, "mode"_a = dip::S::AVERAGE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MultiScaleMorphologicalGradient", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const&, dip::StringArray const& >( &dip::MultiScaleMorphologicalGradient ),
         "in"_a, "upperSize"_a = 9, "lowerSize"_a = 3, "filterShape"_a = dip::S::ELLIPTIC, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MorphologicalLaplace", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StringArray const& >( &dip::MorphologicalLaplace ),
         "in"_a, "se"_a = dip::StructuringElement{}, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "RankFilter", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::String const&, dip::StringArray const& >( &dip::RankFilter ),
         "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "order"_a = dip::S::INCREASING, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "RankMinClosing", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::RankMinClosing ),
         "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "RankMaxOpening", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::RankMaxOpening ),
         "in"_a, "se"_a = dip::StructuringElement{}, "rank"_a = 2, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "Watershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::Watershed ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{} );
   m.def( "SeededWatershed", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::StringSet const& >( &dip::SeededWatershed ),
          "in"_a, "seeds"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1.0, "maxSize"_a = 0, "flags"_a = dip::StringSet{} );
   m.def( "Maxima", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::Maxima ),
          "in"_a, "connectivity"_a = 1, "output"_a = dip::S::BINARY );
   m.def( "Minima", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::Minima ),
          "in"_a, "connectivity"_a = 1, "output"_a = dip::S::BINARY );
   m.def( "WatershedMinima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMinima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = dip::S::BINARY );
   m.def( "WatershedMaxima", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::dfloat, dip::uint, dip::String const& >( &dip::WatershedMaxima ),
          "in"_a, "mask"_a = dip::Image{}, "connectivity"_a = 1, "maxDepth"_a = 1, "maxSize"_a = 0, "output"_a = dip::S::BINARY );
   m.def( "MorphologicalReconstruction", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::MorphologicalReconstruction ),
          "marker"_a, "in"_a, "connectivity"_a = 1, "direction"_a = dip::S::DILATION );
   m.def( "HMinima", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint >( &dip::HMinima ),
          "in"_a, "h"_a, "connectivity"_a = 1 );
   m.def( "HMaxima", py::overload_cast< dip::Image const&, dip::dfloat, dip::uint >( &dip::HMaxima ),
          "in"_a, "h"_a, "connectivity"_a = 1 );
   m.def( "AreaOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::AreaOpening ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a = 50, "connectivity"_a = 1, "polarity"_a = dip::S::OPENING );
   m.def( "AreaClosing", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint >( &dip::AreaClosing ),
          "in"_a, "mask"_a = dip::Image{}, "filterSize"_a = 50, "connectivity"_a = 1 );
   m.def( "PathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::PathOpening ),
          "in"_a, "mask"_a = dip::Image{}, "length"_a = 7, "polarity"_a = dip::S::OPENING, "mode"_a = dip::S::NORMAL );
   m.def( "DirectedPathOpening", py::overload_cast< dip::Image const&, dip::Image const&, dip::IntegerArray const&, dip::String const&, dip::String const& >( &dip::DirectedPathOpening ),
          "in"_a, "mask"_a = dip::Image{}, "filterParam"_a = dip::IntegerArray{}, "polarity"_a = dip::S::OPENING, "mode"_a = dip::S::NORMAL );
   m.def( "OpeningByReconstruction", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::OpeningByReconstruction ),
          "in"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 1, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "ClosingByReconstruction", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::uint, dip::StringArray const& >( &dip::ClosingByReconstruction ),
          "in"_a, "se"_a = dip::StructuringElement{}, "connectivity"_a = 1, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "AlternatingSequentialFilter", py::overload_cast< dip::Image const&, dip::Range const&, dip::String const&, dip::String const&, dip::String const&, dip::StringArray const& >( &dip::AlternatingSequentialFilter ),
          "in"_a, "sizes"_a = dip::Range{ 3, 7, 2 }, "shape"_a = dip::S::ELLIPTIC, "mode"_a = dip::S::STRUCTURAL, "polarity"_a = dip::S::OPENCLOSE, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "HitAndMiss", py::overload_cast< dip::Image const&, dip::StructuringElement const&, dip::StructuringElement const&, dip::String const&, dip::StringArray const& >( &dip::HitAndMiss ),
          "in"_a, "hit"_a, "miss"_a, "mode"_a = dip::S::UNCONSTRAINED, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "HitAndMiss", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::StringArray const& >( &dip::HitAndMiss ),
          "in"_a, "se"_a, "mode"_a = dip::S::UNCONSTRAINED, "boundaryCondition"_a = dip::StringArray{} );

   // diplib/binary.h
   m.def( "BinaryDilation", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryDilation ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "BinaryErosion", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryErosion ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::OBJECT );
   m.def( "BinaryClosing", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryClosing ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::SPECIAL );
   m.def( "BinaryOpening", py::overload_cast< dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryOpening ),
         "in"_a, "connectivity"_a = -1, "iterations"_a = 3, "edgeCondition"_a = dip::S::SPECIAL );
   m.def( "BinaryPropagation", py::overload_cast< dip::Image const&, dip::Image const&, dip::sint, dip::uint, dip::String const& >( &dip::BinaryPropagation ),
         "inSeed"_a, "inMask"_a, "connectivity"_a = 1, "iterations"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "EdgeObjectsRemove", py::overload_cast< dip::Image const&, dip::uint >( &dip::EdgeObjectsRemove ),
         "in"_a, "connectivity"_a = 1 );
   m.def( "FillHoles", py::overload_cast< dip::Image const&, dip::uint >( &dip::FillHoles ),
         "in"_a, "connectivity"_a = 1 );

   m.def( "ConditionalThickening2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThickening2D ),
         "in"_a, "mask"_a = dip::Image{}, "iterations"_a = 0, "endPixelCondition"_a = dip::S::KEEP, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "ConditionalThinning2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::ConditionalThinning2D ),
         "in"_a, "mask"_a = dip::Image{}, "iterations"_a = 0, "endPixelCondition"_a = dip::S::KEEP, "edgeCondition"_a = dip::S::BACKGROUND );

   m.def( "BinaryAreaOpening", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaOpening ),
          "in"_a, "filterSize"_a = 50, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "BinaryAreaClosing", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::BinaryAreaClosing ),
          "in"_a, "filterSize"_a = 50, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );

   m.def( "EuclideanSkeleton", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::EuclideanSkeleton ),
          "in"_a, "endPixelCondition"_a = dip::S::NATURAL, "edgeCondition"_a = dip::S::BACKGROUND );

   m.def( "CountNeighbors", py::overload_cast< dip::Image const&, dip::uint, dip::String const&, dip::String const& >( &dip::CountNeighbors ),
         "in"_a, "connectivity"_a = 0, "mode"_a = dip::S::FOREGROUND, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "MajorityVote", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::MajorityVote ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "GetSinglePixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetSinglePixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "GetEndPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetEndPixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "GetLinkPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetLinkPixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );
   m.def( "GetBranchPixels", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::GetBranchPixels ),
         "in"_a, "connectivity"_a = 0, "edgeCondition"_a = dip::S::BACKGROUND );

   auto intv = py::class_< dip::Interval >( m, "Interval", "Represents an interval to use in inf- and sup-generating operators." );
   intv.def( py::init< dip::Image const& >(), "image"_a );
   intv.def( py::init< dip::Image const&, dip::Image const& >(), "hit"_a, "miss"_a );
   py::implicitly_convertible< dip::Image, dip::Interval >();
   intv.def( "__repr__", []( dip::Interval const& self ) {
      std::ostringstream os;
      os << "<" << self.Sizes() << " Interval>";
      return os.str();
   } );

   m.def( "SupGenerating", py::overload_cast< dip::Image const&, dip::Interval const&, dip::String const& >( &dip::SupGenerating ),
          "in"_a, "interval"_a, "boundaryCondition"_a = "" );
   m.def( "InfGenerating", py::overload_cast< dip::Image const&, dip::Interval const&, dip::String const& >( &dip::InfGenerating ),
          "in"_a, "interval"_a, "boundaryCondition"_a = "" );
   m.def( "UnionSupGenerating", py::overload_cast< dip::Image const&, dip::IntervalArray const&, dip::String const& >( &dip::UnionSupGenerating ),
          "in"_a, "intervals"_a, "boundaryCondition"_a = "" );
   m.def( "UnionSupGenerating2D", py::overload_cast< dip::Image const&, dip::Interval const&, dip::uint, dip::String const&, dip::String const& >( &dip::UnionSupGenerating2D ),
          "in"_a, "interval"_a, "rotationAngle"_a = 45, "rotationDirection"_a = "interleaved clockwise", "boundaryCondition"_a = "" );
   m.def( "IntersectionInfGenerating", py::overload_cast< dip::Image const&, dip::IntervalArray const&, dip::String const& >( &dip::IntersectionInfGenerating ),
          "in"_a, "intervals"_a, "boundaryCondition"_a = "" );
   m.def( "IntersectionInfGenerating2D", py::overload_cast< dip::Image const&, dip::Interval const&, dip::uint, dip::String const&, dip::String const& >( &dip::IntersectionInfGenerating2D ),
          "in"_a, "interval"_a, "rotationAngle"_a = 45, "rotationDirection"_a = "interleaved clockwise", "boundaryCondition"_a = "" );
   m.def( "Thickening", py::overload_cast< dip::Image const&, dip::Image const&, dip::IntervalArray const&, dip::uint, dip::String const& >( &dip::Thickening ),
          "in"_a, "mask"_a = dip::Image{}, "intervals"_a, "iterations"_a = 0, "boundaryCondition"_a = "" );
   m.def( "Thickening2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Interval const&, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Thickening2D ),
          "in"_a, "mask"_a = dip::Image{}, "interval"_a, "iterations"_a = 0, "rotationAngle"_a = 45, "rotationDirection"_a = "interleaved clockwise", "boundaryCondition"_a = "" );
   m.def( "Thinning", py::overload_cast< dip::Image const&, dip::Image const&, dip::IntervalArray const&, dip::uint, dip::String const& >( &dip::Thinning ),
          "in"_a, "mask"_a = dip::Image{}, "intervals"_a, "iterations"_a = 0, "boundaryCondition"_a = "" );
   m.def( "Thinning2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Interval const&, dip::uint, dip::uint, dip::String const&, dip::String const& >( &dip::Thinning2D ),
          "in"_a, "mask"_a = dip::Image{}, "interval"_a, "iterations"_a = 0, "rotationAngle"_a = 45, "rotationDirection"_a = "interleaved clockwise", "boundaryCondition"_a = "" );
   m.def( "HomotopicThinningInterval2D", &dip::HomotopicThinningInterval2D, "connectivity"_a = 2 );
   m.def( "HomotopicThickeningInterval2D", &dip::HomotopicThickeningInterval2D, "connectivity"_a = 2 );
   m.def( "EndPixelInterval2D", &dip::EndPixelInterval2D, "connectivity"_a = 2 );
   m.def( "HomotopicEndPixelInterval2D", &dip::HomotopicEndPixelInterval2D, "connectivity"_a = 2 );
   m.def( "HomotopicInverseEndPixelInterval2D", &dip::HomotopicInverseEndPixelInterval2D, "connectivity"_a = 2 );
   m.def( "SinglePixelInterval", &dip::SinglePixelInterval, "nDims"_a = 2 );
   m.def( "BranchPixelInterval2D", &dip::BranchPixelInterval2D );
   m.def( "BoundaryPixelInterval2D", &dip::BoundaryPixelInterval2D );
   m.def( "ConvexHullInterval2D", &dip::ConvexHullInterval2D );
}
