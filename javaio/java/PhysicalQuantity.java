/*
 * (c)2019, Wouter Caarls
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

package org.diplib;

/// \brief Encapsulates a quantity with physical units.
public class PhysicalQuantity {
   double magnitude; ///< The magnitude
   String units;     ///< The units
   
   public PhysicalQuantity( double _magnitude ) {
      magnitude = _magnitude;
      units = new String();
   }
   public PhysicalQuantity( double _magnitude, String _units ) {
      magnitude = _magnitude;
      // Replace the micro symbol with "u", in case DIPlib is built without Unicode support. "u" always works.
      units = _units.replace("μ", "u"); // U+03BC (mu)
      units = units.replace("µ", "u"); // legacy symbol U+00B5 (micro)
   }
}
