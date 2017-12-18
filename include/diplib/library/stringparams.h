/*
 * DIPlib 3.0
 * This file contains definitions for string constants.
 *
 * (c)2014-2017, Cris Luengo.
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

//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//

#pragma once

#include "diplib/library/types.h"

namespace dip {

namespace S {
// These are some of the string input arguments. Defining them as a constant
// prevents typos in the library code. A library user does not need to use these
// constants, but of course can do so if she wants.
// TODO: Go through the whole library and fish out all the string constants everywhere.

// Binary processing: background/object; special; all/foreground
constexpr char const* BACKGROUND = "background";
constexpr char const* OBJECT = "object";
constexpr char const* SPECIAL = "special";
constexpr char const* ALL = "all";
constexpr char const* FOREGROUND = "foreground";
// Skeleton end pixel conditions
constexpr char const* LOOSEENDSAWAY = "loose ends away";
constexpr char const* NATURAL = "natural";
constexpr char const* ONENEIGHBOR = "one neighbor";
constexpr char const* TWONEIGHBORS = "two neighbors";
constexpr char const* THREENEIGHBORS = "three neighbors";
constexpr char const* KEEP = "keep";
constexpr char const* LOSE = "lose";

// Morphological filtering parameters
constexpr char const* OPENING = "opening";
constexpr char const* CLOSING = "closing";
constexpr char const* DILATION = "dilation";
constexpr char const* EROSION = "erosion";
constexpr char const* TEXTURE = "texture";
//constexpr char const* OBJECT = "object";
constexpr char const* BOTH = "both";
constexpr char const* DYNAMIC = "dynamic";
constexpr char const* WHITE = "white";
constexpr char const* BLACK = "black";
constexpr char const* UNSIGNED = "unsigned";
constexpr char const* SIGNED = "signed";
constexpr char const* AVERAGE = "average";
constexpr char const* OPENCLOSE = "open-close";
constexpr char const* CLOSEOPEN = "close-open";
constexpr char const* INCREASING = "increasing";
constexpr char const* DECREASING = "decreasing";
constexpr char const* NORMAL = "normal";
constexpr char const* CONSTRAINED = "constrained";

// Watershed flags
constexpr char const* CORRECT = "correct";
constexpr char const* FAST = "fast";
constexpr char const* HIGHFIRST = "high first";
constexpr char const* LOWFIRST = "low first";
constexpr char const* LABELS = "labels";
constexpr char const* BINARY = "binary";
constexpr char const* NOGAPS = "no gaps";
constexpr char const* UPHILLONLY = "uphill only";

// Filter shapes
constexpr char const* ELLIPTIC = "elliptic";
constexpr char const* RECTANGULAR = "rectangular";
constexpr char const* DIAMOND = "diamond";
constexpr char const* OCTAGONAL = "octagonal";
constexpr char const* LINE = "line";
constexpr char const* FAST_LINE = "fast line";
constexpr char const* PERIODIC_LINE = "periodic line";
constexpr char const* DISCRETE_LINE = "discrete line";
constexpr char const* INTERPOLATED_LINE = "interpolated line";
constexpr char const* PARABOLIC = "parabolic";

// Interpolation methods
constexpr char const* CUBIC_ORDER_3 = "3-cubic";
constexpr char const* CUBIC_ORDER_4 = "4-cubic";
constexpr char const* LINEAR = "linear";
constexpr char const* NEAREST = "nearest";
constexpr char const* INVERSE_NEAREST = "inverse nearest";
constexpr char const* BSPLINE = "bspline";
constexpr char const* LANCZOS8 = "lanczos8";
constexpr char const* LANCZOS6 = "lanczos6";
constexpr char const* LANCZOS4 = "lanczos4";
constexpr char const* LANCZOS3 = "lanczos3";
constexpr char const* LANCZOS2 = "lanczos2";
constexpr char const* FOURIER = "fourier";

} // namespace S

} // namespace dip
