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

// Assorted
constexpr char const* MINIMUM = "minimum";
constexpr char const* MAXIMUM = "maximum";
constexpr char const* FIRST = "first";
constexpr char const* LAST = "last";
constexpr char const* STABLE = "stable";
constexpr char const* DIRECTIONAL = "directional";
constexpr char const* OTSU = "otsu";
constexpr char const* INCLUDE = "include";
constexpr char const* EXCLUDE = "exclude";
constexpr char const* INTERPOLATE = "interpolate";
constexpr char const* ROUND = "round";
constexpr char const* EMPTY = "empty";
constexpr char const* FILLED = "filled";
constexpr char const* OPEN = "open";
constexpr char const* CLOSED = "closed";

// Binary processing
constexpr char const* BACKGROUND = "background";
constexpr char const* OBJECT = "object";
constexpr char const* SPECIAL = "special";
constexpr char const* ALL = "all";
constexpr char const* FOREGROUND = "foreground";

// Skeleton end pixel conditions
constexpr char const* LOOSE_ENDS_AWAY = "loose ends away";
constexpr char const* NATURAL = "natural";
constexpr char const* ONE_NEIGHBOR = "one neighbor";
constexpr char const* TWO_NEIGHBORS = "two neighbors";
constexpr char const* THREE_NEIGHBORS = "three neighbors";
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
constexpr char const* ZERO_ORDER = "zero order";
constexpr char const* INVERSE_NEAREST = "inverse nearest";
constexpr char const* BSPLINE = "bspline";
constexpr char const* LANCZOS8 = "lanczos8";
constexpr char const* LANCZOS6 = "lanczos6";
constexpr char const* LANCZOS4 = "lanczos4";
constexpr char const* LANCZOS3 = "lanczos3";
constexpr char const* LANCZOS2 = "lanczos2";
constexpr char const* FOURIER = "fourier";

// Convolution flags
constexpr char const* SPATIAL = "spatial";
constexpr char const* FREQUENCY = "frequency";
constexpr char const* BEST = "best";
constexpr char const* EVEN = "even";
constexpr char const* ODD = "odd";
constexpr char const* NORMALIZE = "normalize";
constexpr char const* DONT_NORMALIZE = "don't normalize";
constexpr char const* SMOOTH = "smooth";
constexpr char const* FINITEDIFF = "finitediff";
constexpr char const* DISCRETE_TIME_FIT = "discrete time fit";
constexpr char const* FORWARD_BACKWARD = "forward backward";

// Fourier
constexpr char const* INVERSE = "inverse";
constexpr char const* REAL = "real";
constexpr char const* SYMMETRIC = "SYMMETRIC";
constexpr char const* CORNER = "corner";
//constexpr char const* FAST = "fast";

// Distance transforms
constexpr char const* GDT = "GDT";
constexpr char const* EUCLIDEAN = "Euclidean";
//constexpr char const* FAST = "fast";
constexpr char const* TIES = "ties";
constexpr char const* TRUE = "true";
constexpr char const* BRUTE_FORCE  = "brute force";

// Crop location
constexpr char const* CENTER = "center";
constexpr char const* MIRROR_CENTER = "mirror center";
constexpr char const* TOP_LEFT = "top left";
constexpr char const* BOTTOM_RIGHT = "bottom right";

// Clip options
//constexpr char const* BOTH = "both";
constexpr char const* LOW = "low";
constexpr char const* HIGH = "high";
constexpr char const* RANGE = "range";

// Metrics
constexpr char const* CHAMFER = "chamfer";
constexpr char const* CONNECTED = "connected";
constexpr char const* CITY = "city";
constexpr char const* CHESS = "chess";
constexpr char const* CONNECTED_4 = "4-connected";
constexpr char const* CONNECTED_8 = "8-connected";
constexpr char const* CONNECTED_6 = "6-connected";
constexpr char const* CONNECTED_18 = "18-connected";
constexpr char const* CONNECTED_28 = "28-connected";

// Coordinate systems
constexpr char const* SPHERICAL = "spherical";
constexpr char const* CARTESIAN = "cartesian";
constexpr char const* RIGHT = "right";
constexpr char const* LEFT = "left";
//constexpr char const* TRUE = "true";
//constexpr char const* CORNER = "corner";
//constexpr char const* FREQUENCY = "frequency";
constexpr char const* RADFREQ = "radfreq";
constexpr char const* RADIAL = "radial";
constexpr char const* MATH = "math";
constexpr char const* PHYSICAL = "physical";

// Peak finding methods
//constexpr char const* LINEAR = "linear";
//constexpr char const* PARABOLIC = "parabolic";
constexpr char const* PARABOLIC_SEPARABLE = "parabolic separable";
constexpr char const* GAUSSIAN = "gaussian";
constexpr char const* GAUSSIAN_SEPARABLE = "gaussian separable";
constexpr char const* INTEGER = "integer";


} // namespace S

} // namespace dip
