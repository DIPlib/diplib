/*
 * DIPlib 3.0
 * This file declares internal functions for 1D morphological operators and their compositions.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#ifndef DIP_ONE_DIMENSIONAL_H
#define DIP_ONE_DIMENSIONAL_H

#include "diplib.h"
#include "diplib/morphology.h"
#include "diplib/boundary.h"

namespace dip {

namespace detail {

enum class DIP_NO_EXPORT Polarity {
      DILATION,
      EROSION
};
enum class DIP_NO_EXPORT Mirror {
      NO,
      YES
};
inline Mirror GetMirrorParam( bool mirror ) {
   return mirror ? Mirror::YES : Mirror::NO;
}
inline Mirror InvertMirrorParam( Mirror mirror ) {
   return mirror == Mirror::YES ? Mirror::NO : Mirror::YES;
}

inline BoundaryConditionArray BoundaryConditionForDilation( BoundaryConditionArray const& bc ) {
   return bc.empty() ? BoundaryConditionArray{ BoundaryCondition::ADD_MIN_VALUE } : bc;
}
inline BoundaryConditionArray BoundaryConditionForErosion( BoundaryConditionArray const& bc ) {
   return bc.empty() ? BoundaryConditionArray{ BoundaryCondition::ADD_MAX_VALUE } : bc;
}

DIP_NO_EXPORT void RectangularMorphology(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      Mirror mirror,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
);

DIP_NO_EXPORT void SkewLineMorphology(
      Image const& in,
      Image& out,
      FloatArray const& filterParam,
      Mirror mirror,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
);

DIP_NO_EXPORT void FastLineMorphology(
      Image const& c_in,
      Image& c_out,
      FloatArray const& filterParam,
      StructuringElement::ShapeCode mode, // PERIODIC_LINE, FAST_LINE
      Mirror mirror,
      BoundaryConditionArray const& bc,
      BasicMorphologyOperation operation
);

DIP_NO_EXPORT std::pair< dip::uint, dip::uint > PeriodicLineParameters( FloatArray const& filterParam );

} // namespace detail

} // namespace dip

#endif // DIP_ONE_DIMENSIONAL_H
