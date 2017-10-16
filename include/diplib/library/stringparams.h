/*
 * DIPlib 3.0
 * This file contains definitions for the basic data types.
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

static dip::String::value_type const* BACKGROUND = "background";
static dip::String::value_type const* OBJECT = "object";

} // namespace S
} // namespace dip
