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

} // namespace S

} // namespace dip
